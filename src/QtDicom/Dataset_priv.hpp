/***************************************************************************
 *   Copyright (C) 2011 by Flux Inc.                                       *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#ifndef DICOM_DATASET_PRIV_HPP
#define DICOM_DATASET_PRIV_HPP

#include <QtCore/QSharedData>

#include <dcmtk/config/osconfig.h>

#include <dcmtk/dcmdata/dcdatset.h>

namespace Dicom {

class Dataset;

class Dataset_priv : public QSharedData {
	friend class Dataset;

	public :
		Dataset_priv();
		Dataset_priv( const Dataset_priv & other );
		~Dataset_priv();
		Dataset_priv & operator = ( const Dataset_priv & other );

	private :
		mutable DcmDataset dataset_;

};

}; // Namesapce DICOM ends here.

#endif
