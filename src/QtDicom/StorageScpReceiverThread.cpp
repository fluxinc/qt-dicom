/***************************************************************************
 *   Copyright (C) 2011 by Flux Inc.                                       *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#include "StorageScpReceiverThread.hpp"

#include "QtDicom/AcceptorAssociation.hpp"

#include <QtCore/QDir>
#include <QtCore/QTemporaryFile>

#include <dcmtk/dcmnet/dimse.h>

namespace Dicom {

StorageScp::ReceiverThread::ReceiverThread( 
	AcceptorAssociation * association,
	const Destination & Destination,
	const QSharedPointer< QAtomicInt > & HoldTimePtr,
	QObject * parent
) :
	QThread( parent ),
	ServiceProvider( association ),
	Destination_( Destination ),
	HoldTimePtr_( HoldTimePtr )
{
}


StorageScp::ReceiverThread::~ReceiverThread() {
	delete association();
}


AcceptorAssociation * StorageScp::ReceiverThread::association() {
	return reinterpret_cast< AcceptorAssociation * >(
		ServiceProvider::association()
	);
}


QString StorageScp::ReceiverThread::createUniquePath( const QDir & Dir ) {
	QTemporaryFile f( Dir.absoluteFilePath( "store-scp-XXXXXX.dcm" ) );
	f.setAutoRemove( false );
	if ( f.open() ) {
		f.close();
		return f.fileName();
	}
	else {
		raiseError(
			QString( "Failed to create a file in the `%1' destination." )
			.arg( QDir::toNativeSeparators( Dir.absolutePath() ) )
		);
		return QString();
	}
}


inline const StorageScp::Destination &
	StorageScp::ReceiverThread::destination()
const {
	return Destination_;
}


inline int StorageScp::ReceiverThread::holdTime() const {
	return *HoldTimePtr_;
}


void StorageScp::ReceiverThread::run() {
	try {

	bool releaseRequested = false;
	unsigned char presentationContextId = 0;

	while ( ! hasError() ) {
		presentationContextId = 0;

		const T_DIMSE_Message Message = receiveCommand( 
			presentationContextId, &releaseRequested
		);

		if ( releaseRequested ) {
			association()->confirmRelease();
			break;
		}

		if ( holdTime() > 0 ) {
			qDebug( __FUNCTION__": sleeping for %d miliseconds", holdTime() );

			msleep( holdTime() );
		}

		if ( Message.CommandField == DIMSE_C_STORE_RQ ) {
			if ( destination() == Disk ) {
				const QString Path = createUniquePath( QDir::temp() );
				const bool Result = handleCStore(
					Message.msg.CStoreRQ,
					presentationContextId,
					Path
				);
				if ( ! Path.isEmpty() ) {
					emit stored( Path );
				}
			}
			else {
				const Dataset DataSet = handleCStore(
					Message.msg.CStoreRQ, presentationContextId
				);
				if ( ! DataSet.isEmpty() ) {
					emit stored( DataSet );
				}
			}
		}
		else if ( Message.CommandField == DIMSE_C_ECHO_RQ ) {
			handleCEcho( Message.msg.CEchoRQ, presentationContextId );
			emit echoReceived( association()->callingAeTitle() );
		}
		else {
			raiseError( "Unsupported command received." );
		}
	}

	}
	catch ( std::exception & e ) {
		raiseError( e.what() );
	}
	catch ( ... ) {
		raiseError( "Unknown exception occured." );
	}

	if ( hasError() ) {
		emit failedToStore( errorString() );
	}	
}


}; // Namespace DICOM ends here.
