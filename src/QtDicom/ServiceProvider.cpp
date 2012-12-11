/***************************************************************************
 *   Copyright (C) 2011 by Flux Inc.                                       *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#include "Exceptions.hpp"
#include "ServiceProvider.hpp"

#include <QtDicom/Association.hpp>

#include <QtCore/QDir>
#include <QtCore/QTemporaryFile>

#include <dcmtk/dcmdata/dcostrmf.h>

#include <dcmtk/dcmnet/assoc.h>
#include <dcmtk/dcmnet/dimse.h>


namespace Dicom {

ServiceProvider::ServiceProvider() :
	AbstractService()
{
	clearErrorStatus();
}


ServiceProvider::ServiceProvider( Association * association ) :
	AbstractService( association )
{
	clearErrorStatus();
}


ServiceProvider::~ServiceProvider() {
}


void ServiceProvider::handleCEcho(
	const T_DIMSE_C_EchoRQ & Request, unsigned char presentationContextId
) {
	try {

	if ( ! ( association() || association()->isEstablished() ) ) {
		throw OperationFailedException( "Invalid association." );
	}

	sendCEchoResponse( Request, presentationContextId );
	
	} // End of the try block.
	catch ( std::exception & e ) {
		raiseError( e.what() );
	}
	catch ( ... ) {
		raiseError( "Unknown exception occured." );
	}
}


Dataset ServiceProvider::handleCFind(
	Association * a,
	const T_DIMSE_C_FindRQ & Request,
	unsigned char id
) {
	Association * backup = association();
	setAssociation( a );
	const Dataset Result = handleCFind( Request, id );
	setAssociation( backup );

	return Result;
}


Dataset ServiceProvider::handleCFind(
	const T_DIMSE_C_FindRQ & Request,
	unsigned char presentationContextId
) {
	try {

	if ( ! ( association() || association()->isEstablished() ) ) {
		throw OperationFailedException( "Invalid association." );
	}

	return receiveDataset( presentationContextId );


	} // End of the try block.
	catch ( std::exception & e ) {
		raiseError( e.what() );
	}
	catch ( ... ) {
		raiseError( "Unknown exception occured." );
	}	

	return Dataset();
}


bool ServiceProvider::handleCStore(
	Association * a,
	const T_DIMSE_C_StoreRQ & Request,
	unsigned char id,
	const QString & Path
) {
	Association * backup = association();
	setAssociation( a );
	const bool Result = handleCStore( Request, id, Path );
	setAssociation( backup );

	return Result;
}


bool ServiceProvider::handleCStore(
	const T_DIMSE_C_StoreRQ & Request,
	unsigned char presentationContextId,
	const QString & Path
) {
	try {

	if ( ! ( association() || association()->isEstablished() ) ) {
		throw OperationFailedException( "Invalid association." );
	}

	int status = STATUS_Success;
	if ( ! Path.isEmpty() ) {
		DcmOutputFileStream * filestream = NULL;
		OFCondition result = DIMSE_createFilestream(
			Path.toUtf8().constData(),
			&Request, association()->tAscAssociation(),
			presentationContextId,
			0, &filestream
		);

		if ( result.good() ) {
			receiveDatasetInFile( presentationContextId, filestream );
		}
		else {
			raiseError(
				QString( "Failed to create a file stream: `%1'." )
				.arg( Path )
			);
			status = STATUS_STORE_Refused_OutOfResources;
		}
	}
	else {
		status = STATUS_STORE_Refused_OutOfResources;
	}

	sendCStoreResponse( status, Request, presentationContextId );
	
	return ! hasError();

	} // End of the try block.
	catch ( std::exception & e ) {
		raiseError( e.what() );
	}
	catch ( ... ) {
		raiseError( "Unknown exception occured." );
	}	

	return false;
}


Dataset ServiceProvider::handleCStore(
	const T_DIMSE_C_StoreRQ & Request, unsigned char presentationContextId
) {
	try {

	if ( ! ( association() || association()->isEstablished() ) ) {
		throw OperationFailedException( "Invalid association." );
	}

	Dataset dataSet;
	receiveDatasetInMemory( presentationContextId, &dataSet.dcmDataset() );

	sendCStoreResponse( STATUS_Success, Request, presentationContextId );
	
	return dataSet;

	} // End of the try block.
	catch ( std::exception & e ) {
		raiseError( e.what() );
	}
	catch ( ... ) {
		raiseError( "Unknown exception occured." );
	}	

	return Dataset();
}


void ServiceProvider::receiveDatasetInFile( 
	unsigned char & id, DcmOutputStream * stream
) {
	const OFCondition Result = DIMSE_receiveDataSetInFile(
		association()->tAscAssociation(),
		DIMSE_NONBLOCKING, 
		association()->connectionParameters().timeout(),
		&id, stream, 0, 0
	);

	delete stream;

	if ( Result.good() ) {
		qDebug( "Dataset stored in file." );
	}
	else {
		throw OperationFailedException(
			QString( 
				"Failed to store a dataset in a file. "
				"Internal error description:\n%1"
			)
			.arg( Result.text() )
		);
	}
}


void ServiceProvider::receiveDatasetInMemory(
	unsigned char & id, DcmDataset * dataSet
) {
	const OFCondition Result = DIMSE_receiveDataSetInMemory(
		association()->tAscAssociation(),
		DIMSE_NONBLOCKING,
		association()->connectionParameters().timeout(),
		&id, &dataSet, 0, 0
	);

	if ( Result.good() ) {
		qDebug( "Data Set successfully received." );
	}
	else {
		throw OperationFailedException(
			QString( 
				"Failed to receive a Data Set. "
				"Internal error description:\n%1"
			)
			.arg( Result.text() )
		);
	}
}


void ServiceProvider::sendCEchoResponse(
	const T_DIMSE_C_EchoRQ & Request, unsigned char Id
) {
	T_DIMSE_C_EchoRSP responseParams;
	bzero( ( char * )&responseParams, sizeof( responseParams ) );
	strcpy( responseParams.AffectedSOPClassUID,    Request.AffectedSOPClassUID );
	responseParams.DataSetType = DIMSE_DATASET_NULL;
	responseParams.DimseStatus = STATUS_Success;
	responseParams.MessageIDBeingRespondedTo = Request.MessageID;
    responseParams.opts = O_ECHO_AFFECTEDSOPCLASSUID;

	T_DIMSE_Message response;
	bzero( ( char * )&response, sizeof( response ) );
	response.CommandField = DIMSE_C_ECHO_RSP;
	response.msg.CEchoRSP = responseParams;

	sendCommand( response, Id );
}


void ServiceProvider::sendCStoreResponse(
	int status, const T_DIMSE_C_StoreRQ & Request, unsigned char Id
) {
	T_DIMSE_C_StoreRSP responseParams;
	bzero( ( char * )&responseParams, sizeof( responseParams ) );
	strcpy( responseParams.AffectedSOPClassUID,    Request.AffectedSOPClassUID );
    strcpy( responseParams.AffectedSOPInstanceUID, Request.AffectedSOPInstanceUID );
	responseParams.DataSetType = DIMSE_DATASET_NULL;
	responseParams.DimseStatus = status;
	responseParams.MessageIDBeingRespondedTo = Request.MessageID;
    responseParams.opts = 
		( O_STORE_AFFECTEDSOPCLASSUID | O_STORE_AFFECTEDSOPINSTANCEUID )
	;
    if ( Request.opts & O_STORE_RQ_BLANK_PADDING ) {
		responseParams.opts |= O_STORE_RSP_BLANK_PADDING;
	}
    if ( dcmPeerRequiresExactUIDCopy.get() ) {
		responseParams.opts |= O_STORE_PEER_REQUIRES_EXACT_UID_COPY;
	}

	T_DIMSE_Message response;
	bzero( ( char * )&response, sizeof( response ) );
	response.CommandField = DIMSE_C_STORE_RSP;
	response.msg.CStoreRSP = responseParams;

	sendCommand( response, Id );
}


}; // Namespace DICOM ends here.
