/***************************************************************************
 *   Copyright (C) 2011 by Flux Inc.                                       *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#include "AbstractService.hpp"
#include "Exceptions.hpp"

#include "QtDicom/Association.hpp"
#include "QtDicom/Dataset.hpp"

#include <QtGui/QApplication>

#include <dcmtk/dcmnet/assoc.h>
#include <dcmtk/dcmnet/dimse.h>


namespace Dicom {

AbstractService::AbstractService() :
	association_( 0 )
{
}


AbstractService::AbstractService( Association * association ) :
	association_( association )
{
}


AbstractService::~AbstractService() {
}


Association * AbstractService::association() {
	return association_;
}


const Association * AbstractService::association() const {
	return association_;
}


void AbstractService::clearErrorStatus() {
	errorString_.clear();
}


const char * AbstractService::commandName( int command ) {
	switch ( command ) {
#define CASE( VALUE, NAME ) \
		case VALUE : { \
			static const char * TheName( NAME ); \
			return TheName; \
		}
		CASE( DIMSE_C_ECHO_RQ,    "C-ECHO-RQ"    );
		CASE( DIMSE_C_ECHO_RSP,   "C-ECHO-RSP"   );
		CASE( DIMSE_C_FIND_RQ,    "C-FIND-RQ"    );
		CASE( DIMSE_C_FIND_RSP,   "C-FIND-RSP"   );
		CASE( DIMSE_C_MOVE_RQ,    "C-MOVE-RQ"    );
		CASE( DIMSE_C_MOVE_RSP,   "C-MOVE-RSP"   );
		CASE( DIMSE_C_STORE_RQ,   "C-STORE-RQ"   );
		CASE( DIMSE_C_STORE_RSP,  "C-STORE-RSP"  );
		CASE( DIMSE_N_CREATE_RQ,  "N-CREATE-RQ"  );
		CASE( DIMSE_N_CREATE_RSP, "N-CREATE-RSP" );
		CASE( DIMSE_N_SET_RQ,     "N-SET-RQ"     );
		CASE( DIMSE_N_SET_RSP,    "N-SET-RSP"    );
#undef CASE
		case DIMSE_NOTHING :
		default : {
			static const char * TheName = "DIMSE command";

			return TheName;
		}
	}
}


const QString & AbstractService::errorString() const {
	return errorString_;
}


bool AbstractService::hasError() const {
	return errorString_.size() > 0;
}


void AbstractService::ignoreDataset() {
	DIC_UL bytes = 0;
	DIC_UL count = 0;

	const OFCondition Result = DIMSE_ignoreDataSet(
		association()->tAscAssociation(),
		DIMSE_NONBLOCKING,
		association()->connectionParameters().timeout(),
		&bytes, &count
	);
	if ( Result.good() ) {
		qDebug( "Ignoring a dataset (%d bytes in %d units).", bytes, count );
	}
	else {
		throw OperationFailedException(
			QString( 
				"Failed to ignore a dataset. "
				"Internal error description:\n%1"
			)
			.arg( Result.text() )
		);
	}
}


void AbstractService::progressCallback( void * data, unsigned long count ) {
	qDebug( __FUNCTION__" : with %ld bytes", count );

	/*
	if ( qApp != nullptr ) {
		qApp->processEvents();
	}
	*/
}


void AbstractService::raiseError( const QString & Message ) {
	if ( ! hasError() ) {
		errorString_ = Message;
	}
}


T_DIMSE_Message AbstractService::receiveCommand(
	int command, int seconds,
	unsigned char & id,
	bool * releaseRequested, bool * timedOut
) {
	if ( releaseRequested ) *releaseRequested = false;
	if ( timedOut ) *timedOut = false;

	const T_DIMSE_Command ExpectedCommand = 
		static_cast< T_DIMSE_Command >( command )
	;
	const QString & ExpectedCommandName = commandName( ExpectedCommand );	

	qDebug( "Receiving a %s", qPrintable( ExpectedCommandName ) );

	T_DIMSE_Message message;
	bzero( ( char * )& message, sizeof( message ) );
	const OFCondition Result = DIMSE_receiveCommand(
		association()->tAscAssociation(),
		DIMSE_NONBLOCKING,
		seconds,
		&id,
		&message, 0
	);

	if ( Result.good() ) {
		if ( ExpectedCommand == DIMSE_NOTHING  ) {
			qDebug(
				"A %s received",
				qPrintable( commandName( message.CommandField ) )
			);
		}
		else if ( ExpectedCommand != message.CommandField ) {
			throw OperationFailedException(
				QString( "Received command is not a %1 only a %2 (0x%3)." )
				.arg( ExpectedCommandName )
				.arg( commandName( message.CommandField ) )
				.arg( static_cast< int >( message.CommandField ), 4, 16, QChar( '0' ) )
			);
		}
	}
	else if ( Result == DUL_PEERREQUESTEDRELEASE && releaseRequested ) {
		*releaseRequested = true;
	}
	else if ( Result == DIMSE_NODATAAVAILABLE && timedOut ) {
		*timedOut = true;
	}
	else {
		if ( Result != DUL_PEERABORTEDASSOCIATION ) {
			association()->abort();
		}

		if ( Result == DUL_PEERREQUESTEDRELEASE ) {
			throw OperationFailedException(
				QString( 
					"DIMSE commands mismatch. "
					"A %1 expected but release request was received."
				)
				.arg( ExpectedCommandName )
			);
		}
		else {
			throw OperationFailedException(
				QString(
					"Failed to receive a %1. %2."
				)
				.arg( ExpectedCommandName )
				.arg( Result.text() )
			);
		}
	}

	return message;
}


T_DIMSE_Message AbstractService::receiveCommand(
	unsigned char & id, bool * released, bool * timedOut
) {
	return receiveCommand(
		DIMSE_NOTHING,
		association()->connectionParameters().timeout(),
		id, released, timedOut
	);
}


T_DIMSE_Message AbstractService::receiveCommand(
	int seconds, unsigned char & id, bool * released, bool * timedOut
) {
	return receiveCommand(
		DIMSE_NOTHING,
		seconds,
		id, released, timedOut
	);
}


Dataset AbstractService::receiveDataset( unsigned char & id ) {
	qDebug( "Retrieving a dataset." );

	DcmDataset dataset, * tmp = &dataset;
	OFCondition result = DIMSE_receiveDataSetInMemory(
		association()->tAscAssociation(),
		DIMSE_NONBLOCKING,
		association()->connectionParameters().timeout(),
		&id,
		& tmp,
		progressCallback, nullptr
	);
	if ( result.bad() ) {
		throw OperationFailedException(
			QString( "Failed to receive a dataset. %1." )
			.arg( result.text() )
		);
	}

	return Dataset( dataset );
}


void AbstractService::sendCommand(
	const T_DIMSE_Message & Command, const Dataset & Data, unsigned char id
) {
	const QString & CommandName = commandName( Command.CommandField );

	qDebug( "Sending a %s", qPrintable( CommandName ) );

	// ... fucking DCMTK.
	T_DIMSE_Message & command = const_cast< T_DIMSE_Message & >( Command );

	DcmDataset * dcmDataSet = const_cast< DcmDataset * >( &Data.dcmDataset() );

	OFCondition result = DIMSE_sendMessageUsingMemoryData(
		association()->tAscAssociation(), id,
		&command, 0,
		( Data.isEmpty() ? 0 : dcmDataSet ),
		progressCallback, nullptr
	);
	if ( result.bad() ) {
		throw OperationFailedException(
			QString( "Failed to send a %1. %2." )
			.arg( CommandName )
			.arg( result.text() )
		);
	}
}


void AbstractService::sendCommand(
	const T_DIMSE_Message & command, unsigned char id
) {
	sendCommand( command, Dataset(), id );
}


void AbstractService::setAssociation( Association * association ) {
	association_ = association;
}


}; // Namspace DICOM ends here.
