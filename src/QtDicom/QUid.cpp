/***************************************************************************
 *   Copyright © 2012 by Flux Inc.                                         *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#include "QUid.hpp"

#include <QtCore/QString>


QUid::QUid() {
}


QUid::QUid( const char * Uid ) :
	QByteArray( QByteArray( Uid ).trimmed() )
{
}


QUid::QUid( const QByteArray & Uid ) :
	QByteArray( Uid )
{
}


QUid::QUid( const QString & Uid ) :
	QByteArray( Uid.toAscii().trimmed() )
{
}


QUid::QUid( const QUid & Uid ) :
	QByteArray( Uid )
{
}


QUid::~QUid() {
}


QUid & QUid::operator = ( const char * UID ) {
	QByteArray::operator = ( UID );

	return *this;
}


bool QUid::operator == ( const QUid & UID ) const {
	return qstrcmp( UID, this->operator const char *() ) == 0;
}


bool QUid::operator != ( const QUid & UID ) const {
	return qstrcmp( UID, this->operator const char *() ) != 0;
}


QUid::operator const char * () const {
	return QByteArray::operator const char *();
}
