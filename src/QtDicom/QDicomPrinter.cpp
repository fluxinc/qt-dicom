/***************************************************************************
 *   Copyright © 2013 by Flux Inc.                                         *
 *   Author: Pawel Zak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#include "QDicomPrinter.hpp"
#include "QDicomPrinterDriver.hpp"

#include <QtCore/QSize>
#include <QtCore/QVector>

#include <QtFlux/QInitializeOnce>


// Returns dimensions for film \a film (in inches).
static const QSizeF & filmDimensions( const QDicomPrinter::FilmSize & film );

QDicomPrinter::QDicomPrinter() :
	copyCount_( 1 ),
	depth_( 0 ),
	driver_( new QDicomPrinterDriver() ),
	emptyAreaDensity_( 0.0 ),
	filmDestination_( Processor ),
	filmSize_( IN_8x10 ),
	localAeTitle_( "FLUX" ),
	magnificationType_( None ),
	mediumType_( ClearFilm ),
	orientation_( Horizontal ),
	portNumber_( 0 ),
	quality_( Normal )
{
}


QDicomPrinter::QDicomPrinter(
	const Device & Device,
	const QHostAddress & Address,
	const quint16 & PortNumber,
	const QString & RemoteAe,
	const QString & LocalAe
) :
	copyCount_( 1 ),
	// depth_( 8 ), - initialize that from driver below
	driver_( new QDicomPrinterDriver( QDicomPrinterDriver::forDevice( Device ) ) ),
	emptyAreaDensity_( 0.0 ),
	// filmDestination_( Processor ), - initialize that from driver
	// filmSize_( IN_8x10 ), - initialize that from driver
	hostAddress_( Address ),
	localAeTitle_( LocalAe ),
	// magnificationType( None ), -- initialize that from driver
	// mediumType_( ClearFilm ), - initialize that from driver
	orientation_( Horizontal ),
	portNumber_( PortNumber ),
	// quality_( Normal ), - initialize that from driver
	remoteAeTitle_( RemoteAe )
{
	setDepth( driver().depths().first() );
	setFilmDestination( driver().filmDestinations().first() );
	setFilmSize( driver().filmSizes().first() );
	setMagnificationType( driver().magnificationTypes().first() );
	setMediumType( driver().mediumTypes().first() );
	setQuality(
		driver().hasFeature( QDicomPrinterDriver::HighQuality ) ? High : Normal
	);
}


const quint16 & QDicomPrinter::copyCount() const {
	return copyCount_;
}


QDicomPrinterDriver & QDicomPrinter::driver() {
	Q_ASSERT( driver_ != nullptr );

	return *driver_;
}


const QDicomPrinterDriver & QDicomPrinter::driver() const {
	Q_ASSERT( driver_ != nullptr );

	return *driver_;
}


const qreal & QDicomPrinter::emptyAreaDensity() const {
	return emptyAreaDensity_;
}


const QString & QDicomPrinter::errorString() const {
	return errorString_;
}


const quint16 & QDicomPrinter::filmDestination() const {
	return filmDestination_;
}


const QDicomPrinter::FilmSize & QDicomPrinter::filmSize() const {
	return filmSize_;
}


bool QDicomPrinter::hasError() const {
	return errorString_.size() > 0;
}


const QHostAddress & QDicomPrinter::hostAddress() const {
	return hostAddress_;
}


bool QDicomPrinter::isNull() const {
	return 
		driver().isNull() &&
		hostAddress().isNull() &&
		portNumber() == 0
	;
}


bool QDicomPrinter::isValid() const {
	return
		driver().isValid() &&
		! hostAddress().isNull() &&
		portNumber() != 0
	;
}


const QString & QDicomPrinter::localAeTitle() const {
	return localAeTitle_;
}


const QDicomPrinter::MagnificationType & QDicomPrinter::magnificationType() const {
	return magnificationType_;
}


const QDicomPrinter::MediumType & QDicomPrinter::mediumType() const {
	return mediumType_;
}


int QDicomPrinter::metric( QPaintDevice::PaintDeviceMetric metric ) const {
	if ( isValid() ) {
		int result = -1;

		switch ( metric ) {
		
		case QPaintDevice::PdmWidth :
			result = driver().printableArea(
				filmSize_, orientation_, quality_
			).width();
			break;

		case QPaintDevice::PdmHeight :
			result = driver().printableArea(
				filmSize_, orientation_, quality_
			).height();
			break;

		case QPaintDevice::PdmWidthMM :
			result = ::qRound( width() * physicalDpiX() * 25.4 );
			break;

		case QPaintDevice::PdmHeightMM :
			result = ::qRound( height() * physicalDpiY() * 25.4 );
			break;

		case QPaintDevice::PdmNumColors :
			result = 1 << depth_;
			break;

		case QPaintDevice::PdmDepth :
			result = depth_;
			break;

		case QPaintDevice::PdmDpiX :
			result = driver().resolution( quality_ );
			break;

		case QPaintDevice::PdmDpiY :
			result = driver().resolution( quality_ );
			break;

		case QPaintDevice::PdmPhysicalDpiX :
			result = driver().resolution( quality_ );
			break;

		case QPaintDevice::PdmPhysicalDpiY :
			result = driver().resolution( quality_ );
			break;
		}

		return result;
	}
	else {
		return -1;
	}

}


QString QDicomPrinter::name() const {
	return QString( "%1 %2 on %3:%4 (AE: %5 -> %6)" )
		.arg( driver().vendorName() )
		.arg( driver().modelName() )
		.arg( hostAddress().toString() )
		.arg( portNumber() )
		.arg( localAeTitle() )
		.arg( remoteAeTitle() )
	;
}


bool QDicomPrinter::newPage() {
	return engine_.newPage();
}


const QDicomPrinter::Orientation & QDicomPrinter::orientation() const {
	return orientation_;
}


QPaintEngine * QDicomPrinter::paintEngine() const {
	return &engine_;
}


const quint16 & QDicomPrinter::portNumber() const {
	return portNumber_;
}


const QDicomPrinter::Quality & QDicomPrinter::quality() const {
	return quality_;
}


void QDicomPrinter::raiseError( const QString & Message ) {
	if ( ! hasError() ) {
		errorString_ = Message;
	}
}


const QString & QDicomPrinter::remoteAeTitle() const {
	return remoteAeTitle_;
}


void QDicomPrinter::setCopyCount( const quint16 & Count ) {
	if ( Count > 0 ) {
		copyCount_ = Count;
	}
	else {
		qWarning( __FUNCTION__": copy count cannot be 0" );

		copyCount_ = 1;
	}
}


void QDicomPrinter::setDepth( const quint8 & Depth ) {
	if ( Depth >= 8 ) {
		depth_ = Depth;
	}
	else {
		qWarning( __FUNCTION__": printer depth must be greater or equal 8 bits" );

		depth_ = 8;
	}
}


void QDicomPrinter::setDriver( const QDicomPrinterDriver & Driver ) {
	driver() = Driver;
}


void QDicomPrinter::setEmptyAreaDensity( const qreal & Value ) {
	emptyAreaDensity_ = Value;
}


void QDicomPrinter::setFilmDestination(
	const FilmDestination & Destination, const quint8 & BinNumber
) {

	quint16 value = Destination;

	if ( Destination == Bin ) {
		Q_ASSERT( BinNumber > 0 );
		value |= BinNumber;
	}

	setFilmDestination( value );
}


void QDicomPrinter::setFilmDestination( const quint16 & Value ) {
	filmDestination_ = Value;
}


void QDicomPrinter::setFilmSize( const FilmSize & Size ) {
	filmSize_ = Size;
}


void QDicomPrinter::setHostAddres( const QHostAddress & Address ) {
	hostAddress_ = Address;
}


void QDicomPrinter::setLocalAeTitle( const QString & Title ) {
	localAeTitle_ = Title;
}


void QDicomPrinter::setMagnificationType( const MagnificationType & Type ) {
	magnificationType_ = Type;
}


void QDicomPrinter::setMediumType( const MediumType & Type ) {
	mediumType_ = Type;
}


void QDicomPrinter::setOrientation( const Orientation & Value ) {
	orientation_ = Value;
}


void QDicomPrinter::setPortNumber( const quint16 & Number ) {
	if ( Number > 0 ) {
		portNumber_ = Number;
	}
	else {
		qWarning( __FUNCTION__": port number cannot be 0" );

		portNumber_ = 104;
	}
}


void QDicomPrinter::setQuality( const Quality & Q ) {
	quality_ = Q;
}


void QDicomPrinter::setRemoteAeTitle( const QString & Title ) {
	remoteAeTitle_ = Title;
}


static const QSizeF & filmDimensions( const QDicomPrinter::FilmSize & Film ) {
	static QVector< QSizeF > * dimensions = nullptr;
	
	::qInitializeOnce( dimensions, ( []() throw() -> QVector< QSizeF > {
		static const int Last = QDicomPrinter::IN_14x17;

		QVector< QSizeF > result( Last + 1 );

		#define SET( SIZE, X, Y ) \
			Q_ASSERT( QDicomPrinter::SIZE <= Last ); \
			result[ QDicomPrinter::SIZE ] = QSizeF( X, Y )

		#define SET_IN( SIZE, X, Y ) SET( IN_ ## SIZE, X,        Y        )
		#define SET_CM( SIZE, X, Y ) SET( CM_ ## SIZE, X / 2.54, Y / 25.4 )


		SET( A3, 297, 420 );
		SET( A4, 210, 297 );

		SET_CM(     24x24, 24.0, 24.0 );
		SET_CM(     24x30, 24.0, 30.0 );
		SET_CM( 25_7x36_4, 25.7, 36.4 );

		SET_IN(   8x10,  8.0, 10.0 );
		SET_IN( 8_5x11,  8.5, 11.0 );
		SET_IN(  10x12, 10.0, 12.0 );
		SET_IN(  10x14, 10.0, 14.0 );
		SET_IN(  11x14, 11.0, 14.0 );
		SET_IN(  11x17, 11.0, 17.0 );
		SET_IN(  14x14, 14.0, 14.0 );
		SET_IN(  14x17, 14.0, 17.0 );

		#undef SET_CM
		#undef SET_IN
		#undef SET

		return result;
	} ) );


	Q_ASSERT( dimensions != nullptr );
	Q_ASSERT( Film < dimensions->size() );
	return dimensions->at( Film );
}
