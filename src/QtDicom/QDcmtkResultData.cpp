/***************************************************************************
 *   Copyright © 2013 by Flux Inc.                                         *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#include "QDcmtkResult.hpp"

#include <dcmtk/ofstd/ofcond.h>


QDcmtkResult::Data::Data() :
	status_( new OFCondition() )
{
}


QDcmtkResult::Data::Data( int status, const char * text ) :
	status_( new OFCondition( makeOFCondition( 1024, 1, OF_failure, text ) ) )
{
}

	
QDcmtkResult::Data::Data( const OFCondition & Status ) :
	status_( new OFCondition( Status ) )
{
}


QDcmtkResult::Data::Data( const Data & Other ) :
	status_( new OFCondition( Other.ofCondition() ) )
{
}


QDcmtkResult::Data::~Data() {
	delete status_;
	status_ = NULL;
}


QDcmtkResult::Data & QDcmtkResult::Data::operator = ( const Data & Other ) {
	if ( &Other != this ) {
		delete status_;
		status_ = new OFCondition( Other.ofCondition() );
	}

	return *this;
}
