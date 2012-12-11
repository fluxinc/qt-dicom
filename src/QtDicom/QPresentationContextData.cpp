/***************************************************************************
 *   Copyright © 2012 by Flux Inc.                                         *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#include "QPresentationContextData.hpp"


QPresentationContextData::QPresentationContextData() :
	acceptedTransferSyntaxPosition_( -1 )
{
}


QPresentationContextData::QPresentationContextData( 
	const QPresentationContextData & Other
) :
	QSharedData( Other ),
	abstractSyntax_( Other.abstractSyntax_ ),
	acceptedTransferSyntaxPosition_( Other.acceptedTransferSyntaxPosition_ ),
	transferSyntaxes_( Other.transferSyntaxes_ )
{
}


QPresentationContextData::~QPresentationContextData() {
}
