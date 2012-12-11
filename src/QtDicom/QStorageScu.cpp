/***************************************************************************
 *   Copyright (C) 2011 by Flux Inc.                                       *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#include "ConnectionParameters.hpp"
#include "RequestorAssociation.hpp"
#include "QStorageScu.hpp"
#include "QStorageScu.moc.inl"
#include "UidList.hpp"

#include <QtCore/QDir>

#include <QtDicom/QDicomImageCodec>
#include <QtDicom/QUid>

#include <dcmtk/dcmdata/dcuid.h>


using Dicom::ConnectionParameters;
using Dicom::Dataset;
using Dicom::RequestorAssociation;


QString sopClassString( const char * UID );

QStorageScu::QStorageScu( QObject * parent ) :
	association_( new RequestorAssociation( this ) ),
	error_( NoError ),
	state_( Disconnected )
{
}


QStorageScu::~QStorageScu() {
	qt_noop();
}


RequestorAssociation & QStorageScu::association() {
	Q_ASSERT( association_ );

	return *association_;
}


const RequestorAssociation & QStorageScu::association() const {
	Q_ASSERT( association_ );

	return *association_;
}


QString QStorageScu::associationErrorString() const {
	return association().errorMessage();
}


void QStorageScu::connectToAe(
	const QUid & SopClass,
	const QTransferSyntax & TransferSyntax
) {
	error_ = NoError;

	if ( state() != Disconnected ) {
		qDebug( 
			__FUNCTION__": called when the previous connection is still open; "
			"disconnecting"
		);
		QMetaObject::invokeMethod( this, "releaseAssociation", Qt::QueuedConnection );
	}

	if ( association().connectionParameters().isValid() ) {		
		sopClass_ = SopClass;
		transferSyntax_ = TransferSyntax;
		QMetaObject::invokeMethod( this, "requestAssociation", Qt::QueuedConnection );
	}
	else {
		setError( InvalidConnectionParameters );
	}
}


void QStorageScu::connectToAe(
	const ConnectionParameters & params,
	const QUid & AbstractSyntax,
	const QTransferSyntax & TransferSyntax
) {
	association().setConnectionParameters( params );
	connectToAe( AbstractSyntax, TransferSyntax );
}


QString QStorageScu::dimseErrorString() const {
	return dimseClient_.errorMessage();
}


void QStorageScu::disconnectFromAe() {
	QMetaObject::invokeMethod( this, "releaseAssociation", Qt::QueuedConnection );
}


QStorageScu::Error QStorageScu::error() const {
	return error_;
}


const char * QStorageScu::errorString() const {
	const char * msg = "Unknown error";

	switch ( error_ ) {

	case NoError :
		msg = "No error";
		break;

	case InvalidConnectionParameters :
		msg = "Invalid connection parameters";
		break;

	case Timeout :
		msg = "Operation timed out";
		break;

	case SopClassNotSupported :
		msg = "SOP class not supported";
		break;

	case AssociationError :
		msg = "Association error";
		break;

	case DimseError :
		msg = "Dimse error";
		break;

	}

	return msg;
}


void QStorageScu::releaseAssociation() {
	if ( association().isEstablished() ) {
		association().release();
	}

	setState( Disconnected );
}


void QStorageScu::requestAssociation() {
	setState( Requesting );

	QList< QPresentationContext > contexts;
	contexts.append( QPresentationContext( sopClass_ ) << transferSyntax_ );

	const bool CanDecodeOnTheFly = 
		transferSyntax_.isCompressed() && 
		QDicomImageCodec::supported().contains( transferSyntax_ )
	;

	if ( CanDecodeOnTheFly ) {
		contexts.append( QPresentationContext::defaultFor( sopClass_ ) );
	}

	bool timedOut = false;
	const int Count = association().request( contexts, &timedOut );

	if ( Count > 0 ) {
		if ( CanDecodeOnTheFly ) {
			const QList< QPresentationContext > & Contexts = 
				association().acceptedPresentationContexts()
			;
			Q_ASSERT( Contexts.size() > 0 && Contexts.size() < 3 );

			if ( Contexts.size() == 1 ) {
				const QTransferSyntax & AcceptedTs = 
					Contexts.at( 0 ).acceptedTransferSyntax()
				;

				if ( Contexts.at( 0 ).acceptedTransferSyntax() != transferSyntax_ ) {
					qWarning( __FUNCTION__": "
						"SCP doesn't support requested Transfer Syntax: %s, "
						"enabling on-the-fly conversion to: %s",
						qPrintable( transferSyntax_.uid() ),
						qPrintable( AcceptedTs.uid() )
					);
				}
				transferSyntax_ = AcceptedTs;
			}
		}
		setState( Connected );

		return;
	}

	if ( timedOut ) {
		setError( Timeout );
	}
	else if ( Count == 0 ) {
		setError( SopClassNotSupported );
		association().release();
	}
	else if ( Count == -1 ) {
		setError( AssociationError );
	}

	setState( Disconnected );
}


void QStorageScu::setError( Error e ) {
	if ( error_ == NoError && e != NoError ) {
		error_ = e;
		emit error( e );

		QString msg;
		if ( e == AssociationError ) {
			msg = associationErrorString();
		}
		else if ( e == DimseError ) {
			msg = dimseErrorString();
		}
		else {
			msg = errorString();
		}

		emit error( msg );
	}
}


void QStorageScu::setState( State NewState ) {	
	if ( state_ != NewState ) {
		const State OldState = state_;
		state_ = NewState;

		switch ( NewState ) {

		case Connected : if ( OldState == Requesting ) emit connected(); break;
		case Disconnected : emit disconnected(); break;

		}
	}
}


QStorageScu::State QStorageScu::state() const {
	return state_;
}


void QStorageScu::store( const Dataset & Data ) {
	if ( Data.sopClassUid() == sopClass_ ) {
		QMetaObject::invokeMethod( 
			this, "storeDataset", Qt::QueuedConnection, Q_ARG( Dataset, Data )
		);
	}
	else {
		qWarning( __FUNCTION__": "
			"skipping DataSet: `%s' becuase its "
			"SOP class: `%s' "
			"doesn't match requested: `%s'",
			qPrintable( Data.sopInstanceUid() ),
			qPrintable( sopClassString( Data.sopClassUid() ) ),
			qPrintable( sopClassString( sopClass_ ) )
		);
	}
}


void QStorageScu::storeDataset( Dataset Dataset ) {
	if ( state_ != Disconnected ) {
		if ( Dataset.sopClassUid() != sopClass_ ) {
			qCritical( __FUNCTION__": "
				"SOP class of DataSet to store: %s differs from negotiated: %s",
				Dataset.sopClassUid().constData(),
				sopClass_.constData()
			);
			return;
		}

		if ( Dataset.syntax() != transferSyntax_ ) {
			qWarning( __FUNCTION__": "
				"converting DataSet from %s to negotiated %s transfer syntax",
				qPrintable( Dataset.syntax().toString() ),
				qPrintable( transferSyntax_.toString() )
			);
			Dataset = Dataset.convertedToTransferSyntax( transferSyntax_ );
			if ( Dataset.isEmpty() ) {
				return;
			}
		}

		setState( Sending );

		dimseClient_.setAssociation( &association() );
		const bool Stored = dimseClient_.cStore( Dataset );

		if ( Stored ) {
			emit stored( Dataset.sopInstanceUid() );
			setState( Connected );
		}
		else {
			association().abort();
			setState( Disconnected );
			setError( DimseError );
		}
	}
	else {
		qWarning( __FUNCTION__": "
			"Disconnected; ignoring DataSet: `%s'",
			qPrintable( Dataset.sopInstanceUid() )
		);
	}
}


QString sopClassString( const char * Uid ) {
	return QString( dcmFindNameOfUID( Uid, Uid ) );
}
