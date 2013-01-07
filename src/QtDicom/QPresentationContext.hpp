/***************************************************************************
 *   Copyright © 2012 by Flux Inc.                                         *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#ifndef QTDICOM_QPRESENTATIONCONTEXT_HPP
#define QTDICOM_QPRESENTATIONCONTEXT_HPP

#include <QtCore/QList>
#include <QtCore/QSharedDataPointer>

#include <QtDicom/QPresentationContextData.hpp>
#include <QtDicom/QUid>

class QTransferSyntax;

struct T_ASC_PresentationContext;

/**
 * The \em QPresentationContext class is responsible for keeping information 
 * about proposed/accepted DICOM presentation contexts during association.
 *
 * \author Paweł Żak <pawel.zak@fluxinc.ca>
 */
class QDICOM_DLLSPEC QPresentationContext {
	public :
		static QPresentationContext defaultFor( const QUid & abstract );
		static QPresentationContext fromTAscPresentationContext(
			const T_ASC_PresentationContext & context
		);

	public :
		QPresentationContext( const QUid & abstract = QUid() );
		QPresentationContext( const QPresentationContext & other );
		~QPresentationContext();

		QPresentationContext & operator << ( const QTransferSyntax & syntax );

		const QUid & abstractSyntax() const;
		void accept( const QTransferSyntax & syntax );
		bool accepted() const;
		const QTransferSyntax & acceptedTransferSyntax() const;
		void addTransferSyntax( const QTransferSyntax & syntax );

		bool isNull() const;
		bool isValid() const;

		const QList< QTransferSyntax > proposedTransferSyntaxes() const;
		void setAbstractSyntax( const QUid & syntax );
		QString toString() const;

	private :
		QSharedDataPointer< QPresentationContextData > data_;
		
};


typedef QList< QPresentationContext > QPresentationContextList;

#endif
