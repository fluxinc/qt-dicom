/***************************************************************************
 *   Copyright © 2013 by Flux Inc.                                         *
 *   Author: Pawel Zak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#ifndef QTDICOM_QDICOMPRINTERENGINE_HPP
#define QTDICOM_QDICOMPRINTERENGINE_HPP

#include "Globals.hpp"

#include <QtGui/QPaintEngine>
#include <QtGui/QPainter>


class QDicomPrinter;


class QDICOM_DLLSPEC QDicomPrintEngine : public QPaintEngine {
	public:
		QDicomPrintEngine();
		~QDicomPrintEngine();

		bool begin( QPaintDevice * device );
		void drawEllipse( const QRectF & rect );
		void drawEllipse( const QRect & rect );
		void drawImage(
			const QRectF & rectangle,
			const QImage & image, const QRectF & sr,
			Qt::ImageConversionFlags flags = Qt::AutoColor
		);
		void drawLines( const QLineF * lines, int lineCount );
		void drawLines( const QLine * lines, int lineCount );
		void drawPath( const QPainterPath & path );
		void drawPixmap( const QRectF & r, const QPixmap & pm, const QRectF & sr );
		void drawPoints( const QPointF * points, int pointCount );
		void drawPoints( const QPoint * points, int pointCount );
		void drawPolygon( const QPointF * points, int pointCount, PolygonDrawMode mode );
		void drawPolygon( const QPoint * points, int pointCount, PolygonDrawMode mode );
		void drawRects( const QRectF * rects, int rectCount );
		void drawRects( const QRect * rects, int rectCount );
		void drawTextItem( const QPointF & p, const QTextItem & textItem );
		void drawTiledPixmap( const QRectF & rect, const QPixmap & pixmap, const QPointF & p );
		bool end();
		bool hasFeature( PaintEngineFeatures feature ) const;
		bool newPage();
		Type type() const;
		void updateState( const QPaintEngineState & state );

	private :
		class Scu;

	private :
		bool printPage();

	private :
		QDicomPrintEngine( const QDicomPrintEngine & );

		const bool & isDirty() const;
		void setDirty( bool enable = true );
		bool dirty_;

		const QImage & image() const;
		QImage image_;

		const QPainter & painter() const;
		QPainter & painter();
		QPainter painter_;

		const Scu & scu() const;
		Scu & scu();
		Scu * scu_;

};

#endif // ! QTDICOM_QDICOMPRINTERENGINE_HPP
