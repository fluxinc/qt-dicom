/***************************************************************************
 *   Copyright (C) 2011 by Flux Inc.                                       *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#include "StorageScp.hpp"
#include "StorageScpReceiverThread.hpp"

#include "QtDicom/AcceptorAssociation.hpp"

namespace Dicom {

StorageScp::StorageScp( QObject * parent ) :
	QObject( parent ),
	destination_( Disk )
{
}

StorageScp::StorageScp( Destination dst, QObject * parent ) :
	QObject( parent ),
	destination_( dst )
{
	Q_ASSERT( dst != Unknown );
}


StorageScp::~StorageScp() {
}


AssociationServer & StorageScp::associationServer() {
	return associationServer_;
}


const AssociationServer & StorageScp::associationServer() const {
	return associationServer_;
}


void StorageScp::createReceiverThread() {
	Q_ASSERT( associationServer().hasPendingConnections() );
	AcceptorAssociation * association = reinterpret_cast< AcceptorAssociation * >(
		associationServer().nextPendingAssociation()
	);

	lastAe_ = association->callingAeTitle();
	lastCalledAe_ = association->calledAeTitle();

	ReceiverThread * thread = new ReceiverThread( association, destination(), this );
	thread->start();
	connect( 
		thread, SIGNAL( stored( QString ) ),
		SIGNAL( stored( QString ) )
	);
	connect( 
		thread, SIGNAL( stored( Dicom::Dataset ) ),
		SIGNAL( stored( Dicom::Dataset ) )
	);
	connect( 
		thread, SIGNAL( failedToStore( QString ) ),
		SIGNAL( failedToStore( QString ) )
	);
	
	connect(
		thread, SIGNAL( echoReceived( QString ) ),
		SIGNAL( echoReceived( QString ) )
	);
	connect(
		thread, SIGNAL( finished() ),
		thread, SLOT( deleteLater() )
	);
}


StorageScp::Destination StorageScp::destination() const {
	return destination_;
}


const QString & StorageScp::destinationString( Destination dst ) {
	switch ( dst ) {
#define CASE( ITEM ) \
		case ITEM : { \
			static const QString ITEM = #ITEM; \
			return ITEM; \
		}
		CASE( Disk );
		CASE( Memory );
#undef CASE
		default : {
			static const QString Unknown = "Unknown";
			return Unknown;
		}
	};
}


StorageScp::Destination StorageScp::destinationFromString( const QString & Value ) {
	static const QString & DiskString = destinationString( Disk );
	static const QString & MemoryString = destinationString( Memory );

	if ( Value.compare( DiskString, Qt::CaseInsensitive ) == 0 ) {
		return Disk;
	}
	else if ( Value.compare( MemoryString, Qt::CaseInsensitive ) == 0 ) {
		return Memory;
	}
	else {
		return Unknown;
	}
}


QString StorageScp::errorString() const {
	return errorString_;
}


const QString & StorageScp::lastAe() const {
	return lastAe_;
}


const QString & StorageScp::lastCalledAe() const {
	return lastCalledAe_;
}


void StorageScp::setDestination( Destination destination ) {
	destination_ = destination;
}


bool StorageScp::start( const ConnectionParameters & Parameters ) {
	associationServer().setAbstractSyntaxes( 
		UidList::storageSopClasses() + UidList::echoSopClass()
	);
	associationServer().setTransferSyntaxes( UidList::supportedTransferSyntaxes() );
	if ( associationServer().listen( Parameters ) ) {
		connect(
			&associationServer(), SIGNAL( newAssociation() ),
			SLOT( createReceiverThread() )
		);
		connect(
			&associationServer(), SIGNAL( newAssociationError( QString ) ),
			SIGNAL( failedToStore( QString ) )
		);
		return true;
	}
	else {
		qWarning( qPrintable( associationServer().errorString() ) );
		return false;
	}
}


void StorageScp::stop() {
	if ( associationServer().isListening() ) {
		associationServer().close();
	}
}

}; // Namespace DICOM ends here.
