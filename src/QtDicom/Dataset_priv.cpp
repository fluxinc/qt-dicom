/***************************************************************************
 *   Copyright © 2011-2013 by Flux Inc.                                    *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#include "Dataset_priv.hpp"

#include <dcmtk/config/osconfig.h>

#include <dcmtk/dcmdata/dcdatset.h>


namespace Dicom {

Dataset_priv::Dataset_priv() :
	dcmDataSet_( new DcmDataset() )
{
}


Dataset_priv::Dataset_priv( const Dataset_priv & Other ) :
	dcmDataSet_( new DcmDataset( Other.dcmDataSet() ) )
{
}


Dataset_priv::~Dataset_priv() {
	Q_ASSERT( dcmDataSet_ != nullptr );

	delete dcmDataSet_;
	dcmDataSet_ = nullptr;
}


Dataset_priv & Dataset_priv::operator = ( const Dataset_priv & Other ) {
	if ( this != &Other ) {
		Q_ASSERT( dcmDataSet_ != nullptr );

		delete dcmDataSet_;
		dcmDataSet_ = new DcmDataset( Other.dcmDataSet() );
	}

	return * this;
}


DcmDataset & Dataset_priv::dcmDataSet() const {
	Q_ASSERT( dcmDataSet_ != nullptr );

	return *dcmDataSet_;
}

}; // Namespace DICOM ends here.
