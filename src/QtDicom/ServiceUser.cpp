/***************************************************************************
 *   Copyright (C) 2011 by Flux Inc.                                       *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#include "Exceptions.hpp"
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

	T_DIMSE_C_EchoRQ requestParameters;
	bzero( ( char * )& requestParameters, sizeof( requestParameters ) );
	strcpy( requestParameters.AffectedSOPClassUID, UID_VerificationSOPClass );
	requestParameters.DataSetType = DIMSE_DATASET_NULL;
	requestParameters.MessageID = association()->nextMessageId();

	T_DIMSE_Message request;
	bzero( ( char * )& request, sizeof( request ) );
	request.CommandField = DIMSE_C_ECHO_RQ;
	request.msg.CEchoRQ = requestParameters;
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

	T_DIMSE_C_FindRQ requestParameters;
	bzero( ( char * )& requestParameters, sizeof( requestParameters ) );
	strcpy( requestParameters.AffectedSOPClassUID, SopClass );
	requestParameters.DataSetType = DIMSE_DATASET_PRESENT;
	requestParameters.MessageID = association()->nextMessageId();
	requestParameters.Priority = DIMSE_PRIORITY_HIGH;

	T_DIMSE_Message request;
	bzero( ( char * )& request, sizeof( request ) );
	request.CommandField = DIMSE_C_FIND_RQ;
	request.msg.CFindRQ = requestParameters;

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

	T_DIMSE_C_MoveRQ requestParameters;
	bzero( ( char * )& requestParameters, sizeof( requestParameters ) );
	strcpy( requestParameters.AffectedSOPClassUID, SopClass );
	requestParameters.DataSetType = DIMSE_DATASET_PRESENT;
	requestParameters.MessageID = association()->nextMessageId();
	strcpy( requestParameters.MoveDestination, DestinationAe.toAscii() );
	requestParameters.Priority = DIMSE_PRIORITY_HIGH;

	T_DIMSE_Message request;
	bzero( ( char * )& request, sizeof( request ) );
	request.CommandField = DIMSE_C_MOVE_RQ;
	request.msg.CMoveRQ = requestParameters;

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

	DIC_UI SopClass; DIC_UI SopInstance;
	if ( DU_findSOPClassAndInstanceInDataSet( 
		&Dataset.dcmDataset(), SopClass, SopInstance
	) ) {
		qt_noop();
	}
	else {
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
			.arg( SopClass )
			.arg( TransferSyntax.toString() )
		);
	}

	T_DIMSE_C_StoreRQ requestParameters;
	bzero( ( char * )& requestParameters, sizeof( requestParameters ) );
	strcpy( requestParameters.AffectedSOPClassUID, SopClass );
	strcpy( requestParameters.AffectedSOPInstanceUID, SopInstance );
	requestParameters.DataSetType = DIMSE_DATASET_PRESENT;
	requestParameters.MessageID = association()->nextMessageId();
	if ( ! MoveAe.isEmpty() ) {
		const QByteArray Ae = MoveAe.toAscii();
		strcpy( 
			requestParameters.MoveOriginatorApplicationEntityTitle,
			Ae.constData()
		);
		requestParameters.MoveOriginatorID = ( quint16 )moveId;
	}
	requestParameters.Priority = DIMSE_PRIORITY_HIGH;

	T_DIMSE_Message request;
	bzero( ( char * )& request, sizeof( request ) );
	request.CommandField = DIMSE_C_STORE_RQ;
	request.msg.CStoreRQ = requestParameters;
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


QByteArray ServiceUser::nCreate( 
	const char * SopClass,
	const Dataset & Attributes,
	Dataset * affectedAttrbutes,
	quint16 * status
) {
	return nCreate( SopClass, 0, Attributes, affectedAttrbutes, status );
}


QByteArray ServiceUser::nCreate(
	const char * SopClass, 
	const char * SopInstance, 
	const Dataset & Attributes,
	Dataset * affectedAttributes,
	quint16 * status
) {
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

	T_DIMSE_N_CreateRQ requestParameters;
	bzero( ( char * )& requestParameters, sizeof( requestParameters ) );

	Q_ASSERT( SopClass );
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

	T_DIMSE_Message request;
	bzero( ( char * )& request, sizeof( request ) );
	request.CommandField = DIMSE_N_CREATE_RQ;
	request.msg.NCreateRQ = requestParameters;
	sendCommand( request, Attributes, presentationContextId );

	const T_DIMSE_Message Response = receiveCommand( 
		DIMSE_N_CREATE_RSP, presentationContextId
	);

	validateNCreateResponse( Response, request );

	const QByteArray AffectedSopInstance = 
		Response.msg.NCreateRSP.AffectedSOPInstanceUID
	;
	if ( status ) {
		*status = Response.msg.NCreateRSP.DimseStatus;
	}

	if ( Response.msg.NCreateRSP.DataSetType == DIMSE_DATASET_PRESENT ) {
		if ( affectedAttributes ) {
			*affectedAttributes = receiveDataset( presentationContextId );
		}
		else {
			ignoreDataset();
		}
	}

	return AffectedSopInstance;

	} // Try block ends here.
	catch ( std::exception & e ) {
		raiseError( e.what() );
	}
	catch ( ... ) {
		raiseError( "Unknown exception occured." );
	}

	return QByteArray();
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

	T_DIMSE_N_SetRQ requestParameters;
	bzero( ( char * )& requestParameters, sizeof( requestParameters ) );
	strcpy( requestParameters.RequestedSOPClassUID,    SopClass );
	strcpy( requestParameters.RequestedSOPInstanceUID, SopInstance );
	requestParameters.DataSetType = Attributes.isEmpty() ? 
		DIMSE_DATASET_NULL : DIMSE_DATASET_PRESENT
	;
	requestParameters.MessageID = association()->nextMessageId();

	T_DIMSE_Message request;
	bzero( ( char * )& request, sizeof( request ) );
	request.CommandField = DIMSE_N_SET_RQ;
	request.msg.NSetRQ = requestParameters;
	sendCommand( request, Attributes, presentationContextId );

	const T_DIMSE_Message Response = receiveCommand( 
		DIMSE_N_SET_RSP, presentationContextId
	);

	validateNSetResponse( Response, request );

	const QByteArray AffectedSopInstance = 
		Response.msg.NSetRSP.AffectedSOPInstanceUID
	;

	if ( status ) {
		*status = Response.msg.NSetRSP.DimseStatus;
	}

	if ( Response.msg.NSetRSP.DataSetType == DIMSE_DATASET_PRESENT ) {
		if ( modifiedAttrbutes ) {
			*modifiedAttrbutes = receiveDataset( presentationContextId );
		}
		else {
			ignoreDataset();
		}
	}

	return AffectedSopInstance;

	} // Try block ends here.
	catch ( std::exception & e ) {
		raiseError( e.what() );
	}
	catch ( ... ) {
		raiseError( "Unknown exception occured." );
	}

	return QByteArray();
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
			.arg( ResponseParameters.DimseStatus, 16, 6, QChar( '0' ) )
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

	if ( ResponseParameters.DimseStatus == STATUS_Success ) {
		if ( 
			ResponseParameters.AffectedSOPInstanceUID[ 0 ] &&
			RequestParameters.AffectedSOPInstanceUID[ 0 ]
		) {
			qWarning( 
				"Non-conformant N-service-provider detected: "
				"both N-CREATE request and response contain the Affected SOP "
				"Instance UID. Ignoring."
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
				"a SOP Instance UID."
			);
		}
		return;
	}

	QString message;
	switch ( ResponseParameters.DimseStatus ) {
#define CASE( VALUE, MSG ) \
		case VALUE : message = "Response error status: `"MSG"'."; break
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
#undef CASE
		default :
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

	if ( ResponseParameters.DimseStatus == STATUS_Success ) {
		return;
	}

	QString message;
	switch ( ResponseParameters.DimseStatus ) {
#define CASE( VALUE, MSG ) \
		case VALUE : message = "Response error status: `"MSG"'."; break
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
#undef CASE
		default :
			return;
	};

	throw OperationFailedException( message );
}


}; // Namespace DICOM ends here.
