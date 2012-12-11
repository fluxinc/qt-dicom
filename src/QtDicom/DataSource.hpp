/***************************************************************************
 *   Copyright (C) 2011 by Flux Inc.                                       *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#ifndef DICOM_DATASOURCE_HPP
#define DICOM_DATASOURCE_HPP

#include <QtCore/QHash>
#include <QtCore/QMultiHash>
#include <QtCore/QObject>

#include <QtDicom/Dataset.hpp>
#include <QtDicom/Globals.hpp>


class QXmlStreamReader;

namespace Dicom {

/**
 * The \em DataSource class provides methods for accessing DICOM datasets.
 *
 * To get a number of available datasets, a call to the \ref size() function is
 * required. Then, subsequent invoking the \ref dataset() method allows to read
 * each one of them.
 * The \ref refresh() method was provided to allow reloading internal data and
 * clearing the cache (with the \ref clearCache() function) in sub-classess.
 *
 * The interface is used by the DICOM Service Class Users and Providers to 
 * access datasets from various sources, but the \em DataSource itself serves 
 * only as a base class for further specialization and as such performs no real
 * tasks.
 *
 * You can subclass \em DataSource and use the inheriting object directly;
 * however if you intended to read it from XML stream you'd have to write your
 * own XML parsing routines. To help you, the \em DataSource class comes with
 * its own \ref fromXml() static method and accompanying classes: the \ref 
 * DataSourceFactory and \ref DataSourceCreator. Together they provide means to
 * register a new \em DataSource and make built-in parser aware of its 
 * existance. The parser uses \ref setParameter() method to feed parameters
 * to subclasses.
 * 
 * \author Paweł Żak <pawel.zak@fluxinc.ca>
 */
class QDICOM_DLLSPEC DataSource : public QObject {
	Q_OBJECT;

	public :
		/**
		 * Attempts to read a new \em DataSource from XML \a stream. Since the 
		 * object will be allocated on the heap, it is caller's responsibility 
		 * to either free the memory or through the \a parent argument provide
		 * an object that will take ownership of the data source and deallocate 
		 * it during destruction.
		 *
		 * Method returns a pointer to a newly created \em DataSource object
		 * or -- when an error occurs -- a \c 0. In case of an error and if 
		 * the \a errorMessage parameter is present it will contain explanation 
		 * of the error.
		 */
		static DataSource * fromXml(
			QXmlStreamReader & stream, QObject * parent = 0,
			QString * errorMessage = 0
		);

	public :
		/**
		 * Creates a data source and sets its type \a name and the \a parent 
		 * object.
		 */
		DataSource( const QString & name, QObject * parent = 0 );

		/**
		 * Copy contructor.
		 */
		DataSource( const DataSource & other );

		/**
		 * Destroys a data source.
		 */
		virtual ~DataSource();

		/**
		 * Returns the \a n-th dataset, either from internal cache or through
		 * the \ref readDataset() call.
		 */
		Dataset dataset( int n ) const;

		/**
		 * Refreshes the list of all datasets.
		 */
		void refresh();

		/**
		 * Returns the number of datasets available in the source.
		 *
		 * This method should be reimplemented in sub-classess, \em DataSource
		 * class'es implementation returns \c 0 every time.
		 */
		virtual int size() const;

		/**
		 * Stores data source's properties in an XML \a stream.
		 */
		void toXml( QXmlStreamWriter & stream ) const;

		/**
		 * Returns name of \em DataSource's type.
		 */
		const QString & typeName() const;

	protected :
		/**
		 * Clears internal cache.
		 */
		void clearCache() const;

		/**
		 * Reads the \a n-th dataset and returns it.
		 *
		 * This method should be re-implemented in sub-classess, \em DataSource
		 * class'es implemetation simply returns an empty dataset.
		 */
		virtual Dataset readDataset( int n ) const;

		/**
		 * Returns a list of XML paramters associated with their values.
		 */
		virtual QMultiHash< QString, QString > parameters() const;

		/**
		 * Sets parameter's \a name value \a value.
		 *
		 * This method should be re-inplemented in a sub-class if it intends to
		 * make use of the built-in XML parser.
		 */
		virtual void setParameter( const QString & name, const QString & value );

	private :
		/**
		 * Returns the cache.
		 */
		QHash< int, Dataset > & cache() const;

		/**
		 * Reads the <Parameters> element from XML \a stream.
		 */
		void readXmlParameter( QXmlStreamReader & stream );

	private :
		/**
		 * The cache.
		 */
		mutable QHash< int, Dataset > cache_;

		/**
		 * The type name.
		 */
		const QString & TypeName_;
};

}; // Namespace DICOM ends here.

#endif
