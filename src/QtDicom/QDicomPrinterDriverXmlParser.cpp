/***************************************************************************
 *   Copyright © 2013 by Flux Inc.                                         *
 *   Author: Pawel Zak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#include "QDicomPrinterDriverXmlParser.hpp"
#include "Utilities.hpp"

#include <QtCore/QHash>
#include <QtCore/QMap>
#include <QtCore/QStringList>
#include <QtCore/QXmlStreamReader>

#include <QtFlux/QInitializeOnce>



// Local method declarations //

/**
 * Converts a string \a value into a QDicomPrinterDriver::Feature identifier.
 * In case of an error \c -1 is returned.
 */
static int FeatureFromString( const QString & value );

/**
 * Returns the name of the QDicomPrinterDriver::Feature \a feature.
 */
static const QString & FeatureString(
	const QDicomPrinterDriver::Feature & feature
);


/**
 * Converts a string \a value into a QDicomPrinter::FilmDestination identifier. In 
 * case of an error \c -1 is returned.
 */
static int FilmDestinationFromString( const QString & value );

/**
 * Returns name of the QDicomprinter::FilmDestination \a dst.
 */
static const QString & FilmDestinationString(
	const QDicomPrinter::FilmDestination & dst
);


/**
 * Converts a string \a value into a QDicomPrinter::FilmSize identifier. In 
 * case of an error \c -1 is returned.
 */
static int FilmSizeFromString( const QString & value );

/**
 * Returns name of the QDicomprinter::FilmSize \a size.
 */
static const QString & FilmSizeString( const QDicomPrinter::FilmSize & size );


/**
 * Converts a string \a value into a QDicomPrinter::MagnificationType identifier. In 
 * case of an error \c -1 is returned.
 */
static int MagnificationTypeFromString( const QString & value );

/**
 * Returns name of the QDicomprinter::MagnificationType \a type.
 */
static const QString & MagnificationTypeString(
	const QDicomPrinter::MagnificationType & type
);


/**
 * Converts a string \a value into a QDicomPrinter::MediumType identifier. In 
 * case of an error \c -1 is returned.
 */
static int MediumTypeFromString( const QString & value );

/**
 * Returns name of the QDicomprinter::MediumType \a type.
 */
static const QString & MediumTypeString( const QDicomPrinter::MediumType & type );

/**
 * Removes all white spaces and underscores from \a Value, converting all
 * its characters to lower case.
 */
static QString Normalized( const QString & Value );


/**
 * Returns the pattern used to parse printable area specification:
 *
 *   <film size>: <width> x <height>
 */
static QRegExp PrintableAreaPattern();


// Class methods definitions //
QDicomPrinterDriver::XmlParser::XmlParser(
	QXmlStreamReader & stream, QDicomPrinterDriver & driver 
) :
	driver_( driver ),
	stream_( stream )
{
}


int QDicomPrinterDriver::XmlParser::documentVersionFromString(
	const QStringRef & Value
) {
	int result = -1;

	const QRegExp Pattern( "(\\d+)\\.(\\d+)" );
	if ( Pattern.indexIn( Value.toString() ) > -1 ) {
		bool ok = false;
		const int Major = Pattern.cap( 1 ).toUInt( &ok ); Q_ASSERT( ok );
		const int Minor = Pattern.cap( 2 ).toUInt( &ok ); Q_ASSERT( ok );

#define SET_VERSION( A, B ) \
	Q_ASSERT( Version_ ## A ## _ ## B > -1 ); \
	if ( Major == A && Minor == B ) result = Version_ ## A ## _ ## B

		SET_VERSION( 0, 1 );
#undef SET_VERSION

	}

	return result;
}


QDicomPrinterDriver & QDicomPrinterDriver::XmlParser::driver() {
	return driver_;
}


QDicomPrinterDriver QDicomPrinterDriver::XmlParser::process(
	QXmlStreamReader & stream, QString * message
) {
	static const char * RootElement = "QDicomPrinterDriver";
	if ( stream.readNextStartElement() && stream.name() == RootElement ) {
		QDicomPrinterDriver driver;

		const int Version = documentVersionFromString(
			stream.attributes().value( "version" )
		);

		switch ( Version ) {

		case Version_0_1 : {
			XmlParser parser( stream, driver );
			parser.readQDicomPrinterDriver_1();
			break;
		}

		default :
			stream.raiseError( 
				QString( "Invalid or missing version attribute; `%1' read" )
				.arg( stream.attributes().value( "version" ).toString() )
			);
		}

		if ( ! stream.hasError() ) {
			return driver;
		}
	}
	else {
		stream.raiseError(
			QString( "Missing or invalid document root: the `%1' element" )
			.arg( RootElement )
		);
	}

	Q_ASSERT( stream.hasError() );
	if ( message != nullptr ) {
		*message = QString( "%1 at %2:%3" )
			.arg( QString( stream.errorString() ).remove( QRegExp( "[.]+$" ) ) )
			.arg( stream.lineNumber() )
			.arg( stream.columnNumber() )
		;
	}

	return QDicomPrinterDriver();
}


void QDicomPrinterDriver::XmlParser::readDepths() {
	Q_ASSERT( ! stream().hasError() );
	Q_ASSERT( stream().isStartElement() );
	Q_ASSERT( stream().name() == "Depths" );

	QList< quint8 > tmp;

	const QStringList Items = stream().readElementText().split( ',' );
	for (
		QStringList::const_iterator i = Items.constBegin();
		i != Items.constEnd(); ++i
	) {
		const QString Item = ::Normalized( *i );

		bool ok = false;
		const int Value = Item.toUInt( &ok );
		if ( ok && Value <= 16 ) {
			tmp.append( static_cast< quint8 >( Value ) );
		}
		else {
			stream().raiseError(
				QString( "Unable to read a valid depth from `%1'" )
				.arg( *i )
			);
			return;
		}
	}

	driver().setDepths( tmp );
}


void QDicomPrinterDriver::XmlParser::readFeatures() {
	Q_ASSERT( ! stream().hasError() );
	Q_ASSERT( stream().isStartElement() );
	Q_ASSERT( stream().name() == "Features" );

	QDicomPrinterDriver::Features tmp = 0;

	const QStringList Items = stream().readElementText().split( ',' );

	for (
		QStringList::const_iterator i = Items.constBegin();
		i != Items.constEnd(); ++i
	) {
		const int Value = ::FeatureFromString( ::Normalized( *i ) );
		if ( Value >= 0 ) {
			tmp |= static_cast< QDicomPrinterDriver::Feature >( Value );
		}
		else {
			stream().raiseError(
				QString( "Unable to read a valid feature name from `%1'" )
				.arg( *i )
			);
			return;
		}
	}

	driver().setFeatures( tmp );
}


void QDicomPrinterDriver::XmlParser::readFilmDestinations() {
	Q_ASSERT( ! stream().hasError() );
	Q_ASSERT( stream().isStartElement() );
	Q_ASSERT( stream().name() == "FilmDestinations" );

	QList< QDicomPrinter::FilmDestination > tmp;

	const QStringList Items = stream().readElementText().split( ',' );
	for (
		QStringList::const_iterator i = Items.constBegin();
		i != Items.constEnd(); ++i
	) {
		const int Value = ::FilmDestinationFromString( ::Normalized( *i ) );
		if ( Value >= 0 ) {
			tmp.append( static_cast< QDicomPrinter::FilmDestination >( Value ) );
		}
		else {
			stream().raiseError(
				QString( "Unable to read a valid film destination from `%1'" )
				.arg( *i )
			);
			return;
		}
	}

	driver().setFilmDestinations( tmp );
}


void QDicomPrinterDriver::XmlParser::readFilmSizes() {
	Q_ASSERT( ! stream().hasError() );
	Q_ASSERT( stream().isStartElement() );
	Q_ASSERT( stream().name() == "FilmSizes" );

	QList< QDicomPrinter::FilmSize > tmp;

	const QStringList Items = stream().readElementText().split( ',' );
	for (
		QStringList::const_iterator i = Items.constBegin();
		i != Items.constEnd(); ++i
	) {
		const int Value = ::FilmSizeFromString( ::Normalized( *i ) );
		if ( Value >= 0 ) {
			tmp.append( static_cast< QDicomPrinter::FilmSize >( Value ) );
		}
		else {
			stream().raiseError(
				QString( "Unable to read a valid film size from `%1'" )
				.arg( *i )
			);
			return;
		}
	}

	driver().setFilmSizes( tmp );
}


void QDicomPrinterDriver::XmlParser::readMagnificationTypes() {
	Q_ASSERT( ! stream().hasError() );
	Q_ASSERT( stream().isStartElement() );
	Q_ASSERT( stream().name() == "MagnificationTypes" );

	QList< QDicomPrinter::MagnificationType > tmp;

	const QStringList Items = stream().readElementText().split( ',' );

	for (
		QStringList::const_iterator i = Items.constBegin();
		i != Items.constEnd(); ++i
	) {
		const int Value = ::MagnificationTypeFromString( ::Normalized( *i ) );
		if ( Value >= 0 ) {
			tmp.append( static_cast< QDicomPrinter::MagnificationType >( Value ) );
		}
		else {
			stream().raiseError(
				QString( "Unable to read a magnification type from `%1'" )
				.arg( *i )
			);
			return;
		}
	}

	driver().setMagnificationTypes( tmp );
}


void QDicomPrinterDriver::XmlParser::readMediumTypes() {
	Q_ASSERT( ! stream().hasError() );
	Q_ASSERT( stream().isStartElement() );
	Q_ASSERT( stream().name() == "MediumTypes" );

	QList< QDicomPrinter::MediumType > tmp;

	const QStringList Items = stream().readElementText().split( ',' );

	for (
		QStringList::const_iterator i = Items.constBegin();
		i != Items.constEnd(); ++i
	) {
		const int Value = ::MediumTypeFromString( ::Normalized( *i ) );
		if ( Value >= 0 ) {
			tmp.append( static_cast< QDicomPrinter::MediumType >( Value ) );
		}
		else {
			stream().raiseError(
				QString( "Unable to read a medium type from `%1'" )
				.arg( *i )
			);
			return;
		}
	}

	driver().setMediumTypes( tmp );
}


void QDicomPrinterDriver::XmlParser::readPrintableAreas() {
	Q_ASSERT( ! stream().hasError() );
	Q_ASSERT( stream().isStartElement() );
	Q_ASSERT( stream().name() == "PrintableAreas" );


	QDicomPrinter::Quality quality = QDicomPrinter::NormalQuality;
	while ( stream().readNextStartElement() && ! stream().atEnd() ) {
		const QStringRef Name = stream().name();


		if ( Name == "NormalQuality" ) {
			quality = QDicomPrinter::NormalQuality;
		}
		else if ( Name == "HighQuality" ) {
			quality = QDicomPrinter::HighQuality;
		}
		else {
			stream().raiseError(
				QString( "Unexpected XML element found: `%1'" )
				.arg( Name.toString() )
			);
			break;
		}


		QString message;
		const QStringList Items = stream().readElementText().split( ',' );
		const bool Status = readPrintableAreas_Items(
			Items, quality, message
		);
		if ( ! Status ) {
			stream().raiseError( message );
		}
	}

	if ( ! stream().hasError() ) {
		// Make sure that every film has an area set for both qualities

		const QList< QDicomPrinter::FilmSize > & Sizes = driver().filmSizes();
		for (
			QList< QDicomPrinter::FilmSize >::const_iterator i = Sizes.constBegin();
			i != Sizes.constEnd(); ++i
		) {
			const bool NotCool = 
				driver().printableArea( *i, QDicomPrinter::NormalQuality ).isEmpty()  &&
				driver().printableArea( *i, QDicomPrinter::HighQuality ).isEmpty()
			;

			if ( NotCool ) {
				stream().raiseError(
					QString( "Missing area specification for `%1' film" )
					.arg( ::FilmSizeString( *i ) )
				);
				break;
			}
		}
	}
}



QPair< QDicomPrinter::FilmSize, QSize >
QDicomPrinterDriver::XmlParser::readPrintableAreas_Item(
	const QString & Item, QString & errorString
) {
	const QRegExp Pattern = PrintableAreaPattern();

	if ( Pattern.indexIn( Item ) > -1 ) {
		bool ok = false;

		const int Value = ::FilmSizeFromString( ::Normalized( Pattern.cap( 1 ) ) );
		const int Width  = Pattern.cap( 2 ).toUInt( &ok ); Q_ASSERT( ok );
		const int Height = Pattern.cap( 3 ).toUInt( &ok ); Q_ASSERT( ok );

		if ( Value >= 0 ) {
			const QDicomPrinter::FilmSize & FilmSize = 
				static_cast< QDicomPrinter::FilmSize >( Value )
			; 

			if ( driver().filmSizes().contains( FilmSize ) ) {
				const bool IsVertical =
					( FilmSize != QDicomPrinter::IN_14x14 ) ?
					( Width <= Height ) :
					// For the rectangular film (14" x 14") we allow the width
					// to be up to 6% larger than the height
					( Width - ( Width >> 4 ) ) <= Height 
				;

				if ( IsVertical ) {
					return ::qMakePair( FilmSize, QSize( Width, Height ) );
				}
				else {
					errorString =
						"Invalid printable area specification: "
						"the width must be less or equal to the height"
					;
				}
			}
			else {
				errorString =
					QString( "Unsupported film size encoutered (%1) in printable area" )
					.arg( ::FilmSizeString( FilmSize ) )
				;
			}
		}
		else {
			errorString =
				QString( "Invalid film size identifier in printable area: `%1'" )
				.arg( Item )
			;
		}
	}
	else {
		errorString = 
			QString( "Invalid printable area specification: `%1'" )
			.arg( Item )
		;
	}

	return QPair< QDicomPrinter::FilmSize, QSize >();
}


bool QDicomPrinterDriver::XmlParser::readPrintableAreas_Items(
	const QStringList & Items,
	const QDicomPrinter::Quality & Quality,
	QString & errorString
) {
	QMap< QDicomPrinter::FilmSize, QSize > areas;

	for (
		QStringList::const_iterator i = Items.constBegin();
		i != Items.constEnd(); ++i
	) {
		const QPair< QDicomPrinter::FilmSize, QSize > Pair = 
			readPrintableAreas_Item( *i, errorString )
		;

		if ( Pair.second.isValid() ) {
			areas[ Pair.first ] = Pair.second;
		}
		else {
			Q_ASSERT( errorString.size() > 0 );
			return false;
		}
	}

	driver().setPrintableAreas( Quality, areas );
	if ( Quality == QDicomPrinter::NormalQuality ) {
		// Init high resolution DPI with value from normal quality. They'll be
		// later overwritten if they exist in the XML
		driver().setPrintableAreas( QDicomPrinter::HighQuality, areas );
	}

	return true;
}


void QDicomPrinterDriver::XmlParser::readQDicomPrinterDriver_1() {
	Q_ASSERT( ! stream().hasError() );
	Q_ASSERT( stream().isStartElement() );
	Q_ASSERT( stream().name() == "QDicomPrinterDriver" );

	while ( stream().readNextStartElement() && ! stream().atEnd() ) {
		const QStringRef Name = stream().name();

		// First check the generic properties:
		if ( Name == "VendorName" ) {
			const QString Text = stream().readElementText().trimmed();
			if ( Text.size() > 0 ) {
				driver().setVendorName( Text );
			}
			else {
				stream().raiseError( "Empty vendor name" );
			}
		}
		else if ( Name == "ModelName" ) {
			const QString Text = stream().readElementText().trimmed();
			if ( Text.size() > 0 ) {
				driver().setModelName( Text );
			}
			else {
				stream().raiseError( "Empty model name" );
			}
		}
		else if ( Name == "Features" ) {
			readFeatures();
		}

		// Then film session-related:
		else if ( Name == "FilmDestinations" ) {
			readFilmDestinations();
		}
		else if ( Name == "MaxCopyCount" ) {
			const QString Text = stream().readElementText();
			bool ok = false;
			const unsigned Value = Text.toUInt( &ok );
			if ( ok && Value > 0 && Value < 0x10000 ) {
				driver().setMaxCopyCount( static_cast< quint16 >( Value ) );
			}
			else {
				stream().raiseError(
					QString( "Invalid max copy count value: `%1'" )
					.arg( Text )
				);
			}
		}
		else if ( Name == "MediumTypes" ) {
			readMediumTypes();
		}

		// Afterwards film box-related:
		else if ( Name == "FilmSizes" ) {
			readFilmSizes();
		}
		else if ( Name == "MagnificationTypes" ) {
			readMagnificationTypes();
		}

		// and finally, image box-related
		else if ( Name == "Depths" ) {
			readDepths();
		}
		else if ( Name == "Resolutions" ) {
			readResolutions();
		}
		else if ( Name == "PrintableAreas" ) {
			readPrintableAreas();
		}
		else {
			stream().raiseError(
				QString( "Unexpected XML element found: `%1'" )
				.arg( Name.toString() )
			);
		}
	}
}


void QDicomPrinterDriver::XmlParser::readResolutions() {
	Q_ASSERT( ! stream().hasError() );
	Q_ASSERT( stream().isStartElement() );
	Q_ASSERT( stream().name() == "Resolutions" );

	static const quint16 MaxDpi = 2400;
	quint16 normalDpi = 0, highDpi = 0;

	while ( stream().readNextStartElement() && ! stream().atEnd() ) {
		const QStringRef Name = stream().name();
		const QString Text = stream().readElementText();

		if ( Name == "NormalQuality" ) {
			bool ok = false;

			normalDpi = Text.toUInt( &ok );
			if ( ok && normalDpi <= MaxDpi ) {
				continue;
			}
			else {
				stream().raiseError(
					QString( "Invalid normal resolution value: `%1'" )
					.arg( Text )
				);
			}
		}
		else if ( Name == "HighQuality" ) {
			bool ok = false;

			highDpi = Text.toUInt( &ok );
			if ( ok && highDpi <= MaxDpi ) {
				continue;
			}
			else {
				stream().raiseError(
					QString( "Invalid high resolution value: `%1'" )
					.arg( Text )
				);
			}
		}
		else {
			stream().raiseError(
				QString( "Unexpected XML element found: `%1'" )
				.arg( Name.toString() )
			);
		}
	}

	if ( ! stream().hasError() ) {
		if ( normalDpi > 0 ) {
			driver().setResolutions(
				normalDpi, highDpi > 0 ? highDpi : normalDpi
			);
		}
		else {
			stream().raiseError( "Missing normal resolution value" );
		}
	}
}


QXmlStreamReader & QDicomPrinterDriver::XmlParser::stream() {
	return stream_;
}


// Local methods definitions //

int FeatureFromString( const QString & Value ) {
	int result = -1;

#define COMPARE( LABEL ) \
	else if ( Value == ::FeatureString( QDicomPrinterDriver::LABEL ) ) { \
		Q_ASSERT( QDicomPrinterDriver::LABEL > -1 ); \
		result = QDicomPrinterDriver::LABEL; \
	}

	if ( false ) {}
	COMPARE( FilmSessionLabel )
	COMPARE( BorderDensity )
	COMPARE( EmptyImageDensity )
	COMPARE( Trim )
	COMPARE( HighQuality )
	COMPARE( RequestedImageSize )
#undef COMPARE

	return result;
}


static inline int FeatureIndex( const QDicomPrinterDriver::Feature & Feature ) {
	int index = 0;
	int value = Feature;
	while ( value >>= 1 ) ++index;

	return index;
}


const QString & FeatureString( const QDicomPrinterDriver::Feature & Feature ) {
	static QVector< QString > * names = nullptr;

	::qInitializeOnce( names, ( []() throw() -> QVector< QString > {
		static const int Last =
			::FeatureIndex( QDicomPrinterDriver::RequestedImageSize )
		;

		QVector< QString > result( Last + 1 );

#define SET( LABEL, VALUE ) { \
			const int Index = ::FeatureIndex( QDicomPrinterDriver::LABEL ); \
			Q_ASSERT( Index <= Last ); \
			result[ Index ] = ::WArrayString( VALUE ); \
		}

		SET( FilmSessionLabel,   L"filmsessionlabel" )
		SET( BorderDensity,      L"borderdensity" )
		SET( EmptyImageDensity,  L"emptyimagedensity" )
		SET( Trim,               L"trim" )
		SET( HighQuality,        L"highquality" )
		SET( RequestedImageSize, L"requestedimagesize" )
#undef SET

		return result;
	} ) );

	Q_ASSERT( names != nullptr );
	
	const int Index = ::FeatureIndex( Feature );
	Q_ASSERT( Index < names->size() );

	return names->at( Index );
}


int FilmDestinationFromString( const QString & Value ) {
	static QHash< QString, int > * destinations = nullptr;

	::qInitializeOnce( destinations, ( []() throw() -> QHash< QString, int > {
		QHash< QString, int > result;

#define SET( LABEL ) \
	result[ ::FilmDestinationString( QDicomPrinter::LABEL ) ] = QDicomPrinter::LABEL 

		SET( Processor );
		SET( Magazine );
		SET( Bin_1 );
		SET( Bin_2 );
		SET( Bin_3 );
		SET( Bin_4 );
		SET( Bin_5 );
		SET( Bin_6 );
		SET( Bin_7 );
		SET( Bin_8 );
		SET( Bin_9 );
		SET( Bin_10 );
#undef SET

		return result;

	} ) );

	Q_ASSERT( destinations != nullptr );

	return destinations->value( Value, -1 );
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

		SET( Magazine,  L"magazine" )
		SET( Processor, L"processor" )
		SET( Bin_1,     L"bin1" );
		SET( Bin_2,     L"bin2" );
		SET( Bin_3,     L"bin3" );
		SET( Bin_4,     L"bin4" );
		SET( Bin_5,     L"bin5" );
		SET( Bin_6,     L"bin6" );
		SET( Bin_7,     L"bin7" );
		SET( Bin_8,     L"bin8" );
		SET( Bin_9,     L"bin9" );
		SET( Bin_10,    L"bin10" );
#undef SET

		return result;
	} ) );

	Q_ASSERT( names != nullptr );
	Q_ASSERT( Destination < names->size() );

	return names->at( Destination );
}


int FilmSizeFromString( const QString & Value ) {
	static QHash< QString, int > * filmSizes = nullptr;


	::qInitializeOnce( filmSizes, []() throw() -> QHash< QString, int > {
		QHash< QString, int > result;

#define SET( LABEL ) \
	result[ ::FilmSizeString( QDicomPrinter::LABEL ) ] = QDicomPrinter::LABEL

		SET( A3 );
		SET( A4 );
		SET( CM_24x24 );
		SET( CM_24x30 );
		SET( CM_25_7x36_4 );
		SET( IN_8x10 );
		SET( IN_8_5x11 );
		SET( IN_10x12 );
		SET( IN_10x14 );
		SET( IN_11x14 );
		SET( IN_11x17 );
		SET( IN_14x14 );
		SET( IN_14x17 );
#undef SET

		return result;
	} );

	Q_ASSERT( filmSizes != nullptr );

	return filmSizes->value( Value, -1 );
}


const QString & FilmSizeString( const QDicomPrinter::FilmSize & Film ) {
	static QVector< QString > * names = nullptr;

	::qInitializeOnce( names, ( []() throw() -> QVector< QString > {
		static const int Last = QDicomPrinter::IN_14x17;

		QVector< QString > result( Last + 1 );

#define SET( LABEL, VALUE ) \
		Q_ASSERT( QDicomPrinter::LABEL <= Last ); \
		result[ QDicomPrinter::LABEL ] = ::WArrayString( VALUE )

		SET( A3,           L"a3" );
		SET( A4,           L"a4" );
		SET( CM_24x24,     L"24cmx24cm" );
		SET( CM_24x30,     L"24cmx30cm" );
		SET( CM_25_7x36_4, L"257cmx364cm" );
		SET( IN_8x10,      L"8inx10in" );
		SET( IN_8_5x11,    L"85inx11in" );
		SET( IN_10x12,     L"10inx12in" );
		SET( IN_10x14,     L"10inx14in" );
		SET( IN_11x14,     L"11inx14in" );
		SET( IN_11x17,     L"11inx17in" );
		SET( IN_14x14,     L"14inx14in" );
		SET( IN_14x17,     L"14inx17in" ); 
#undef SET

		return result;
	} ) );

	Q_ASSERT( names != nullptr );
	Q_ASSERT( Film < names->size() );

	return names->at( Film );
}


int MagnificationTypeFromString( const QString & Value ) {
	int result = -1;

#define COMPARE( LABEL ) \
	else if ( Value == ::MagnificationTypeString( QDicomPrinter::LABEL ) ) { \
		Q_ASSERT( QDicomPrinter::LABEL > -1 ); \
		result = QDicomPrinter::LABEL; \
	}

	if ( false ) {}
	COMPARE( None )
	COMPARE( Replicate )
	COMPARE( Bilinear )
	COMPARE( Cubic )
#undef COMPARE

	return result;
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

		SET( None,      L"none" );
		SET( Replicate, L"replicate" );
		SET( Bilinear,  L"bilinear" );
		SET( Cubic,     L"cubic" );
#undef SET

		return result;
	} ) );

	Q_ASSERT( values != nullptr );
	Q_ASSERT( Type < values->size() );

	return values->at( Type );
}


int MediumTypeFromString( const QString & Value ) {
	int result = -1;

#define COMPARE( LABEL ) \
	else if ( Value == ::MediumTypeString( QDicomPrinter::LABEL ) ) { \
		Q_ASSERT( QDicomPrinter::LABEL > -1 ); \
		result = QDicomPrinter::LABEL; \
	}

	if ( false ) {}
	COMPARE( BlueFilm )
	COMPARE( ClearFilm )
	COMPARE( MammoBlueFilm )
	COMPARE( MammoClearFilm )
	COMPARE( Paper )
#undef COMPARE

	return result;
}


const QString & MediumTypeString(
	const QDicomPrinter::MediumType & Type 
) {
	static QVector< QString > * values = nullptr;

	::qInitializeOnce( values, ( []() throw() -> QVector< QString > {
		static const int Last = QDicomPrinter::Paper;

		QVector< QString > result( Last + 1 );

#define SET( LABEL, VALUE ) \
		Q_ASSERT( QDicomPrinter::LABEL <= Last ); \
		result[ QDicomPrinter::LABEL ] = ::WArrayString( VALUE )

		SET( Paper,          L"paper" );
		SET( ClearFilm,      L"clearfilm" );
		SET( BlueFilm,       L"bluefilm" );
		SET( MammoClearFilm, L"mammoclearfilm" );
		SET( MammoBlueFilm,  L"mammobluefilm" );
#undef SET

		return result;
	} ) );

	Q_ASSERT( values != nullptr );
	Q_ASSERT( Type < values->size() );

	return values->at( Type );
}


inline QString Normalized( const QString & Value ) {
	return QString( Value ).remove( QRegExp( "[\\s_]" ) ).toLower();
}


inline static QRegExp PrintableAreaPattern() {
	static QRegExp * pattern = nullptr;
	
	::qInitializeOnce(
		pattern, "([0-9A-Za-z_]+)\\s*:\\s*(\\d+)\\s*x?\\s*(\\d+)"
	);

	Q_ASSERT( pattern != nullptr );
	Q_ASSERT( pattern->indexIn( "8INX10IN: 100 x 100" ) > -1 );
	Q_ASSERT( pattern->indexIn( "10inx11in:100x100" ) > -1 );
	Q_ASSERT( pattern->indexIn( "   20cmx15cm   :   100  x  100  " ) > -1 );

	return *pattern;
}
