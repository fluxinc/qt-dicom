/***************************************************************************
 *   Copyright (C) 2011 by Flux Inc.                                       *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#ifndef DICOM_SERVICEPROVIDER_HPP
#define DICOM_SERVICEPROVIDER_HPP

#include "QtDicom/AbstractService.hpp"
#include "QtDicom/Dataset.hpp"
#include "QtDicom/Globals.hpp"

class DcmOutputStream;
class QDir;

struct T_DIMSE_C_EchoRQ;
struct T_DIMSE_C_FindRQ;
struct T_DIMSE_C_StoreRQ;

namespace Dicom {

class Association;

class QDICOM_DLLSPEC ServiceProvider : protected AbstractService {
	public :
		ServiceProvider();
		ServiceProvider( Association * association );
		virtual ~ServiceProvider();

		/**
		 * Handles a C-ECHO request by sending a C-ECHO response.
		 */
		void handleCEcho(
			const T_DIMSE_C_EchoRQ & Request,
			unsigned char ID
		);

		Dataset handleCFind(
			Association * association,
			const T_DIMSE_C_FindRQ & Request,
			unsigned char ID
		);
		Dataset handleCFind(
			const T_DIMSE_C_FindRQ & Request,
			unsigned char ID
		);


		/**
		 * Attempts to store incoming Data Set in file specified by the \a path.
		 */
		bool handleCStore(
			Association * association,
			const T_DIMSE_C_StoreRQ & Request,
			unsigned char ID,
			const QString & path
		);
		bool handleCStore(
			const T_DIMSE_C_StoreRQ & Request,
			unsigned char ID,
			const QString & path
		);

		/**
		 * Saves incoming Data Set in the \ref Dataset structure.
		 */
		Dataset handleCStore(
			const T_DIMSE_C_StoreRQ & Request,
			unsigned char ID
		);

	private :
		void receiveDatasetInFile( unsigned char & ID, DcmOutputStream * stream );
		void receiveDatasetInMemory( unsigned char & ID, DcmDataset * dataset );

		void sendCEchoResponse(
			const T_DIMSE_C_EchoRQ & Request,
			unsigned char ID
		);
		void sendCStoreResponse(
			int status,
			const T_DIMSE_C_StoreRQ & Request,
			unsigned char ID
		);
};

}; // Namespace DICOM ends here.



#endif