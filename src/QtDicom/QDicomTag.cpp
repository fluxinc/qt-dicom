/***************************************************************************
 *   Copyright © 2013 by Flux Inc.                                         *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#include "QDicomTag.hpp"

#include <QtCore/QHash>
#include <QtCore/QMutex>
#include <QtCore/QRegExp>
#include <QtCore/QVector>

#include <QtFlux/QInitializeOnce>

#include <dcmtk/dcmdata/dctag.h>


static const QString & cacheKeyword( const quint32 & );
static const QString & cachedKeyword( const quint32 & );
static QHash< quint32, QString * > & keywordCache();
static QMutex & keywordCacheLock();
static QString keywordFromDcmTagName( const char * );
static QRegExp stringPattern();


QDicomTag::QDicomTag() :
	id_( Unknown )
{
}


QDicomTag::QDicomTag( const Id & Id ) :
	id_( Id )
{
}


QDicomTag::QDicomTag( const quint16 & Group, const quint16 & Element) {
	setGroup( Group );
	setElement(Element);
}


QDicomTag::~QDicomTag() {
}


QDicomTag::operator DcmTag () const {
	return toDcmTag();
}


QDicomTag::operator Id() const {
	return id_;
}


quint16 QDicomTag::element() const {
	return static_cast< quint16 >( id_ );
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


QDicomTag QDicomTag::fromUInt32( const quint32 & Value ) {
	return QDicomTag( static_cast< Id >( Value ) );
}


quint16 QDicomTag::group() const {
	return static_cast< quint16 > ( id_ >> 16 );
}


bool QDicomTag::isNull() const {
	return id_ == Unknown;
}


bool QDicomTag::isValid() const {
	return id_ != Unknown;
}


const QString & QDicomTag::keyword() const {
	return ::cachedKeyword( toUInt32() );
}

void QDicomTag::setElement( quint16 e ) {
	quint32 value = id_;
	value &= static_cast< quint32 >( 0xFFFF0000 );
	value |= e;

	id_ = static_cast< Id >( value );
}


void QDicomTag::setGroup( quint16 g ) {
	quint32 value = id_;
	value &= static_cast< quint32 >( 0x0000FFFF );
	value |= static_cast< quint32 >( g ) << 16;

	id_ = static_cast< Id >( value );
}


DcmTag QDicomTag::toDcmTag() const {
	return DcmTag( group(), element() );
}


QString QDicomTag::toString() const {
	return 
		QString( "(%1,%2)" )
		.arg( group(), 4, 16, QChar( '0' ) )
		.arg( element(), 4, 16, QChar( '0' ) )
	;
}


quint32 QDicomTag::toUInt32() const {
	return id_;
}


const QString & cacheKeyword( const quint32 & Id ) {
	keywordCacheLock().lock();

	DcmTag tag( static_cast< quint16 >( Id >> 16 ), static_cast< quint16 >( Id ) );
	QString * keyword = new QString(
		::keywordFromDcmTagName( tag.getTagName() )
	); 
	::keywordCache()[ Id ] = keyword;

	keywordCacheLock().unlock();


	return *keyword;
}


const QString & cachedKeyword( const quint32 & Id ) {
	keywordCacheLock().lock();
	const QString * Keyword = ::keywordCache().value( Id, nullptr );
	keywordCacheLock().unlock();

	if ( Keyword != nullptr ) {
		return *Keyword;
	}
	else {
		return ::cacheKeyword( Id );
	}
}


QHash< quint32, QString * > & keywordCache() {
	static QHash< quint32, QString * > * cache = nullptr;

	return ::qInitializeOnce( cache, [] () -> QHash< quint32, QString * > {
		return QHash< quint32, QString * >();
	} );
}


QMutex & keywordCacheLock() {
	static QMutex ** mutex = nullptr;

	return *::qInitializeOnce( mutex, [] () -> QMutex * {
		return new QMutex();
	} );
}


QString keywordFromDcmTagName( const char * Name ) {
	Q_ASSERT( ::strlen( Name ) > 0 );

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


	static const QRegExp IdPattern( "\\sId(\\s|$)" );
	result.replace( IdPattern, " ID\\1" );

	static const QRegExp UidPattern( "\\sUid(\\s|$)" );
	result.replace( IdPattern, " UID\\1" );

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
