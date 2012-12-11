/***************************************************************************
 *   Copyright (C) 2011 by Flux Inc.                                       *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#ifndef QDICOM_DATASOURCECREATORBASE_HPP
#define QDICOM_DATASOURCECREATORBASE_HPP

#include "QtDicom/Globals.hpp"

class QObject;

namespace Dicom {

class DataSource;

/**
 * The \em DataSourceCreatorBase is a base class for all data source creators.
 *
 * \author Paweł Żak <pawel.zak@fluxinc.ca>
 */
class QDICOM_DLLSPEC DataSourceCreatorBase {
	public :
		/**
		 * Destroys a creator object.
		 */
		virtual ~DataSourceCreatorBase();

		/**
		 * Reimplement this function to return a copy of the \a object.
		 */
		virtual DataSource * cloneObject( const DataSource & object ) = 0;

		/**
		 * Reimplement this function to return a new instance of \ref 
		 * DataSource class.
		 */
		virtual DataSource * createObject( QObject * parent ) = 0;
};

}; // Namespace DICOM ends here.

#endif
