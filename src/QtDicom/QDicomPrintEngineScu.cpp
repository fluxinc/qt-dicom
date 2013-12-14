/***************************************************************************
 *   Copyright © 2013 by Flux Inc.                                         *
 *   Author: Pawel Zak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#include "Dataset.hpp"
#include "QDicomPrintEngineScu.hpp" 
#include "QDicomPrinter.hpp"
#include "QDicomPrinterDriver.hpp"
#include "QSopClass.hpp"
#include "Utilities.hpp"

#include <QtCore/QDir>
#include <QtCore/QDateTime>

#include <QtFlux/QInitializeOnce>

#include <QtGui/QImage>

#include <dcmtk/dcmdata/dcdatset.h>
#include <dcmtk/dcmdata/dcitem.h>
#include <dcmtk/dcmdata/dcsequen.h>
#include <dcmtk/dcmdata/dcuid.h>
#include <dcmtk/dcmdata/dcvrui.h>
#include <dcmtk/dcmdata/dcvrobow.h>


using Dicom::ConnectionParameters;
using Dicom::Dataset;
using Dicom::RequestorAssociation;
using Dicom::ServiceUser;

template < int Depth >
static int ConvertPixel( const QRgb &  );
template < typename Type >
static Type * CreatePixelData( const QImage & Image, const quint8 & Depth );
static const QString & FilmDestinationString( const QDicomPrinter::FilmDestination & );
static const QString & FilmSizeString( const QDicomPrinter::FilmSize & );
static const QString & MagnificationTypeString( const QDicomPrinter::MagnificationType & );
static const QString & MediumTypeString( const QDicomPrinter::MediumType & );
static const QString & OrientationString( const QDicomPrinter::Orientation & );

#ifdef _DEBUG

extern bool TestQDicomPrintEngineScu_ConvertPixel();
#endif // _DEBUG


QDicomPrintEngine::Scu::Scu() :
	ServiceUser( &association_ ),
	printer_( nullptr )
{
}


inline const RequestorAssociation & QDicomPrintEngine::Scu::association() const {
	return association_;
}


inline RequestorAssociation & QDicomPrintEngine::Scu::association() {
	return association_;
}


bool QDicomPrintEngine::Scu::beginSession( const QDicomPrinter * PrinterAddress ) {
	Q_ASSERT( ! isOpen() );

	const QDicomPrinter & Printer = *PrinterAddress;

	qDebug(
		__FUNCTION__": requesting association from %s",
		qPrintable( Printer.name() )
	);

	ConnectionParameters params;
	params.setHostAddress( Printer.hostAddress() );
	params.setPort( Printer.portNumber() );
	params.setMyAeTitle( Printer.localAeTitle().size() > 0 ? Printer.localAeTitle() : "FLUX" );
	params.setPeerAeTitle( Printer.remoteAeTitle().size() > 0 ? Printer.remoteAeTitle() : Printer.driver().modelName() );
	params.setTimeout( 60 );

	static const QSopClass SopClass = QSopClass::BasicGrayscalePrintManagement;

	bool timedOut = false;
	const int Count = association().request(
		params, SopClass.uid(), &timedOut
	);

	if ( Count > 0 ) {
		setPrinter( Printer );
		return createSession();
	}
	else if ( timedOut ) {
		raiseError(
			QString( "Timeout occured when connecting to %1" )
			.arg( Printer.name() )
		);
	}
	else if ( association().hasError() ) {
		raiseError( association().errorMessage() );
	}
	else {
		raiseError(
			QString( "Device doesn't support %1 SOP Class" )
			.arg( SopClass.name() )
		);
	}

	return false;
}


bool QDicomPrintEngine::Scu::createFilmBox( const QImage & Image ) {
	Q_ASSERT( association().isEstablished() );

	const QDicomPrinter & Printer = printer();
	const QDicomPrinterDriver & Driver = Printer.driver();


	// Prepare string values for all Attributes we're going to set. First the
	// Basic Film Box Presentation Module, in order they appear in PS3.3,
	// Table C.13-3

	const QString ImageDisplayFormat = "STANDARD\\1,1";
	// skip Annotation Display Format ID
	const QString FilmOrientation = ::OrientationString(
		( Printer.orientation() != QDicomPrinter::AutomaticOrientation ) ?
		( Printer.orientation() ) :
		(
			( Image.width() <= Image.height() ) ?
			( QDicomPrinter::VerticalOrientation ) :
			( QDicomPrinter::HorizontalOrientation )
		)
	);
	const QString FilmSizeId = ::FilmSizeString( Printer.filmSize() );
	const QString MagnificationType = ::MagnificationTypeString(
		Driver.magnificationTypes().contains( QDicomPrinter::None ) ?
		QDicomPrinter::None :
		Driver.magnificationTypes().first()
	);
	// skip Smoothing Type 
	const bool SetBorderDensity = 
		Driver.hasFeature( QDicomPrinterDriver::BorderDensity )
	;
	const QString BorderDensity =
		SetBorderDensity ?
		( printer().emptyAreaDensity() <= 0.5 ? "BLACK" : "WHITE" ) : ""
	;
	const bool SetEmptyImageDensity =
		Driver.hasFeature( QDicomPrinterDriver::EmptyImageDensity )
	;
	const QString EmptyImageDensity =
		SetEmptyImageDensity ?
		( printer().emptyAreaDensity() <= 0.5 ? "BLACK" : "WHITE" ) : ""
	;
	// skip Min Density
	// skip Max Density
	const bool SetTrim = Driver.hasFeature( QDicomPrinterDriver::Trim );
	const QString Trim = SetTrim ? "NO" : "";
	// skip Configuration Information
	// skip Illumination
	// skip Reflected Ambient Light
	const bool SetRequestedResolutionId = 
		Driver.hasFeature( QDicomPrinterDriver::HighQuality ) && 
		Printer.quality() == QDicomPrinter::HighQuality
	;
	const QString RequestedResolutionId = 
		SetRequestedResolutionId ? "HIGH" : ""
	;


	qDebug( "Preparing Basic Film Box:" );

	Dataset attributes;
#define SET( ATTRIBUTE ) { \
		const QDicomTag Tag( QDicomTag::ATTRIBUTE ); \
		qDebug( "\t: %s = %s", qPrintable( Tag.keyword() ), qPrintable( ATTRIBUTE ) ); \
		attributes.setAttribute( Tag, ATTRIBUTE ); \
	}

	SET( ImageDisplayFormat );
	SET( FilmOrientation );
	SET( FilmSizeId );
	SET( MagnificationType );
	// skip Smoothing type
	if ( SetBorderDensity ) {
		SET( BorderDensity );
	}
	if ( SetEmptyImageDensity ) {
		SET( EmptyImageDensity );
	}
	// skip Min Density
	// skip Max Density
	if ( SetTrim ) {
		SET( Trim );
	}
	// skip Configuration Information
	// skip Illumination
	// skip Reflected Ambient Light
	if ( SetRequestedResolutionId ) {
		SET( RequestedResolutionId );
	}

#undef SET 


	// Now take care of the Basic Film Box Relationship module. We're only
	// going to populate the Referenced Film Session Sequence with session UID.
	DcmSequenceOfItems * sequence = createReference(
		QDicomTag::ReferencedFilmSessionSequence,
		QSopClass::BasicFilmSession,
		sessionUid_
	);
	attributes.dcmDataset().insert( sequence, true );


	Dataset result;
	const QByteArray FilmBoxUid = 
		nCreate( QSopClass::BasicFilmBox, attributes, &result )
	;
	if ( FilmBoxUid.size() > 0 ) {

		const QByteArray ImageBoxUid = readReferencedInstance(
			QDicomTag::ReferencedImageBoxSequence, result 
		);

		if ( ImageBoxUid.size() > 0 ) {
			setFilmBoxUid( FilmBoxUid );
			setImageBoxUid( ImageBoxUid );

			return true;
		}
		else {
			raiseError(
				QString( "Unable to read image box SOP instance from\n%1")
				.arg( result.toString() )
			);
		}
	}

	return false;
}


DcmSequenceOfItems * QDicomPrintEngine::Scu::createReference(
	const QDicomTag & Tag, const QSopClass & SopClass, const QByteArray & Uid
) {
	// Now take care of the Basic Film Box Relationship module. We're only
	// going to populate the Referenced Film Session Sequence with session UID.
	DcmItem * item = new DcmItem();

	static const QDicomTag ReferencedSopClassUidTag = 
		QDicomTag::ReferencedSopClassUid
	;
	item->putAndInsertString(
		ReferencedSopClassUidTag.toDcmTag(), SopClass.uid()
	);


	static const QDicomTag ReferencedSopInstanceUidTag = 
		QDicomTag::ReferencedSopInstanceUid
	;
	item->putAndInsertString(
		ReferencedSopInstanceUidTag.toDcmTag(), Uid
	);

	DcmSequenceOfItems * sequence = new DcmSequenceOfItems( Tag );

	sequence->insert( item );
	return sequence;
}


bool QDicomPrintEngine::Scu::createSession() {
	Q_ASSERT( association().isEstablished() );

	const QDicomPrinter & Printer = printer();
	const QDicomPrinterDriver & Driver = Printer.driver();


	// Prepare string values for all attributes we're going to set
	const QString NumberOfCopies = QString::number(
		::qMax< quint16 >( ::qMin( Printer.copyCount(), Driver.maxCopyCount() ), 1 )
	);
	const QString PrintPriority = "HIGH";
	const QString MediumType = ::MediumTypeString( printer().mediumType() );
	const QString FilmDestination = ::FilmDestinationString(
		printer().filmDestination()
	);
	const QString FilmSessionLabel = 
		Driver.hasFeature( QDicomPrinterDriver::FilmSessionLabel ) ?
		QString( "QT_DICOM_ON_%1" )
		.arg( QDateTime::currentDateTime().toString( "yyyyMMdd_hhmmss" ) ) :
		""
	;


	// Now set the attributes
	Dataset attributes;

	qDebug( "Preparing Basic Film Session:" );
#define SET( ATTRIBUTE ) { \
		const QDicomTag Tag( QDicomTag::ATTRIBUTE ); \
		qDebug( "\t: %s = %s", qPrintable( Tag.keyword() ), qPrintable( ATTRIBUTE ) ); \
		attributes.setAttribute( Tag, ATTRIBUTE ); \
	}

	SET( NumberOfCopies );
	SET( PrintPriority );
	SET( MediumType );
	SET( FilmDestination );

	if ( Driver.hasFeature( QDicomPrinterDriver::FilmSessionLabel ) ) {
		SET( FilmSessionLabel );
	}
#undef SET


	// And perforn N-CREATE
	const QByteArray Uid = nCreate( QSopClass::BasicFilmSession, attributes );
	if ( Uid.size() > 0 ) {
		setSessionUid( Uid );
		return true;
	}

	return false;
}


void QDicomPrintEngine::Scu::endSession() {
	Q_ASSERT( isOpen() );

	if ( association().release() ) {
		return;
	}
	else {
		qWarning( qPrintable( association().errorMessage() ) );
	}

	printer_ = nullptr;
}


inline const QByteArray & QDicomPrintEngine::Scu::filmBoxUid() const {
	return filmBoxUid_;
}


inline const QByteArray & QDicomPrintEngine::Scu::imageBoxUid() const {
	return filmBoxUid_;
}


bool QDicomPrintEngine::Scu::isOpen() const {
	return association().isEstablished();
}


DcmSequenceOfItems * QDicomPrintEngine::Scu::prepareImageSequence(
	const QImage & Image, const quint8 & Depth 
) {
	/*Image.save(
		QDir::temp().absoluteFilePath( "image_box.png" )
	);*/
	const bool Is8 = Depth == 8;

	DcmItem * item = new DcmItem();
	OFCondition status;


#define PUT( TYPE, TAG, VALUE ) \
	status = status.good() ? ( item->putAndInsert ## TYPE( \
		QDicomTag( QDicomTag::TAG ).toDcmTag(), VALUE \
	) ) : status

#define PUT_I( TAG, VALUE ) \
	qDebug( "\t: %s = %d", qPrintable( QDicomTag( QDicomTag::TAG ).keyword() ), VALUE ); \
	PUT( Uint16, TAG, VALUE )

#define PUT_S( TAG, VALUE ) \
	qDebug( "\t: %s = %s", qPrintable( QDicomTag( QDicomTag::TAG ).keyword() ), VALUE ); \
	PUT( String, TAG, VALUE )

	PUT_I( SamplesPerPixel, 1 );
	PUT_S( PhotometricInterpretation, "MONOCHROME2" );
	PUT_I( Rows, Image.height() );
	PUT_I( Columns, Image.width() );
	PUT_I( BitsAllocated, Is8 ? 8 : 16 );
	PUT_I( BitsStored, Depth );
	PUT_I( HighBit, Depth - 1 );
	PUT_S( PixelRepresentation, "0" );

#undef PUT_S
#undef PUT_I
#undef PUT


	if ( status.good() ) {
		const unsigned long Count = Image.width() * Image.height();
		if ( Is8 ) {
			status = item->putAndInsertUint8Array(
				QDicomTag( QDicomTag::PixelData ).toDcmTag(),
				::CreatePixelData< quint8 >( Image, Depth ),
				Count
			);
		}
		else {
			status = item->putAndInsertUint16Array(
				QDicomTag( QDicomTag::PixelData ).toDcmTag(),
				::CreatePixelData< quint16 >( Image, Depth ),
				Count
			);
		}
		qDebug(
			"\t: Pixel Data = <%d bytes>",
			Count * ( Is8 ? 1 : 2 )
		);

		if ( status.good() ) {
			DcmSequenceOfItems * sequence = new DcmSequenceOfItems(
				QDicomTag( QDicomTag::BasicGrayscaleImageSequence ).toDcmTag()
			);
			status = sequence->insert( item );
			if ( status.good() ) {
				return sequence;
			}
			else {
				delete sequence;
			}
		}
	}

	delete item;
	raiseError( status.text() );
	return nullptr;
}


bool QDicomPrintEngine::Scu::printImage( const QImage & Image ) {
	const bool Printed = 
		createFilmBox( Image ) &&
		setImageBox( Image ) &&
		nAction( QSopClass::BasicFilmBox, filmBoxUid(), 1 )
	;

	if ( Printed ) {
		if ( nDelete( QSopClass::BasicFilmBox, filmBoxUid() ) ) {
			return true;
		}
		else {
			qWarning( qPrintable( errorString() ) );
			clearErrorStatus();
		}
		return true;
	}

	return false;
}


inline const QDicomPrinter & QDicomPrintEngine::Scu::printer() const {
	Q_ASSERT( printer_ != nullptr );

	return *printer_;
}


QByteArray QDicomPrintEngine::Scu::readReferencedInstance(
	const QDicomTag & SequenceTag, const Dicom::Dataset & DataSet
) {
	// ... fucking DCMTK, you can't even search without modifying an object
	DcmDataset & dcmDataSet = const_cast< DcmDataset & >( DataSet.dcmDataset() );

	DcmStack stack;
	OFCondition status = dcmDataSet.search( SequenceTag.toDcmTag(), stack );

	unsigned long count = stack.card();
	Q_ASSERT( count == 1 );
	if ( count > 0 ) {
		// We found the sequence, cool. Now look for the instance
		DcmSequenceOfItems * sequence =
			static_cast< DcmSequenceOfItems * >( stack.top() )
		;
		Q_ASSERT( sequence != nullptr && sequence->card() == 1 );
		if ( sequence->card() > 0 ) {
			DcmItem * item = sequence->getItem( 0 );
			Q_ASSERT( item != nullptr );

			static const QDicomTag ReferencedSopInstanceUid =
				QDicomTag::ReferencedSopInstanceUid
			;

			stack.clear();
			status = item->search(
				ReferencedSopInstanceUid, stack, ESM_fromHere, false
			);
			count = stack.card();

			Q_ASSERT( count == 1 );
			if ( count > 0 ) {
				DcmUniqueIdentifier * uid =
					static_cast< DcmUniqueIdentifier * >( stack.top() )
				;

				char * value = nullptr;
				status = uid->getString( value );
				if ( value != nullptr ) {
					return value;
				}
				else {
					qWarning( status.text() );
				}
			}
			else if ( status.bad() ) {
				qWarning( status.text() );
			}
		}
	}
	else if ( status.bad() ) {
		qWarning( status.text() );
	}

	return QByteArray();
}


inline void QDicomPrintEngine::Scu::setFilmBoxUid( const QByteArray & Uid ) {
	filmBoxUid_ = Uid;
}


bool QDicomPrintEngine::Scu::setImageBox( const QImage & Image ) {
	Q_ASSERT( association().isEstablished() && ! hasError() );

	const QDicomPrinter & Printer = printer();
	const QDicomPrinterDriver & Driver = Printer.driver();

	const quint8 & Depth =
		Driver.depths().contains( Printer.depth() ) ?
		Printer.depth() : Driver.depths().first()
	;

	Dicom::Dataset attributes;

	qDebug( "Preparing Basic Grayscale Image Box:" );
	qDebug( "\t: Image Position = 1" );
	attributes.setAttribute( QDicomTag::ImagePosition, "1" );
	
	DcmSequenceOfItems * sequence = prepareImageSequence( Image, Depth );
	if ( sequence != nullptr ) {
		attributes.dcmDataset().insert( sequence );

		nSet(
			QSopClass( QSopClass::BasicGrayscaleImageBox ).uid(),
			imageBoxUid(), attributes
		);
		return true;
	}
	else {
		return false;
	}
}


inline void QDicomPrintEngine::Scu::setImageBoxUid( const QByteArray & Uid ) {
	filmBoxUid_ = Uid;
}


inline void QDicomPrintEngine::Scu::setPrinter( const QDicomPrinter & Printer ) {
	Q_ASSERT( printer_ == nullptr );

	printer_ = &Printer;
}


inline void QDicomPrintEngine::Scu::setSessionUid( const QByteArray & Uid ) {
	sessionUid_ = Uid;
}


inline const QByteArray & QDicomPrintEngine::Scu::sessionUid() const {
	return sessionUid_;
}


template < int Bits >
struct Mask {
	static const int Value = ( 1 << ( Bits - 1 ) ) | Mask< Bits - 1 >::Value;
};

template <>
struct Mask< 0 > {
	static const int Value = 0;
};


template < int Depth >
inline int ConvertPixel( const QRgb & Pixel ) {
	const int Gray = (
		 ::qRed(   Pixel ) * 13932 + // = 0.2126 + 
		 ::qGreen( Pixel ) * 46872 + // = 0.7152 + 
		 ::qBlue(  Pixel ) *  4732   // = 0.0722
	) >> 8; // ( / 256)

	// `Gray' contains a 16-bit gray

	const int Result =
		( Gray >> ( 16 - Depth ) ) |
		( Gray >> ( 24 - Depth ) )
	;
	Q_ASSERT( Result >= 0 && Result < ( 1 << Depth ) );

	return Result;
}


template < typename Type >
Type * CreatePixelData( const QImage & Image, const quint8 & Depth ) {
	int ( * ConvertPixel)( const QRgb & ) = nullptr;

	switch ( Depth ) {
#define CASE( VAL ) \
		case VAL : \
			ConvertPixel = ::ConvertPixel< VAL >; \
			break

		CASE(  8 );
		CASE(  9 );
		CASE( 10 );
		CASE( 11 );
		CASE( 12 );
		CASE( 13 );
		CASE( 14 );
		CASE( 15 );
		CASE( 16 );
#undef CASE
	}

	Q_ASSERT( ConvertPixel != nullptr );

	const int Width = Image.width();
	const int Height = Image.height();

	Type * data = new Type [ Width * Height ];

	for ( int row = 0; row < Height; ++row ) {
		const QRgb * Line = reinterpret_cast< const QRgb * >( Image.scanLine( row ) );

		for ( int col = 0; col < Width; ++col ) {
			data[ row * Width + col ] = ConvertPixel( Line[ col ] );
		}
	}

	return data;
}


const QString & FilmDestinationString(
	const QDicomPrinter::FilmDestination & Destination
) {
	static QVector< QString > * names = nullptr;

	::qInitializeOnce( names, ( []() throw() -> QVector< QString > {
		static const int Last = QDicomPrinter::Bin_10;

		QVector< QString > result( Last + 1 );

#define SET( LABEL, VALUE ) { \
			static const int Index = QDicomPrinter::LABEL; \
			Q_ASSERT( Index <= Last ); \
			result[ Index ] = ::WArrayString( VALUE ); \
		}

		SET( Magazine,  L"MAGAZINE" )
		SET( Processor, L"PROCESSOR" )
		SET( Bin_1,     L"BIN_1" );
		SET( Bin_2,     L"BIN_2" );
		SET( Bin_3,     L"BIN_3" );
		SET( Bin_4,     L"BIN_4" );
		SET( Bin_5,     L"BIN_5" );
		SET( Bin_6,     L"BIN_6" );
		SET( Bin_7,     L"BIN_7" );
		SET( Bin_8,     L"BIN_8" );
		SET( Bin_9,     L"BIN_9" );
		SET( Bin_10,    L"BIN_10" );
#undef SET

		return result;
	} ) );

	Q_ASSERT( names != nullptr );
	Q_ASSERT( Destination < names->size() );

	return names->at( Destination );
}


const QString & FilmSizeString( const QDicomPrinter::FilmSize & Film ) {
	static QVector< QString > * values = nullptr;

	::qInitializeOnce( values, ( []() throw() -> QVector< QString > {
		static const int Last = QDicomPrinter::IN_14x17;

		QVector< QString > result( Last + 1 );


#define SET( LABEL, VALUE ) \
		Q_ASSERT( QDicomPrinter::LABEL <= Last ); \
		result[ QDicomPrinter::LABEL ] = WArrayString( VALUE )

		SET( A3,           L"A3" );
		SET( A4,           L"A4" );
		SET( CM_24x24,     L"24CMX24CM" );
		SET( CM_24x30,     L"24CMX30CM" );
		SET( CM_25_7x36_4, L"25.7CMX36.4CM" );
		SET( IN_8x10,      L"8INX10IN" );
		SET( IN_8_5x11,    L"8.5INX11IN" );
		SET( IN_10x12,     L"10INX12IN" );
		SET( IN_10x14,     L"10INX14IN" );
		SET( IN_11x14,     L"11INX14IN" );
		SET( IN_11x17,     L"11INX17IN" );
		SET( IN_14x14,     L"14INX14IN" );
		SET( IN_14x17,     L"14INX17IN" ); 
#undef SET

		return result;
	} ) );

	Q_ASSERT( values != nullptr );
	Q_ASSERT( Film < values->size() );

	return values->at( Film );
}


const QString & MagnificationTypeString(
	const QDicomPrinter::MagnificationType & Type 
) {
	static QVector< QString > * values = nullptr;

	::qInitializeOnce( values, ( []() throw() -> QVector< QString > {
		static const int Last = QDicomPrinter::Cubic;

		QVector< QString > result( Last + 1 );


#define SET( LABEL, VALUE ) \
		Q_ASSERT( QDicomPrinter::LABEL <= Last ); \
		result[ QDicomPrinter::LABEL ] = ::WArrayString( VALUE )

		SET( None,      L"NONE" );
		SET( Replicate, L"REPLICATE" );
		SET( Bilinear,  L"BILINEAR" );
		SET( Cubic,     L"CUBIC" );
#undef SET

		return result;
	} ) );

	Q_ASSERT( values != nullptr );
	Q_ASSERT( Type < values->size() );

	return values->at( Type );
}


const QString & MediumTypeString(
	const QDicomPrinter::MediumType & Medium
) {
	static QVector< QString > * values = nullptr;

	::qInitializeOnce( values, ( []() throw() -> QVector< QString > {
		static const int Last = QDicomPrinter::Paper;

		QVector< QString > result( Last + 1 );

#define SET( LABEL, VALUE ) \
		Q_ASSERT( QDicomPrinter::LABEL <= Last ); \
		result[ QDicomPrinter::LABEL ] = ::WArrayString( VALUE ) 

		SET( Paper,          L"PAPER" );
		SET( BlueFilm,       L"BLUE FILM" );
		SET( ClearFilm,      L"CLEAR FILM" );
		SET( MammoBlueFilm,  L"MAMMO BLUE FILM" );
		SET( MammoClearFilm, L"MAMMO CLEAR FILM" );
#undef SET

		return result;
	} ) );

	Q_ASSERT( values != nullptr );
	Q_ASSERT( Medium < values->size() );

	return values->at( Medium );
}


const QString & OrientationString(
	const QDicomPrinter::Orientation & Orientation
) {
	Q_ASSERT(
		Orientation == QDicomPrinter::VerticalOrientation ||
		Orientation == QDicomPrinter::HorizontalOrientation
	);

	static QVector< QString > * values = nullptr;

	::qInitializeOnce( values, ( []() throw() -> QVector< QString > {
		static const int Last = QDicomPrinter::HorizontalOrientation;

		QVector< QString > result( Last + 1 );


#define SET( LABEL, VALUE ) \
		Q_ASSERT( QDicomPrinter::LABEL ## Orientation <= Last ); \
		result[ QDicomPrinter::LABEL ## Orientation ] = ::WArrayString( VALUE )

		SET( Vertical,   L"PORTRAIT" );
		SET( Horizontal, L"LANDSCAPE" );
#undef SET

		return result;
	} ) );

	Q_ASSERT( values != nullptr );
	Q_ASSERT( Orientation < values->size() );

	return values->at( Orientation );
}


#ifdef _DEBUG

// Color conversion methods are static so we have to test them here
bool TestQDicomPrintEngineScu_ConvertPixel() {
	static const QRgb White = QColor( Qt::white ).rgb();
	Q_ASSERT( ::ConvertPixel<  8 >( White ) == 0x00FF );
	Q_ASSERT( ::ConvertPixel<  9 >( White ) == 0x01FF );
	Q_ASSERT( ::ConvertPixel< 10 >( White ) == 0x03FF );
	Q_ASSERT( ::ConvertPixel< 11 >( White ) == 0x07FF );
	Q_ASSERT( ::ConvertPixel< 12 >( White ) == 0x0FFF );
	Q_ASSERT( ::ConvertPixel< 13 >( White ) == 0x1FFF ); 
	Q_ASSERT( ::ConvertPixel< 14 >( White ) == 0x3FFF );
	Q_ASSERT( ::ConvertPixel< 15 >( White ) == 0x7FFF );
	Q_ASSERT( ::ConvertPixel< 16 >( White ) == 0xFFFF );

	static const QRgb Black = QColor( Qt::black ).rgb();
	Q_ASSERT( ::ConvertPixel<  8 >( Black ) == 0x0 );
	Q_ASSERT( ::ConvertPixel<  9 >( Black ) == 0x0 );
	Q_ASSERT( ::ConvertPixel< 10 >( Black ) == 0x0 );
	Q_ASSERT( ::ConvertPixel< 11 >( Black ) == 0x0 );
	Q_ASSERT( ::ConvertPixel< 12 >( Black ) == 0x0 );
	Q_ASSERT( ::ConvertPixel< 13 >( Black ) == 0x0 ); 
	Q_ASSERT( ::ConvertPixel< 14 >( Black ) == 0x0 );
	Q_ASSERT( ::ConvertPixel< 15 >( Black ) == 0x0 );
	Q_ASSERT( ::ConvertPixel< 16 >( Black ) == 0x0 );

	static const QRgb Red = QColor( Qt::red ).rgb();
	Q_ASSERT( ::ConvertPixel<  8 >( Red ) == 0x0036 );
	Q_ASSERT( ::ConvertPixel< 12 >( Red ) == 0x0363 );
	Q_ASSERT( ::ConvertPixel< 16 >( Red ) == 0x3637 );

	static const QRgb Green = QColor( Qt::green ).rgb();
	Q_ASSERT( ::ConvertPixel<  8 >( Green ) == 0x00B6 );
	Q_ASSERT( ::ConvertPixel< 12 >( Green ) == 0x0B6F );
	Q_ASSERT( ::ConvertPixel< 16 >( Green ) == 0xB6F6 );

	static const QRgb Blue = QColor( Qt::blue ).rgb();
	Q_ASSERT( ::ConvertPixel<  8 >( Blue ) == 0x0012 );
	Q_ASSERT( ::ConvertPixel< 12 >( Blue ) == 0x0127 );
	Q_ASSERT( ::ConvertPixel< 16 >( Blue ) == 0x127B );

	return true;
}
#endif // _DEBUG
