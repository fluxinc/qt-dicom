/***************************************************************************
 *   Copyright © 2012 by Flux Inc.                                         *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#include "QTransferSyntax.hpp"
#include "QUid.hpp"
#include "Utilities.hpp"

#include <QtCore/QHash>
#include <QtCore/QVector>

#include <QtFlux/QInitializeOnce>

#include <dcmtk/dcmdata/dcuid.h>


#ifdef _DEBUG

extern bool TestQTransferSyntax();

#endif // _DEBUG


QTransferSyntax::QTransferSyntax() :
	id_( Unknown )
{
}


QTransferSyntax::QTransferSyntax( Id id ) :
	id_( id )
{
}


QTransferSyntax::~QTransferSyntax() {
}


QTransferSyntax::operator Id() const {
	return id();
}

/*
bool QTransferSyntax::operator == ( const QTransferSyntax & Ts ) const {
	return id_ == Ts.id_;
}


bool QTransferSyntax::operator != ( const QTransferSyntax & Ts ) const {
	return id_ != Ts.id_;
}*/


QTransferSyntax QTransferSyntax::fromName( const QString & Name ) {
	static QHash< QString, QTransferSyntax > * syntaxes = nullptr;

	::qInitializeOnce( syntaxes, []() throw() -> QHash< QString, QTransferSyntax > {
		QHash< QString, QTransferSyntax > result;

#define SET( LABEL ) { \
		const QTransferSyntax Ts = QTransferSyntax::LABEL; \
		result[ Ts.name() ] = Ts; \
	}

		SET( LittleEndianImplicit );
		SET( LittleEndian );
		SET( LittleEndianDeflated );
		SET( BigEndian );
		SET( JpegProcess1 );
		SET( JpegProcess2_4 );
		SET( JpegProcess3_5 );
		SET( JpegProcess6_8 );
		SET( JpegProcess7_9 );
		SET( JpegProcess10_12 );
		SET( JpegProcess11_13 );
		SET( JpegProcess14 );
		SET( JpegProcess15 );
		SET( JpegProcess16_18 );
		SET( JpegProcess17_19 );
		SET( JpegProcess20_22 );
		SET( JpegProcess21_23 );
		SET( JpegProcess24_26 );
		SET( JpegProcess25_27 );
		SET( JpegProcess28 );
		SET( JpegProcess29 );
		SET( JpegProcess14Sv1 );
		SET( JpegLsLossless );
		SET( JpegLsLossy );
		SET( Jpeg2000Lossless );
		SET( Jpeg2000Lossy );
		SET( Jpeg2000P2Lossless );
		SET( Jpeg2000P2Lossy );
		SET( Jpip );
		SET( JpipDeflated );
		SET( Mpeg2Main );
		SET( Mpeg2High );
		SET( Mpeg4 );
		SET( Mpeg4Bd );
		SET( Rle );
		SET( Mime );
		SET( Xml );
#undef SET

		return result;
	} );

	Q_ASSERT( syntaxes != nullptr );

	return syntaxes->value( Name, QTransferSyntax() );
}


QTransferSyntax QTransferSyntax::fromUid( const QByteArray & Uid ) {
	static QHash< QByteArray, QTransferSyntax > * syntaxes = nullptr;

	::qInitializeOnce( syntaxes, []() throw() -> QHash< QByteArray, QTransferSyntax > {
		QHash< QByteArray, QTransferSyntax > result;

#define SET( LABEL ) { \
		const QTransferSyntax Ts = QTransferSyntax::LABEL; \
		result[ Ts.uid() ] = Ts; \
	}

		SET( LittleEndianImplicit );
		SET( LittleEndian );
		SET( LittleEndianDeflated );
		SET( BigEndian );
		SET( JpegProcess1 );
		SET( JpegProcess2_4 );
		SET( JpegProcess3_5 );
		SET( JpegProcess6_8 );
		SET( JpegProcess7_9 );
		SET( JpegProcess10_12 );
		SET( JpegProcess11_13 );
		SET( JpegProcess14 );
		SET( JpegProcess15 );
		SET( JpegProcess16_18 );
		SET( JpegProcess17_19 );
		SET( JpegProcess20_22 );
		SET( JpegProcess21_23 );
		SET( JpegProcess24_26 );
		SET( JpegProcess25_27 );
		SET( JpegProcess28 );
		SET( JpegProcess29 );
		SET( JpegProcess14Sv1 );
		SET( JpegLsLossless );
		SET( JpegLsLossy );
		SET( Jpeg2000Lossless );
		SET( Jpeg2000Lossy );
		SET( Jpeg2000P2Lossless );
		SET( Jpeg2000P2Lossy );
		SET( Jpip );
		SET( JpipDeflated );
		SET( Mpeg2Main );
		SET( Mpeg2High );
		SET( Mpeg4 );
		SET( Mpeg4Bd );
		SET( Rle );
		SET( Mime );
		SET( Xml );
#undef SET

		return result;
	} );

	Q_ASSERT( syntaxes != nullptr );

	return syntaxes->value( Uid, QTransferSyntax() );
}


QTransferSyntax::Id QTransferSyntax::id() const {
	return id_;
}


bool QTransferSyntax::isCompressed() const {
	bool result = true;

	switch ( id_ ) {

	case Unknown :
	case LittleEndianImplicit :
	case LittleEndian :
	case BigEndian :
		result = false;
		break;

	}

	return result;
}


bool QTransferSyntax::isNull() const {
	return id_ == Unknown;
}


bool QTransferSyntax::isRetired() const {
	bool result = false;

	switch ( id_ ) {

	case JpegProcess3_5 :
	case JpegProcess6_8 :
	case JpegProcess7_9 :
	case JpegProcess10_12 :
	case JpegProcess11_13 :
	case JpegProcess15 :
	case JpegProcess16_18 :
	case JpegProcess17_19 :
	case JpegProcess20_22 :
	case JpegProcess21_23 :
	case JpegProcess24_26 :
	case JpegProcess25_27 :
	case JpegProcess28 :
	case JpegProcess29 :
		result = true;
		break;
	}

	return result;
}


bool QTransferSyntax::isValid() const {
	return id_ != Unknown;
}


const QString & QTransferSyntax::name() const {
	return name( id_ );
}


const QString & QTransferSyntax::name( Id Ts ) {
	static QVector< QString > * names = nullptr;

	::qInitializeOnce( names, ( []() throw() -> QVector< QString > {
		static const int Last = QTransferSyntax::Xml;

		QVector< QString > result( Last + 1 );

#define SET( VAL, STR ) \
	Q_ASSERT( QTransferSyntax::VAL <= Last ); \
	result[ QTransferSyntax::VAL ] = ::WArrayString( STR )

		SET( Unknown, L"UNKNOWN TRANSFER SYNTAX" );

		SET( LittleEndianImplicit, L"Implicit VR Little Endian" );
		SET( LittleEndian,         L"Explicit VR Little Endian" );
		SET( LittleEndianDeflated, L"Deflated Explicit VR Little Endian" );
		SET( BigEndian,            L"Explicit VR Big Endian" );

		SET( JpegProcess1,
			L"JPEG Baseline (Process 1)" );
		SET( JpegProcess2_4,
			L"JPEG Extended (Process 2 & 4)" );
		SET( JpegProcess3_5,
			L"JPEG Extended (Process 3 & 5)" );
		SET( JpegProcess6_8,
			L"JPEG Spectral Selection, Non-Hierarchical (Process 6 & 8)" );
		SET( JpegProcess7_9,
			L"JPEG Spectral Selection, Non-Hierarchical (Process 7 & 9)" );
		SET( JpegProcess10_12,
			L"JPEG Full Progression, Non-Hierarchical (Process 10 & 12)" );
		SET( JpegProcess11_13,
			L"JPEG Full Progression, Non-Hierarchical (Process 11 & 13)" );
		SET( JpegProcess14,
			L"JPEG Lossless, Non-Hierarchical (Process 14)" );
		SET( JpegProcess15,
			L"JPEG Lossless, Non-Hierarchical (Process 15)" );
		SET( JpegProcess16_18,
			L"JPEG Extended, Hierarchical (Process 16 & 18)" );
		SET( JpegProcess17_19,
			L"JPEG Extended, Hierarchical (Process 17 & 19)" );
		SET( JpegProcess20_22,
			L"JPEG Spectral Selection, Hierarchical (Process 20 & 22)" );
		SET( JpegProcess21_23,
			L"JPEG Spectral Selection, Hierarchical (Process 21 & 23)" );
		SET( JpegProcess24_26,
			L"JPEG Full Progression, Hierarchical (Process 24 & 26)" );
		SET( JpegProcess25_27,
			L"JPEG Full Progression, Hierarchical (Process 25 & 27)" );
		SET( JpegProcess28,
			L"JPEG Lossless, Hierarchical (Process 28)" );
		SET( JpegProcess29,
			L"JPEG Lossless, Hierarchical (Process 29)" );
		SET( JpegProcess14Sv1,
			L"JPEG Lossless, Non-Hierarchical, First-Order Prediction (Process 14 "
			L"[Selection Value 1])" );

		SET( JpegLsLossless, L"JPEG-LS Lossless Image Compression" );
		SET( JpegLsLossy,    L"JPEG-LS Lossy (Near-Lossless) Image Compression" );

		SET( Jpeg2000Lossless,
			L"JPEG 2000 Image Compression (Lossless Only)" );
		SET( Jpeg2000Lossy,
			L"JPEG 2000 Image Compression" );
		SET( Jpeg2000P2Lossless,
			L"JPEG 2000 Part 2 Multi-component Image Compression (Lossless Only)" );
		SET( Jpeg2000P2Lossy,
			L"JPEG 2000 Part 2 Multi-component Image Compression" );

		SET( Jpip,         L"JPIP Referenced" );
		SET( JpipDeflated, L"JPIP Referenced Deflate" );

		SET( Mpeg2Main, L"MPEG2 Main Profile @ Main Level" );
		SET( Mpeg2High, L"MPEG2 Main Profile @ High Level" );

		SET( Mpeg4,   L"MPEG-4 AVC/H.264 High Profile / Level 4.1" );
		SET( Mpeg4Bd, L"MPEG-4 AVC/H.264 BD-compatible High Profile / Level 4.1" );

		SET( Rle, L"RLE Lossless" );

		SET( Mime, L"RFC 2557 MIME encapsulation" );

		SET( Xml, L"XML Encoding" );
#undef SET

		return result;
	} ) );

	Q_ASSERT( names != nullptr );
	Q_ASSERT( names->size() > Ts );

	return names->at( Ts );
}

/*
int QTransferSyntax::toInt() const {
	return id_;
}*/


const QByteArray & QTransferSyntax::uid() const {
	return uid( id_ );
}


const QByteArray & QTransferSyntax::uid( Id Ts ) {
	static QVector< QByteArray > * uids = nullptr;

	::qInitializeOnce( uids, ( []() throw() -> QVector< QByteArray > {
		static const int Last = QTransferSyntax::Xml;
		QVector< QByteArray > result( Last + 1 );

#define SET( LABEL, UID ) \
	Q_ASSERT( QTransferSyntax::LABEL <= Last ); \
	result[ QTransferSyntax::LABEL ] = QByteArray::fromRawData( UID, sizeof( UID ) - 1 );

		SET( Unknown,              "" );
		SET( LittleEndianImplicit, "1.2.840.10008.1.2" );
		SET( LittleEndian,         "1.2.840.10008.1.2.1" );
		SET( LittleEndianDeflated, "1.2.840.10008.1.2.1.99" );
		SET( BigEndian,            "1.2.840.10008.1.2.2" );
		SET( JpegProcess1,         "1.2.840.10008.1.2.4.50" );
		SET( JpegProcess2_4,       "1.2.840.10008.1.2.4.51" );
		SET( JpegProcess3_5,       "1.2.840.10008.1.2.4.52" );
		SET( JpegProcess6_8,       "1.2.840.10008.1.2.4.53" );
		SET( JpegProcess7_9,       "1.2.840.10008.1.2.4.54" );
		SET( JpegProcess10_12,     "1.2.840.10008.1.2.4.55" );
		SET( JpegProcess11_13,     "1.2.840.10008.1.2.4.56" );
		SET( JpegProcess14,        "1.2.840.10008.1.2.4.57" );
		SET( JpegProcess15,        "1.2.840.10008.1.2.4.58" );
		SET( JpegProcess16_18,     "1.2.840.10008.1.2.4.59" );
		SET( JpegProcess17_19,     "1.2.840.10008.1.2.4.60" );
		SET( JpegProcess20_22,     "1.2.840.10008.1.2.4.61" );
		SET( JpegProcess21_23,     "1.2.840.10008.1.2.4.62" );
		SET( JpegProcess24_26,     "1.2.840.10008.1.2.4.63" );
		SET( JpegProcess25_27,     "1.2.840.10008.1.2.4.64" );
		SET( JpegProcess28,        "1.2.840.10008.1.2.4.65" );
		SET( JpegProcess29,        "1.2.840.10008.1.2.4.66" );
		SET( JpegProcess14Sv1,     "1.2.840.10008.1.2.4.70" );
		SET( JpegLsLossless,       "1.2.840.10008.1.2.4.80" );
		SET( JpegLsLossy,          "1.2.840.10008.1.2.4.81" );
		SET( Jpeg2000Lossless,     "1.2.840.10008.1.2.4.90" );
		SET( Jpeg2000Lossy,        "1.2.840.10008.1.2.4.91" );
		SET( Jpeg2000P2Lossless,   "1.2.840.10008.1.2.4.92" );
		SET( Jpeg2000P2Lossy,      "1.2.840.10008.1.2.4.93" );
		SET( Jpip,                 "1.2.840.10008.1.2.4.94" );
		SET( JpipDeflated,         "1.2.840.10008.1.2.4.95" );
		SET( Mpeg2Main,            "1.2.840.10008.1.2.4.100" );
		SET( Mpeg2High,            "1.2.840.10008.1.2.4.101" );
		SET( Mpeg4,                "1.2.840.10008.1.2.4.102" );
		SET( Mpeg4Bd,              "1.2.840.10008.1.2.4.103" );
		SET( Rle,                  "1.2.840.10008.1.2.5" );
		SET( Mime,                 "1.2.840.10008.1.2.6.1" );
		SET( Xml,                  "1.2.840.10008.1.2.6.2" );
#undef SET

		return result;

	} ) );

	Q_ASSERT( uids != nullptr );
	Q_ASSERT( uids->size() > Ts );

	return uids->at( Ts );
}


#ifdef _DEBUG

bool TestQTransferSyntax() {
	const QTransferSyntax Unknown;
	Q_ASSERT( ! Unknown.isCompressed() );
	Q_ASSERT( Unknown.isNull() );
	Q_ASSERT( ! Unknown.isRetired() );
	Q_ASSERT( ! Unknown.isValid() );
	Q_ASSERT( Unknown.name().contains( "Unknown", Qt::CaseInsensitive ) );
	Q_ASSERT( Unknown.uid().isEmpty() );
	Q_ASSERT( Unknown == QTransferSyntax::Unknown );
	Q_ASSERT( Unknown != QTransferSyntax::LittleEndian );


	const QTransferSyntax LittleEndian = QTransferSyntax::LittleEndian;
	Q_ASSERT( LittleEndian != Unknown );
	Q_ASSERT( ! LittleEndian.isCompressed() );
	Q_ASSERT( ! LittleEndian.isNull() );
	Q_ASSERT( ! LittleEndian.isRetired() );
	Q_ASSERT( LittleEndian.isValid() );
	Q_ASSERT( LittleEndian.name().contains( "little endian", Qt::CaseInsensitive ) );
	Q_ASSERT( LittleEndian.uid() == "1.2.840.10008.1.2.1" );

	Q_ASSERT( QTransferSyntax::fromName( LittleEndian.name() ) == LittleEndian );
	Q_ASSERT( QTransferSyntax::fromUid( LittleEndian.uid() ) == LittleEndian );


	const QTransferSyntax JpegRetired = QTransferSyntax::JpegProcess6_8;
	Q_ASSERT( JpegRetired != Unknown );
	Q_ASSERT( JpegRetired.isCompressed() );
	Q_ASSERT( ! JpegRetired.isNull() );
	Q_ASSERT( JpegRetired.isRetired() );
	Q_ASSERT( JpegRetired.isValid() );
	Q_ASSERT( JpegRetired.name().contains( "Process 6", Qt::CaseInsensitive ) );
	Q_ASSERT( JpegRetired.uid() == "1.2.840.10008.1.2.4.53" );

	Q_ASSERT( QTransferSyntax::fromName( JpegRetired.name() ) == JpegRetired );
	Q_ASSERT( QTransferSyntax::fromUid( JpegRetired.uid() ) == JpegRetired );


	Q_ASSERT( ::qHash( Unknown )      != ::qHash( LittleEndian ) );
	Q_ASSERT( ::qHash( LittleEndian ) != ::qHash( JpegRetired ) );
	Q_ASSERT( ::qHash( JpegRetired )  != ::qHash( Unknown ) );


	QTransferSyntax someSyntax;
	Q_ASSERT( someSyntax == QTransferSyntax::Unknown );

	someSyntax = LittleEndian;
	Q_ASSERT( someSyntax == QTransferSyntax::LittleEndian );
	Q_ASSERT( someSyntax == LittleEndian );


	return true;
}

#endif // _DEBUG
