/***************************************************************************
 *   Copyright © 2013 by Flux Inc.                                         *
 *   Author: Pawel Zak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#ifdef _DEBUG

#include "QSopClass.hpp"

#include <QtCore/QList>
#include <QtCore/QString>

#include <dcmtk/dcmdata/dcuid.h>


extern bool TestQSopClass() {
	const QSopClass NullClass;
	Q_ASSERT( NullClass == QSopClass::Unknown );
	Q_ASSERT( ! NullClass.hasMeta() );
	Q_ASSERT( ! NullClass.isMeta() );
	Q_ASSERT( NullClass.isNull() );
	Q_ASSERT( ! NullClass.isValid() );
	Q_ASSERT( NullClass.meta().isNull() );
	Q_ASSERT( NullClass.name() == "Unknown" );
	Q_ASSERT( NullClass.uid() == "" );

	const QSopClass BasicClass = QSopClass::BasicAnnotationBox;
	Q_ASSERT( BasicClass == QSopClass::BasicAnnotationBox );
	Q_ASSERT( ! BasicClass.hasMeta() );
	Q_ASSERT( ! BasicClass.isMeta() );
	Q_ASSERT( ! BasicClass.isNull() );
	Q_ASSERT( BasicClass.isValid() );
	Q_ASSERT( BasicClass.meta().isNull() );
	Q_ASSERT( BasicClass.name() == "Basic Annotation Box" );
	Q_ASSERT( BasicClass.uid() == UID_BasicAnnotationBoxSOPClass );

	const QSopClass MetaClass = QSopClass::BasicGrayscalePrintManagement;
	Q_ASSERT( MetaClass == QSopClass::BasicGrayscalePrintManagement );
	Q_ASSERT( ! MetaClass.hasMeta() );
	Q_ASSERT( MetaClass.isMeta() );
	Q_ASSERT( ! MetaClass.isNull() );
	Q_ASSERT( MetaClass.isValid() );
	Q_ASSERT( MetaClass.meta().isNull() );
	Q_ASSERT( MetaClass.name() == "Basic Grayscale Print Management" );
	Q_ASSERT( MetaClass.uid() == UID_BasicGrayscalePrintManagementMetaSOPClass );

	const QSopClass SubClass = QSopClass::Printer;
	Q_ASSERT( SubClass == QSopClass::Printer );
	Q_ASSERT( SubClass.hasMeta() );
	Q_ASSERT( ! SubClass.isMeta() );
	Q_ASSERT( ! SubClass.isNull() );
	Q_ASSERT( SubClass.isValid() );
	Q_ASSERT( SubClass.meta() == QSopClass::BasicGrayscalePrintManagement );
	Q_ASSERT( SubClass.name() == "Printer" );
	Q_ASSERT( SubClass.uid() == UID_PrinterSOPClass );

	Q_ASSERT( SubClass != MetaClass );
	Q_ASSERT( NullClass != BasicClass );


	const QList< QSopClass > AllClasses = QList< QSopClass >()
		<< NullClass << BasicClass << MetaClass << SubClass
	;


	foreach ( const QSopClass & Class, AllClasses ) {
		Q_ASSERT( QSopClass::fromUid( Class.uid() ) == Class );
	}

	return true;
}

#endif // _DEBUG
