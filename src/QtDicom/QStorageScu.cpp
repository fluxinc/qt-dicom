/***************************************************************************
 *   Copyright © 2011-2012 by Flux Inc.                                    *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#include "ConnectionParameters.hpp"
#include "RequestorAssociation.hpp"
#include "QStorageScu.hpp"
#include "UidList.hpp"

#include <QtCore/QDir>
#include <QtCore/QMetaType>

#include <QtDicom/QDicomImageCodec>
#include <QtDicom/QUid>

#include <dcmtk/dcmdata/dcuid.h>


using Dicom::ConnectionParameters;
using Dicom::RequestorAssociation;


QString sopClassString( const char * UID );

QStorageScu::QStorageScu( QObject * parent ) :
	association_( new RequestorAssociation( this ) ),
	error_( NoError ),
	state_( Disconnected )
{
	static const int ErrorTypeId = 
		qRegisterMetaType< QStorageScu::Error >( "QStorageScu::Error" )
	;
	static const int DatasetTypeId = 
		qRegisterMetaType< Dicom::Dataset >( "Dicom::Dataset" )
	;
	Q_ASSERT( ErrorTypeId > 0 );
}


QStorageScu::~QStorageScu() {
	releaseAssociation();
}


QList< QTransferSyntax > QStorageScu::acceptedTransferSyntaxes(
	const QUid & Uid
) const {
	QList< QTransferSyntax > result;

	QList< QPresentationContext > All = 
		association().acceptedPresentationContexts()
	;

	for (
		QList< QPresentationContext >::const_iterator i = All.constBegin();
		i != All.constEnd(); ++i
	) {
		if ( i->abstractSyntax() == Uid ) {
			result.append( i->acceptedTransferSyntax() );
		}
	}

	return result;
}


bool QStorageScu::areAllSopClassesAccepted(
	const QList< QPresentationContext > & ProposedContexts
) const {
	const QList< QPresentationContext > AcceptedContexts = 
			association().acceptedPresentationContexts()
	;
	Q_ASSERT(
		AcceptedContexts.size() > 0 && 
		AcceptedContexts.size() <= ProposedContexts.size()
	);

	QList< QUid > acceptedSopClasses;
	for (
		QList< QPresentationContext >::const_iterator i = AcceptedContexts.constBegin();
		i != AcceptedContexts.constEnd(); ++i
	) {
		acceptedSopClasses.append( i->abstractSyntax() );
	}

	// Make sure each SOP class is supported
	for (
		QList< QUid >::const_iterator i = sopClasses_.constBegin();
		i != sopClasses_.constEnd(); ++i 
	) {
		if ( acceptedSopClasses.contains( *i ) ) {
			continue;
		}
		else {
			return false;
		}
	}

	return true;
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


void QStorageScu::connectToAe() {
	error_ = NoError;

	if ( state() != Disconnected ) {
		qDebug( 
			__FUNCTION__": called when the previous connection is still open; "
			"disconnecting"
		);
		releaseAssociation();
	}

	if ( association().connectionParameters().isValid() ) {
		if ( sopClasses_.size() > 0 ) {
			QMetaObject::invokeMethod( this, "requestAssociation", Qt::QueuedConnection );
		}
		else {
			setError( InvalidSopClass );
		}
	}
	else {
		setError( InvalidConnectionParameters );
	}
}


void QStorageScu::connectToAe(
	const QUid & SopClass,
	const QTransferSyntax & TransferSyntax
) {
	sopClasses_.clear();
	sopClasses_.append( SopClass );

	transferSyntax_ = TransferSyntax;

	connectToAe();		
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


QString QStorageScu::errorString() const {
	QString msg = "Unknown error";

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
		msg = associationErrorString();
		break;

	case DimseError :
		msg = dimseErrorString();
		break;

	case InvalidSopClass :
		msg = "Invalid SOP Class";
		break;

	case InvalidTransferSyntax :
		msg = "Invalid Transfer Syntax";
		break;

	}

	return msg;
}


QList< QPresentationContext > QStorageScu::preparePresentationContexts() const {	
	QList< QPresentationContext > contexts;

	for (
		QList< QUid >::const_iterator i = sopClasses_.constBegin();
		i != sopClasses_.constEnd(); ++i
	) {
		if ( transferSyntax_.isValid() ) {
			contexts.append( QPresentationContext( *i ) << transferSyntax_ );

			const bool CanDecodeOnTheFly = 
				transferSyntax_.isCompressed() && 
				QDicomImageCodec::supportedTransferSyntaxes().contains( transferSyntax_ )
			;		

			if ( CanDecodeOnTheFly ) {
				contexts.append( QPresentationContext::defaultFor( *i ) );
			}
		}
		else {
			contexts.append( QPresentationContext::defaultFor( *i ) );
		}
	}

	return contexts;
}


void QStorageScu::releaseAssociation() {
	if ( association().isEstablished() ) {
		association().release();
	}

	setState( Disconnected );
}


void QStorageScu::requestAssociation() {
	setState( Requesting );

	const QList< QPresentationContext > ProposedContexts = 
		preparePresentationContexts()
	;

	bool timedOut = false, allSopClassesAccepted = false;
	const int Count = association().request( ProposedContexts, &timedOut );

	if ( Count > 0 ) {
		allSopClassesAccepted =
			areAllSopClassesAccepted( ProposedContexts )
		;

		if ( allSopClassesAccepted ) {
			setState( Connected );

			return;
		}
	}


	if ( timedOut ) {
		setError( Timeout );
	}	
	else if ( Count == -1 ) {
		setError( AssociationError );
	}
	else if ( Count == 0 || ! allSopClassesAccepted ) {
		setError( SopClassNotSupported );
		association().release();
	}

	setState( Disconnected );
}


void QStorageScu::setConnectionParameters(
	const Dicom::ConnectionParameters & Parameters
) {
	association().setConnectionParameters( Parameters );
}


void QStorageScu::setError( Error e ) {
	if ( error_ == NoError && e != NoError ) {
		error_ = e;
		emit error( e );
		emit error( errorString() );
	}
}


void QStorageScu::setSopClasses( const QList< QUid > & SopClasses ) {
	sopClasses_ = SopClasses;
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


void QStorageScu::setTransferSyntax( const QTransferSyntax & Ts ) {
	transferSyntax_ = Ts;
}


QStorageScu::State QStorageScu::state() const {
	return state_;
}


void QStorageScu::store( Dicom::Dataset dataset ) {
	const QUid SopClass = dataset.sopClassUid();

	if ( sopClasses_.contains( SopClass ) ) {
		QMetaObject::invokeMethod( 
			this, "storeDataset", Qt::QueuedConnection, Q_ARG( Dicom::Dataset, dataset )
		);
	}
	else {
		qWarning( __FUNCTION__": "
			"Data Set's: %s SOP class: %s doesn't match requested",
			dataset.sopInstanceUid().constData(),
			qPrintable( sopClassString( SopClass ) )
		);
		setError( InvalidSopClass );

		releaseAssociation();
	}
}


void QStorageScu::storeDataset( Dicom::Dataset dataset ) {
	if ( state_ != Disconnected ) {
		const QUid SopClass = dataset.sopClassUid();

		QList< QTransferSyntax > AcceptedTs = acceptedTransferSyntaxes(
			SopClass
		);
		Q_ASSERT( AcceptedTs.size() > 0 );

		// We propose maximum two presentation contexts per SOP class. If only
		// one of them was accepted, check if that's the preferred one
		if (
			AcceptedTs.size() == 1 && 
			( AcceptedTs.at( 0 ) != transferSyntax_ )
		) {
			qWarning( __FUNCTION__": "
				"SCP doesn't support preferred Transfer Syntax: %s "
				"for SOP class: %s; the default: %s will be used instead",
				transferSyntax_.name(),
				SopClass.constData(),
				AcceptedTs.at( 0 ).name()
			);
		}

		if ( ! AcceptedTs.contains( dataset.syntax() ) ) {
			bool converted = false;
			for (
				QList< QTransferSyntax >::const_iterator i = AcceptedTs.constBegin();
				i != AcceptedTs.constEnd(); ++i
			) {
				if ( dataset.canConvertToTransferSyntax( *i ) ) {
					const Dicom::Dataset Tmp = dataset.convertedToTransferSyntax( *i );

					if ( ! Tmp.isEmpty() ) {
						qDebug( __FUNCTION__": "
							"converted Data Set from %s to negotiated %s transfer syntax",
							dataset.syntax().name(), i->name()
						);

						dataset = Tmp;
						converted = true;
						break;
					}
					else {
						qWarning( __FUNCTION__": "
							"failed to convert Data Set from %s to %s transfer syntax",
							dataset.syntax().name(), i->name()
						);
					}
				}				
			}

			if ( ! converted ) {
				qWarning( __FUNCTION__": "
					"Data Set's: %s Transfer Syntax: %s "
					"cannot be converted to those accpeted by SCP",
					dataset.sopInstanceUid().constData(),
					dataset.syntax().name()
				);
				setError( InvalidTransferSyntax );

				releaseAssociation();
				return;
			}
		}

		setState( Sending );

		dimseClient_.setAssociation( &association() );
		const bool Stored = dimseClient_.cStore( dataset );

		if ( Stored ) {
			emit stored( dataset.sopInstanceUid() );
			setState( Connected );
		}
		else {
			setError( DimseError );

			releaseAssociation();
		}
	}
	else {
		qWarning( __FUNCTION__": "
			"Disconnected; ignoring Data Set: %s",
			qPrintable( dataset.sopInstanceUid() )
		);
	}
}


QString sopClassString( const char * Uid ) {
	return QString( dcmFindNameOfUID( Uid, Uid ) );
}
