/***************************************************************************
 *   Copyright (C) 2011 by Flux Inc.                                       *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#include "Dataset_priv.hpp"


namespace Dicom {

Dataset_priv::Dataset_priv() {
}


Dataset_priv::Dataset_priv( const Dataset_priv & Other ) :
	dataset_( Other.dataset_ )
{
}


Dataset_priv::~Dataset_priv() {
}


Dataset_priv & Dataset_priv::operator = ( const Dataset_priv & Other ) {
	if ( this != &Other ) {
		dataset_ = Other.dataset_;
	}

	return * this;
}


}; // Namespace DICOM ends here.
