/***************************************************************************
 *   Copyright © 2013 by Flux Inc.                                         *
 *   Author: Pawel Zak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#include "QDicomPrinterDriver.hpp" 
#include "QDicomPrinterDriverXmlParser.hpp" 
#include "Utilities.hpp"

#include <algorithm>

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QMap>
#include <QtCore/QXmlStreamReader>

#include <QtFlux/QInitializeOnce>


// Helper methods declarations //

/**
 * Returns offset within the Data::PrintableAreas table where printable area for
 * particular film size and quality is stored.
 */
static int PrintableAreasTableOffset(
	const QDicomPrinter::FilmSize &,
	const QDicomPrinter::Quality &
);

/**
 * Returns offset within the Data::Resolutions table where resolution for
 * particular quality setting is stored.
 */
static int ResolutionsTableOffset( const QDicomPrinter::Quality & );


// Class methods //

QDicomPrinterDriver::QDicomPrinterDriver() :
	d_( new Data() )
{
}


QMap< QDicomPrinter::Device, QDicomPrinterDriver > &
QDicomPrinterDriver::availableDirvers()
{
	static QMap< QDicomPrinter::Device, QDicomPrinterDriver > * map = nullptr;

	return ::qInitializeOnce( map,

		( []() throw() -> QMap< QDicomPrinter::Device, QDicomPrinterDriver > {
			QMap< QDicomPrinter::Device, QDicomPrinterDriver > map;

	#define ADD( DEVICE ) { \
				QString message; \
				const QDicomPrinterDriver Driver = QDicomPrinterDriver::fromXmlFile( \
					":/QtDicom/QDicomPrinterDrivers/" ## #DEVICE ## ".xml", &message \
				); \
				if ( Driver.isValid() ) \
					map[ QDicomPrinter::DEVICE ] = Driver; \
				else  \
					qWarning( "Invalid driver " ## #DEVICE ## "; %s", qPrintable( message ) );\
			}

#ifdef _DEBUG
			ADD( Debug );
#endif

			ADD( Agfa_DRYSTAR_5300 );
			ADD( Agfa_DRYSTAR_5302 );
			ADD( Agfa_DRYSTAR_5500 );
			ADD( Agfa_DRYSTAR_5503 );
			ADD( Agfa_DRYSTAR_AXYS );

			ADD( Carestream_DryView_5700 );
			ADD( Carestream_DryView_5800 );
			ADD( Carestream_DryView_5850 );
			ADD( Carestream_DryView_5950 );
			ADD( Carestream_DryView_6800 );
			ADD( Carestream_DryView_6850 );
			ADD( Carestream_DryView_8150 );
			ADD( Carestream_DryView_8900 );
			ADD( Carestream_DryView_Chroma );

			ADD( Codonics_Horizon_v2 );

			ADD( Fuji_DRYPIX_1000 );
			ADD( Fuji_DRYPIX_2000 );
			ADD( Fuji_DRYPIX_3000 );
			ADD( Fuji_DRYPIX_4000 );
			ADD( Fuji_DRYPIX_5000 );
			ADD( Fuji_DRYPIX_7000 );
			ADD( Fuji_DRYPIX_FMDPL );

			ADD( Konica_Minolta_Drypro_722 );
			ADD( Konica_Minolta_Drypro_751 );
			ADD( Konica_Minolta_Drypro_752 );
			ADD( Konica_Minolta_Drypro_771 );
			ADD( Konica_Minolta_Drypro_793 );
			ADD( Konica_Minolta_Drypro_832 );
			ADD( Konica_Minolta_Drypro_873 );

			ADD( Sony_UP_970AD );
			ADD( Sony_UP_990AD );
			ADD( Sony_UP_D72XR );
			ADD( Sony_UP_D74XRD );
			ADD( Sony_UP_D77MD );
			ADD( Sony_UP_D897 );
			ADD( Sony_UP_DF500 );
			ADD( Sony_UP_DF550 );
			ADD( Sony_UP_DF750 );
	#undef ADD


			return map;
		} )
	);
}


const QDicomPrinterDriver::Data & QDicomPrinterDriver::data() const {
	Q_ASSERT( d_ != nullptr );

	return *d_;
}


QDicomPrinterDriver::Data & QDicomPrinterDriver::data() {
	Q_ASSERT( d_ != nullptr );

	return *d_;
}


const QList< quint8 > & QDicomPrinterDriver::depths() const {
	return data().Depths;
}


const QDicomPrinterDriver::Features & QDicomPrinterDriver::features() const {
	return data().Features;
}


const QList< QDicomPrinter::FilmDestination > &
QDicomPrinterDriver::filmDestinations() const
{
	return data().FilmDestinations;
}


const QList< QDicomPrinter::FilmSize > & QDicomPrinterDriver::filmSizes() const {
	return data().FilmSizes;
}


QDicomPrinterDriver QDicomPrinterDriver::forDevice(
	const QDicomPrinter::Device & Device
) {
	return availableDirvers().value( Device, QDicomPrinterDriver() );
}


QDicomPrinterDriver QDicomPrinterDriver::fromXml(
	QXmlStreamReader & stream, QString * message
) {
	return XmlParser::process( stream, message );
}


QDicomPrinterDriver QDicomPrinterDriver::fromXml(
	const QByteArray & Data, QString * message
) {
	QXmlStreamReader stream( Data );

	return fromXml( stream, message );

}


QDicomPrinterDriver QDicomPrinterDriver::fromXmlFile(
	const QString & Path, QString * message
) {
	Q_ASSERT( QFile::exists( Path ) );

	QFile file( Path );
	if ( file.open( QIODevice::ReadOnly ) ) {
		const QByteArray Data = file.readAll();

		return fromXml( Data, message );
	}
	else {
		*message =
			QString( "Unable to open file: `%1'" )
			.arg( QDir::toNativeSeparators( Path ) )
		;
	}

	return QDicomPrinterDriver();
}


bool QDicomPrinterDriver::hasFeature( const Feature & Feature ) const {
	return features().testFlag( Feature );
}


bool QDicomPrinterDriver::isNull() const {
	return 
		depths().isEmpty() &&
		features() == 0 &&
		filmDestinations().isEmpty() &&
		filmSizes().isEmpty() &&
		magnificationTypes().isEmpty() &&
		mediumTypes().isEmpty() &&
		modelName().isEmpty() &&
		resolution( QDicomPrinter::HighQuality ) == 0 &&
		resolution( QDicomPrinter::NormalQuality ) == 0 &&
		vendorName().isEmpty()
	;
}


bool QDicomPrinterDriver::isValid() const {
	return
		depths().size() > 0 &&
		filmDestinations().size() > 0 &&
		filmSizes().size() > 0 &&
		magnificationTypes().size() > 0 &&
		mediumTypes().size() > 0 &&
		modelName().size() > 0 &&
		resolution( QDicomPrinter::HighQuality ) != 0 &&
		resolution( QDicomPrinter::NormalQuality ) != 0 &&
		vendorName().size() > 0
	;
}


const QList< QDicomPrinter::MagnificationType > &
QDicomPrinterDriver::magnificationTypes() const {
	return data().MagnificationTypes;
}


const quint16 & QDicomPrinterDriver::maxCopyCount() const {
	return data().MaxCopyCount;
}


const QList< QDicomPrinter::MediumType > & QDicomPrinterDriver::mediumTypes() const {
	return data().MediumTypes;
}


const QString & QDicomPrinterDriver::modelName() const {
	return data().ModelName;
}


const QSize & QDicomPrinterDriver::printableArea(
	const QDicomPrinter::FilmSize & FilmSize,
	const QDicomPrinter::Quality & Quality
) const {
	const int Offset = ::PrintableAreasTableOffset(
		FilmSize, Quality
	);
	Q_ASSERT( Offset < Data::MaxAreas );

	return data().PrintableAreas[ Offset ];
}


const quint16 & QDicomPrinterDriver::resolution(
	const QDicomPrinter::Quality & Quality
) const {
	const int Offset = ::ResolutionsTableOffset( Quality );
	Q_ASSERT( Offset < Data::MaxResolutions );

	return data().Resolutions[ Offset ];
}


void QDicomPrinterDriver::setDepths( const QList< quint8 > & Value ) {
	QList< quint8 > & depths = data().Depths;

	depths = Value;
	::qSort( depths );
	std::reverse( depths.begin(), depths.end() );
}


void QDicomPrinterDriver::setFeatures( const Features & Features ) {
	data().Features = Features;
}


void QDicomPrinterDriver::setFilmDestinations(
	const QList< QDicomPrinter::FilmDestination > & Value
) {
	data().FilmDestinations = Value;
}


void QDicomPrinterDriver::setFilmSizes(
	const QList< QDicomPrinter::FilmSize > & Sizes
) {
	data().FilmSizes = Sizes;
}


void QDicomPrinterDriver::setMagnificationTypes(
	const QList< QDicomPrinter::MagnificationType > & Value
) {
	data().MagnificationTypes = Value;
}


void QDicomPrinterDriver::setMaxCopyCount( const quint16 & Value ) {
	data().MaxCopyCount = Value;
}


void QDicomPrinterDriver::setMediumTypes(
	const QList< QDicomPrinter::MediumType > & Value
) {
	data().MediumTypes = Value;
}


void QDicomPrinterDriver::setModelName( const QString & Name ) {
	data().ModelName = Name;
}


void QDicomPrinterDriver::setPrintableAreas(
	const QDicomPrinter::Quality & Quality,
	const QMap< QDicomPrinter::FilmSize, QSize > & Map 
) {
	QSize ( &Areas)[ Data::MaxAreas ] = data().PrintableAreas;

	for (
		QMap< QDicomPrinter::FilmSize, QSize >::const_iterator i = Map.constBegin();
		i != Map.constEnd(); ++i
	) {
		const int Offset = ::PrintableAreasTableOffset(
			i.key(), Quality
		);
		Areas[ Offset ] = i.value();
	}
}


void QDicomPrinterDriver::setResolutions(
	const quint16 & Normal, const quint16 & High
) {
	quint16 ( &Resolutions)[ 2 ] = data().Resolutions;

	Resolutions[ ::ResolutionsTableOffset( QDicomPrinter::NormalQuality ) ] = Normal;
	Resolutions[ ::ResolutionsTableOffset( QDicomPrinter::HighQuality   ) ] = High;
}


void QDicomPrinterDriver::setVendorName( const QString & Name ) {
	data().VendorName = Name;
}


const QString & QDicomPrinterDriver::vendorName() const {
	return data().VendorName;
}



QDicomPrinterDriver::Data::Data() :
	MaxCopyCount( 50 )
{
	Resolutions[ 0 ] = 0;
	Resolutions[ 1 ] = 0;
}



// Helper method definitions //


inline int PrintableAreasTableOffset(
	const QDicomPrinter::FilmSize & FilmSize,
	const QDicomPrinter::Quality & Quality
) {
	Q_ASSERT( FilmSize < 0x10 );
	Q_ASSERT(
		Quality == QDicomPrinter::NormalQuality ||
		Quality == QDicomPrinter::HighQuality
	);

	const int Offset =
		( FilmSize & 0x0F ) |
		( Quality == QDicomPrinter::HighQuality ? 0x10 : 0x00 )
	;

	return Offset;
}


inline int ResolutionsTableOffset(
	const QDicomPrinter::Quality & Quality
) {
	return Quality == QDicomPrinter::NormalQuality ? 0 : 1;
}
