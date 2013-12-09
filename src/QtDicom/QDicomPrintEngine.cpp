/***************************************************************************
 *   Copyright © 2013 by Flux Inc.                                         *
 *   Author: Pawel Zak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#include "QDicomPrintEngine.hpp" 
#include "QDicomPrintEngineScu.hpp" 
#include "QDicomPrinter.hpp"
#include "QDicomPrinterDriver.hpp"
#include "RequestorAssociation.hpp"
#include "ServiceUser.hpp"


QDicomPrintEngine::QDicomPrintEngine() :
	dirty_( false ),
	scu_( new Scu() )
{
}


QDicomPrintEngine::~QDicomPrintEngine() {
	delete scu_;
	scu_ = nullptr;
}


bool QDicomPrintEngine::begin( QPaintDevice * device ) {
	Q_ASSERT( ! isActive() );
	Q_ASSERT( ! scu().isOpen() );
	Q_ASSERT( device != nullptr );

	QDicomPrinter & printer = *static_cast< QDicomPrinter * >( device );
	Q_ASSERT( printer.isValid() );

	const int Height = device->height();
	const int Width = device->width();

	if ( Width > 0 && Height > 0 ) {
		image_ = QImage( Width, Height, QImage::Format_ARGB32_Premultiplied );
		image_.fill( Qt::white );

		const int Status =
			scu().beginSession( &printer ) &&
			painter().begin( &image_ )
		;

		if ( Status ) {
			setDirty( false );
			return true;
		}
		else {
			printer.raiseError( scu().errorString() );
		}
	}
	else {
		printer.raiseError( "Invalid print driver" );
	}

	return false;
}


void QDicomPrintEngine::drawEllipse( const QRectF & Rect ) {
	setDirty();

	painter().drawEllipse( Rect );
}


void QDicomPrintEngine::drawEllipse( const QRect & Rect ) {
	setDirty();

	painter().drawEllipse( Rect );
}


void QDicomPrintEngine::drawImage(
	const QRectF & DstRect,
	const QImage & Image, const QRectF & SrcRect,
	Qt::ImageConversionFlags Flags
) {
	setDirty();

	painter().drawImage( DstRect, Image, SrcRect, Flags );
}


void QDicomPrintEngine::drawLines( const QLineF * Lines, int Count ) {
	setDirty();

	painter().drawLines( Lines, Count );
}


void QDicomPrintEngine::drawLines( const QLine * Lines, int Count ) {
	setDirty();

	painter().drawLines( Lines, Count );
}

void QDicomPrintEngine::drawPath( const QPainterPath & Path ) {
	setDirty();

	painter().drawPath( Path );
}


void QDicomPrintEngine::drawPixmap(
	const QRectF & DstRect, const QPixmap & Pixmap, const QRectF & SrcRect
) {
	setDirty();

	painter().drawPixmap( DstRect, Pixmap, SrcRect );
}


void QDicomPrintEngine::drawPoints( const QPointF * Points, int Count ) {
	setDirty();

	painter().drawPoints( Points, Count );
}


void QDicomPrintEngine::drawPoints( const QPoint * Points, int Count ) {
	setDirty();

	painter().drawPoints( Points, Count );
}


void QDicomPrintEngine::drawPolygon(
	const QPointF * Points, int Count, PolygonDrawMode Mode
) {
	setDirty();

	painter().drawPolygon( Points, Count );
}


void QDicomPrintEngine::drawPolygon(
	const QPoint * Points, int Count, PolygonDrawMode Mode
) {
	setDirty();

	painter().drawPolygon( Points, Count );
}


void QDicomPrintEngine::drawRects( const QRectF * Rects, int Count ) {
	setDirty();

	painter().drawRects( Rects, Count );
}


void QDicomPrintEngine::drawRects( const QRect * Rects, int Count ) {
	setDirty();

	painter().drawRects( Rects, Count );
}


void QDicomPrintEngine::drawTextItem(
	const QPointF & Point, const QTextItem & Item
) {
	setDirty();

	painter().drawTextItem( Point, Item );
}


void QDicomPrintEngine::drawTiledPixmap(
	const QRectF & SrcRect, const QPixmap & Pixmap, const QPointF & Point
) {
	setDirty();

	painter().drawTiledPixmap( SrcRect, Pixmap, Point );
}


bool QDicomPrintEngine::end() {
	Q_ASSERT( isActive() );

	bool status = true;
	if ( isDirty() ) {
		status = printPage();
	}
	scu().endSession();
	painter().end();

	return status;
}


bool QDicomPrintEngine::hasFeature( PaintEngineFeatures feature ) const {
	return painter().paintEngine()->hasFeature( feature );
}


inline const QImage & QDicomPrintEngine::image() const {
	return image_;
}


inline const bool & QDicomPrintEngine::isDirty() const {
	return dirty_;
}


bool QDicomPrintEngine::newPage() {
	if ( isDirty() ) {
		const bool Status = printPage();
		if ( Status ) {
			setDirty( false );
			image_.fill( Qt::white );

			return true;
		}
		else {
			return false;
		}
	}
	else {
		qWarning( __FUNCTION__": no page to print (engine canvas is clear)" );
		return true;
	}
}


inline const QPainter & QDicomPrintEngine::painter() const {
	return painter_;
}


inline QPainter & QDicomPrintEngine::painter() {
	return painter_;
}


bool QDicomPrintEngine::printPage() {
	Q_ASSERT( isActive() );

	const bool Result = scu().printImage( image() );
	if ( Result ) {
		return true;
	}
	else {
		QDicomPrinter & printer = *static_cast< QDicomPrinter * >( paintDevice() );
		printer.raiseError( scu().errorString() );

		return false;
	}
}


inline const QDicomPrintEngine::Scu & QDicomPrintEngine::scu() const {
	Q_ASSERT( scu_ != nullptr );

	return *scu_;
}


inline QDicomPrintEngine::Scu & QDicomPrintEngine::scu() {
	Q_ASSERT( scu_ != nullptr );

	return *scu_;
}


inline void QDicomPrintEngine::setDirty( bool value ) {
	dirty_ = value;
}


QDicomPrintEngine::Type QDicomPrintEngine::type() const {
	return static_cast< QDicomPrintEngine::Type >( QDicomPrintEngine::User + 1 );
}

void QDicomPrintEngine::updateState( const QPaintEngineState & State ) {
	QPainter & p = painter();

	const QPaintEngine::DirtyFlags & Flags = State.state();

#define UPDATE( SET, GET ) \
	if ( Flags.testFlag( QPaintEngine::Dirty ## SET ) ) \
		p.set ## SET( State.GET() )

	UPDATE( Background,      backgroundBrush );
	UPDATE( BackgroundMode , backgroundMode );
	UPDATE( Brush,           brush );
	UPDATE( BrushOrigin,     brushOrigin );
	UPDATE( CompositionMode, compositionMode );
	UPDATE( Font,            font );
	UPDATE( Opacity,         opacity );
	UPDATE( Pen,             pen );
	UPDATE( Transform,       transform );

#undef UPDATE 

	if ( Flags.testFlag( QPaintEngine::DirtyClipPath ) ) {
		p.setClipPath( State.clipPath(), State.clipOperation() );
	}
	if ( Flags.testFlag( QPaintEngine::DirtyClipRegion ) ) {
		p.setClipRegion( State.clipRegion(), State.clipOperation() );
	}
	if ( Flags.testFlag( QPaintEngine::DirtyClipEnabled ) ) {
		p.setClipping( State.isClipEnabled() );
	}
	if ( Flags.testFlag( QPaintEngine::DirtyHints ) ) {
		p.setRenderHints( State.renderHints() );
	}
}
