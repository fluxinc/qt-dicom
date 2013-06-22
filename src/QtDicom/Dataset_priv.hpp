/***************************************************************************
 *   Copyright © 2011-2013 by Flux Inc.                                    *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#ifndef DICOM_DATASET_PRIV_HPP
#define DICOM_DATASET_PRIV_HPP

#include <QtCore/QSharedData>

class DcmDataset;

namespace Dicom {

class Dataset;

class Dataset_priv : public QSharedData {
	public :
		Dataset_priv();
		Dataset_priv( const Dataset_priv & other );
		~Dataset_priv();
		Dataset_priv & operator = ( const Dataset_priv & other );

		DcmDataset & dcmDataSet() const;

	private :
		mutable DcmDataset * dcmDataSet_;

};

}; // Namesapce DICOM ends here.

#endif
