/***************************************************************************
 *   Copyright (C) 2011 by Flux Inc.                                       *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#ifndef DICOM_REQUESTORASSOCIATION_HPP
#define DICOM_REQUESTORASSOCIATION_HPP

#include <QtDicom/Association.hpp>
#include <QtDicom/Globals.hpp>
#include <QtDicom/QPresentationContext>

#include <QtCore/QMultiHash>


namespace Dicom {

/**
 * The \em RequestorAssociation represents client side of association
 * establishment process.
 *
 * With the \ref request() and \ref release methods the \em RequestorAssociation
 * class provides means to connect other DICOM entity.
 *
 * \author Paweł Żak <pawel.zak@fluxinc.ca>
 */
class QDICOM_DLLSPEC RequestorAssociation : public Association {
	Q_OBJECT;


	public :
		/**
		 * Default constructor.
		 */
		RequestorAssociation( QObject * parent = 0 );

		/**
		 * Overloaded constructor, allows to set connection \a parameters.
		 */
		RequestorAssociation( 
			const ConnectionParameters & parameters, QObject * parent = 0 
		);

		/**
		 * Destroys the association.
		 */
		~RequestorAssociation();

		/**
		 * Returns the list of accepted presentation contexts.
		 */
		QList< QPresentationContext > acceptedPresentationContexts() const;

		/**
		 * Returns a text block containing parameters accepted during last-issued
		 * association request.
		 */
		QString parametersText() const;		

		/**
		 * Releases an association, i.e. sends the A-RELEASE messages, and
		 * and frees allocated memory.
		 */
		bool release();

		/**
		 * Attempts to connect a remote host and request an association 
		 * using the \a parameters.
		 */
		int RequestorAssociation::request(
			const QList< QPresentationContext > & contexts,
			bool * timedOut
		);
		int request(
			const QByteArray & abstractSyntax,
			const QByteArray & transferSyntax,
			bool * timedOut = 0
		);
		int request(
			const UidList & abstractSyntaxes,
			const UidList & transferSyntaxes,
			bool * timedOut = 0
		);
		int request(
			const ConnectionParameters & parameters,
			const QByteArray & abstractSyntax,
			const QByteArray & transferSyntax,
			bool * timedOut = 0
		);
		int request(
			const ConnectionParameters & parameters,
			const UidList & abstractSyntaxes,
			const UidList & transferSyntaxes,
			bool * timedOut = 0
		);
		int request(
			const ConnectionParameters & parameters,
			const UidList & abstractSyntaxes,
			bool * timedOut = nullptr
		);
		int request(
			const ConnectionParameters & parameters,
			const QByteArray & abstractSyntax,
			bool * timedOut = nullptr
		);

	private :
		/**
		 * Fills the association \a parameters with the AE titles.
		 */
		void setAeTitles(
			T_ASC_Parameters *& parameters,
			const QString & myAeTitle,
			const QString & hostAeTitle
		) const;

		/**
		 * Fills the assocaition \a parameters with presentation addresses.
		 */
		void setConnectionSettings(
			T_ASC_Parameters *& parameters,
			const QHostAddress & hostAddress,
			quint16 port
		) const;

		/**
		 * Fills the association \a parameters structure with the presentation
		 * contexts.
		 */
		void setPresentationContexts(
			T_ASC_Parameters *& parameters,
			const QList< QPresentationContext > & contexts
		) const;

};

}; // Namespace DICOM ends here.

#endif
