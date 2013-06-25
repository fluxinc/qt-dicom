/***************************************************************************
 *   Copyright © 2013 by Flux Inc.                                         *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#include "QDicomTag.hpp"

#include <QtCore/QHash>
#include <QtCore/QVector>

#include <QtFlux/QInitializeOnce>


#include <dcmtk/dcmdata/dctag.h>


static const QString & cachedKeyword( quint16, quint16 );
static QMutex & cacheLock();
static const QString & cacheKeyword(
	QHash< quint16, QVector< QString * > > &, quint16, quint16
);
static QString keywordFromDcmTagName( const char * );


QDicomTag::QDicomTag() :
	value_( 0 )
{
}


QDicomTag::QDicomTag( const Id & Id ) :
	value_( Id )
{
}


QDicomTag::~QDicomTag() {
}


QDicomTag::operator quint32() const {
	return value_;
}


quint16 QDicomTag::element() const {
	return static_cast< quint16 >( value_ );
}


quint16 QDicomTag::group() const {
	return static_cast< quint16 > ( value_ >> 16 );
}


bool QDicomTag::isNull() const {
	return value_ == 0;
}


bool QDicomTag::isValid() const {
	return value_ != 0;
}


const QString & QDicomTag::keyword() const {
	return cachedKeyword( group(), element() );
}


const QString & cacheKeyword(
	QHash< quint16, QVector< QString * > > & cache, quint16 group, quint16 element
) {
	QString * keyword = nullptr;

	cacheLock().lock();

	QVector< QString * > & groupKeywords = cache[ group ];
	if ( groupKeywords.isEmpty() ) {
		groupKeywords.fill( nullptr );
		if ( groupKeywords.size() < element ) {
			groupKeywords.resize( element );
			groupKeywords.fill( nullptr );
		}
	}
	
	if ( groupKeywords[ element ] == nullptr ) {
		DcmTag tag( group, element );
		keyword = new QString( keywordFromDcmTagName( tag.getTagName() ) );
		cache[ group ][ element ] = keyword;
	}
	else {
		keyword = cache[ group ][ element ];
	}
	cacheLock().unlock();

	Q_ASSERT( keyword != nullptr );
	return *keyword;
}


QMutex & cacheLock() {
	static QMutex ** mutex = nullptr;

	return *::qInitializeOnce( mutex, [] () -> QMutex * {
		return new QMutex;
	} );
}


const QString & cachedKeyword( quint16 group, quint16 element ) {
	static QHash< quint16, QVector< QString * > > cache;

	QVector< QString * > groupKeywords = cache[ group ];
	if ( groupKeywords.size() > element ) {
		const QString * keyword = groupKeywords[ element ];
		if (  keyword != nullptr ) {
			return *keyword;
		}
	}

	return cacheKeyword( cache, group, element );
}


QString keywordFromDcmTagName( const char * Name ) {
	QString result = Name;

	QChar previous;
	for ( int i = 0; i < result.size(); ++i ) {
		const QChar & Current = result[ i ];

		if ( previous.isLower() && Current.isUpper() ) {
			result.insert( i, " " );
			++i;
		}

		previous = Current;
	}

	return result;
}
