/***************************************************************************
 *   Copyright (C) 2011 by Flux Inc.                                       *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#include "DataSourceCreator.hpp"


namespace Dicom {

template < class T >
DataSource * DataSourceCreator< T >::createObject( QObject * parent ) {
	return new T( parent );
}


template < class T >
DataSource * DataSourceCreator< T >::cloneObject( const DataSource & object ) {
	return new T( *reinterpret_cast< const T * >( &object ) );
}


}; // Namespace DICOM ends here.