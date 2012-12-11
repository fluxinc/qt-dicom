/***************************************************************************
 *   Copyright (C) 2011 by Flux Inc.                                       *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#ifndef DICOM_MOVESCU_HPP
#define DICOM_MOVESCU_HPP

#include "QtDicom/Globals.hpp"
#include "QtDicom/ServiceUser.hpp"


namespace Dicom {

class ConnectionParameters;
class Dataset;

class QDICOM_DLLSPEC MoveScu : public ServiceUser {
	public :
		MoveScu();
		~MoveScu();

		/**
		 * Queries a DICOM AE using the parameters.
		 */
		int move(
			const ConnectionParameters & parameters,
			const char * abstractSyntax,
			const Dataset & dataset,
			const QString & Ae,
			UidList * failedSopInstances = 0,
			int * warned = 0
		);
};

}; // Namespace DICOM ends here.


#endif
