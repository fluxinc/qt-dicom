/***************************************************************************
 *   Copyright (C) 2011 by Flux Inc.                                       *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#include "DataSource.hpp"
#include "DataSourceCreatorBase.hpp"
#include "DataSourceFactory.hpp"


namespace Dicom {

QCache< QString, DataSourceCreatorBase > * DataSourceFactory::creators_ = 0;


DataSource * DataSourceFactory::clone( const DataSource & DataSource ) {
	const QString & TypeName = DataSource.typeName();
	if ( creators().contains( TypeName ) ) {
		return creators().object( TypeName )->cloneObject( DataSource );
	}
	else {
		qWarning(
			"Unknown data source type: `%s'", qPrintable( TypeName )
		);
		return 0;
	}
}


DataSource * DataSourceFactory::create( const QString & TypeName, QObject * parent ) {
	Q_ASSERT( creators().contains( TypeName ) );

	if ( creators().contains( TypeName ) ) {
		return creators().object( TypeName )->createObject( parent );
	}
	else {
		qWarning(
			"Unknown data source type: `%s'", qPrintable( TypeName )
		);
		return 0;
	}
}


QCache< QString, DataSourceCreatorBase > & DataSourceFactory::creators() {
	if ( creators_ ) {
		return *creators_;
	}
	else {
		creators_ = new QCache< QString, DataSourceCreatorBase >();
		return *creators_;
	}
}


bool DataSourceFactory::registerType(
	const QString & TypeName, DataSourceCreatorBase * creator
) {
	Q_ASSERT( ! creators().contains( TypeName ) );
	Q_ASSERT( creator );

	if ( ! creators().contains( TypeName ) && creator ) {
		creators().insert( TypeName, creator, 0 );
		return true;
	}
	else if ( creator ) {
		qWarning(
			"Creator for the: `%s' type was registered already",
			qPrintable( TypeName )
		);

		delete creator;
	}
	else {
		qWarning( "Null creator provided" );
	}

	return false;
}



}; // Namespace DICOM ends here.
