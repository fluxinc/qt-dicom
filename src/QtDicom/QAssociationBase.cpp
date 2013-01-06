/***************************************************************************
 *   Copyright © 2012-2013 by Flux Inc.                                    *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#include "ConnectionParameters.hpp"

#include "QAssociationBase.hpp"
#include "QAssociationBase.moc.inl"
#include "QDcmtkTask.hpp"

#include <QtCore/QString>
#include <QtCore/QtConcurrentRun>

#include <dcmtk/dcmnet/assoc.h>
#include <dcmtk/dcmnet/dimse.h>

typedef Dicom::ConnectionParameters QConnectionParameters;


QAssociationBase::QAssociationBase( QObject * parent ) :
	QObject( parent ),
	association_( 0 ),
	network_( 0 ),
	state_( Unconnected )
{
}

QAssociationBase::QAssociationBase( 
	const QConnectionParameters & Parameters, QObject * parent
) :
	QObject( parent ),
	association_( 0 ),
	connectionParameters_( Parameters ),
	network_( 0 ),
	state_( Unconnected )
{
}


QAssociationBase::~QAssociationBase() {
	if ( isEstablished() ) {
		qWarning( "Destroying an established association forces abort." );

		abort();
	}
	dropTAscAssociation();
	dropTAscNetwork();
}


void QAssociationBase::abort() {
	if ( isEstablished() ) {
		Q_ASSERT( tAscAssociation() );

		setState( Aborting );
		QMetaObject::invokeMethod( this, "startAbortTask", Qt::QueuedConnection );
	}
}


const QConnectionParameters & QAssociationBase::connectionParameters() const {
	return connectionParameters_;
}


void QAssociationBase::dropTAscAssociation() {
	Q_ASSERT( ! isEstablished() );

	if ( ! isEstablished() && tAscAssociation() ) {
		const OFCondition Result = ASC_destroyAssociation( &tAscAssociation() );
		if ( Result.bad() ) {
			qWarning( __FUNCTION__
				"Error occured when destroying an association; %s",
				Result.text()
			);
		}

		association_ = 0;
	}
}


void QAssociationBase::dropTAscNetwork() {
	dropTAscAssociation();

	if ( tAscNetwork() ) {
		const OFCondition Result = ASC_dropNetwork( & network_ );
		if ( Result.bad() ) {
			qWarning( __FUNCTION__": "
				"error occured when dropping network; %s", Result.text()
			);
		}

		network_ = 0;
	}
}


const QString & QAssociationBase::errorMessage() const {
	return errorMessage_;
}


void QAssociationBase::finishAbortTask( QDcmtkResult result ) {
	Q_ASSERT( state_ == Aborting );

	if ( result.ofCondition().good() ) {
		qDebug( __FUNCTION__": association aborted successfully" );
	}
	else {
		qWarning( __FUNCTION__": "
			"failed to abort association; %s", result.ofCondition().text()
		);
	}

	dropTAscAssociation();

	setState( Unconnected );
	emit disconnected();
}


bool QAssociationBase::hasError() const {
	return state() == Error;
}


bool QAssociationBase::initializeTAscNetwork( int role ) {
	if ( port() == 0 ) {
		raiseError( "Invliad port number." );
		return false;
	}

	if ( tAscNetwork() ) {
		qWarning( "Initializing allocated network." );

		dropTAscNetwork();
	}


	const OFCondition Result = ASC_initializeNetwork(
		role == 1 ? NET_ACCEPTOR : ( role == -1 ? NET_REQUESTOR : NET_ACCEPTORREQUESTOR ),
		role == 1 ? static_cast< int >( port() ) : 0,
		timeout(),
		&tAscNetwork()
	);

	if ( Result.good() ) {
		return true;
	}
	else {
		raiseError(
			QString( 
				"Failed to initialize network. "
				"Internal error description:\n%1"
			)
			.arg( Result.text() )
		);
		return false;
	}
}


bool QAssociationBase::isEstablished() const {
	return state() == Established;
}


unsigned QAssociationBase::maxPdu() const {
	return connectionParameters().maxPdu();
}


const QString & QAssociationBase::myAeTitle() const {
	return connectionParameters().myAeTitle();
}


quint16 QAssociationBase::nextMessageId() {
	if ( tAscAssociation() ) {
		return tAscAssociation()->nextMsgID++;
	}
	else {
		Q_ASSERT( tAscAssociation() );

		return 0;
	}
}


quint16 QAssociationBase::port() const {
	return connectionParameters().port();
}


void QAssociationBase::raiseError( const QString & Message ) {
	setState( Error );
	errorMessage_ = Message;
}


void QAssociationBase::setConnectionParameters(
	const QConnectionParameters & Parameters
) {
	if (
		connectionParameters().port()    != Parameters.port() ||
		connectionParameters().timeout() != Parameters.timeout()
	)
	{
		// Network parameters changed, drop current network.
		dropTAscNetwork();
	}

	connectionParameters_ = Parameters;
}


void QAssociationBase::setState( State s ) {
	state_ = s;
}


void QAssociationBase::startAbortTask() {
	QDcmtkTask * task = QDcmtkTask::create( 
		::ASC_abortAssociation, tAscAssociation()
	);

	connect( 
		task, SIGNAL( finished( QDcmtkResult ) ),
		SLOT( finishAbortTask( QDcmtkResult ) )
	);
	connect(
		task, SIGNAL( finished( QDcmtkResult ) ),
		task, SLOT( deleteLater() )
	);

	task->start();
}


const QAssociationBase::State & QAssociationBase::state() const {
	return state_;
}


T_ASC_Association *& QAssociationBase::tAscAssociation() const {
	return association_;
}


T_ASC_Network *& QAssociationBase::tAscNetwork() const {
	return network_;
}


int QAssociationBase::timeout() const {
	return connectionParameters().timeout();
}

