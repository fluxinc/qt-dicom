/***************************************************************************
 *   Copyright (C) 2011 by Flux Inc.                                       *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#ifndef QDICOM_DATASOURCECREATOR_HPP
#define QDICOM_DATASOURCECREATOR_HPP

#include "QtDicom/DataSourceCreatorBase.hpp"
#include "QtDicom/Globals.hpp"

namespace Dicom {

/**
 * The \em DataSourceCreator class is a template class that provides \ref 
 * DataSourceFactory with a \ref createObject() method for a specific data 
 * source type.
 *
 * \author Paweł Żak <pawel.zak@fluxinc.ca>
 */
template < class T >
class QDICOM_DLLSPEC DataSourceCreator : public DataSourceCreatorBase {
	public :
		/**
		 * Creates a copy of the \a object.
		 */
		DataSource * cloneObject( const DataSource & object );

		/**
		 * Creates a \a T subclass of \ref DataSource and sets its parent 
		 * object.
		 */
		DataSource * createObject( QObject * parent );
};

}; // Namespace DICOM ends here.

#include "QtDicom/DataSourceCreator.inl"

#endif
