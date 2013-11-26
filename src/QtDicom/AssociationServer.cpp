/***************************************************************************
 *   Copyright (C) 2011 by Flux Inc.                                       *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#include "AssociationServer.hpp"
#include "ServerAssociation.hpp"

#include <dcmtk/dcmnet/assoc.h>


namespace Dicom {

AssociationServer::AssociationServer( QObject * parent ) :
	QThread( parent ),
	closing_( false ),
	connectionParameters_( ConnectionParameters::Server ),
	tAscNetwork_( 0 )
{
}


AssociationServer::~AssociationServer() {
	close();
}


const UidList & AssociationServer::abstractSyntaxes() const {
	return abstractSyntaxes_;
}


void AssociationServer::close() {
	if ( isListening() ) {
		setClosingFlag();

		// Give thread polling interval + a hundred miliseconds to quit run()
		if ( ! wait( pollingInterval() + 100 ) ) {
			qWarning( "Terminating thread." );
			terminate();
		}
	}
	if ( tAscNetwork() ) {
		const OFCondition Result = ASC_dropNetwork( & tAscNetwork() );
		if ( Result.bad() ) {
			qWarning(
				"Error occured when dropping network. "
				"Internal error description:\n%s",
				Result.text()
			);
		}

		tAscNetwork_ = 0;
	}
}


const ConnectionParameters & AssociationServer::connectionParameters() const {
	return connectionParameters_;
}


QMutex & AssociationServer::dataLock() const {
	return dataLock_;
}


void AssociationServer::enqueuePendingAssociation( ServerAssociation * association ) {
	dataLock().lock();
	pendingAssociations_.enqueue( association );
	dataLock().unlock();

	emit newAssociation();
}


QString AssociationServer::errorString() const {
	return errorString_;
}


bool AssociationServer::hasPendingConnections() const {
	dataLock().lock();
	const bool Result = ! pendingAssociations_.isEmpty();
	dataLock().unlock();

	return Result;
}


bool AssociationServer::isClosing() const {
	return closing_;
}


bool AssociationServer::isListening() const {
	return isRunning();
}


bool AssociationServer::listen( const ConnectionParameters & Parameters ) {
	if ( isListening() ) {
		raiseError( "Server is listening already." );
		return false;
	}

	if ( ! Parameters.isValid() ) {
		raiseError( "Invalid connection parameters." );
		return false;
	}

	setConnectionParameters( Parameters );

	Q_ASSERT( ! tAscNetwork() );

	const OFCondition Result = ASC_initializeNetwork(
		NET_ACCEPTOR,
		static_cast< int >( Parameters.port() ),
		Parameters.timeout(),
		&tAscNetwork()
	);

	if ( Result.good() ) {
		start();
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


ServerAssociation * AssociationServer::nextPendingAssociation() {
	dataLock().lock();
	ServerAssociation * tmp =
		pendingAssociations_.size() > 0 ?
		pendingAssociations_.dequeue() :
		0
	;
	dataLock().unlock();

	return tmp;
}


int AssociationServer::pollingInterval() {
	return 10;
}


void AssociationServer::raiseError( const QString & Message ) {
	errorString_ = Message;
}


void AssociationServer::run() {
	ServerAssociation * association = 0;
	bool timedOut = false;

	while ( ! ( isClosing() ) ) {
		if ( ! association ) {
			association = new ServerAssociation(
				tAscNetwork(), connectionParameters()
			);
		}

		if ( ! association->isPending() ) {
			QThread::msleep( pollingInterval() );
			continue;
		}

		bool result = association->receive( &timedOut );
		if ( result && ! isClosing() ) {
			result = association->accept( 
				abstractSyntaxes(), transferSyntaxes()
			);
			if ( result ) {
				enqueuePendingAssociation( association );
				association = 0;
			}
			else {
				emit newAssociationError( association->errorMessage() );
				association->abort();
			}
		}
		else if ( timedOut ) {
			emit newAssociationError(
				"Timeout occured when receiving an association."
			);
			association->abort();
		}
		else {
			if ( ! isClosing() ) {
				emit newAssociationError( association->errorMessage() );
			}
			association->abort();
		}
	}
	qt_noop();
}



void AssociationServer::setAbstractSyntaxes( const UidList & List ) {
	dataLock().lock();
	abstractSyntaxes_ = List;
	dataLock().unlock();
}


void AssociationServer::setClosingFlag() {
	dataLock().lock();
	closing_ = true;
	dataLock().unlock();
}


void AssociationServer::setConnectionParameters( const ConnectionParameters & Params ) {
	dataLock().lock();
	connectionParameters_ = Params;
	dataLock().unlock();
}



void AssociationServer::setTransferSyntaxes( const UidList & List ) {
	dataLock().lock();
	transferSyntaxes_ = List;
	dataLock().unlock();
}


T_ASC_Network *& AssociationServer::tAscNetwork() {
	return tAscNetwork_;
}


const UidList & AssociationServer::transferSyntaxes() const {
	return transferSyntaxes_;
}

}; // Namespace DICOM ends here.
