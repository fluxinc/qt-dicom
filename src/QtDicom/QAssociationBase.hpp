/***************************************************************************
 *   Copyright © 2012-2013 by Flux Inc.                                    *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#ifndef	QTDICOM_QASSOCIATIONBASE_HPP
#define QTDICOM_QASSOCIATIONBASE_HPP

#include <QtCore/QObject>
#include <QtCore/QRunnable>

#include <QtDicom/ConnectionParameters.hpp>
#include <QtDicom/Globals.hpp>
#include <QtDicom/UidList.hpp>
#include <QtDicom/QDcmtkResult>


class QHostAddress;

struct T_ASC_Association;
struct T_ASC_Network;
struct T_ASC_Parameters;

/**
 * The \em QAssociationBase class proivdes the base functionality common to all
 * association types.
 *
 * \author Paweł Żak <pawel.zak@fluxinc.ca>
 */
class QDICOM_DLLSPEC QAssociationBase : public QObject {
	Q_OBJECT;

	public :
		enum State {
			Unconnected,
			Requesting,
			Established,
			Releasing,
			Aborting,
			Error,
			Listening
		};

	public :
		/**
		 * Default contructor, creates an association and sets its \a parent 
		 * Qt object, if provided.
		 */
		QAssociationBase( QObject * parent = 0 );

		/**
		 * Overloaded constructor, allows to pre-set connection \a parameters.
		 */
		QAssociationBase(
			const Dicom::ConnectionParameters & parameters, QObject * parent = 0
		);

		/**
		 * Destroys the association.
		 */
		virtual ~QAssociationBase();

		/**
		 * Returns the connection parameters used during last \a request().
		 */
		const Dicom::ConnectionParameters & connectionParameters() const;

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
			const Dicom::ConnectionParameters & parameters
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

	public slots :
		/**
		 * Aborts the association, i.e. sends the A-ABORT message.
		 *
		 * This method merely schedules the abort task and sets object state to
		 * \em Aborting, almost immediately returning to the caller.
		 * 
		 * The actuall business of aborting association is done in the 
		 * background. Association users are notified about the fact that the 
		 * task finished by the \ref disconnected() signal.
		 *
		 * Note, that even unsuccessfull, the abort operation does not give
		 * any error indication, except for logging a warning message.
		 */
		void abort();

	signals :
		/**
		 * This signal is emitted whenever an established association is 
		 * released or aborted -- from client's perspective -- or when its 
		 * release has been acknowledged by the server.
		 *
		 * \sa abort(), 
		 */
		void disconnected();

	protected :
		/**
		 * Drops an association.
		 */
		void dropTAscAssociation();

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

	private slots :
		void startAbortTask();
		void finishAbortTask( QDcmtkResult result );

	private :		
		/**
		 * The DCMTK's association structure.
		 */
		mutable T_ASC_Association * association_;

		/**
		 * Connection parameters used by the association.
		 */
		Dicom::ConnectionParameters connectionParameters_;

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

#endif
