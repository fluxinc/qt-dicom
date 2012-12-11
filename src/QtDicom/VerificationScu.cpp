/***************************************************************************
 *   Copyright (C) 2011 by Flux Inc.                                       *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#include "ConnectionParameters.hpp"
#include "RequestorAssociation.hpp"
#include "VerificationScu.hpp"

#include <dcmtk/config/osconfig.h>

#include <dcmtk/dcmdata/dcuid.h>

namespace Dicom {


VerificationScu::VerificationScu() :
	ServiceUser()
{
}


VerificationScu::~VerificationScu() {
}


const UidList & VerificationScu::sopClasses() {
	static const UidList TheList( UID_VerificationSOPClass );

	return TheList;
}


bool VerificationScu::verify( 
	Association * a, const ConnectionParameters & Parameters
) {
	Association * backup = association();
	setAssociation( a );
	const bool Result = verify( Parameters );
	setAssociation( backup );

	return Result;
}


bool VerificationScu::verify( const ConnectionParameters & Parameters ) {
	RequestorAssociation a( Parameters );
	setAssociation( &a );

	a.request( Parameters, sopClasses() );
	if ( a.isEstablished() ) {
		bool result = cEcho();
		a.release();

		return result;
	}
	else {
		return false;
	}
}

}; // Namespace DICOM ends here.
