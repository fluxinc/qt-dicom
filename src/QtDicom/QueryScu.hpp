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

class QDICOM_DLLSPEC QueryScu : public ServiceUser {
	public :
		QueryScu();
		~QueryScu();

		/**
		 * Queries a DICOM AE using the parameters.
		 */
		QList< Dataset > query(
			const ConnectionParameters & parameters,
			const char * abstractSyntax,
			const Dataset & dataset
		);
};

}; // Namespace DICOM ends here.


#endif
