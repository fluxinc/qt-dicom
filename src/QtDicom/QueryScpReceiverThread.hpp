/***************************************************************************
 *   Copyright (C) 2011 by Flux Inc.                                       *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#ifndef DICOM_QUERYSCP_RECEIVERTHREAD_HPP
#define DICOM_QUERYSCP_RECEIVERTHREAD_HPP

#include <QtCore/QMutex>
#include <QtCore/QThread>
#include <QtCore/QQueue>

#include "QtDicom/Dataset.hpp"
#include "QtDicom/Globals.hpp"
#include "QtDicom/QueryScp.hpp"
#include "QtDicom/ServiceProvider.hpp"

namespace Dicom {

class AcceptorAssociation;

class QDICOM_DLLSPEC QueryScp::ReceiverThread : public QThread, private ServiceProvider {
	Q_OBJECT;

	public :
		ReceiverThread( AcceptorAssociation * association, QObject * parent = 0 );
		~ReceiverThread();

		bool finished() const;
		bool queryFinishing() const;
		bool hasQueuedIdentifiers() const;
		bool receivingCommands() const;

	public slots :
		void finish( int status = 0 );
		void queueIdentifier( Dataset dataset );

	private :
		enum State {
			Idle = 0,
			ReceivingCommands,
			QueryInProgress,
			QueryFinishing,
			Finished
		};

	private :
		AcceptorAssociation * association();
		QMutex & dataLock() const;
		QQueue< Dataset > & queue();
		const QQueue< Dataset > & queue() const;
		void run();
		void sendCancelConfirmation( 
			const T_DIMSE_C_FindRQ & request, unsigned char ID
		);
		void sendIdentifier(
			const T_DIMSE_C_FindRQ & request, unsigned char ID, 
			const Dataset & dataset
		);
		void sendStatus( 
			const T_DIMSE_C_FindRQ & request, unsigned char ID, int status
		);
		void setState( State state );
		State state() const;
		int status() const;

	private :
		mutable QMutex dataLock_;
		QQueue< Dataset > queue_;
		State state_;
		int status_;

	signals :
		void failedToQuery( QString message, ReceiverThread * );
		void newQuery( Dataset dataset, ReceiverThread * );
};

}; // Namespace DICOM ends here.

#endif
