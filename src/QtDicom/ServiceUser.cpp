/***************************************************************************
 *   Copyright (C) 2011 by Flux Inc.                                       *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#include "Exceptions.hpp"
#include "QSopClass.hpp"
#include "ServiceUser.hpp"

#include <QtCore/QStringList>

#include <QtDicom/QPresentationContext>
#include <QtDicom/RequestorAssociation.hpp>

#include <dcmtk/dcmdata/dcdeftag.h>
#include <dcmtk/dcmdata/dcuid.h>

#include <dcmtk/dcmnet/dimse.h>
#include <dcmtk/dcmnet/diutil.h>


namespace Dicom {

ServiceUser::ServiceUser() :
	AbstractService()
{
	clearErrorStatus();
}


ServiceUser::ServiceUser( Association * association ) :
	AbstractService( association )
{
	clearErrorStatus();
}


ServiceUser::~ServiceUser() {
}


bool ServiceUser::cEcho() {
	qDebug( "Performing a C-ECHO operation." );

	clearErrorStatus();

	try {

	if ( ! ( association() || association()->isEstablished() ) ) {
		throw OperationFailedException( "Invalid association." );
	}

	T_ASC_PresentationContextID presentationContextId = 
		association()->acceptedPresentationContextId( UID_VerificationSOPClass )
	;

	if ( presentationContextId < 1 ) {
		throw OperationFailedException(
			QString( 
				"Unable to find presentation context ID "
				"matching Verfication SOP Class: `%1'."
			)
			.arg( UID_VerificationSOPClass )
		);
	}

	T_DIMSE_Message request;
	::bzero( ( char * )& request, sizeof( request ) );
	request.CommandField = DIMSE_C_ECHO_RQ;

	T_DIMSE_C_EchoRQ & requestParameters = request.msg.CEchoRQ;
	strcpy( requestParameters.AffectedSOPClassUID, UID_VerificationSOPClass );
	requestParameters.DataSetType = DIMSE_DATASET_NULL;
	requestParameters.MessageID = association()->nextMessageId();

	sendCommand( request, presentationContextId );

	const T_DIMSE_Message Response = receiveCommand( 
		DIMSE_C_ECHO_RSP, presentationContextId
	);

	validateCEchoResponse( Response, request );

	return true;

	} // End of the try block.
	catch ( std::exception & e ) {
		raiseError( e.what() );
	}
	catch ( ... ) {
		raiseError( "Unknown exception occured." );
	}	

	return false;
}


QList< Dataset > ServiceUser::cFind( 
	const Dataset & Attributes, const char * SopClass
) {
	qDebug( "Performing a C-FIND operation." );

	clearErrorStatus();

	QList< Dicom::Dataset > responseIdentifiers;
	try {
	
	if ( ! ( association() || association()->isEstablished() ) ) {
		throw OperationFailedException( "Invalid association." );
	}

	T_ASC_PresentationContextID presentationContextId = 
		association()->acceptedPresentationContextId( SopClass )
	;

	if ( presentationContextId < 1 ) {
		throw OperationFailedException(
			QString( 
				"Unable to find presentation context ID "
				"matching SOP Class: `%1'."
			)
			.arg( SopClass )
		);
	}

	T_DIMSE_Message request;
	::bzero( ( char * )& request, sizeof( request ) );
	request.CommandField = DIMSE_C_FIND_RQ;

	T_DIMSE_C_FindRQ & requestParameters = request.msg.CFindRQ;
	strcpy( requestParameters.AffectedSOPClassUID, SopClass );
	requestParameters.DataSetType = DIMSE_DATASET_PRESENT;
	requestParameters.MessageID = association()->nextMessageId();
	requestParameters.Priority = DIMSE_PRIORITY_HIGH;

	sendCommand( request, Attributes, presentationContextId );

	bool finished = false;
	do {
		const T_DIMSE_Message Response = receiveCommand( 
			DIMSE_C_FIND_RSP, presentationContextId
		);

		finished = validateCFindResponse( Response, request );

		if ( finished ) {
			break;
		}

		if ( Response.msg.CFindRSP.DataSetType == DIMSE_DATASET_NULL ) {
			qWarning( 
				"Non-conformant Query SCP detected: C-FIND pending response "
				"does not contain a dataset. Ignoring."
			);
			continue;
		}

		const Dataset Identifier = receiveDataset( presentationContextId );
		
		responseIdentifiers.append( Identifier );
	} while ( ! finished );

	return responseIdentifiers;

	} // End of the try block.
	catch ( std::exception & e ) {
		raiseError( e.what() );
	}
	catch ( ... ) {
		raiseError( "Unknown exception occured." );
	}	

	return responseIdentifiers;
}


int ServiceUser::cMove( 
	const Dataset & Attributes, const char * SopClass,
	const QString & DestinationAe,
	int * failed, UidList * failedInstances,
	int * warned
) {
	qDebug( "Performing a C-MOVE operation." );

	clearErrorStatus();

	if ( failed ) *failed = 0;
	if ( failedInstances ) failedInstances->clear();
	if ( warned ) *warned = 0;

	try {
	
	if ( ! ( association() || association()->isEstablished() ) ) {
		throw OperationFailedException( "Invalid association." );
	}

	T_ASC_PresentationContextID presentationContextId = 
		association()->acceptedPresentationContextId( SopClass )
	;

	if ( presentationContextId < 1 ) {
		throw OperationFailedException(
			QString( 
				"Unable to find presentation context ID "
				"matching SOP Class: `%1'."
			)
			.arg( SopClass )
		);
	}

	T_DIMSE_Message request;
	::bzero( ( char * )& request, sizeof( request ) );
	request.CommandField = DIMSE_C_MOVE_RQ;

	T_DIMSE_C_MoveRQ & requestParameters = request.msg.CMoveRQ;
	strcpy( requestParameters.AffectedSOPClassUID, SopClass );
	requestParameters.DataSetType = DIMSE_DATASET_PRESENT;
	requestParameters.MessageID = association()->nextMessageId();
	strcpy( requestParameters.MoveDestination, DestinationAe.toLatin1() );
	requestParameters.Priority = DIMSE_PRIORITY_HIGH;

	sendCommand( request, Attributes, presentationContextId );

	int completed = 0;

	bool finished = false;
	do {
		const T_DIMSE_Message Response = receiveCommand( 
			DIMSE_C_MOVE_RSP, presentationContextId
		);

		finished = validateCMoveResponse( Response, request );

		if ( finished ) {
			const T_DIMSE_C_MoveRSP Parameters = Response.msg.CMoveRSP;
			if ( warned ) {
				*warned = Parameters.NumberOfWarningSubOperations;
			}
			if ( failed ) {
				*failed = Parameters.NumberOfFailedSubOperations;
			}

			if ( Parameters.DimseStatus == STATUS_Success ) {
				return Parameters.NumberOfCompletedSubOperations;
			}

			if ( Parameters.DataSetType != DIMSE_DATASET_PRESENT ) {
				qWarning( 
					"Non-conformant Move SCP detected: C-MOVE final response "
					"does NOT contain a dataset. Ignoring."
				);
			}
			else if ( failedInstances ) {
				const Dataset Dset = receiveDataset( presentationContextId );

				bool exists;
				const QString Value = Dset.tagValue( 
					DCM_FailedSOPInstanceUIDList, &exists
				);
				if ( exists && ! Value.isEmpty() ) {
					*failedInstances = Value.split( '\\' );
				}
				else {
					qWarning( 
						"Non-conformant Move SCP detected: C-MOVE final response's "
						"dataset does not contain Failed SOP Instance UID List."
					);
				}
			}
			else {
				ignoreDataset();
			}

			return 0;
		}
		else if ( Response.msg.CMoveRSP.DataSetType == DIMSE_DATASET_PRESENT ) {
			qWarning( 
				"Non-conformant Move SCP detected: C-MOVE pending response "
				"does contain a dataset. Ignoring."
			);
			ignoreDataset();
		}		
	} while ( ! finished );

	Q_ASSERT( 0 );

	} // End of the try block.
	catch ( std::exception & e ) {
		raiseError( e.what() );
	}
	catch ( ... ) {
		raiseError( "Unknown exception occured." );
	}	

	return -1;
}


bool ServiceUser::cStore( const Dataset & dataset ) {
	return cStore( dataset, QString(), -1 );
}


bool ServiceUser::cStore( 
	const Dataset & Dataset, const QString & MoveAe, int moveId
) {
	qDebug( "Performing a C-STORE operation." );

	clearErrorStatus();

	try {

	if ( ! ( association() || association()->isEstablished() ) ) {
		throw OperationFailedException( "Invalid association." );
	}

	const QByteArray SopClass = Dataset.sopClassUid();
	const QByteArray SopInstance = Dataset.sopInstanceUid();
	if ( SopClass.isEmpty() || SopInstance.isEmpty() ) {
		throw OperationFailedException(
			"Failed to read SOP Class and SOP Instance UIDs from the dataset."
		);
	}

	const QTransferSyntax TransferSyntax = Dataset.syntax();
#ifdef _DEBUG
	RequestorAssociation * a = reinterpret_cast< RequestorAssociation * >( association() );
	const QList< QPresentationContext > Contexts = a->acceptedPresentationContexts();

	bool foundPresentationContext = false;
	foreach( const QPresentationContext & Context, Contexts ) {
		if ( Context.abstractSyntax() == QUid( SopClass ) ) {
			if ( Context.acceptedTransferSyntax() == TransferSyntax ) {
				foundPresentationContext = true;
				break;
			}
		}
	}

	Q_ASSERT( foundPresentationContext );
#endif

	T_ASC_PresentationContextID presentationContextId = 
		association()->acceptedPresentationContextId( 
			SopClass, TransferSyntax.uid()
		);

	if ( presentationContextId < 1 ) {
		throw OperationFailedException(
			QString( 
				"Unable to find presentation context ID "
				"matching SOP Class: %1 and Transfer Syntax: %2"
			)
			.arg( SopClass.constData() )
			.arg( TransferSyntax.name() )
		);
	}

	T_DIMSE_Message request;
	::bzero( ( char * )& request, sizeof( request ) );
	request.CommandField = DIMSE_C_STORE_RQ;

	T_DIMSE_C_StoreRQ & requestParameters = request.msg.CStoreRQ;
	strcpy( requestParameters.AffectedSOPClassUID, SopClass );
	strcpy( requestParameters.AffectedSOPInstanceUID, SopInstance );
	requestParameters.DataSetType = DIMSE_DATASET_PRESENT;
	requestParameters.MessageID = association()->nextMessageId();
	if ( ! MoveAe.isEmpty() ) {
		const QByteArray Ae = MoveAe.toLatin1();
		strcpy( 
			requestParameters.MoveOriginatorApplicationEntityTitle,
			Ae.constData()
		);
		requestParameters.MoveOriginatorID = ( quint16 )moveId;
	}
	requestParameters.Priority = DIMSE_PRIORITY_HIGH;

	sendCommand( request, Dataset, presentationContextId );

	const T_DIMSE_Message Response = receiveCommand( 
		DIMSE_C_STORE_RSP, presentationContextId
	);

	validateCStoreResponse( Response, request );

	return true;

	} // End of the try block.
	catch ( std::exception & e ) {
		raiseError( e.what() );
	}
	catch ( ... ) {
		raiseError( "Unknown exception occured." );
	}	

	return false;
}


bool ServiceUser::nAction(
	const char * SopClass,
	const char * SopInstance,
	const quint16 & ActionId,
	const Dataset & ActionData,
	Dataset * replyData,
	quint16 * status
) {
	qDebug( "Performing an N-ACTION operation." );

	try {

	if ( ! ( association() || association()->isEstablished() ) ) {
		throw OperationFailedException( "Invalid association." );
	}

	T_ASC_PresentationContextID presentationContextId = 
		association()->acceptedPresentationContextId( SopClass )
	;

	if ( presentationContextId < 1 ) {
		throw OperationFailedException(
			QString( 
				"Unable to find presentation context ID "
				"matching SOP Class: `%1'."
			)
			.arg( SopClass )
		);
	}

	T_DIMSE_Message request;
	::bzero( ( char * )& request, sizeof( request ) );
	request.CommandField = DIMSE_N_ACTION_RQ;

	T_DIMSE_N_ActionRQ & requestParameters = request.msg.NActionRQ;
	requestParameters.ActionTypeID = ActionId;
	requestParameters.DataSetType = ActionData.isEmpty() ?
		DIMSE_DATASET_NULL : DIMSE_DATASET_PRESENT
	;
	requestParameters.MessageID = association()->nextMessageId();
	strcpy( requestParameters.RequestedSOPClassUID,    SopClass );
	strcpy( requestParameters.RequestedSOPInstanceUID, SopInstance );

	sendCommand( request, ActionData, presentationContextId );

	do {
		const T_DIMSE_Message Response = receiveCommand( presentationContextId );
		const T_DIMSE_Command & Command = Response.CommandField;

		if ( Command == DIMSE_N_ACTION_RSP) {
			validateNActionResponse( Response, request );

			const T_DIMSE_N_ActionRSP & ResponseParams = Response.msg.NActionRSP;
			if ( status ) {
				*status = ResponseParams.DimseStatus;
			}

			if ( ResponseParams.DataSetType == DIMSE_DATASET_PRESENT ) {
				if ( replyData != nullptr ) {
					*replyData = receiveDataset( presentationContextId );
				}
				else {
					ignoreDataset();
				}
			}

			return true;
		}
		else if ( Command == DIMSE_N_EVENT_REPORT_RQ ) {
			processNEventReport( Response, presentationContextId );
		}
		else {
			throw OperationFailedException(
				QString(
					"Received command is neither an N-ACTION response nor an N-EVENT "
					"request. Got a %1 (0x%2) instead" )
				.arg( commandName( Command ) )
				.arg( static_cast< int >( Command ), 4, 16, QChar( '0' ) )
			);
		}
	} while ( true );

	} // Try block ends here.
	catch ( std::exception & e ) {
		raiseError( e.what() );
	}
	catch ( ... ) {
		raiseError( "Unknown exception occured." );
	}

	return false;
}


bool ServiceUser::nAction(
	const char * SopClass,
	const char * SopInstance,
	const quint16 & Action,
	quint16 * status
) {
	return nAction( SopClass, SopInstance, Action, Dataset(), nullptr, status );
}


bool ServiceUser::nAction(
	const QSopClass & SopClass,
	const char * SopInstance,
	const quint16 & ActionId,
	const Dataset & ActionData,
	Dataset * replyData,
	quint16 * status
) {
	return nAction( SopClass.uid(), SopInstance, ActionId, ActionData, replyData, status );
}


bool ServiceUser::nAction(
	const QSopClass & SopClass,
	const char * SopInstance,
	const quint16 & Action,
	quint16 * status
) {
	return nAction( SopClass.uid(), SopInstance, Action, Dataset(), nullptr, status );
}




QByteArray ServiceUser::nCreate(
	const char * SopClass, 
	const char * SopInstance, 
	const Dataset & Attributes,
	Dataset * affectedAttributes,
	quint16 * status
) {
	Q_ASSERT( SopClass );

	qDebug( "Performing a N-CREATE operation." );

	try {

	if ( ! ( association() || association()->isEstablished() ) ) {
		throw OperationFailedException( "Invalid association." );
	}

	T_ASC_PresentationContextID presentationContextId = 
		association()->acceptedPresentationContextId( SopClass )
	;

	if ( presentationContextId < 1 ) {
		throw OperationFailedException(
			QString( 
				"Unable to find presentation context ID "
				"matching SOP Class: `%1'."
			)
			.arg( SopClass )
		);
	}

	T_DIMSE_Message request;
	::bzero( ( char * )& request, sizeof( request ) );
	request.CommandField = DIMSE_N_CREATE_RQ;

	T_DIMSE_N_CreateRQ & requestParameters = request.msg.NCreateRQ;
	strcpy( requestParameters.AffectedSOPClassUID, SopClass );
	requestParameters.opts |= O_NCREATE_AFFECTEDSOPCLASSUID;

	if ( SopInstance ) {
		strcpy( requestParameters.AffectedSOPInstanceUID, SopInstance );
		requestParameters.opts |= O_NCREATE_AFFECTEDSOPINSTANCEUID;
	}
	else {
		requestParameters.AffectedSOPInstanceUID[ 0 ] = 0;
	}
	requestParameters.DataSetType = Attributes.isEmpty() ? 
		DIMSE_DATASET_NULL : DIMSE_DATASET_PRESENT
	;
	requestParameters.MessageID = association()->nextMessageId();

	sendCommand( request, Attributes, presentationContextId );

	do {
		const T_DIMSE_Message Response = receiveCommand( presentationContextId );
		const T_DIMSE_Command & Command = Response.CommandField;

		if ( Command == DIMSE_N_CREATE_RSP ) {
			validateNCreateResponse( Response, request );

			const T_DIMSE_N_CreateRSP & ResponseParams = Response.msg.NCreateRSP;
			const QByteArray AffectedSopInstance = 
				ResponseParams.AffectedSOPInstanceUID
			;

			if ( status ) {
				*status = ResponseParams.DimseStatus;
			}

			if ( ResponseParams.DataSetType == DIMSE_DATASET_PRESENT ) {
				if ( affectedAttributes ) {
					*affectedAttributes = receiveDataset( presentationContextId );
				}
				else {
					ignoreDataset();
				}
			}

			return AffectedSopInstance;
		}
		else if ( Command == DIMSE_N_EVENT_REPORT_RQ ) {
			processNEventReport( Response, presentationContextId );
		}
		else {
			throw OperationFailedException(
				QString(
					"Received command is neither an N-CREATE response nor an N-EVENT "
					"request. Got a %1 (0x%2) instead" )
				.arg( commandName( Command ) )
				.arg( static_cast< int >( Command ), 4, 16, QChar( '0' ) )
			);
		}
	} while ( true );

	} // Try block ends here.
	catch ( std::exception & e ) {
		raiseError( e.what() );
	}
	catch ( ... ) {
		raiseError( "Unknown exception occured." );
	}

	return QByteArray();
}


QByteArray ServiceUser::nCreate(
	const QSopClass & SopClass, 
	const char * SopInstance, 
	const Dataset & Attributes,
	Dataset * affectedAttributes,
	quint16 * status
) {
	Q_ASSERT( SopClass.isValid() );
	return nCreate(
		SopClass.uid(), SopInstance, Attributes, affectedAttributes, status
	);
}


QByteArray ServiceUser::nCreate( 
	const char * SopClass,
	const Dataset & Attributes,
	Dataset * affectedAttrbutes,
	quint16 * status
) {
	return nCreate( SopClass, nullptr, Attributes, affectedAttrbutes, status );
}


QByteArray ServiceUser::nCreate( 
	const QSopClass & SopClass,
	const Dataset & Attributes,
	Dataset * affectedAttrbutes,
	quint16 * status
) {
	Q_ASSERT( SopClass.isValid() );
	return nCreate(
		SopClass.uid(), nullptr, Attributes, affectedAttrbutes, status
	);
}


bool ServiceUser::nDelete(
	const char * SopClass,
	const char * SopInstance,
	quint16 * status
) {
	qDebug( "Performing an N-DELETE operation." );

	try {

	if ( ! ( association() || association()->isEstablished() ) ) {
		throw OperationFailedException( "Invalid association." );
	}

	T_ASC_PresentationContextID presentationContextId = 
		association()->acceptedPresentationContextId( SopClass )
	;

	if ( presentationContextId < 1 ) {
		throw OperationFailedException(
			QString( 
				"Unable to find presentation context ID "
				"matching SOP Class: `%1'."
			)
			.arg( SopClass )
		);
	}

	T_DIMSE_Message request;
	::bzero( ( char * )& request, sizeof( request ) );
	request.CommandField = DIMSE_N_DELETE_RQ;

	T_DIMSE_N_DeleteRQ & requestParameters = request.msg.NDeleteRQ;
	requestParameters.DataSetType = DIMSE_DATASET_NULL;
	requestParameters.MessageID = association()->nextMessageId();
	strcpy( requestParameters.RequestedSOPClassUID,    SopClass );
	strcpy( requestParameters.RequestedSOPInstanceUID, SopInstance );

	sendCommand( request, presentationContextId );

	do {
		const T_DIMSE_Message Response = receiveCommand( presentationContextId );
		const T_DIMSE_Command & Command = Response.CommandField;

		if ( Command == DIMSE_N_DELETE_RSP ) {
			validateNDeleteResponse( Response, request );

			const T_DIMSE_N_DeleteRSP & ResponseParams = Response.msg.NDeleteRSP;
			if ( status ) {
				*status = ResponseParams.DimseStatus;
			}

			if ( ResponseParams.DataSetType == DIMSE_DATASET_PRESENT ) {
				ignoreDataset();
			}

			return true;
		}
		else if ( Command == DIMSE_N_EVENT_REPORT_RQ ) {
			processNEventReport( Response, presentationContextId );
		}
		else {
			throw OperationFailedException(
				QString(
					"Received command is neither an N-DELETE response nor an N-EVENT "
					"request. Got a %1 (0x%2) instead" )
				.arg( commandName( Command ) )
				.arg( static_cast< int >( Command ), 4, 16, QChar( '0' ) )
			);
		}
	} while ( true );

	} // Try block ends here.
	catch ( std::exception & e ) {
		raiseError( e.what() );
	}
	catch ( ... ) {
		raiseError( "Unknown exception occured." );
	}

	return false;
}


bool ServiceUser::nDelete(
	const QSopClass & SopClass,
	const char * SopInstance,
	quint16 * status
) {
	return nDelete( SopClass.uid(), SopInstance, status );
}


QByteArray ServiceUser::nSet(
	const char * SopClass, 
	const char * SopInstance, 
	const Dataset & Attributes,
	Dataset * modifiedAttrbutes,
	quint16 * status
) {
	qDebug( "Performing a N-SET operation." );

	try {

	if ( ! ( association() || association()->isEstablished() ) ) {
		throw OperationFailedException( "Invalid association." );
	}

	T_ASC_PresentationContextID presentationContextId = 
		association()->acceptedPresentationContextId( SopClass )
	;

	if ( presentationContextId < 1 ) {
		throw OperationFailedException(
			QString( 
				"Unable to find presentation context ID "
				"matching SOP Class: `%1'."
			)
			.arg( SopClass )
		);
	}

	T_DIMSE_Message request;
	::bzero( ( char * )& request, sizeof( request ) );
	request.CommandField = DIMSE_N_SET_RQ;

	T_DIMSE_N_SetRQ & requestParameters = request.msg.NSetRQ;
	strcpy( requestParameters.RequestedSOPClassUID,    SopClass );
	strcpy( requestParameters.RequestedSOPInstanceUID, SopInstance );
	requestParameters.DataSetType = Attributes.isEmpty() ? 
		DIMSE_DATASET_NULL : DIMSE_DATASET_PRESENT
	;
	requestParameters.MessageID = association()->nextMessageId();

	sendCommand( request, Attributes, presentationContextId );

	do {
		const T_DIMSE_Message Response = receiveCommand( presentationContextId );
		const T_DIMSE_Command & Command = Response.CommandField;

		if ( Command == DIMSE_N_SET_RSP) {
			validateNSetResponse( Response, request );

			const T_DIMSE_N_SetRSP & ResponseParams = Response.msg.NSetRSP;
			const QByteArray AffectedSopInstance = 
				ResponseParams.AffectedSOPInstanceUID
			;

			if ( status ) {
				*status = ResponseParams.DimseStatus;
			}

			if ( ResponseParams.DataSetType == DIMSE_DATASET_PRESENT ) {
				if ( modifiedAttrbutes ) {
					*modifiedAttrbutes = receiveDataset( presentationContextId );
				}
				else {
					ignoreDataset();
				}
			}

			return AffectedSopInstance;
		}
		else if ( Command == DIMSE_N_EVENT_REPORT_RQ ) {
			processNEventReport( Response, presentationContextId );
		}
		else {
			throw OperationFailedException(
				QString(
					"Received command is neither an N-SET response nor an N-EVENT "
					"request. Got a %1 (0x%2) instead" )
				.arg( commandName( Command ) )
				.arg( static_cast< int >( Command ), 4, 16, QChar( '0' ) )
			);
		}
	} while ( true );

	} // Try block ends here.
	catch ( std::exception & e ) {
		raiseError( e.what() );
	}
	catch ( ... ) {
		raiseError( "Unknown exception occured." );
	}

	return QByteArray();
}


Dataset ServiceUser::processNEventReport(
	const T_DIMSE_Message & Request, unsigned char & id
) {
	const T_DIMSE_N_EventReportRQ & RequestParams = Request.msg.NEventReportRQ;
	Dataset result;

	if ( RequestParams.DataSetType == DIMSE_DATASET_PRESENT ) {
		result = receiveDataset( id );

		qDebug(
			__FUNCTION__" event Data Set: `%s'",
			qPrintable( result.toString() )
		);
	}

	T_DIMSE_Message response;
	::bzero( &response, sizeof( response ) );
	response.CommandField = DIMSE_N_EVENT_REPORT_RSP;

	T_DIMSE_N_EventReportRSP & responseParams = response.msg.NEventReportRSP;
	responseParams.AffectedSOPClassUID[ 0 ] = '\0';
	responseParams.AffectedSOPInstanceUID[ 0 ] = '\0';
	responseParams.DataSetType = DIMSE_DATASET_NULL;
	responseParams.DimseStatus = STATUS_Success;
	responseParams.EventTypeID = RequestParams.EventTypeID;
	responseParams.MessageIDBeingRespondedTo = RequestParams.MessageID;
	responseParams.opts = O_NEVENTREPORT_EVENTTYPEID;

	sendCommand( response, id );

	return result;
}


void ServiceUser::validateCEchoResponse(
	const T_DIMSE_Message & Response,
	const T_DIMSE_Message & Request
) {
	Q_ASSERT( Response.CommandField == DIMSE_C_ECHO_RSP );

	const T_DIMSE_C_EchoRSP ResponseParameters = Response.msg.CEchoRSP;
	const T_DIMSE_C_EchoRQ  RequestParameters =  Request.msg.CEchoRQ;

	if ( 
		ResponseParameters.MessageIDBeingRespondedTo != 
		RequestParameters.MessageID
	) {
		throw OperationFailedException(
			QString( 
				"Response's Message ID is different than request's: %1 vs %2"
			)
			.arg( ResponseParameters.MessageIDBeingRespondedTo )
			.arg( RequestParameters.MessageID )
		);
	}
	else if ( ResponseParameters.DimseStatus != STATUS_Success ) {
		throw OperationFailedException(
			QString( 
				"Response status does not contain a SUCCESS value, 0x%1 found."
			)
			.arg( ResponseParameters.DimseStatus, 4, 16, QChar( '0' ) )
		);
	}

	if ( ResponseParameters.DataSetType != DIMSE_DATASET_NULL ) {
		qWarning( 
			"Non-conformant Echo SCP detected: C-ECHO response contains a "
			"dataset. Ignoring."
		);
	}
}



bool ServiceUser::validateCFindResponse(
	const T_DIMSE_Message & Response,
	const T_DIMSE_Message & Request
) {
	Q_ASSERT( Response.CommandField == DIMSE_C_FIND_RSP );

	const T_DIMSE_C_FindRSP ResponseParameters = Response.msg.CFindRSP;
	const T_DIMSE_C_FindRQ  RequestParameters =  Request.msg.CFindRQ;

	if ( 
		ResponseParameters.MessageIDBeingRespondedTo != 
		RequestParameters.MessageID
	) {
		throw OperationFailedException(
			QString( 
				"Response's Message ID is different than request's: %1 vs %2"
			)
			.arg( ResponseParameters.MessageIDBeingRespondedTo )
			.arg( RequestParameters.MessageID )
		);
	}

	bool finished = true;
	switch ( ResponseParameters.DimseStatus ) {
		case STATUS_Success :
			break;
		case STATUS_FIND_Pending_WarningUnsupportedOptionalKeys :
			qWarning(
				"One or more Optional Keys were not "
				"supported for existence and/or matching "
				"for this Identifier."
			);
			// Fallthrough
		case STATUS_Pending : {
			if ( ResponseParameters.DataSetType != DIMSE_DATASET_PRESENT ) {
				qWarning( "Recieved pending status without a dataset." );
			}
			finished = false;
			break;
		}
		case STATUS_FIND_Refused_OutOfResources :
			qWarning(
				"Received status: `Refused: Out of Resources'."
			);
			break;
		case STATUS_FIND_Failed_IdentifierDoesNotMatchSOPClass :
			throw OperationFailedException(
				"Received status: `Identifier does not match SOP Class'."
			);
		case STATUS_FIND_Cancel_MatchingTerminatedDueToCancelRequest :
			qDebug(
				"Received status: `Matching Terminated Due To Cancel Request'."
			);
			break;
		default :
			if ( ( ResponseParameters.DimseStatus & 0xF000 ) == 0xC000  ) {
				throw OperationFailedException(
					"Received status: `Unable to process'."
				);
			}
			else {
				qWarning(
					"Unknown status received: 0x%04X",
					( int )ResponseParameters.DimseStatus
				);
				finished = true;
			}
	};

	return finished;
}


bool ServiceUser::validateCMoveResponse(
	const T_DIMSE_Message & Response,
	const T_DIMSE_Message & Request
) {
	Q_ASSERT( Response.CommandField == DIMSE_C_MOVE_RSP );

	const T_DIMSE_C_MoveRSP ResponseParameters = Response.msg.CMoveRSP;
	const T_DIMSE_C_MoveRQ  RequestParameters =  Request.msg.CMoveRQ;

	if ( 
		ResponseParameters.MessageIDBeingRespondedTo != 
		RequestParameters.MessageID
	) {
		throw OperationFailedException(
			QString( 
				"Response's Message ID is different than request's: %1 vs %2"
			)
			.arg( ResponseParameters.MessageIDBeingRespondedTo )
			.arg( RequestParameters.MessageID )
		);
	}

	// Those status codes that do not include the number of 
	// Completed/Failed/Warned sub-operations throw an exception.
	bool finished = true;
	switch ( ResponseParameters.DimseStatus ) {
		case STATUS_Success :
			break;
		case STATUS_Pending :
			finished = false;
			break;
		case STATUS_MOVE_Refused_OutOfResourcesNumberOfMatches :
			throw OperationFailedException(
				"Received status: `Refused: Out of Resources - "
				"Unable to calculate number of matches'."
			);
		case STATUS_MOVE_Refused_OutOfResourcesSubOperations :
			raiseError(
				"Received status: `Refused: Out of Resources - "
				"Unable to perform sub-operations'."
			);
			break;
		case STATUS_MOVE_Failed_MoveDestinationUnknown :
			throw OperationFailedException(
				"Received status: `Refused: Move Destination unknown'."
			);
		case STATUS_MOVE_Failed_IdentifierDoesNotMatchSOPClass :
			throw OperationFailedException(
				"Received status: `Identifier does not match SOP Class'."
			);
		case STATUS_MOVE_Cancel_SubOperationsTerminatedDueToCancelIndication :
			qDebug(
				"Received status: `Sub-operations terminated due to Cancel Indication'."
			);
			break;
		case STATUS_MOVE_Warning_SubOperationsCompleteOneOrMoreFailures :
			qWarning(
				"Received status: `Sub-operations Complete - "
				"One or more Failures'."
			);
			break;
		default :
			if ( ( ResponseParameters.DimseStatus & 0xF000 ) == 0xC000  ) {
				throw OperationFailedException(
					"Received status: `Unable to process'."
				);
			}
			else {
				qWarning(
					"Unrecognized status received: 0x%04X.",
					( int )ResponseParameters.DimseStatus
				);
				finished = true;
			}
	};

	return finished;
}


void ServiceUser::validateCStoreResponse(
	const T_DIMSE_Message & Response,
	const T_DIMSE_Message & Request
) {
	Q_ASSERT( Response.CommandField == DIMSE_C_STORE_RSP );

	const T_DIMSE_C_StoreRSP ResponseParameters = Response.msg.CStoreRSP;
	const T_DIMSE_C_StoreRQ  RequestParameters =  Request.msg.CStoreRQ;

	if ( 
		ResponseParameters.MessageIDBeingRespondedTo != 
		RequestParameters.MessageID
	) {
		throw OperationFailedException(
			QString( 
				"Response's Message ID is different than request's: %1 vs %2"
			)
			.arg( ResponseParameters.MessageIDBeingRespondedTo )
			.arg( RequestParameters.MessageID )
		);
	}

	if ( ResponseParameters.DataSetType != DIMSE_DATASET_NULL ) {
		qWarning( 
			"Non-conformant Store SCP detected: C-STORE response contains a "
			"dataset. Ignoring."
		);
	}

	if ( ResponseParameters.DimseStatus == STATUS_Success ) {
		return;
	}

	const quint16 Status = ResponseParameters.DimseStatus;

	char * message = 0;
	switch ( Status & 0xFF00 ) {
		case STATUS_STORE_Error_DataSetDoesNotMatchSOPClass :
			message = "Error: Data Set does not match SOP Class"; break;
		case STATUS_STORE_Refused_OutOfResources :
			message = "Refused: Out of Resources"; break;
		case STATUS_STORE_Refused_SOPClassNotSupported :
			message = "Refused: SOP Class not supported"; break;
	};

	if ( ( Status & 0xF000 ) == STATUS_STORE_Error_CannotUnderstand ) {
		message = "Error: Cannot understand";
	}

	if ( message ) {
		throw OperationFailedException( 
			QString( "Response status: `%1'." )
			.arg( message )
		);
	}

	switch ( Status ) {
		case STATUS_STORE_Warning_CoersionOfDataElements :
			message = "Coercion of Data Elements"; break;
		case STATUS_STORE_Warning_DataSetDoesNotMatchSOPClass :
			message = "Data Set does not match SOP Class"; break;
		case STATUS_STORE_Warning_ElementsDiscarded :
			message = "Elements discarded"; break;
	}

	if ( message ) {
		qWarning( "Response status: `%s'", message );
	}
	else {
		qWarning( "Unknown Response status: 0x%04X", Status );
	}
}


void ServiceUser::validateNActionResponse(
	const T_DIMSE_Message & Response,
	const T_DIMSE_Message & Request
) {
	Q_ASSERT( Response.CommandField == DIMSE_N_ACTION_RSP );

	const T_DIMSE_N_ActionRSP ResponseParameters = Response.msg.NActionRSP;
	const T_DIMSE_N_ActionRQ  RequestParameters =  Request.msg.NActionRQ;

	if ( 
		ResponseParameters.MessageIDBeingRespondedTo != 
		RequestParameters.MessageID
	) {
		throw OperationFailedException(
			QString( 
				"Response's Message ID is different than request's: %1 vs %2"
			)
			.arg( ResponseParameters.MessageIDBeingRespondedTo )
			.arg( RequestParameters.MessageID )
		);
	}

	const quint16 & Status = ResponseParameters.DimseStatus ;
	if ( Status == STATUS_Success ) {
		if ( 
			( ResponseParameters.opts & O_NACTION_ACTIONTYPEID ) &&
			ResponseParameters.ActionTypeID != RequestParameters.ActionTypeID
		) {
			qWarning( 
				"Non-conformant N-service-provider detected: "
				"N-ACTION request and response contain different Action Type IDs "
				"(0x%X vs. 0x%X)",
				RequestParameters.ActionTypeID,
				ResponseParameters.ActionTypeID
			);
		}
		if ( 
			( ResponseParameters.opts & O_NDELETE_AFFECTEDSOPCLASSUID ) &&
			::qstrcmp(
				ResponseParameters.AffectedSOPClassUID,
				RequestParameters.RequestedSOPClassUID
			) != 0
		) {
			qWarning( 
				"Non-conformant N-service-provider detected: "
				"Affected SOP Class UID in N-ACTION response differs from "
				"Requested SOP Class UID. Ignoring"
			);
		}
		if (
			( ResponseParameters.opts & O_NDELETE_AFFECTEDSOPINSTANCEUID ) &&
			::qstrcmp(
				ResponseParameters.AffectedSOPInstanceUID,
				RequestParameters.RequestedSOPInstanceUID
			) != 0
		) {
			qWarning( 
				"Non-conformant N-service-provider detected: "
				"Affected SOP Instance UID in N-ACTION response differs from "
				"Requested SOP Instance UID. Ignoring"
			);
		}
		return;
	}

	QString message;
	switch ( Status ) {
#define CASE(VALUE, MSG) case VALUE : message = "Response error status: `"MSG"'."; break

		// Statuses defined for the N-ACTION in PS3.7 10.1.4.1.10
		CASE(STATUS_N_ClassInstanceConflict, "Class interface conflict"); //  1
		CASE(STATUS_N_DuplicateInvocation,   "Duplicate invokation");     //  2
		CASE(STATUS_N_InvalidArgumentValue,  "Invalid argument value");   //  3
		CASE(STATUS_N_InvalidObjectInstance, "Invalid SOP Instance");     //  4
		CASE(STATUS_N_MistypedArgument,      "Mistyped argument");        //  5
		CASE(STATUS_N_NoSuchAction,          "No such action");           //  6
		CASE(STATUS_N_NoSuchArgument,        "No such argument");         //  7
		CASE(STATUS_N_NoSuchSOPClass,        "No such SOP Class");        //  8
		CASE(STATUS_N_NoSuchObjectInstance,  "No such SOP Instance");     //  9
		CASE(STATUS_N_ProcessingFailure,     "Processing failure");       // 10
		CASE(STATUS_N_ResourceLimitation,    "Resource limitation");      // 11
		CASE(STATUS_N_UnrecognizedOperation, "Unrecognized operation");   // 12

		// Statuses specific for the N-ACTION with Basic Film Box SOP Class
		// from PS3.4 4.2.2.4.2
		CASE(
			STATUS_N_PRINT_BSB_Fail_PrintQueueFull,
			"Unable to create Print Job SOP Instance; print queue is full"
		);
		CASE(
			STATUS_N_PRINT_BFS_BFB_Fail_ImageSize,
			"Image size is larger than Image Box size"
		);
		CASE(
			0xC613,
			"Combined print image size is larger than the Image Box size"
		);

		// Statuses specific for the N-ACTION with Basic Film Session SOP Class
		// from PS3.4 4.1.2.4.2
		CASE(
			STATUS_N_PRINT_BFS_Fail_NoFilmBox,
			"Film Session SOP Instance hierarchy does not contain Film Box "
			"SOP Instances"
		);
		CASE(
			STATUS_N_PRINT_BFS_Fail_PrintQueueFull,
			"Unable to create Print Job SOP Instance; print queue is full"
		);

#undef CASE
		default :
			// Warnings specific for the N-ACTION with Basic Film Box SOP Class
			// from PS3.4 4.2.2.4.2
			if ( Status == STATUS_N_PRINT_BFB_Warn_EmptyPage ) {
				qWarning(
					"Film Box SOP Instance hierarchy does not contain Image Box "
					"SOP Instances (empty page)"
				);
			}
			else if ( Status == 0xB604 ) {
				qWarning(
					"Image size is larger than image box size, the image has been "
					"demagnified"
				);
			}
			else if ( Status == 0xB609 ) {
				qWarning(
					"Image size is larger than the Image Box size. The Image has "
					"been cropped to fit"
				);
			}
			else if ( Status == 0xB60A ) {
				qWarning(
					"Image size or Combined Print Image size is larger than the Image "
					"Box size. Image or Combined Print Image has been decimated to "
					"fit"
				);
			}

			// Warnings specific for the N-ACTION with Basic Film Session SOP Class
			// from PS3.4 4.1.2.4.2
			else if ( Status == STATUS_N_PRINT_BFS_Warn_NoSessionPrinting ) {
				qWarning(
					"Film session printing (collation) is not supported"
				);
			}
			else if ( Status == STATUS_N_PRINT_BFS_Warn_EmptyPage ) {
				qWarning(
					"Film Session SOP Instance hierarchy does not contain Image Box "
					"SOP Instances (empty page)"
				);
			}

			// Any status that fall down here is non-standard. We can guess 
			// whether it's an error or warning by testing it's high nibble,
			// hoping that at least the general convention of starting error
			// codes with 0xC*** and warnings with 0xB*** has been preserved.
			else if ( Status & 0xC000 ) {
				message =
					QString( "Unrecognized failure status code: 0x%1" )
					.arg( Status, 4, 16, QChar( '0' ) )
				;

				// Break here to omit the return down below and throw an
				// exception
				break;
			}
			else if ( Status & 0xB000 ) {
				qWarning( "Unrecognized warning status code: 0x%04X. Ignoring", Status );
			}
			else {
				qWarning( "Unrecognized response status: 0x%04X. Ignoring", Status );
			}

			// The above statuses were warnings, ignore.
			return;
	};

	throw OperationFailedException( message );
}


void ServiceUser::validateNCreateResponse(
	const T_DIMSE_Message & Response,
	const T_DIMSE_Message & Request
) {
	Q_ASSERT( Response.CommandField == DIMSE_N_CREATE_RSP );

	const T_DIMSE_N_CreateRSP ResponseParameters = Response.msg.NCreateRSP;
	const T_DIMSE_N_CreateRQ  RequestParameters =  Request.msg.NCreateRQ;

	if ( 
		ResponseParameters.MessageIDBeingRespondedTo != 
		RequestParameters.MessageID
	) {
		throw OperationFailedException(
			QString( 
				"Response's Message ID is different than request's: %1 vs %2"
			)
			.arg( ResponseParameters.MessageIDBeingRespondedTo )
			.arg( RequestParameters.MessageID )
		);
	}

	const quint16 & Status = ResponseParameters.DimseStatus ;
	if ( Status == STATUS_Success ) {
		if ( 
			ResponseParameters.AffectedSOPInstanceUID[ 0 ] &&
			RequestParameters.AffectedSOPInstanceUID[ 0 ]
		) {
			qWarning( 
				"Non-conformant N-service-provider detected: "
				"both N-CREATE request and response contain the Affected SOP "
				"Instance UID. Ignoring"
			);
		}
		else if (
			ResponseParameters.AffectedSOPInstanceUID[ 0 ] == 0 &&
			RequestParameters.AffectedSOPInstanceUID[ 0 ] == 0
		) {
			throw OperationFailedException(
				"Non-conformant N-service-provider detected: "
				"both N-CREATE request and response contain empty Affected SOP "
				"Instance UID; that is, N-service-user was unable to retrieve "
				"a SOP Instance UID"
			);
		}
		return;
	}

	QString message;
	switch ( Status ) {
#define CASE( VALUE, MSG ) \
		case VALUE : message = "Response error status: `"MSG"'."; break

		// Statuses defined for the N-CREATE in PS3.7 10.1.5.1.6
		CASE( STATUS_N_ClassInstanceConflict, "Class interface conflict" ); //  1
		CASE( STATUS_N_DuplicateInvocation,   "Duplicate invokation" );     //  2
		CASE( STATUS_N_DuplicateSOPInstance,  "Duplicate SOP Instance" );   //  3
		CASE( STATUS_N_InvalidAttributeValue, "Invalid attribute value" );  //  4
		CASE( STATUS_N_InvalidObjectInstance, "Invalid SOP Instance" );     //  5
		CASE( STATUS_N_MissingAttribute,      "Missing attribute" );        //  6
		CASE( STATUS_N_MissingAttributeValue, "Missing attribute value" );  //  7
		CASE( STATUS_N_MistypedArgument,      "Mistyped argument" );        //  8
		CASE( STATUS_N_NoSuchAttribute,       "No such attribute" );        //  9
		CASE( STATUS_N_NoSuchSOPClass,        "No such SOP Class" );        // 10
		CASE( STATUS_N_NoSuchObjectInstance,  "No such SOP Instance" );     // 11		
		CASE( STATUS_N_ProcessingFailure,     "Processing failure" );       // 12
		CASE( STATUS_N_ResourceLimitation,    "Resource limitation" );      // 13
		CASE( STATUS_N_UnrecognizedOperation, "Unrecognized operation" );   // 14

		// Statuses specific for the N-CREATE with Basic Film Box SOP Class from PS3.4 4.2.2.1.2
		CASE(
			0xC616,
			"A new Film Box will not be created when a previous "
			"Film Box has not been printed"
		);

#undef CASE
		default :
			if ( Status == 0xB605 ) {
				qWarning(
					"Requested Min Density or Max Density are outside "
					"of printer's operating range. Ignoring"
				);
			}

			// Any status that fall down here is non-standard. We can guess 
			// whether it's an error or warning by testing it's high nibble,
			// hoping that at least the general convention of starting error
			// codes with 0xC*** and warnings with 0xB*** has been preserved.
			else if ( Status & 0xC000 ) {
				message =
					QString( "Unrecognized failure status code: 0x%1" )
					.arg( Status, 4, 16, QChar( '0' ) )
				;

				// Break here to omit the return down below and throw an
				// exception
				break;
			}
			else if ( Status & 0xB000 ) {
				qWarning( "Unrecognized warning status code: 0x%04X. Ignoring", Status );
			}
			else {
				qWarning( "Unrecognized response status: 0x%04X. Ignoring", Status );
			}

			// The above statuses were warnings, ignore.
			return;
	};

	throw OperationFailedException( message );
}


void ServiceUser::validateNDeleteResponse(
	const T_DIMSE_Message & Response,
	const T_DIMSE_Message & Request
) {
	Q_ASSERT( Response.CommandField == DIMSE_N_DELETE_RSP );

	const T_DIMSE_N_DeleteRSP ResponseParameters = Response.msg.NDeleteRSP;
	const T_DIMSE_N_DeleteRQ  RequestParameters =  Request.msg.NDeleteRQ;

	if ( 
		ResponseParameters.MessageIDBeingRespondedTo != 
		RequestParameters.MessageID
	) {
		throw OperationFailedException(
			QString( 
				"Response's Message ID is different than request's: %1 vs %2"
			)
			.arg( ResponseParameters.MessageIDBeingRespondedTo )
			.arg( RequestParameters.MessageID )
		);
	}

	const quint16 & Status = ResponseParameters.DimseStatus ;
	if ( Status == STATUS_Success ) {
		if ( 
			( ResponseParameters.opts & O_NDELETE_AFFECTEDSOPCLASSUID ) &&
			::qstrcmp(
				ResponseParameters.AffectedSOPClassUID,
				RequestParameters.RequestedSOPClassUID
			) != 0
		) {
			qWarning( 
				"Non-conformant N-service-provider detected: "
				"Affected SOP Class UID in N-DELETE response differs from "
				"Requested SOP Class UID. Ignoring"
			);
		}
		if (
			( ResponseParameters.opts & O_NDELETE_AFFECTEDSOPINSTANCEUID ) &&
			::qstrcmp(
				ResponseParameters.AffectedSOPInstanceUID,
				RequestParameters.RequestedSOPInstanceUID
			) != 0
		) {
			qWarning( 
				"Non-conformant N-service-provider detected: "
				"Affected SOP Instance UID in N-DELETE response differs from "
				"Requested SOP Instance UID. Ignoring"
			);
		}
		return;
	}

	QString message;
	switch ( Status ) {
#define CASE( VALUE, MSG ) \
		case VALUE : message = "Response error status: `"MSG"'."; break

		// Statuses defined for the N-DELETE in PS3.7 10.1.6.1.7
		CASE( STATUS_N_ClassInstanceConflict, "Class interface conflict" ); // 1
		CASE( STATUS_N_DuplicateInvocation,   "Duplicate invokation" );     // 2
		CASE( STATUS_N_InvalidObjectInstance, "Invalid SOP Instance" );     // 3
		CASE( STATUS_N_MistypedArgument,      "Mistyped argument" );        // 4
		CASE( STATUS_N_NoSuchSOPClass,        "No such SOP Class" );        // 5
		CASE( STATUS_N_NoSuchObjectInstance,  "No such SOP Instance" );     // 6
		CASE( STATUS_N_ProcessingFailure,     "Processing failure" );       // 7
		CASE( STATUS_N_ResourceLimitation,    "Resource limitation" );      // 8
		CASE( STATUS_N_UnrecognizedOperation, "Unrecognized operation" );   // 9

#undef CASE
		default :
			// Any status that fall down here is non-standard. We can guess 
			// whether it's an error or warning by testing it's high nibble,
			// hoping that at least the general convention of starting error
			// codes with 0xC*** and warnings with 0xB*** has been preserved.
			if ( Status & 0xC000 ) {
				message =
					QString( "Unrecognized failure status code: 0x%1" )
					.arg( Status, 4, 16, QChar( '0' ) )
				;

				// Break here to omit the return down below and throw an
				// exception
				break;
			}
			else if ( Status & 0xB000 ) {
				qWarning( "Unrecognized warning status code: 0x%04X. Ignoring", Status );
			}
			else {
				qWarning( "Unrecognized response status: 0x%04X. Ignoring", Status );
			}

			// The above statuses were warnings, ignore.
			return;
	};

	throw OperationFailedException( message );
}

void ServiceUser::validateNSetResponse(
	const T_DIMSE_Message & Response,
	const T_DIMSE_Message & Request
) {
	Q_ASSERT( Response.CommandField == DIMSE_N_SET_RSP );

	const T_DIMSE_N_SetRSP ResponseParameters = Response.msg.NSetRSP;
	const T_DIMSE_N_SetRQ  RequestParameters =  Request.msg.NSetRQ;

	if ( 
		ResponseParameters.MessageIDBeingRespondedTo != 
		RequestParameters.MessageID
	) {
		throw OperationFailedException(
			QString( 
				"Response's Message ID is different than request's: %1 vs %2"
			)
			.arg( ResponseParameters.MessageIDBeingRespondedTo )
			.arg( RequestParameters.MessageID )
		);
	}

	const quint16 & Status = ResponseParameters.DimseStatus;
	if ( Status == STATUS_Success ) {
		return;
	}

	QString message;
	switch ( Status ) {
#define CASE( VALUE, MSG ) \
		case VALUE : message = "Response error status: `"MSG"'."; break

		// Status for N-SET defined in PS3.7 10.1.3.1.9
		CASE( STATUS_N_ClassInstanceConflict, "Class interface conflict" ); //  1
		CASE( STATUS_N_DuplicateInvocation,   "Duplicate invokation" );     //  2
		CASE( STATUS_N_InvalidAttributeValue, "Invalid attribute value" );  //  3
		CASE( STATUS_N_MistypedArgument,      "Mistyped argument" );        //  4
		CASE( STATUS_N_InvalidObjectInstance, "Invalid SOP Instance" );     //  5
		CASE( STATUS_N_MissingAttributeValue, "Missing attribute value" );  //  6
		CASE( STATUS_N_NoSuchAttribute,       "No such attribute" );        //  7
		CASE( STATUS_N_NoSuchSOPClass,        "No such SOP Class" );        //  8
		CASE( STATUS_N_NoSuchObjectInstance,  "No such SOP Instance" );     //  9		
		CASE( STATUS_N_ProcessingFailure,     "Processing failure" );       // 10
		CASE( STATUS_N_ResourceLimitation,    "Resource limitation" );      // 11
		CASE( STATUS_N_UnrecognizedOperation, "Unrecognized operation" );   // 12

		// Statuses specific to N-SET of Basic Grayscale Image Box SOP Class defined in PS3.4 H.4.3.1.2.1.2
		CASE(
			0xC603,
			"Image is larger than image box size"
		);
		CASE(
			STATUS_N_PRINT_IB_Fail_InsufficientMemory,
			"Insufficient memory in printer to store the image"
		);
		CASE(
			0xC613,
			"Combined Print Image size is larger than the Image Box size"
		);
#undef CASE
		default :
			// Warning statuses specific to N-SET of Basic Grayscale Image Box SOP
			// Class, defined in PS3.4 H.4.3.1.2.1.2
			if ( Status == 0xB604 ) {
				qWarning(
					"Image size larger than image box size, "
					"the image has been demagnified. Ignoring"
				);
			}
			else if ( Status == 0xB605 ) {
				qWarning(
					"Requested Min Density or Max Density outside of printer's "
					"operating range. The printer will use its respective minimum "
					"or maximum density value instead. Ignoring"
				);
			}
			else if ( Status == 0xB609 ) {
				qWarning(
					"Image size is larger than the Image Box size. "
					"The image has been cropped to fit. Ignoring"
				);
			}
			else if ( Status == 0xB60A ) {
				qWarning(
					"Image size or Combined Print Image size is larger than the "
					"Image Box size. The Image or Combined Print Image has been "
					"decimated to fit. Ignoring"
				);
			}

			// Any status that falls down here is non-standard. We can guess 
			// whether it's an error or warning by testing it's high nibble,
			// hoping that at least the general convention of starting error
			// codes with 0xC*** and warnings with 0xB*** has been preserved.
			else if ( Status & 0xC000 ) {
				message =
					QString( "Unrecognized failure status code: 0x%1" )
					.arg( Status, 4, 16, QChar( '0' ) )
				;

				// Break here to omit the return down below and throw an
				// exception
				break;
			}
			else if ( Status & 0xB000 ) {
				qWarning(
					"Unrecognized warning status code: 0x%04X. Ignoring",
					Status
				);
			}
			else {
				qWarning(
					"Unrecognized response status: 0x%04X. Ignoring",
					Status
				);
			}

			// The above statuses were warnings, ignore.
			return;
	};

	throw OperationFailedException( message );
}


}; // Namespace DICOM ends here.
