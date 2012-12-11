/***************************************************************************
 *   Copyright (C) 2011 by Flux Inc.                                       *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#include "ServerAssociation.hpp"
#include "ServerAssociation.moc.inl"

#include <dcmtk/dcmnet/assoc.h>


namespace Dicom {

ServerAssociation::ServerAssociation( 
	T_ASC_Network * external,
	const ConnectionParameters & Parameters, QObject * parent
) :
	AcceptorAssociation( Parameters, parent ),
	externalTAscNetwork_( external )
{
	Q_ASSERT( external );
	Q_ASSERT( Parameters.isValid() );
}


ServerAssociation::~ServerAssociation() {
}


T_ASC_Network *& ServerAssociation::externalTAscNetwork() const {
	return externalTAscNetwork_;
}


bool ServerAssociation::isPending() const {
	if ( ! ( externalTAscNetwork() ) ) {
		return false;
	}

	return ASC_associationWaiting( externalTAscNetwork(), 0 );
}


bool ServerAssociation::receive( bool * timedOut ) {
	if ( ! externalTAscNetwork() ) {
		raiseError( "Invalid network." );
		return false;
	}

	T_ASC_Network * backup = tAscNetwork();
	tAscNetwork() = externalTAscNetwork();
	const bool Result = AcceptorAssociation::receive( timedOut );
	tAscNetwork() = backup;

	return Result;
}

}; // Namespace DICOM ends here.
