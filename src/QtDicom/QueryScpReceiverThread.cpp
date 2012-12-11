/***************************************************************************
 *   Copyright (C) 2011 by Flux Inc.                                       *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#include "AcceptorAssociation.hpp"
#include "QueryScpReceiverThread.hpp"
#include "QueryScpReceiverThread.moc.inl"

#include <dcmtk/dcmnet/dimse.h>


namespace Dicom {

QueryScp::ReceiverThread::ReceiverThread(
	AcceptorAssociation * association, QObject * parent
) :
	QThread( parent ),
	ServiceProvider( association ),
	state_( Idle ),
	status_( 0 )
{
}


QueryScp::ReceiverThread::~ReceiverThread() {
	delete association();
}


AcceptorAssociation * QueryScp::ReceiverThread::association() {
	return reinterpret_cast< AcceptorAssociation * >( 
		ServiceProvider::association()
	);
}


QMutex & QueryScp::ReceiverThread::dataLock() const {
	return dataLock_;
}


bool QueryScp::ReceiverThread::hasQueuedIdentifiers() const {
	dataLock().lock();
	const bool Result = ! queue().isEmpty();
	dataLock().unlock();

	return Result;
}


void QueryScp::ReceiverThread::finish( int status ) {
	dataLock().lock();
	status_ = status;
	dataLock().unlock();
	setState( QueryFinishing );
}


bool QueryScp::ReceiverThread::finished() const {
	return state() == Finished;
}


bool QueryScp::ReceiverThread::queryFinishing() const {
	return state() == QueryFinishing;
}


QQueue< Dataset > & QueryScp::ReceiverThread::queue() {
	Q_ASSERT( ! dataLock().tryLock() ); // We can only access this member if
	                                    // the mutex was locked already.

	return queue_;
}


const QQueue< Dataset > & QueryScp::ReceiverThread::queue() const {
	Q_ASSERT( ! dataLock().tryLock() ); // We can only access this member if
	                                    // the mutex was locked already.

	return queue_;
}


void QueryScp::ReceiverThread::queueIdentifier( Dataset identifier ) {
	dataLock().lock();
	queue().enqueue( identifier );
	dataLock().unlock();
}


bool QueryScp::ReceiverThread::receivingCommands() const {
	return state() == ReceivingCommands;
}


void QueryScp::ReceiverThread::run() {
	Q_ASSERT( state() == Idle );
	if ( state() != Idle ) {
		qWarning( "Reusing Query SCP Receiver Thread object is invalid." );

		return;
	}

	try {

	bool released, timedOut;
	unsigned char presentationContextId = 0;
	T_DIMSE_C_FindRQ request;

	setState( ReceivingCommands );

	while ( ! ( finished() || hasError() ) ) {	
		const T_DIMSE_Message Message = receivingCommands() ? 
			receiveCommand( -1, presentationContextId, &released, &timedOut ) :
			receiveCommand(  0, presentationContextId,         0, &timedOut )
		;

		if ( hasError() ) {
			break;
		}

		if ( receivingCommands() ) {
			if ( released ) {
				association()->confirmRelease();
				setState( Finished );
				break;
			}
			else if ( timedOut ) {
				msleep( 100 );
				continue;
			}

			if ( Message.CommandField == DIMSE_C_FIND_RQ ) {
				const Dataset Mask = handleCFind( 
					Message.msg.CFindRQ, presentationContextId
				);
				request = Message.msg.CFindRQ;
				setState( QueryInProgress );
				emit newQuery( Mask, this );
			}
			else if ( Message.CommandField == DIMSE_C_ECHO_RQ ) {
				handleCEcho( Message.msg.CEchoRQ, presentationContextId );
			}
			else {
				raiseError( 
					QString( "Unsupported command received (%1)." )
					.arg( commandName( Message.CommandField ) )
				);
			}
		}
		// No cancel request.
		else if ( timedOut ) {
			dataLock().lock();
			while ( ! queue().isEmpty() ) {				
				sendIdentifier( 
					request, presentationContextId, queue().dequeue()
				);
			}
			dataLock().unlock();

			if ( queryFinishing() ) {
				sendStatus( request, presentationContextId, status() );
				setState( ReceivingCommands );
			}

			msleep( 100 );
		}
		else {
			if ( Message.CommandField == DIMSE_C_CANCEL_RQ ) {
				sendStatus( 
					request, presentationContextId,
					STATUS_FIND_Cancel_MatchingTerminatedDueToCancelRequest
				);
				setState( ReceivingCommands );
			}
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
		emit failedToQuery( errorMessage(), this );
	}
}


void QueryScp::ReceiverThread::setState( State state ) {
	dataLock().lock();
	state_ = state;
	dataLock().unlock();
}


void QueryScp::ReceiverThread::sendIdentifier(
	const T_DIMSE_C_FindRQ & Request, unsigned char ID, const Dataset & Identifier
) {
	T_DIMSE_Message response;
	bzero( ( char * )&response, sizeof( response ) );
	response.CommandField = DIMSE_C_FIND_RSP;

	T_DIMSE_C_FindRSP & params = response.msg.CFindRSP;	
	strcpy( params.AffectedSOPClassUID, Request.AffectedSOPClassUID );
	params.DataSetType = DIMSE_DATASET_PRESENT;
	params.DimseStatus = STATUS_Pending;
	params.MessageIDBeingRespondedTo = Request.MessageID;
	params.opts |= O_FIND_AFFECTEDSOPCLASSUID;

	sendCommand( response, Identifier, ID );
}


void QueryScp::ReceiverThread::sendStatus(
	const T_DIMSE_C_FindRQ & Request, unsigned char ID, int status
) {
	T_DIMSE_Message response;
	bzero( ( char * )&response, sizeof( response ) );
	response.CommandField = DIMSE_C_FIND_RSP;

	T_DIMSE_C_FindRSP & params = response.msg.CFindRSP;	
	strcpy( params.AffectedSOPClassUID, Request.AffectedSOPClassUID );
	params.DataSetType = DIMSE_DATASET_NULL;
	params.DimseStatus = status;
	params.MessageIDBeingRespondedTo = Request.MessageID;
	params.opts |= O_FIND_AFFECTEDSOPCLASSUID;

	sendCommand( response, ID );
}


int QueryScp::ReceiverThread::status() const {
	dataLock().lock();
	const int Result = status_;
	dataLock().unlock();

	return Result;
}


QueryScp::ReceiverThread::State QueryScp::ReceiverThread::state() const {
	dataLock().lock();
	const State Result = state_;
	dataLock().unlock();

	return Result;
}


}; // Namespace DICOM ends here.
