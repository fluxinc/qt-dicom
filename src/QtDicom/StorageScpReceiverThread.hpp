/***************************************************************************
 *   Copyright (C) 2011 by Flux Inc.                                       *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#ifndef DICOM_STORAGESCP_RECEIVERTHREAD_HPP
#define DICOM_STORAGESCP_RECEIVERTHREAD_HPP

#include "QtDicom/Globals.hpp"
#include "QtDicom/ServiceProvider.hpp"
#include "QtDicom/StorageScp.hpp"

#include <QtCore/QThread>

class QDir;

namespace Dicom {

class AcceptorAssociation;

class QDICOM_DLLSPEC StorageScp::ReceiverThread : public QThread, public ServiceProvider {
	Q_OBJECT;

	public :
		/**
		 * Creates a receive thread object and sets its \a association and
		 * \a destination.
		 */
		ReceiverThread( 
			AcceptorAssociation * association,
			const Destination & destination,
			const QSharedPointer< QAtomicInt > & holdTimePtr,
			QObject * parent = 0
		);

		/**
		 * Destroys a receiver thread and, if necessarry, aborts the
		 * \ref association() and deallocates memory.
		 */
		~ReceiverThread();

	private :
		/**
		 * Thread's body.
		 * 
		 * Uses \ref association() to retrieve and handle DIMSE commands. 
		 * Only two are supported: C-ECHO-RQ and C-STORE-RQ.
		 */
		void run();

	private :
		/**
		 * Returns the association.
		 */
		AcceptorAssociation * association();

		/** 
		 * Creates a file in the \a directory.
		 */
		QString createUniquePath( const QDir & directory );

		/**
		 * Returns the destination.
		 */

	private :
		const Destination & destination() const;
		const Destination Destination_;

		int holdTime() const;
		const QSharedPointer< QAtomicInt > HoldTimePtr_;

	signals :
		/**
		 * Signal emitted when C-ECHO request was received and handled. The AE
		 * parameter contains Application Entity title of the requesting node.
		 */
		void echoReceived( QString AE );

		/**
		 * Signal emitted when the Storage SCP thread failed to store a DICOM
		 * dataset. The \a message contains an error explanation.
		 */
		void failedToStore( QString message );

		/**
		 * Signal emitted when a dataset was successfully stored in a \a path.
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
