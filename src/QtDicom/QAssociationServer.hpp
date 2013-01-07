/***************************************************************************
 *   Copyright (C) 2011 by Flux Inc.                                       *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#ifndef DICOM_ASSOCIATIONSERVER_HPP
#define DICOM_ASSOCIATIONSERVER_HPP

#include <QtCore/QMutex>
#include <QtCore/QQueue>
#include <QtCore/QThread>

#include "QtDicom/ConnectionParameters.hpp"
#include "QtDicom/Globals.hpp"
#include "QtDicom/UidList.hpp"

class QHostAddress;

struct T_ASC_Network;

namespace Dicom {

class Association;
class ServerAssociation;

class QDICOM_DLLSPEC AssociationServer : public QThread {
	Q_OBJECT;

	public :
		AssociationServer( QObject * parent = 0 );
		~AssociationServer();

		/**
		 * Returns a list of allowed abstract syntaxes.
		 */
		const UidList & abstractSyntaxes() const;

		/**
		 * Stops listening.
		 */
		void close();

		/**
		 * Returns error string.
		 */
		QString errorString() const;

		/**
		 * Returns \c true when server has pending connections.
		 */
		bool hasPendingConnections() const;

		/**
		 * Returns \c true when server is listening for incoming associations.
		 */
		bool isListening() const;

		/**
		 * Starts listening for incoming association on \a address and \a port.
		 *
		 * Returns \c true when successfull, otherwise \c false is returned and
		 * \ref errorString() provides error description.
		 */
		bool listen( const ConnectionParameters & parameters );

		/**
		 * Returns the next pending association as an accepted \ref Association
		 * object.
		 *
		 * It is caller's responsibility to delete retrieved pointer.
		 *
		 * \c 0 is returned when there's no pending connection.
		 */
		ServerAssociation * nextPendingAssociation();

		/**
		 * Sets a list of abstract \a syntaxes for this server to accept.
		 */
		void setAbstractSyntaxes( const UidList & syntaxes );

		/**
		 * Sets a list of transfer \a syntaxes for the server to accept.
		 */
		void setTransferSyntaxes( const UidList & syntaxes );

		/**
		 * Returns a list of allowed transfer syntaxes.
		 */
		const UidList & transferSyntaxes() const;

		/**
		 */
		bool waitForNewAssociation( int msec = 0, bool * timedOut = 0 );

	private :
		/**
		 * Returns default polling interval in miliseconds. 100 is the default.
		 */
		static int pollingInterval();

	private :
		/**
		 * Returns \c true when thread is closing.
		 */
		bool isClosing() const;

		/**
		 * Thread body.
		 */
		void run();

		/**
		 * Forces thread to quit.
		 */
		void setClosingFlag();


	private :
		UidList abstractSyntaxes_;
		// UidList & abstractSyntaxes();

		bool closing_;

		ConnectionParameters connectionParameters_;
		const ConnectionParameters & connectionParameters() const;
		void setConnectionParameters( const ConnectionParameters & );

		mutable QMutex dataLock_;
		QMutex & dataLock() const;

		QString errorString_;
		void raiseError( const QString & message );

		void enqueuePendingAssociation( ServerAssociation * association );
		QQueue< ServerAssociation * > pendingAssociations_;

		T_ASC_Network * tAscNetwork_;
		T_ASC_Network *& tAscNetwork();

		UidList transferSyntaxes_;
		// UidList & transferSyntaxes();

	signals :
		/**
		 * Signal emitted when server fails to establish a new association.
		 *
		 * The \a message parameters contains a description of the error.
		 */
		void newAssociationError( QString message );

		/**
		 * Signal emitted when a new association is available.
		 */
		void newAssociation();
};

}; // Namespace DICOM ends here.

#endif
