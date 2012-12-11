/***************************************************************************
 *   Copyright (C) 2011 by Flux Inc.                                       *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#ifndef QDICOM_ACCEPTORASSOCIATION_HPP
#define QDICOM_ACCEPTORASSOCIATION_HPP

#include "QtDicom/Association.hpp"
#include "QtDicom/Globals.hpp"


namespace Dicom {

/**
 * Acceptor association represents a provider-side of association negotiation.
 *
 * The class allows to \ref receive() an association in either blocking or 
 * non-blocking fashion. Once received, the association can be examined with
 * \ref callingAe() and \ref calledAe() methods for security reasons and then
 * accepted or rejcted with, respectively \ref accept() and \ref reject() 
 * methods.
 *
 * When peer requests association to be released, the \ref confirmRelease()
 * method should be invoked to acknowledge that request.
 *
 * \author Paweł Żak <pawel.zak@fluxinc.ca>
 */
class QDICOM_DLLSPEC AcceptorAssociation : public Association {
	Q_OBJECT;

	public :
		/**
		 * Creates an acceptor association and sets its \a parent.
		 */
		AcceptorAssociation( QObject * parent = 0 );

		/**
		 * Overloaded constructor, creates an acceptor associations ans sets 
		 * its connection \a parameters and a \a parent object.
		 */
		AcceptorAssociation( 
			const ConnectionParameters & parameters,
			QObject * parent = 0
		);

		/**
		 * Destroys the association.
		 */
		virtual ~AcceptorAssociation();

		/**
		 * Accepts proposed presentation contexts using the \a abstract and
		 * \a transfer syntaxes list.
		 *
		 * The function combines each abstract syntax from the list with every
		 * transfer syntax to create a set of presentation contexts, then 
		 * compares it with those received from the requestor and accepts
		 * the common ones.
		 *
		 * Returns \c true unless an error occured. In such case \c false is 
		 * returned and \ref hasError() indicates \c true as well.
		 *
		 * \note This method do not abort association on error. It's callers
		 *       responsibility.
		 */
		bool accept(
			const UidList & abstract,
			const UidList & transfers
		);

		/**
		 * Returns AE title that was called during last association request.
		 *
		 * The function can be invoked only after \ref receive() returned 
		 * successfully. At any other circumstances an empty string is returned.
		 */
		QString calledAeTitle() const;

		/**
		 * Returns requestor's AE title of the last association.
		 *
		 * The function can be invoked only after \ref receive() returned 
		 * successfully. At any other circumstances an empty string is returned.
		 */
		QString callingAeTitle() const;

		/**
		 * Sends a confirming message to an A-RELEASE request.
		 *
		 * This method is used to acknowledge peer releasing an association.
		 * Returns \c true when succeeded. \c false is returned if error 
		 * occured, in this case \ref hasError() will return \c true as well.
		 */
		bool confirmRelease();

		/**
		 * Returns a text block containing parameters requested by last-received
		 * association request.
		 */
		QString parametersText() const;

		/**
		 * Initiates a network and listents on specified through connection 
		 * \a parameters port for an incoming association.
		 *
		 * The \em timeout from connection \a parameters specifies the amount of 
		 * seconds method waits for a new connection. When it is set to \c -1,
		 * calling thread is blocked until a connection is received. Otherwise, 
		 * if for \em timeout seconds no association has been established, method 
		 * returns with \c false and the \a timedOut flag (if present) is set.
		 *
		 * When an association has been received, \c true is returned.
		 *
		 * In case of an error, the method returns \c false, the \a timedOut
		 * flag is unset and the \ref hasError() method returns \c true.
		 */
		bool receive( 
			const ConnectionParameters & parameters, bool * timedOut = 0
		);

		/**
		 * Overloaded method provided for conveniance.
		 *
		 * Receives an association on a pre-set \ref connectionParameters().
		 */
		bool receive( bool * timedOut = 0 );
};

}; // Namespace DICOM ends here.


#endif
