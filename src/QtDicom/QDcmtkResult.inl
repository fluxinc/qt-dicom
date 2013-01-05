/***************************************************************************
 *   Copyright © 2013 by Flux Inc.                                         *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#include <QtCore/QMetaType>


QDcmtkResult::QDcmtkResult() :
	data_( new Data() )
{
}


QDcmtkResult::QDcmtkResult( int status, const char * text ) :
	data_( new Data( status, text ) )
{
}


QDcmtkResult::QDcmtkResult( const OFCondition & status ) :
	data_( new Data( status ) )
{
}


QDcmtkResult::QDcmtkResult( const QDcmtkResult & Other ) :
	data_( Other.data_ )
{
}


QDcmtkResult::~QDcmtkResult() {
}


QDcmtkResult::operator const OFCondition & () const {
	return ofCondition();
}


QDcmtkResult::operator OFCondition & () {
	return ofCondition();
}


const OFCondition & QDcmtkResult::ofCondition() const {
	return data_->ofCondition();
}


OFCondition & QDcmtkResult::ofCondition() {
	return data_->ofCondition();
}


const OFCondition & QDcmtkResult::Data::ofCondition() const {
	Q_ASSERT( status_ != NULL );

	return *status_;
}


OFCondition & QDcmtkResult::Data::ofCondition() {
	Q_ASSERT( status_ != NULL );

	return *status_;
}
