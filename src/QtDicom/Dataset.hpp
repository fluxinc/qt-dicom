/***************************************************************************
 *   Copyright (C) 2011-2012 by Flux Inc.                                  *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#ifndef DICOM_DATASET_HPP
#define DICOM_DATASET_HPP

#include <QtCore/QPair>
#include <QtCore/QSharedDataPointer>

#include <QtDicom/Dataset_priv.hpp>
#include <QtDicom/Globals.hpp>


class DcmItem;
class DcmObject;
class DcmTagKey;

class QTransferSyntax;
class QXmlStreamReader;
class QXmlStreamWriter;

namespace Dicom {

class QDICOM_DLLSPEC Dataset {
	public :
		class ConstIterator;
		typedef ConstIterator const_iterator;

	public :
		Dataset();
		Dataset( const Dataset & other );
		Dataset( const DcmDataset & DCM );
		~Dataset();
		Dataset & operator = ( const Dataset & other );

		bool canConvertToTransferSyntax( const QTransferSyntax & ts ) const;
		const_iterator constBegin() const;
		const_iterator constEnd() const;

		Dataset match( const Dataset & mask ) const;

		bool containsTag( quint16 group, quint16 element ) const;
		bool containsTag( const DcmTagKey & key ) const;

		Dataset convertedToTransferSyntax( const QTransferSyntax & ts ) const;

		DcmDataset & dcmDataset() const;
		bool isEmpty() const;
		bool readXml( QXmlStreamReader & input, QString * errorMessage = 0 );
		bool save( const QString & path ) const;
		void setDcmDataset( const DcmDataset & dataset );
		QByteArray sopClassUid() const;
		QByteArray sopInstanceUid() const;
		QString toString() const;
		bool toDicomFile( const QString & path, QString * errorMessage = 0 ) const;

		/**
		 * Returns value of a tag given by the \a group and \a element 
		 * designators.
		 */
		QString tagValue( quint16 group, quint16 element, bool * exists = 0 ) const;
		QString tagValue( const DcmTagKey & key, bool * exists = 0 ) const;

		QTransferSyntax syntax() const;

		void writeXml( QXmlStreamWriter & output ) const;

	public :
		/**
		 * Reads a Data Set from the DICOM \a file. File should have a valid 
		 * header and conform to DICOM Media Services.
		 */
		static Dataset fromDicomFile( const QString & file, QString * errorMessage = 0 );

		/**
		 * Reads a raw Data Set from the \a file.
		 */
		static Dataset fromFile( const QString & file, QString * errorMessage = 0 );

		/**
		 * Reads a Data Set from XML description file.
		 */
		static Dataset fromXmlFile( const QString & path, QString * errorMessage = 0 );

		/**
		 * Reads a Data Set from XML stream.
		 */
		static Dataset fromXmlStream( QXmlStreamReader & input, QString * errorMessage = 0 );

	private :

	private :
		/**
		 * Matches an \a identifier's item agains the \a mask. If the do match,
		 * a newly created object is returned, otherwise the result is \c 0.
		 */
		bool matchItem( DcmItem & mask, DcmItem & identifier, DcmItem & result ) const;
		DcmElement * matchElement( DcmElement & requestElement, DcmElement & identifierElement ) const;
		DcmSequenceOfItems * matchSequence(
			DcmSequenceOfItems & requestSequence, DcmSequenceOfItems & identifier
		) const;

		/**
		 * Reads all children from current element of the \a input stream and 
		 * stores them in a \a container.
		 */
		void readContainerItems(
			QXmlStreamReader & input, DcmItem & constainer
		) const;

		/**
		 * Parses \em group and \em element attributes from current element
		 * of the \a input stream.
		 */
		QPair< int, int > readGroupAndElementAttributes( 
			QXmlStreamReader & input
		) const;

		/**
		 * Reads a <Item> element and inserts its contents into the
		 * \a sequence.
		 */
		void readItem(
			QXmlStreamReader & input, DcmSequenceOfItems & sequence
		) const;

		/**
		 * Reads a <Sequence> element and inserts its contents into the 
		 * \a container.
		 */
		void readSequence(
			QXmlStreamReader & input, DcmItem & container
		) const;

		/**
		 * Reads a <Tag> element and inserts its contents into the \a container.
		 */
		void readTag(
			QXmlStreamReader & input, DcmItem & container
		) const;

		/**
		 * Writes all children elements of the \a container to the \a output
		 * XML stream.
		 */
		void writeContainerItems(
			QXmlStreamWriter & output, DcmItem & container
		) const;

		/**
		 * Appends \c group and \c element attributes to the current element.
		 */
		void writeGroupAndElementAttributes(
			QXmlStreamWriter & output, DcmElement & element
		) const;

		/**
		 * Creates the <Item> element and with the \ref writeContainerItems()
		 * appends an \a item to the XML \a output stream.
		 */
		void writeItem( QXmlStreamWriter & output, DcmItem & item ) const;

		/**
		 * Starts the <Sequence> element and sends all \a sequence's items
		 * to the \a output XML stream.
		 */
		void writeSequence( 
			QXmlStreamWriter & output, DcmSequenceOfItems & sequence
		) const;

		/**
		 * Creates a <Tag> element in the \a output XML stream and sets its
		 * value to that of a \a tag.
		 */
		void writeTag( QXmlStreamWriter & output, DcmElement & tag ) const;

	private :
		QSharedDataPointer< Dataset_priv > d_;
};

}; // Namespace DICOM ends here.

#endif
