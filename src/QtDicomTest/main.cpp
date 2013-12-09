/***************************************************************************
 *   Copyright © 2012-2013 by Flux Inc.                                    *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#include <QtCore/QCoreApplication>

#include <QtTest/QTest>

#include "PrintTester.hpp"
#include "QtDicomTest.hpp"


static int RunPrinterTest( int argc, char * argv[] ) {
    PrintTester app( argc, argv );

	const int Status = app.exec();

	if ( Status != 0 ) {
		qWarning( qPrintable( app.errorString() ) );
	}

	return Status;
}


static int RunQDicomTest( int argc, char * argv[] ) {
    QCoreApplication app( argc, argv );
    QtDicomTest tc;

    return QTest::qExec( &tc, argc, argv );
}

int main( int argc, char *argv[] ) {

	::RunPrinterTest( argc, argv );
	return ::RunQDicomTest( argc, argv );
}
