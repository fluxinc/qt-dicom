/***************************************************************************
 *   Copyright © 2013 by Flux Inc.                                         *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#include "QDicomTag.hpp"

#include <QtCore/QHash>
#include <QtCore/QRegExp>
#include <QtCore/QVector>

#include <QtFlux/QInitializeOnce>


#include <dcmtk/dcmdata/dctag.h>


static const QString & cacheKeyword( quint16, quint16 );
static const QString & cachedKeyword( quint16, quint16 );
static QHash< quint16, QVector< QString * > > & keywordCache();
static QMutex & keywordCacheLock();
static QString keywordFromDcmTagName( const char * );
static QRegExp stringPattern();


QDicomTag::QDicomTag() :
	value_( 0 )
{
}


QDicomTag::QDicomTag( const Id & Id ) :
	value_( Id )
{
}


QDicomTag::QDicomTag( quint16 group, quint16 element ) {
	setGroup( group );
	setElement( element );
}


QDicomTag::~QDicomTag() {
}


QDicomTag::operator quint32() const {
	return value_;
}


quint16 QDicomTag::element() const {
	return static_cast< quint16 >( value_ );
}


QDicomTag QDicomTag::fromString( const QString & Value ) {
	const QRegExp Pattern = stringPattern();
	if ( Pattern.exactMatch( Value ) ) {
		bool status = false;

		const quint16 Group = Pattern.cap( 1 ).toUInt( &status, 16 );
		Q_ASSERT( status );
		const quint16 Element = Pattern.cap( 2 ).toUInt( &status, 16 );
		Q_ASSERT( status );


		return QDicomTag( Group, Element );
	}
	else {
		qDebug( __FUNCTION__": "
			"unable to parse DICOM tag from string: `%s'",
			qPrintable( Value )
		);
	}

	return QDicomTag();
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


void QDicomTag::setElement( quint16 e ) {
	value_ &= static_cast< quint32 >( 0xFFFF0000 );
	value_ |= e;
}


void QDicomTag::setGroup( quint16 g ) {
	value_ &= static_cast< quint32 >( 0x0000FFFF );
	value_ |= static_cast< quint32 >( g ) << 16;
}


QString QDicomTag::toString() const {
	return 
		QString( "(%1,%2)" )
		.arg( group(), 4, 16, QChar( '0' ) )
		.arg( element(), 4, 16, QChar( '0' ) )
	;
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


QRegExp stringPattern() {
	static QRegExp * pattern = nullptr;

	return ::qInitializeOnce( pattern, [] () -> QRegExp {
		QRegExp pattern( "\\(?([0-9A-Fa-f]{4}),([0-9A-Fa-f]{4})\\)?" );
		Q_ASSERT( pattern.isValid() );
		Q_ASSERT( pattern.exactMatch( "(0010,0010)" ) );
		Q_ASSERT( pattern.exactMatch( "0010,0010" ) );
		Q_ASSERT( ! pattern.exactMatch( "001g,0010" ) );
		Q_ASSERT( ! pattern.exactMatch( "(001,001)" ) );

		Q_ASSERT( pattern.exactMatch( "(0010,0020)" ) );
		Q_ASSERT( pattern.captureCount() == 2 );
		Q_ASSERT( pattern.cap( 1 ).size() == 4 );
		Q_ASSERT( pattern.cap( 1 ) == "0010" );
		Q_ASSERT( pattern.cap( 2 ).size() == 4 );
		Q_ASSERT( pattern.cap( 2 ) == "0020" );

		// Force compilation in non-debug builds where Q_ASSERT is not evaluated
		pattern.exactMatch( "0000,0000" );

		return pattern;
	} );
}
