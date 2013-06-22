/***************************************************************************
 *   Copyright © 2011-2013 by Flux Inc.                                    *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#include "DatasetConstIterator.hpp"

#include <dcmtk/dcmdata/dcdatset.h>
#include <dcmtk/dcmdata/dcitem.h>
#include <dcmtk/dcmdata/dcstack.h>

namespace Dicom {


Dataset::ConstIterator::ConstIterator( const ConstIterator & Other ) :
	baseItem_( Other.baseItem_ ),
	position_( Other.position_ ),
	stack_( new DcmStack( Other.stack() ) )
{
	this->operator ++();
}


Dataset::ConstIterator::ConstIterator( const Dataset & Dset, bool end ) :
	baseItem_( Dset.dcmDataset() ),
	position_( end ? End : Start ),
	stack_( new DcmStack() )
{
	if ( ! end ) {
		this->operator ++();
	}
}

Dataset::ConstIterator::ConstIterator( const DcmItem & item, bool end ) :
	baseItem_( item ),
	position_( end ? End : Start ),
	stack_( new DcmStack() )
{
	if ( ! end ) {
		this->operator ++();
	}
}


Dataset::ConstIterator::~ConstIterator() {
	Q_ASSERT( stack_ != nullptr );

	delete stack_;
	stack_ = nullptr;
}


bool Dataset::ConstIterator::operator !=( const ConstIterator & Other ) const {
	return ! ( this->operator==( Other ) );
}


DcmObject & Dataset::ConstIterator::operator *() const {
	Q_ASSERT( ! atEnd() );
	return *( stack().top() );
}


Dataset::ConstIterator & Dataset::ConstIterator::operator ++() {
	if ( atEnd() ) {
		return *this;
	}

	DcmObject * currentObject = 0;

	const OFCondition Result = baseItem().nextObject( stack(), true );

	if ( Result.good() ) {
		currentObject = stack().top();

		if ( currentObject->isLeaf() ) {
			setPosition( Element );
		}
		else if ( currentObject->ident() == EVR_SQ ) {
			setPosition( Sequence );
		}
		else {
			Q_ASSERT( currentObject->ident() == EVR_item );
			setPosition( Item );
		}
	}
	else {
		setPosition( End );
		stack().clear();
	}

	return *this;
}


DcmObject * Dataset::ConstIterator::operator->() const {
	return &( this->operator*() );
}


bool Dataset::ConstIterator::operator == ( const ConstIterator & Other ) const {
	return 
		&baseItem_ == &Other.baseItem_ &&
		stack() == Other.stack() &&
		position() == Other.position()
	;
}


bool Dataset::ConstIterator::atElement() const {
	return position() == Element;
}


bool Dataset::ConstIterator::atEnd() const {
	return position() == End;
}


bool Dataset::ConstIterator::atItem() const {
	return position() == Item;
}


bool Dataset::ConstIterator::atSequence() const {
	return position() == Sequence;
}


DcmItem & Dataset::ConstIterator::baseItem() const {
	return *const_cast< DcmItem * >( &baseItem_ );
}


DcmElement & Dataset::ConstIterator::element() const {
	Q_ASSERT( atElement() );

	return *reinterpret_cast< DcmElement * >( stack().top() );
}


DcmItem & Dataset::ConstIterator::item() const {
	Q_ASSERT( atItem() );

	return *reinterpret_cast< DcmItem * >( stack().top() );
}


int Dataset::ConstIterator::level() const {
	Q_ASSERT( ! atEnd() );
	if ( ! atEnd() ) {
		return stack().card();
	}
	else {
		return -1;
	}
}


Dataset::ConstIterator::Position Dataset::ConstIterator::position() const {
	return position_;
}


DcmSequenceOfItems & Dataset::ConstIterator::sequence() const {
	Q_ASSERT( atSequence() );

	return *reinterpret_cast< DcmSequenceOfItems * >( stack().top() );
}


void Dataset::ConstIterator::setPosition( Position position ) {
	position_ = position;
}


const DcmStack & Dataset::ConstIterator::stack() const {
	Q_ASSERT( stack_ != nullptr );

	return *stack_;
}


DcmStack & Dataset::ConstIterator::stack() {
	Q_ASSERT( stack_ != nullptr );

	return *stack_;
}

} // Namespace DICOM ends here.
