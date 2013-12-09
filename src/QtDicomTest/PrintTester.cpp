/***************************************************************************
 *   Copyright © 2013 by Flux Inc.                                         *
 *   Author: Pawel Zak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#include "PrintTester.hpp"

#include <QtDicom/QDicomPrinter>

#include <QtGui/QBrush>
#include <QtGui/QPainter>


PrintTester::PrintTester( int argc, char * argv[] ) :
	QCoreApplication( argc, argv )
{
	QMetaObject::invokeMethod( this, "run", Qt::QueuedConnection );
}


const QString & PrintTester::errorString() const {
	return errorString_;
}


bool PrintTester::hasError() const {
	return errorString_.size() > 0;
}


void PrintTester::raiseError( const QString & Message ) {
	if ( ! hasError() ) {
		errorString_ = Message;
	}
}


void PrintTester::run() {
	QDicomPrinter printer( QDicomPrinter::Agfa_DRYSTAR_AXYS, QHostAddress( "172.16.9.10" ), 104 );
	printer.setQuality( QDicomPrinter::High );
	printer.setDepth( 16 );

	QPainter painter;
	if ( painter.begin( &printer ) ) {
		painter.setPen( Qt::blue );
		painter.setFont(QFont("Arial", 30));
		painter.setBrush( QBrush( Qt::red ) );
		painter.drawLine( 1, 1, 300, 300 );
		painter.drawRect( 300, 300, 100, 100 );
		painter.drawText( 400, 400, "KURWA!" );

		if ( printer.newPage() ) {
			painter.setPen( Qt::yellow );
			painter.setFont(QFont("Courier New", 30));
			painter.setBrush( QBrush( Qt::green ) );
			painter.drawLine( 1, 1, 300, 300 );
			painter.drawRect( 300, 300, 100, 100 );
			painter.drawText( 400, 400, "KURWA MAC!" );
			if ( painter.end() ) {
				exit( 0 );
				return;
			}
		}
	}

	Q_ASSERT( printer.hasError() );
	raiseError( printer.errorString() );
	exit( 1 );
}
 