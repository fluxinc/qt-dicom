/***************************************************************************
 *   Copyright (C) 2011 by Flux Inc.                                       *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#ifndef DICOM_ABSTRACTSERVICE_HPP
#define DICOM_ABSTRACTSERVICE_HPP

#include <QtCore/QString>

#include <QtDicom/Globals.hpp>

struct T_DIMSE_Message;

namespace Dicom {

class Association;
class Dataset;

/**
 * The \em AbstractService is a base class for two kinds of DICOM entities:
 * service class users (SCUs) and service class providers (SCPs).
 *
 * The class provides common methods and mechanisms. Above all it allowes
 * to store a pointer an association object with the parameterized
 * constructor and the \ref setAssociation() method. Apart from that,
 * \ref hasError() and \ref errorMessage() methods were provided for basic
 * error reporting features.
 *
 * \author Paweł Żak <pawel.zak@fluxinc.ca>
 */
class QDICOM_DLLSPEC AbstractService {
	public :
		/**
		 * Returns address of an association being used by a service.
		 */
		Association * association();
		const Association * association() const;		

		/**
		 * Returns error string of last error.
		 */
		const QString & errorString() const;

		/**
		 * Returns \c true when error occured during last operation.
		 */
		bool hasError() const;

		/**
		 * Sets \a association to be used by DIMSE messages.
		 */
		void setAssociation( Association * association );

	protected :
		/**
		 * Returns name of a DIMSE \a command. The \a command parameter is 
		 * DCMTK's enumerator value.
		 */
		static const char * commandName( int command );

	protected :
		AbstractService();
		AbstractService( Association * association );
		virtual ~AbstractService();
		
		/**
		 * Clears the error flag and the error message.
		 */
		void clearErrorStatus();

		/**
		 * Ignores a dataset from the command.
		 */
		void ignoreDataset();

		/**
		 * When a \a message is a non-empty string, sets both the error flag 
		 * and the accompanying \a message.
		 */
		void raiseError( const QString & message );

		/**
		 * Attempts to receive a DIMSE \a command using pre-set \ref association()
		 * for \a timeoutSecs seconds. 
		 *
		 * Returns the received message and the presentation context \a ID 
		 * which was used to transfer messages.
		 *
		 * The \a released flag is used to indicate that peer requested 
		 * association to be released. Note, that if caller doesn't use this
		 * flag, in case of a release request an exception will be thrown as
		 * with the other kinds of errors.
		 * Similarily, the \a timedOut flag is used to mark the situation when
		 * the command couldn't be received during \a timeoutSecs.
		 *
		 * \note Any error except abort request causes this method to abort the
		 *       association.
		 */
		T_DIMSE_Message receiveCommand(
			int command, int timeoutSecs, unsigned char & ID,
			bool * released = 0, bool * timedOut = 0
		);

		/**
		 * Overloaded method provided for conveniance. Attempts to receive any
		 * command and uses \ref association()'s timeout.
		 */
		T_DIMSE_Message receiveCommand(
			unsigned char & ID,
			bool * released = 0, bool * timedOut = 0
		);

		/**
		 * Overloaded method provided for conveniance. Attempts to receive any
		 * command for \a timeoutSecs second.
		 */
		T_DIMSE_Message receiveCommand(
			int timeoutSecs, unsigned char & ID,
			bool * released = 0, bool * timedOut = 0
		);

		/**
		 * Receives a dataset.
		 */
		Dataset receiveDataset( unsigned char & ID );

		/**
		 * Sends a DIMSE \a command using the \ref association() and the 
		 * presentation context \a ID.
		 */
		void sendCommand(
			const T_DIMSE_Message & command, unsigned char ID
		);
		void sendCommand(
			const T_DIMSE_Message & command, const Dataset & dataset, unsigned char ID
		);

	private :

		/**
		 * Callback for the \ref sendCommand(), \ref receiveCommand() and \ref 
		 * receiveDataset() methods, set to process Qt events if application is
		 * running.
		 */
		static void progressCallback( void * data, unsigned long bytes );

	private :
		Association * association_;
		QString errorString_;
};

}; // Namespace DICOM ends here.

#endif
