/***************************************************************************
 *   Copyright (C) 2011 by Flux Inc.                                       *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#include "Association.hpp"
#include "Association.moc.inl"
#include "ConnectionParameters.hpp"

#include <QtCore/QString>

#include <dcmtk/dcmnet/assoc.h>
#include <dcmtk/dcmnet/dimse.h>


namespace Dicom {

Association::Association( QObject * parent ) :
	QObject( parent ),
	association_( 0 ),
	network_( 0 ),
	state_( Disconnected )
{
}

Association::Association( 
	const ConnectionParameters & Parameters, QObject * parent
) :
	QObject( parent ),
	association_( 0 ),
	connectionParameters_( Parameters ),
	network_( 0 ),
	state_( Disconnected )
{
}


Association::~Association() {
	if ( isEstablished() ) {
		qWarning( "Destroying an established association forces abort." );

		abort();
	}
	dropTAscAssociation();
	dropTAscNetwork();
}


void Association::abort() {
	if ( tAscAssociation() ) {
		const OFCondition Result = ASC_abortAssociation( tAscAssociation() );
		if ( Result.bad() ) {
			qWarning( "Failed to abort association; %s", Result.text() );
		}
		setState( Disconnected );
		dropTAscAssociation();
	}
}


int Association::acceptedPresentationContextId(
	const char * AbstractSyntax, const char * TransferSyntax
) const {
	if ( tAscAssociation() ) {
		if ( ! TransferSyntax ) {
			return ASC_findAcceptedPresentationContextID( 
				tAscAssociation(), AbstractSyntax
			);
		}
		else {
			return ASC_findAcceptedPresentationContextID( 
				tAscAssociation(), AbstractSyntax, TransferSyntax
			);
		}
	}
	else {
		return -1;
	}
}


const ConnectionParameters & Association::connectionParameters() const {
	return connectionParameters_;
}


void Association::dropTAscAssociation() {
	Q_ASSERT( ! isEstablished() );
	if ( isEstablished() ) {
		qWarning( "Dropping established association." );
	}

	if ( tAscAssociation() ) {
		const OFCondition Result = ASC_destroyAssociation( &tAscAssociation() );
		if ( Result.bad() ) {
			qWarning(
				"Error occured when destroying an association; %s",
				Result.text()
			);
		}

		association_ = 0;
	}
}


void Association::dropTAscNetwork() {
	dropTAscAssociation();

	if ( tAscNetwork() ) {
		const OFCondition Result = ASC_dropNetwork( & network_ );
		if ( Result.bad() ) {
			qWarning(
				"Error occured when dropping network; %s", Result.text()
			);
		}

		network_ = 0;
	}
}


const QString & Association::errorMessage() const {
	return errorMessage_;
}


bool Association::hasError() const {
	return state() == Error;
}


bool Association::initializeTAscNetwork( int role ) {
	if ( port() == 0 ) {
		raiseError( "Invliad port number." );
		return false;
	}

	if ( tAscNetwork() ) {
		qWarning( "Initializing allocated network." );

		dropTAscNetwork();
	}


	const OFCondition Result = ASC_initializeNetwork(
		role == 1 ? NET_ACCEPTOR : NET_REQUESTOR,
		role == 1 ? static_cast< int >( port() ) : 0,
		// port(),
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


bool Association::isEstablished() const {
	return state() == Established;
}


unsigned Association::maxPdu() const {
	return connectionParameters().maxPdu();
}


const QString & Association::myAeTitle() const {
	return connectionParameters().myAeTitle();
}


quint16 Association::nextMessageId() {
	if ( tAscAssociation() ) {
		return tAscAssociation()->nextMsgID++;
	}
	else {
		Q_ASSERT( tAscAssociation() );

		return 0;
	}
}


quint16 Association::port() const {
	return connectionParameters().port();
}


void Association::raiseError( const QString & Message ) {
	setState( Error );
	errorMessage_ = Message;
}


void Association::setConnectionParameters(
	const ConnectionParameters & Parameters
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


void Association::setState( State s ) {
	state_ = s;
}


const Association::State & Association::state() const {
	return state_;
}


T_ASC_Association *& Association::tAscAssociation() const {
	return association_;
}


T_ASC_Network *& Association::tAscNetwork() const {
	return network_;
}


int Association::timeout() const {
	return connectionParameters().timeout();
}


}; // Namespace DICOM ends here.
