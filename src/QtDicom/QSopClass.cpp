/***************************************************************************
 *   Copyright © 2013 by Flux Inc.                                         *
 *   Author: Pawel Zak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#include "QSopClass.hpp"
#include "Utilities.hpp"

#include <QtCore/QByteArray>
#include <QtCore/QString>
#include <QtCore/QVector>

#include <QtFlux/QInitializeOnce>


QSopClass::QSopClass() :
	id_( Unknown )
{
}
 

QSopClass::QSopClass( const Id & id ) :
	id_( id )
{
}


QSopClass::~QSopClass() {
}


QSopClass::operator const QSopClass::Id &() const {
	return id_;
}


QSopClass QSopClass::fromUid( const QByteArray & Uid ) {
	Id result = Unknown;

	if ( false ) {}
	
#define COMPARE( ID ) \
	else if ( Uid == uid( QSopClass::ID ) ) result = ID 

	COMPARE( BasicAnnotationBox );
	COMPARE( BasicGrayscalePrintManagement );
	COMPARE( BasicGrayscaleImageBox );
	COMPARE( BasicFilmBox );
	COMPARE( BasicFilmSession );
	COMPARE( Printer );
#undef COMPARE

	return result;
}


bool QSopClass::hasMeta() const {
	return ( id_ & 0x80 ) != 0;
}


bool QSopClass::isMeta() const {
	return ( id_ & 0x100 ) != 0;
}


bool QSopClass::isNull() const {
	return id_ == 0;
}


bool QSopClass::isValid() const {
	return id_ != 0;
}


QSopClass QSopClass::meta() const {
	if ( hasMeta() ) {
		const Id MetaId = static_cast< Id >(
			( ( id_ & 0x70 ) >> 4 ) | 0x100
		);
		Q_ASSERT( MetaId >= 0x100 && MetaId <= MaxId );

		return QSopClass( MetaId );
	}
	else {
		return QSopClass();
	}
}


const QString & QSopClass::name() const {
	return name( id_ );
}


const QString & QSopClass::name( const Id & Id ) {
	static QVector< QString > * names = nullptr;

	::qInitializeOnce( names, ( []() throw() -> QVector< QString > {
		QVector< QString > result( MaxId + 1 );


#define SET( ID, VALUE ) \
			Q_ASSERT( QSopClass::ID <= MaxId ); \
			result[ QSopClass::ID ] = ::WArrayString( VALUE )


		SET( Unknown,                       L"Unknown" );
		SET( BasicAnnotationBox,            L"Basic Annotation Box" );
		SET( BasicGrayscalePrintManagement, L"Basic Grayscale Print Management" );
		SET( BasicGrayscaleImageBox,        L"Basic Grayscale Image Box" );
		SET( BasicFilmBox,                  L"Basic Film Box" );
		SET( BasicFilmSession,              L"Basic Film Session" );
		SET( Printer,                       L"Printer" );
#undef SET

		return result;
	} ) );

	Q_ASSERT( names != nullptr );
	Q_ASSERT( names->size() > Id );

	return names->at( Id );
}


const QByteArray & QSopClass::uid() const {
	return uid( id_ );
}


const QByteArray & QSopClass::uid( const Id & Id ) {
	static QVector< QByteArray > * uids = nullptr;

	::qInitializeOnce( uids, ( []() throw() -> QVector< QByteArray > {
		QVector< QByteArray > result( MaxId + 1 );

#define SET( ID, VALUE ) \
			Q_ASSERT( QSopClass::ID <= MaxId ); \
			result[ QSopClass::ID ] = QByteArray::fromRawData( \
				VALUE, sizeof( VALUE ) - 1 \
			)

		SET( Unknown,                       "" );
		SET( BasicAnnotationBox,            "1.2.840.10008.5.1.1.15" );
		SET( BasicGrayscalePrintManagement, "1.2.840.10008.5.1.1.9" );
		SET( BasicGrayscaleImageBox,        "1.2.840.10008.5.1.1.4" );
		SET( BasicFilmBox,                  "1.2.840.10008.5.1.1.2" );
		SET( BasicFilmSession,              "1.2.840.10008.5.1.1.1" );
		SET( Printer,                       "1.2.840.10008.5.1.1.16" );
#undef SET

		return result;
	} ) );

	Q_ASSERT( uids != nullptr );
	Q_ASSERT( uids->size() > Id );

	return uids->at( Id );
}
