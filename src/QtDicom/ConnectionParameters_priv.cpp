/***************************************************************************
 *   Copyright (C) 2011 by Flux Inc.                                       *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#include "ConnectionParameters_priv.hpp"


namespace Dicom {


ConnectionParameters_priv::ConnectionParameters_priv() :
	hostAddress_(),
	maxPdu_( 16384 ),
	myAeTitle_( "FLUX" ),
	peerAeTitle_(),
	port_( 0 ),
	timeout_( 10 ),
	type_( 0 ) // Client
{
}


ConnectionParameters_priv::ConnectionParameters_priv( 
	const ConnectionParameters_priv & Other 
) :
	hostAddress_( Other.hostAddress_ ),
	maxPdu_( Other.maxPdu_ ),
	myAeTitle_( Other.myAeTitle_ ),
	peerAeTitle_( Other.peerAeTitle_ ),
	port_( Other.port_ ),
	timeout_( Other.timeout_ ),
	type_( Other.type_ )
{
}


ConnectionParameters_priv::~ConnectionParameters_priv() {
}


ConnectionParameters_priv & ConnectionParameters_priv::operator = (
	const ConnectionParameters_priv & Other 
) {
	if ( this != &Other ) {
		hostAddress_ = Other.hostAddress_;
		maxPdu_ = Other.maxPdu_;
		myAeTitle_ = Other.myAeTitle_;
		peerAeTitle_ = Other.peerAeTitle_;
		port_ = Other.port_;
		timeout_ = Other.timeout_;
		type_ = Other.type_;
	}

	return * this;
}

}; // Namespace DICOM end.

