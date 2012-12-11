/***************************************************************************
 *   Copyright (C) 2011 by Flux Inc.                                       *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#ifndef QDICOM_DATASET_CONSTITERATOR_HPP
#define QDICOM_DATASET_CONSTITERATOR_HPP

#include "QtDicom/Dataset.hpp"
#include "QtDicom/Globals.hpp"

#include <dcmtk/dcmdata/dcstack.h>

class DcmElement;
class DcmItem;
class DcmSequenceOfItems;

namespace Dicom {

class QDICOM_DLLSPEC Dataset::ConstIterator {
	public :
		ConstIterator( const ConstIterator & other );
		ConstIterator( const Dataset & dataset, bool atEnd = false );
		ConstIterator( const DcmItem & item, bool atEnd = false );
		bool operator !=( const ConstIterator & other ) const;
		DcmObject & operator *() const;
		ConstIterator & operator ++();
		DcmObject * operator-> () const;
		bool operator == ( const ConstIterator & other ) const;
		bool atEnd() const;
		bool atElement() const;
		bool atItem() const;
		bool atSequence() const;

		DcmElement & element() const;
		DcmItem & item() const;
		int level() const;
		DcmSequenceOfItems & sequence() const;

	private :
		enum Position {
			Start = 0,
			Element = 1,
			Sequence = 2,
			Item = 3,
			End = 4
		};

	private :
		Position position_;
		Position position() const;
		void setPosition( Position position );

		DcmItem & baseItem() const;
		const DcmItem & baseItem_;

		const DcmStack & stack() const;
		DcmStack & stack();
		DcmStack stack_;
};

}; // Namespace DICOM ends here.

#endif
