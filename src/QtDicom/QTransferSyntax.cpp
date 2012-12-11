/***************************************************************************
 *   Copyright © 2012 by Flux Inc.                                         *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#include "QTransferSyntax.hpp"
#include "QUid.hpp"

#include <QtCore/QHash>

#include <dcmtk/dcmdata/dcuid.h>


QTransferSyntax::QTransferSyntax( Id id ) :
	id_( id )
{
}


QTransferSyntax::~QTransferSyntax() {
}


bool QTransferSyntax::operator == ( const QTransferSyntax & Ts ) const {
	return id_ == Ts.id_;
}


bool QTransferSyntax::operator != ( const QTransferSyntax & Ts ) const {
	return id_ != Ts.id_;
}


QTransferSyntax QTransferSyntax::fromName( const QString & String ) {
	Id id = Unknown;
	if ( false ) {
	}
#define ELSEIF( TS ) \
	else if ( String.compare( name( TS ), Qt::CaseInsensitive ) == 0 ) \
		id = TS

	ELSEIF( LittleEndianImplicit );
	ELSEIF( LittleEndian );
	ELSEIF( LittleEndianDeflated );
	ELSEIF( BigEndian );
	ELSEIF( JpegProcess1 );
	ELSEIF( JpegProcess2_4 );
	ELSEIF( JpegProcess3_5 );
	ELSEIF( JpegProcess6_8 );
	ELSEIF( JpegProcess7_9 );
	ELSEIF( JpegProcess10_12 );
	ELSEIF( JpegProcess11_13 );
	ELSEIF( JpegProcess14 );
	ELSEIF( JpegProcess15 );
	ELSEIF( JpegProcess16_18 );
	ELSEIF( JpegProcess17_19 );
	ELSEIF( JpegProcess20_22 );
	ELSEIF( JpegProcess21_23 );
	ELSEIF( JpegProcess24_26 );
	ELSEIF( JpegProcess25_27 );
	ELSEIF( JpegProcess28 );
	ELSEIF( JpegProcess29 );
	ELSEIF( JpegProcess14Sv1 );
	ELSEIF( JpegLsLossless );
	ELSEIF( JpegLsLossy );
	ELSEIF( Jpeg2000Lossless );
	ELSEIF( Jpeg2000Lossy );
	ELSEIF( Jpeg2000P2Lossless );
	ELSEIF( Jpeg2000P2Lossy );
	ELSEIF( Jpip );
	ELSEIF( JpipDeflated );
	ELSEIF( Mpeg2Main );
	ELSEIF( Mpeg2High );
	ELSEIF( Mpeg4 );
	ELSEIF( Mpeg4Bd );
	ELSEIF( Rle );
	ELSEIF( Mime );
	ELSEIF( Xml );
#undef ELSEIF

	return id;
}


QTransferSyntax QTransferSyntax::fromUid( const char * Uid ) {
	Id id = Unknown;
	if ( false ) {
	}
#define ELSEIF( TS ) \
	else if ( QByteArray( Uid ).trimmed() == uid( TS ) ) \
		id = TS

	ELSEIF( LittleEndianImplicit );
	ELSEIF( LittleEndian );
	ELSEIF( LittleEndianDeflated );
	ELSEIF( BigEndian );
	ELSEIF( JpegProcess1 );
	ELSEIF( JpegProcess2_4 );
	ELSEIF( JpegProcess3_5 );
	ELSEIF( JpegProcess6_8 );
	ELSEIF( JpegProcess7_9 );
	ELSEIF( JpegProcess10_12 );
	ELSEIF( JpegProcess11_13 );
	ELSEIF( JpegProcess14 );
	ELSEIF( JpegProcess15 );
	ELSEIF( JpegProcess16_18 );
	ELSEIF( JpegProcess17_19 );
	ELSEIF( JpegProcess20_22 );
	ELSEIF( JpegProcess21_23 );
	ELSEIF( JpegProcess24_26 );
	ELSEIF( JpegProcess25_27 );
	ELSEIF( JpegProcess28 );
	ELSEIF( JpegProcess29 );
	ELSEIF( JpegProcess14Sv1 );
	ELSEIF( JpegLsLossless );
	ELSEIF( JpegLsLossy );
	ELSEIF( Jpeg2000Lossless );
	ELSEIF( Jpeg2000Lossy );
	ELSEIF( Jpeg2000P2Lossless );
	ELSEIF( Jpeg2000P2Lossy );
	ELSEIF( Jpip );
	ELSEIF( JpipDeflated );
	ELSEIF( Mpeg2Main );
	ELSEIF( Mpeg2High );
	ELSEIF( Mpeg4 );
	ELSEIF( Mpeg4Bd );
	ELSEIF( Rle );
	ELSEIF( Mime );
	ELSEIF( Xml );
#undef ELSEIF

	return QTransferSyntax( id );
}


bool QTransferSyntax::isCompressed() const {
	bool result = true;

	switch ( id_ ) {

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


const char * QTransferSyntax::name() const {
	return name( id_ );
}


const char * QTransferSyntax::name( Id Ts ) {
	const char * result = "UNKNOWN TRANSFER SYNTAX";

	switch ( Ts ) {
#define CASE( VAL, STR ) \
		case VAL : result = STR; break

	CASE( LittleEndianImplicit, "Implicit VR Little Endian" );
	CASE( LittleEndian,         "Explicit VR Little Endian" );
	CASE( LittleEndianDeflated, "Deflated Explicit VR Little Endian" );
	CASE( BigEndian,            "Explicit VR Big Endian" );

	CASE( JpegProcess1,
		"JPEG Baseline (Process 1)" );
	CASE( JpegProcess2_4,
		"JPEG Extended (Process 2 & 4)" );
	CASE( JpegProcess3_5,
		"JPEG Extended (Process 3 & 5)" );
	CASE( JpegProcess6_8,
		"JPEG Spectral Selection, Non-Hierarchical (Process 6 & 8)" );
	CASE( JpegProcess7_9,
		"JPEG Spectral Selection, Non-Hierarchical (Process 7 & 9)" );
	CASE( JpegProcess10_12,
		"JPEG Full Progression, Non-Hierarchical (Process 10 & 12)" );
	CASE( JpegProcess11_13,
		"JPEG Full Progression, Non-Hierarchical (Process 11 & 13)" );
	CASE( JpegProcess14,
		"JPEG Lossless, Non-Hierarchical (Process 14)" );
	CASE( JpegProcess15,
		"JPEG Lossless, Non-Hierarchical (Process 15)" );
	CASE( JpegProcess16_18,
		"JPEG Extended, Hierarchical (Process 16 & 18)" );
	CASE( JpegProcess17_19,
		"JPEG Extended, Hierarchical (Process 17 & 19)" );
	CASE( JpegProcess20_22,
		"JPEG Spectral Selection, Hierarchical (Process 20 & 22)" );
	CASE( JpegProcess21_23,
		"JPEG Spectral Selection, Hierarchical (Process 21 & 23)" );
	CASE( JpegProcess24_26,
		"JPEG Full Progression, Hierarchical (Process 24 & 26)" );
	CASE( JpegProcess25_27,
		"JPEG Full Progression, Hierarchical (Process 25 & 27)" );
	CASE( JpegProcess28,
		"JPEG Lossless, Hierarchical (Process 28)" );
	CASE( JpegProcess29,
		"JPEG Lossless, Hierarchical (Process 29)" );
	CASE( JpegProcess14Sv1,
		"JPEG Lossless, Non-Hierarchical, First-Order Prediction (Process 14 "
		"[Selection Value 1])" );

	CASE( JpegLsLossless, "JPEG-LS Lossless Image Compression" );
	CASE( JpegLsLossy,    "JPEG-LS Lossy (Near-Lossless) Image Compression" );

	CASE( Jpeg2000Lossless,
		"JPEG 2000 Image Compression (Lossless Only)" );
	CASE( Jpeg2000Lossy,
		"JPEG 2000 Image Compression" );
	CASE( Jpeg2000P2Lossless,
		"JPEG 2000 Part 2 Multi-component Image Compression (Lossless Only)" );
	CASE( Jpeg2000P2Lossy,
		"JPEG 2000 Part 2 Multi-component Image Compression" );

	CASE( Jpip,         "JPIP Referenced" );
	CASE( JpipDeflated, "JPIP Referenced Deflate" );

	CASE( Mpeg2Main, "MPEG2 Main Profile @ Main Level" );
	CASE( Mpeg2High, "MPEG2 Main Profile @ High Level" );

	CASE( Mpeg4,   "MPEG-4 AVC/H.264 High Profile / Level 4.1" );
	CASE( Mpeg4Bd, "MPEG-4 AVC/H.264 BD-compatible High Profile / Level 4.1" );

	CASE( Rle, "RLE Lossless" );

	CASE( Mime, "RFC 2557 MIME encapsulation" );

	CASE( Xml, "XML Encoding" );
#undef CASE

	}

	return result;
}


QString QTransferSyntax::toString() const {
	return QString( "%1" ).arg( name() );
}


const QUid & QTransferSyntax::uid() const {
	return uid( id_ );
}


const QUid & QTransferSyntax::uid( Id Ts ) {
	const QUid * result = NULL;

	switch ( Ts ) {

#define CASE( ID, UID ) \
	case ID : {  \
		static const QUid Uid( UID ); \
		result = &Uid; \
	} break

	CASE( Unknown, "" );
	CASE( LittleEndianImplicit, "1.2.840.10008.1.2" );
	CASE( LittleEndian,         "1.2.840.10008.1.2.1" );
	CASE( LittleEndianDeflated, "1.2.840.10008.1.2.1.99" );
	CASE( BigEndian,            "1.2.840.10008.1.2.2" );
	CASE( JpegProcess1,         "1.2.840.10008.1.2.4.50" );
	CASE( JpegProcess2_4,       "1.2.840.10008.1.2.4.51" );
	CASE( JpegProcess3_5,       "1.2.840.10008.1.2.4.52" );
	CASE( JpegProcess6_8,       "1.2.840.10008.1.2.4.53" );
	CASE( JpegProcess7_9,       "1.2.840.10008.1.2.4.54" );
	CASE( JpegProcess10_12,     "1.2.840.10008.1.2.4.55" );
	CASE( JpegProcess11_13,     "1.2.840.10008.1.2.4.56" );
	CASE( JpegProcess14,        "1.2.840.10008.1.2.4.57" );
	CASE( JpegProcess15,        "1.2.840.10008.1.2.4.58" );
	CASE( JpegProcess16_18,     "1.2.840.10008.1.2.4.59" );
	CASE( JpegProcess17_19,     "1.2.840.10008.1.2.4.60" );
	CASE( JpegProcess20_22,     "1.2.840.10008.1.2.4.61" );
	CASE( JpegProcess21_23,     "1.2.840.10008.1.2.4.62" );
	CASE( JpegProcess24_26,     "1.2.840.10008.1.2.4.63" );
	CASE( JpegProcess25_27,     "1.2.840.10008.1.2.4.64" );
	CASE( JpegProcess28,        "1.2.840.10008.1.2.4.65" );
	CASE( JpegProcess29,        "1.2.840.10008.1.2.4.66" );
	CASE( JpegProcess14Sv1,     "1.2.840.10008.1.2.4.70" );
	CASE( JpegLsLossless,       "1.2.840.10008.1.2.4.80" );
	CASE( JpegLsLossy,          "1.2.840.10008.1.2.4.81" );
	CASE( Jpeg2000Lossless,     "1.2.840.10008.1.2.4.90" );
	CASE( Jpeg2000Lossy,        "1.2.840.10008.1.2.4.91" );
	CASE( Jpeg2000P2Lossless,   "1.2.840.10008.1.2.4.92" );
	CASE( Jpeg2000P2Lossy,      "1.2.840.10008.1.2.4.93" );
	CASE( Jpip,                 "1.2.840.10008.1.2.4.94" );
	CASE( JpipDeflated,         "1.2.840.10008.1.2.4.95" );
	CASE( Mpeg2Main,            "1.2.840.10008.1.2.4.100" );
	CASE( Mpeg2High,            "1.2.840.10008.1.2.4.101" );
	CASE( Mpeg4,                "1.2.840.10008.1.2.4.102" );
	CASE( Mpeg4Bd,              "1.2.840.10008.1.2.4.103" );
	CASE( Rle,                  "1.2.840.10008.1.2.5" );
	CASE( Mime,                 "1.2.840.10008.1.2.6.1" );
	CASE( Xml,                  "1.2.840.10008.1.2.6.2" );
#undef CASE

	default :
		Q_ASSERT( 0 );
		static const QUid Default;
		result = &Default;
	}

	return *result;
}


uint qHash( const QTransferSyntax & Ts ) {
	return qHash( static_cast< int >( Ts.id_ ) );
}
