/***************************************************************************
 *   Copyright © 2013 by Flux Inc.                                         *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#include "QDicomTag.hpp"

#include <QtCore/QHash>
#include <QtCore/QVector>

#include <QtFlux/QInitializeOnce>


#include <dcmtk/dcmdata/dctag.h>


static const QString & cacheKeyword( quint16, quint16 );
static const QString & cachedKeyword( quint16, quint16 );
static QHash< quint16, QVector< QString * > > & keywordCache();
static QMutex & keywordCacheLock();
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


QDicomTag QDicomTag::fromUInt32( quint32 Value ) {
	QDicomTag result;
	result.value_ = Value;

	return result;
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


const QString & cacheKeyword( quint16 group, quint16 element ) {
	QString * keyword = nullptr;

	keywordCacheLock().lock();

	QHash< quint16, QVector< QString * > > & cache = keywordCache();
	QVector< QString * > & groupKeywords = cache[ group ];
	if ( groupKeywords.size() < element ) {
		groupKeywords.resize( element + 1 );
		groupKeywords.fill( nullptr );
	}
	
	if ( groupKeywords[ element ] == nullptr ) {
		DcmTag tag( group, element );
		keyword = new QString( keywordFromDcmTagName( tag.getTagName() ) );
		cache[ group ][ element ] = keyword;
	}
	else {
		keyword = cache[ group ][ element ];
	}
	keywordCacheLock().unlock();

	Q_ASSERT( keyword != nullptr );
	return *keyword;
}


const QString & cachedKeyword( quint16 group, quint16 element ) {
	const QHash< quint16, QVector< QString * > > & Cache = keywordCache();

	QVector< QString * > groupKeywords = Cache[ group ];
	if ( groupKeywords.size() > element ) {
		const QString * keyword = groupKeywords[ element ];
		if (  keyword != nullptr ) {
			return *keyword;
		}
	}

	return cacheKeyword( group, element );
}


QHash< quint16, QVector< QString * > > & keywordCache() {
	static QHash< quint16, QVector< QString * > > * cache = nullptr;

	return ::qInitializeOnce( cache, [] () -> QHash< quint16, QVector< QString * > > {
		return QHash< quint16, QVector< QString * > >();
	} );
}


QMutex & keywordCacheLock() {
	static QMutex ** mutex = nullptr;

	return *::qInitializeOnce( mutex, [] () -> QMutex * {
		return new QMutex;
	} );
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
