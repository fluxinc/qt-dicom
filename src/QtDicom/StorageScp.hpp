/***************************************************************************
 *   Copyright (C) 2011 by Flux Inc.                                       *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#ifndef DICOM_STORAGESCP_HPP
#define DICOM_STORAGESCP_HPP

#include <QtCore/QDir>
#include <QtCore/QObject>

#include "QtDicom/AssociationServer.hpp"
#include "QtDicom/ConnectionParameters.hpp"
#include "QtDicom/Dataset.hpp"
#include "QtDicom/Globals.hpp"


namespace Dicom {

/**
 * The Storage SCP object allows to receive and save DICOM datasets sent with a
 * C-STORE DIMSE command.
 *
 * Storage SCP object allows to choose where incoming Data Sets should be stored.
 * Either to a disk, using temporary folder, or to memory.
 *
 * \author Paweł Żak <pawel.zak@fluxinc.ca>
 */
class QDICOM_DLLSPEC StorageScp : public QObject {
	Q_OBJECT;

	public :
		/**
		 * Specifies place where incoming Data Sets will be stored.
		 */
		enum Destination {
			Unknown,
			Disk,   /*< System's temporary storage space. */
			Memory  /*< \ref Dataset struture. */
		};

	public :
		/**
		 * Returns string representation of the \a destination.
		 */
		static const QString & destinationString( Destination destination );

		/**
		 * Converts the \a value to a \em Destination flag.
		 */
		static Destination destinationFromString( const QString & value );

	public :
		/**
		 * Creates a Storage SCP object and sets its \a parent.
		 */
		StorageScp( QObject * parent = 0 );

		/**
		 * Creates a Storage SCP object and sets both the \a destination and a
		 * \a parent object.
		 */
		StorageScp( Destination destination, QObject * parent = 0 );

		/**
		 * Destroys the Storage SCP object.
		 */
		~StorageScp();

		/**
		 * Returns the directory where Storage SCP will attempt to save incoming
		 * dataset.
		 */
		Destination destination() const;

		/**
		 * Returns human-readable description of the last occured error.
		 */
		QString errorString() const;

		/**
		 * Returns the amount of time the SCP waits before receiving the Data 
		 * Set (after the association has been accepted).
		 */
		const int & holdTime() const;

		/**
		 * Returns AE title of last-connected node.
		 */
		const QString & lastAe() const;

		/**
		 * Returns AE title called by last connected node.
		 */
		const QString & lastCalledAe() const;

		/**
		 * Sets the \a directory where Storage SCP will drop received datasets.
		 */
		void setDestination( Destination destination );

		/**
		 * Sets the amount of \a milliseconds the SCP waits before attempting to
		 * receive the Data Set. The default is \c 0.
		 */
		void setHoldTime( const int & milliseconds = 0 );

		/**
		 * Starts the Storage SCP and binds it to the TCP port number
		 * provided in the \a parameters.
		 *
		 * Returns \c true on success. In case of an error \c false is 
		 * returned and the \ref errorString() routine provides 
		 * the explanation.
		 */
		bool start( const ConnectionParameters & parameters );

		void stop();

	private :
		/**
		 * Forward definition of the Receiver thread.
		 */
		class ReceiverThread;

	private :
		/**
		 * Returns the associatoin server.
		 */
		const AssociationServer & associationServer() const;
		AssociationServer & associationServer();

		/**
		 * Raises an error with the \a description.
		 */
		void raiseError( const QString & description );

	private slots :
		/**
		 * Called each time a new assocaition has been successfully received.
		 * Spawns a thread that handles incoming DIMSE messages.
		 */
		void createReceiverThread();

	private :
		/**
		 * The assocaition server.
		 */
		AssociationServer associationServer_;

		/**
		 * The destination.
		 */
		Destination destination_;

		/**
		 * The error string.
		 */
		QString errorString_;

		int holdTime_;

		QString lastAe_;
		QString lastCalledAe_;

	signals :
		/**
		 * Signal emitted when C-ECHO request was received and handled. The AE
		 * parameter contains Application Entity title of the requesting node.
		 */
		void echoReceived( QString AE );

		/**
		 * Signal emitted when the Storage SCP failed to store a file. The \a 
		 * message parameter contains an explanation of the error.
		 */
		void failedToStore( QString message );		

		/**
		 * Signal emitted when the Storage SCP successfully stored a DICOM 
		 * dataset in the \a path.
		 */
		void stored( QString path );

		/**
		 * Signal emitted when Storage SCP successfully stores a DICOM Data
		 * Set in the \a dataSet struvture.
		 */
		void stored( Dicom::Dataset dataSet );

};

}; // Namespace DICOM ends here.

#endif
