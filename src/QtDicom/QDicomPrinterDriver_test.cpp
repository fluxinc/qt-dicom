/***************************************************************************
 *   Copyright © 2013 by Flux Inc.                                         *
 *   Author: Pawel Zak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#ifdef _DEBUG

#include "QDicomPrinterDriver.hpp"

#include <QtCore/QXmlStreamReader>


// QDicomPrinterDriver unit tests //



// Declarations:

/**
 * Test setting/retrieving properties
 */
static void TestProperties();

/**
 * Runs a series of unit tests with assertions and returns \c true.
 */
extern bool TestQDicomPrinterDriver();

/**
 * Tests loading a driver from XML file.
 */
static void TestXmlLoading();


// Definitions:

bool TestQDicomPrinterDriver() {
	TestProperties();
	TestXmlLoading();

	return true;
}

 
void TestProperties() {
	QDicomPrinterDriver driver;
	Q_ASSERT( driver.isNull() );
	Q_ASSERT( ! driver.isValid() );

	Q_ASSERT( driver.depths().isEmpty() );
	Q_ASSERT( driver.features() == 0 );
	Q_ASSERT( driver.filmSizes().isEmpty() );
	Q_ASSERT( driver.modelName().isEmpty() );
	Q_ASSERT( driver.printableArea( QDicomPrinter::A3, QDicomPrinter::NormalQuality ).isEmpty() );
	Q_ASSERT( driver.resolution( QDicomPrinter::NormalQuality ) == 0 );
	Q_ASSERT( driver.resolution( QDicomPrinter::HighQuality ) == 0 );
	Q_ASSERT( driver.vendorName().isEmpty() );

	driver.setDepths( QList< quint8 >() << 8 );
	Q_ASSERT( ! driver.isNull() );
	Q_ASSERT( ! driver.isValid() );
	Q_ASSERT( ! driver.depths().isEmpty() );
	Q_ASSERT( driver.depths().contains( 8 ) );

	driver.setFeatures( QDicomPrinterDriver::RequestedImageSize );
	Q_ASSERT( ! driver.isNull() );
	Q_ASSERT( ! driver.isValid() );
	Q_ASSERT( driver.hasFeature( QDicomPrinterDriver::RequestedImageSize ) );

	driver.setFilmDestinations(
		QList< QDicomPrinter::FilmDestination >() << QDicomPrinter::Magazine
	);
	Q_ASSERT( ! driver.isNull() );
	Q_ASSERT( ! driver.isValid() );
	Q_ASSERT( ! driver.filmDestinations().isEmpty() );
	Q_ASSERT( driver.filmDestinations().contains( QDicomPrinter::Magazine ) );

	driver.setFilmSizes( QList< QDicomPrinter::FilmSize >() << QDicomPrinter::A3 );
	Q_ASSERT( ! driver.isNull() );
	Q_ASSERT( ! driver.isValid() );
	Q_ASSERT( ! driver.filmSizes().isEmpty() );
	Q_ASSERT( driver.filmSizes().contains( QDicomPrinter::A3 ) );

	driver.setMagnificationTypes(
		QList< QDicomPrinter::MagnificationType >() << QDicomPrinter::Cubic
	);
	Q_ASSERT( ! driver.isNull() );
	Q_ASSERT( ! driver.isValid() );
	Q_ASSERT( ! driver.magnificationTypes().isEmpty() );
	Q_ASSERT( driver.magnificationTypes().contains( QDicomPrinter::Cubic ) );

	driver.setMediumTypes(
		QList< QDicomPrinter::MediumType >() << QDicomPrinter::ClearFilm
	);
	Q_ASSERT( ! driver.isNull() );
	Q_ASSERT( ! driver.isValid() );
	Q_ASSERT( ! driver.mediumTypes().isEmpty() );
	Q_ASSERT( driver.mediumTypes().contains( QDicomPrinter::ClearFilm) );

	driver.setModelName( "TEST MODEL" );
	Q_ASSERT( ! driver.isNull() );
	Q_ASSERT( ! driver.isValid() );
	Q_ASSERT( driver.modelName() == "TEST MODEL" ); 
	QMap< QDicomPrinter::FilmSize, QSize > areas;
	areas[ QDicomPrinter::A3 ] = QSize( 100, 100 );
	areas[ QDicomPrinter::A4 ] = QSize( 200, 200 );
	driver.setPrintableAreas( QDicomPrinter::HighQuality, areas );
	Q_ASSERT( ! driver.isNull() );
	Q_ASSERT( ! driver.isValid() );
	Q_ASSERT( driver.printableArea( QDicomPrinter::A3, QDicomPrinter::HighQuality).isValid() );
	Q_ASSERT( driver.printableArea( QDicomPrinter::A4, QDicomPrinter::HighQuality).isValid() );

	driver.setResolutions( 100, 200 );
	Q_ASSERT( ! driver.isNull() );
	Q_ASSERT( ! driver.isValid() );
	Q_ASSERT( driver.resolution( QDicomPrinter::NormalQuality ) == 100 );
	Q_ASSERT( driver.resolution( QDicomPrinter::HighQuality ) == 200 );

	driver.setVendorName( "TEST VENDOR" );
	Q_ASSERT( ! driver.isNull() );
	Q_ASSERT( driver.isValid() );
	Q_ASSERT( driver.vendorName() == "TEST VENDOR" );
}


void TestXmlLoading() {
	QXmlStreamReader stream;
	QString msg;
	QDicomPrinterDriver drv;

	drv = QDicomPrinterDriver::fromXml( stream, &msg );
	Q_ASSERT( drv.isNull() );
	Q_ASSERT( msg.contains( "Missing" ) && msg.contains( "root" ) );


	stream.clear(); msg.clear();
	stream.addData( "<QDicomPrinterDriver></QDicomPrinterDriver>" );
	drv = QDicomPrinterDriver::fromXml( stream, &msg );
	Q_ASSERT( drv.isNull() );
	Q_ASSERT( msg.contains( "missing version attribute" ) );


	stream.clear(); msg.clear();
	stream.addData( "<QDicomPrinterDriver version='0.1'></QDicomPrinterDriver>" );
	drv = QDicomPrinterDriver::fromXml( stream, &msg );
	Q_ASSERT( drv.isNull() );
	Q_ASSERT( msg.isEmpty() );


	stream.clear(); msg.clear();
	stream.addData(
		"<QDicomPrinterDriver version='0.1'>"
		  "<Vendor"
		"</QDicomPrinterDriver>"
	);
	drv = QDicomPrinterDriver::fromXml( stream, &msg );
	Q_ASSERT( drv.isNull() );
	Q_ASSERT( msg.size() > 0 );


	stream.clear(); msg.clear();
	stream.addData(
		"<QDicomPrinterDriver version='0.1'>"
		  "<VendorName>asdf"
		"</QDicomPrinterDriver>"
	);
	drv = QDicomPrinterDriver::fromXml( stream, &msg );
	Q_ASSERT( drv.isNull() );
	Q_ASSERT( msg.contains( "mismatch" ) );


	stream.clear(); msg.clear();
	stream.addData(
		"<QDicomPrinterDriver version='0.1'>"
		  "<Vendor>"
		"</QDicomPrinterDriver>"
	);
	drv = QDicomPrinterDriver::fromXml( stream, &msg );
	Q_ASSERT( drv.isNull() );
	Q_ASSERT( msg.contains( "Unexpected" ) );


	stream.clear(); msg.clear();
	stream.addData(
		"<QDicomPrinterDriver version='0.1'>"
		  "<VendorName>TEST VENDOR</VendorName>"
		"</QDicomPrinterDriver>"
	);
	drv = QDicomPrinterDriver::fromXml( stream, &msg );
	Q_ASSERT( drv.vendorName() == "TEST VENDOR" );
	Q_ASSERT( ! ( drv.isValid() || drv.isNull() ) );
	Q_ASSERT( msg.isEmpty() );


	stream.clear(); msg.clear();
	stream.addData(
		"<QDicomPrinterDriver version='0.1'>"
		  "<ModelName>TEST MODEL</ModelName>"
		"</QDicomPrinterDriver>"
	);
	drv = QDicomPrinterDriver::fromXml( stream, &msg );
	Q_ASSERT( drv.modelName() == "TEST MODEL" );
	Q_ASSERT( ! ( drv.isValid() || drv.isNull() ) );
	Q_ASSERT( msg.isEmpty() );


	stream.clear(); msg.clear();
	stream.addData(
		"<QDicomPrinterDriver version='0.1'>"
		  "<ModelName>	\t\n</ModelName>"
		"</QDicomPrinterDriver>"
	);
	drv = QDicomPrinterDriver::fromXml( stream, &msg );
	Q_ASSERT( ! drv.isValid() && drv.isNull() );
	Q_ASSERT( msg.contains( "model name", Qt::CaseInsensitive ) );


	stream.clear(); msg.clear();
	stream.addData(
		"<QDicomPrinterDriver version='0.1'>"
		  "<Depths>8, 10, 12</Depths>"
		"</QDicomPrinterDriver>"
	);
	drv = QDicomPrinterDriver::fromXml( stream, &msg );
	Q_ASSERT( drv.depths().count() == 3 );
	Q_ASSERT( drv.depths()[ 0 ] == 12 );
	Q_ASSERT( ! ( drv.isValid() || drv.isNull() ) );
	Q_ASSERT( msg.isEmpty() );


	stream.clear(); msg.clear();
	stream.addData(
		"<QDicomPrinterDriver version='0.1'>"
		  "<Depths>8, 10, 12, -1</Depths>"
		"</QDicomPrinterDriver>"
	);
	drv = QDicomPrinterDriver::fromXml( stream, &msg );
	Q_ASSERT( drv.isNull() );
	Q_ASSERT( msg.contains( "depth" ) && msg.contains( "-1" ) );

	#define CHECK( FEATURE ) \
		stream.clear(); msg.clear(); \
		stream.addData( \
			"<QDicomPrinterDriver version='0.1'>" \
			"<Features>" ## #FEATURE ## "</Features>" \
			"</QDicomPrinterDriver>" \
		); \
		drv = QDicomPrinterDriver::fromXml( stream, &msg ); \
		Q_ASSERT( ! ( drv.isValid() || drv.isNull() ) ); \
		Q_ASSERT( msg.isEmpty() ); \
		Q_ASSERT( drv.features().testFlag( QDicomPrinterDriver::FEATURE ) )

	CHECK( FilmSessionLabel );
	CHECK( BorderDensity );
	CHECK( EmptyImageDensity );
	CHECK( Trim );
	CHECK( HighQuality );
	CHECK( RequestedImageSize );
#undef CHECK


	stream.clear(); msg.clear();
	stream.addData(
		"<QDicomPrinterDriver version='0.1'>"
		  "<Features>RequestedImageSize</Features>"
		"</QDicomPrinterDriver>"
	);
	drv = QDicomPrinterDriver::fromXml( stream, &msg );
	Q_ASSERT( ! ( drv.isValid() || drv.isNull() ) );
	Q_ASSERT( msg.isEmpty() );
	Q_ASSERT( drv.features().testFlag( QDicomPrinterDriver::RequestedImageSize ) );
	Q_ASSERT( ! drv.features().testFlag( QDicomPrinterDriver::HighQuality ) );


	stream.clear(); msg.clear();
	stream.addData(
		"<QDicomPrinterDriver version='0.1'>"
		  "<Features>RequestedImageSize, InvalidFeature</Features>"
		"</QDicomPrinterDriver>"
	);
	drv = QDicomPrinterDriver::fromXml( stream, &msg );
	Q_ASSERT( drv.isNull() );
	Q_ASSERT( msg.contains( "feature" ) && msg.contains( "InvalidFeature" ) );


	stream.clear(); msg.clear();
	stream.addData(
		"<QDicomPrinterDriver version='0.1'>"
		  "<Resolutions>"
		  "<NormalQuality>100</NormalQuality>"
		  "<HighQuality>200</HighQuality>"
		  "</Resolutions>"
		"</QDicomPrinterDriver>"
	);
	drv = QDicomPrinterDriver::fromXml( stream, &msg );
	Q_ASSERT( ! ( drv.isValid() || drv.isNull() ) );
	Q_ASSERT( msg.isEmpty() );
	Q_ASSERT( drv.resolution( QDicomPrinter::NormalQuality ) == 100 );
	Q_ASSERT( drv.resolution( QDicomPrinter::HighQuality   ) == 200 );


	stream.clear(); msg.clear();
	stream.addData(
		"<QDicomPrinterDriver version='0.1'>"
		  "<Resolutions>"
		  "<Invalid></Invalid>"
		  "</Resolutions>"
		"</QDicomPrinterDriver>"
	);
	drv = QDicomPrinterDriver::fromXml( stream, &msg );
	Q_ASSERT( drv.isNull() );
	Q_ASSERT( msg.contains( "Unexpected" ) && msg.contains( "Invalid" ) );


	stream.clear(); msg.clear();
	stream.addData(
		"<QDicomPrinterDriver version='0.1'>"
		  "<Resolutions>"
		  "<NormalQuality>InvalidDpi</NormalQuality>"
		  "</Resolutions>"
		"</QDicomPrinterDriver>"
	);
	drv = QDicomPrinterDriver::fromXml( stream, &msg );
	Q_ASSERT( drv.isNull() );
	Q_ASSERT( msg.contains( "resolution" ) && msg.contains( "InvalidDpi" ) );


	stream.clear(); msg.clear();
	stream.addData(
		"<QDicomPrinterDriver version='0.1'>"
		  "<FilmSizes>A4</FilmSizes>"
		  "<PrintableAreas>"
		    "<NormalQuality>A4: 100 x 200</NormalQuality>"
			"<HighQuality>A4: 300 x 400</HighQuality>"
		  "</PrintableAreas>"
		"</QDicomPrinterDriver>"
	);
	drv = QDicomPrinterDriver::fromXml( stream, &msg );
	Q_ASSERT( ! ( drv.isValid() || drv.isNull() ) );
	Q_ASSERT( msg.isEmpty() );
	Q_ASSERT(
		drv.printableArea( QDicomPrinter::A4, QDicomPrinter::NormalQuality ) == 
		QSize( 100, 200 )
	);
	Q_ASSERT(
		drv.printableArea( QDicomPrinter::A4, QDicomPrinter::HighQuality ) == 
		QSize( 300, 400 )
	);


	stream.clear(); msg.clear();
	stream.addData(
		"<QDicomPrinterDriver version='0.1'>"
		  "<FilmSizes>A4</FilmSizes>"
		  "<PrintableAreas>"
		    "<NormalQuality>A4: 100 x 200</NormalQuality>"
		  "</PrintableAreas>"
		"</QDicomPrinterDriver>"
	);
	drv = QDicomPrinterDriver::fromXml( stream, &msg );
	Q_ASSERT( ! ( drv.isValid() || drv.isNull() ) );
	Q_ASSERT( msg.isEmpty() );
	Q_ASSERT(
		drv.printableArea( QDicomPrinter::A4, QDicomPrinter::NormalQuality ) == 
		QSize( 100, 200 )
	);
	Q_ASSERT(
		drv.printableArea( QDicomPrinter::A4, QDicomPrinter::HighQuality ) == 
		QSize( 100, 200 )
	);


	stream.clear(); msg.clear();
	stream.addData(
		"<QDicomPrinterDriver version='0.1'>"
		  "<PrintableAreas>"
		    "<Invalid></Invalid>"
		  "</PrintableAreas>"
		"</QDicomPrinterDriver>"
	);
	drv = QDicomPrinterDriver::fromXml( stream, &msg );
	Q_ASSERT( drv.isNull() );
	Q_ASSERT( msg.contains( "Unexpected" ) && msg.contains( "Invalid" ) );


	stream.clear(); msg.clear();
	stream.addData(
		"<QDicomPrinterDriver version='0.1'>"
		  "<PrintableAreas>"
		    "<NormalQuality>InvalidArea</NormalQuality>"
		  "</PrintableAreas>"
		"</QDicomPrinterDriver>"
	);
	drv = QDicomPrinterDriver::fromXml( stream, &msg );
	Q_ASSERT( drv.isNull() );
	Q_ASSERT( msg.contains( "area" ) && msg.contains( "InvalidArea" ) );


	stream.clear(); msg.clear();
	stream.addData(
		"<QDicomPrinterDriver version='0.1'>"
		  "<FilmSizes>A4</FilmSizes>"
		  "<PrintableAreas>"
		    "<NormalQuality>A3: 100 x 200</NormalQuality>"
		  "</PrintableAreas>"
		"</QDicomPrinterDriver>"
	);
	drv = QDicomPrinterDriver::fromXml( stream, &msg );
	Q_ASSERT( drv.isNull() );
	Q_ASSERT( msg.contains( "film" ) && msg.contains( "a3" ) );


	stream.clear(); msg.clear();
	stream.addData(
		"<QDicomPrinterDriver version='0.1'>"
		  "<FilmSizes>A3, A4</FilmSizes>"
		  "<PrintableAreas>"
		    "<NormalQuality>A3: 100 x 200</NormalQuality>"
		  "</PrintableAreas>"
		"</QDicomPrinterDriver>"
	);
	drv = QDicomPrinterDriver::fromXml( stream, &msg );
	Q_ASSERT( drv.isNull() );
	Q_ASSERT( msg.contains( "area" ) && msg.contains( "a4" ) );
}


#endif // _DEBUG
