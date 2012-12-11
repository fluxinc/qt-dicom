/***************************************************************************
 *   Copyright (C) 2011 by Flux Inc.                                       *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#ifndef QDICOM_DATASOURCEFACTORY_HPP
#define QDICOM_DATASOURCEFACTORY_HPP

#include <QtCore/QCache>
#include <QtCore/QString>

#include "QtDicom/DataSourceCreatorBase.hpp"
#include "QtDicom/Globals.hpp"

namespace Dicom {

class DataSource;

/**
 * The \em DataSourceFactory is a singleton factory for creating and 
 * registering \ref DataSource types.
 *
 * To register a new data source type use the \ref registerType() method. Once
 * a type is registered it can be instantinatized with the \ref create() method.
 * A list of all registered types is returned by the \ref types() routine.
 *
 * Typically you'd want to register your sub-class of \ref DataSource in a 
 * constructor:
 * <code>
 * class MyDataSource : public DataSource {
 *   public :
 *     MyDataSource( QObject * parent ) :
 *       DataSource( parent )
 *     {
 *       static const bool IsRegistered = DataSourceFactory::registerType(
 *         "MyDataSource", new DataSourceCreator< MyDataSource >
 *       );
 *     }
 * };
 * </code>
 * After that the type will be availabe to the \ref create() method and can
 * be read by the \ref DataSource::fromXml() routine as well.
 *
 * \sa DataSourceCreator, DataSource
 * 
 * \author Paweł Żak <pawel.zak@fluxinc.ca>
 */
class QDICOM_DLLSPEC DataSourceFactory {
	public :
		/**
		 * Creates a clone of a \em DataSource \a object.
		 */
		static DataSource * clone( const DataSource & object );

		/**
		 * Creates a data source of a \a type and sets its \a prent object.
		 *
		 * Note that if no \a parent was provided, it's caller responsibility
		 * to free allocated memory.
		 */
		static DataSource * create( const QString & type, QObject * parent = 0 );

		/**
		 * Registers a new \ref type using the \ref creator object. The factory
		 * takes ownership of the \em creators and deallocate them when program
		 * is finishing.
		 *
		 * Returns \c true on success. In case of an error, e.g. when the \a 
		 * type was already registered or \a creator is null, a \c false is
		 * returned.
		 */
		static bool registerType(
			const QString & type, DataSourceCreatorBase * creator
		);

		/**
		 * Returns a list of known types.
		 */
		static QStringList types();

	private :
		/**
		 * Returns the creators.
		 */
		static QCache< QString, DataSourceCreatorBase > & creators();

		/**
		 * The creators.
		 */
		static QCache< QString, DataSourceCreatorBase > * creators_;

	private :
		DataSourceFactory();
		~DataSourceFactory();
};

}; // Namespace DICOM ends here.

#endif
