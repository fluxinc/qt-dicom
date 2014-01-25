/***************************************************************************
 *   Copyright © 2010-2014 by Flux Inc.                                    *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#include "QDcmPluginTest.hpp"

#include <QtGui/QApplication>

#include <QtTest/QTest>


int main( int argc, char *argv[] ) {
    QApplication app( argc, argv, true );
    QDcmPluginTest tc;

    return QTest::qExec( &tc, argc, argv );
}
