/***************************************************************************
 *   Copyright (C) 2011 by Flux Inc.                                       *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#ifndef	DICOM_ASSOCIATION_HPP
#define DICOM_ASSOCIATION_HPP

#include <QtCore/QObject>

#include "QtDicom/ConnectionParameters.hpp"
#include "QtDicom/Globals.hpp"
#include "QtDicom/UidList.hpp"

class QHostAddress;

struct T_ASC_Association;
struct T_ASC_Network;
struct T_ASC_Parameters;

namespace Dicom {

/**
 * \em Association class proivdes a DICOM association object.
 */
class QDICOM_DLLSPEC Association : public QObject {
	Q_OBJECT;

	public :
		enum State {
			Disconnected,
			Established,
			Error
		};

	public :
		/**
		 * Default contructor, creates an association and sets its \a parent 
		 * Qt object, if provided.
		 */
		Association( QObject * parent = 0 );

		/**
		 * Overloaded constructor, allows to pre-set connection \a parameters.
		 */
		Association( const ConnectionParameters & parameters, QObject * parent = 0 );

		/**
		 * Destroys the association.
		 */
		virtual ~Association();

		/**
		 * Aborts the association, i.e. sends the A-ABORT message, and changes
		 * \ref state() to \em Disconnected.
		 */
		void abort();

		/**
		 * Returns ID of an accepted presentation context that is using
		 * \a abstract and \a transfer syntaxes.
		 *
		 * If such a presentation context wasn't accepted during association, 
		 * a value of \c 0 is returned.
		 *
		 * In case of an error method returns with \c -1.
		 */
		int acceptedPresentationContextId( 
			const char * abstract, const char * transfer = 0
		) const;

		/**
		 * Returns the connection parameters used during last \a request().
		 */
		const ConnectionParameters & connectionParameters() const;

		/**
		 * Returns the error message.
		 */
		const QString & errorMessage() const;

		/**
		 * Returns \c true if an error occured during last operation.
		 */
		bool hasError() const;

		/**
		 * Returns \c true when association has been established successfully.
		 */
		bool isEstablished() const;

		/**
		 * Returns calling (in case of requestor) or responsing (acceptor)
		 * Application Entity title.
		 */
		const QString & myAeTitle() const;

		/**
		 * Returns a maximum nunber of Protocol Data Units the association can
		 * handle during negotiaiton.
		 */
		unsigned maxPdu() const;

		/**
		 * Generates a new, unique for this association, message ID.
		 */
		quint16 nextMessageId();

		/**
		 * Returns a port number the requestor is connecting to or acceptor
		 * is listening on.
		 */
		quint16 port() const;

		/**
		 * Sets connection \a parameters for the next association request.
		 *
		 * If either \ref timeout() or a \a port() differs from the previous
		 * settings, method aborts current association (if present) and drops
		 */
		void setConnectionParameters(
			const ConnectionParameters & parameters
		);

		/**
		 * Return current state of the association.
		 */
		const State & state() const;

		/**
		 * Returns the amount of seconds used as a timeout value.
		 */
		int timeout() const;

		/**
		 * Returns a pointer to DCMTK's association structure.
		 */
		T_ASC_Association *& tAscAssociation() const;

	protected :
		/**
		 * Drops an association.
		 */
		virtual void dropTAscAssociation();

		/**
		 * The DCMTK's network structure.
		 */
		void dropTAscNetwork();

		/**
		 * Initializes DCMTK's netwrok structure.
		 *
		 * The \a role parameters can be either:
		 * - \c -1 for Service Users,
		 * - \c  1 for Service Providers,
		 * - \c  0 for mixed operations.
		 */
		bool initializeTAscNetwork( int role );
		
		/**
		 * Raises an error flag and sets the error \a message.
		 */
		void raiseError( const QString & message );

		/**
		 * Sets state of the association.
		 */
		void setState( State state );

		/**
		 * Returns DCMTK's network structure.
		 */
		T_ASC_Network *& tAscNetwork() const;		

	private :		
		/**
		 * The DCMTK's association structure.
		 */
		mutable T_ASC_Association * association_;

		/**
		 * Connection parameters used by the association.
		 */
		ConnectionParameters connectionParameters_;

		/**
		 * The error message.
		 */
		QString errorMessage_;

		/**
		 * The DCMTK's network structure.
		 */
		mutable T_ASC_Network * network_;

		/**
		 * The current state.
		 */
		State state_;
};

}; // Namespace DICOM ends here.


#endif
