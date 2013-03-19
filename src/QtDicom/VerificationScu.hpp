/***************************************************************************
 *   Copyright (C) 2011 by Flux Inc.                                       *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/


#ifndef DICOM_VERIFICATIONSCU_HPP
#define DICOM_VERIFICATIONSCU_HPP

#include "QtDicom/Globals.hpp"
#include "QtDicom/ServiceUser.hpp"
#include "QtDicom/UidList.hpp"

namespace Dicom {

class ConnectionParameters;

class QDICOM_DLLSPEC VerificationScu : public ServiceUser {
	public :
		VerificationScu();
		~VerificationScu();

		bool verify( Association * association, const ConnectionParameters & params );
		bool verify( const ConnectionParameters & params );
};

}; // Namespace DICOM ends here.


#endif
