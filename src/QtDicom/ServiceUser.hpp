/***************************************************************************
 *   Copyright (C) 2011 by Flux Inc.                                       *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#ifndef DICOM_NAMESAPCEUSER_HPP
#define DICOM_NAMESAPCEUSER_HPP

#include "QtDicom/AbstractService.hpp"
#include "QtDicom/Association.hpp"
#include "QtDicom/Dataset.hpp"
#include "QtDicom/Globals.hpp"


class DcmDataset;
class QSopClass;

struct T_DIMSE_Message;

namespace Dicom {

class QDICOM_DLLSPEC ServiceUser : public AbstractService {
	public :
		ServiceUser();
		ServiceUser( Association * association );
		virtual ~ServiceUser();

		/**
		 * Using the association performs a C-ECHO operation.
		 * 
		 * Returns \c true when SCP responds with a \c SUCCESS status, otherwise
		 * \c false is the result. In case of an error, an appriopriate flag is 
		 * set, the \ref hasError() returns \c true and the \ref errorMessage() 
		 * an explanation.
		 */
		bool cEcho();

		/**
		 * Performs a C-FIND operation.
		 *
		 * Query with a \a dataset using the \a SOP class. Requires established
		 * \ref association(). Returns a list of retrieved identifiers. In case
		 * of an error, an empty list is returned and the \ref hasError()
		 * returns \c true.
		 */
		QList< Dataset > cFind( 
			const Dataset & dataset, const char * SOP
		);

		/**
		 * Performs a C-MOVE operation.
		 *
		 * Selects one or more SOP Instances from called AE using unique key 
		 * elements stored in the \a dataset and with corresponding \a SOP class;
		 * next instructs called AE to store them to destination \a AE.
		 *
		 * Returns a number of completed sub-operations. In addition, the 
		 * \a warned and \a failed output parameters allow to retrieve a number 
		 * of store sub-operations that finished with warnings or failed.
		 *
		 * In case of an error return value depends on the following conditions
		 * - when a method was able to read the number of \a failed instances
		 *   it is equal to \c 0,
		 * - otherwise \c -1 is returned.
		 * Either way the \ref hasError() flag is set.
		 *
		 * \note The number of failed store sub-operations is irrelevant when
		 *       status of this method operation is evaluated; that is, even if
		 *       none sub-operations completed successfully, when the DIMSE
		 *       protocol or connection was not violated in any way, the \ref
		 *       hasError() will return \c false.
		 */
		int cMove( 
			const Dataset & dataset, const char * SOP, const QString & AE,
			int * failed = 0, UidList * failedInstances = 0,
			int * warned = 0
		);

		/**
		 * Performs a C-STORE operation.
		 *
		 * Attempts to store a \a dataset on a PACS using the \a SOP class and
		 * the \a transfer syntax.
		 */
		bool cStore( 
			const Dataset & dataset
		);
		bool cStore( 
			const Dataset & dataset,
			const QString & moveAe, int moveId
		);


		/**
		 * Performs an N-ACTION operation.
		 */
		bool nAction(
			const char * Class,
			const char * Instance,
			const quint16 & action,
			const Dataset & information,
			Dataset * reply = nullptr,
			quint16 * status = nullptr
		);
		bool nAction(
			const char * Class,
			const char * Instance,
			const quint16 & action,
			quint16 * status = nullptr
		);
		bool nAction(
			const QSopClass & Class,
			const char * Instance,
			const quint16 & action,
			const Dataset & information,
			Dataset * reply = nullptr,
			quint16 * status = nullptr
		);
		bool nAction(
			const QSopClass & Class,
			const char * Instance,
			const quint16 & action,
			quint16 * status = nullptr
		);

		/**
		 * Performs a N-CREATE operation. Returns affected SOP Instance UID
		 * (when applicable).
		 */
		QByteArray nCreate( 
			const char * Class,
			const char * Instance = nullptr,
			const Dataset & attributes = Dataset(),
			Dataset * created = nullptr,
			quint16 * status = nullptr 
		);
		QByteArray nCreate( 
			const QSopClass & Class,
			const char * Instance = nullptr,
			const Dataset & attributes = Dataset(),
			Dataset * created = nullptr,
			quint16 * status = nullptr 
		);
		QByteArray nCreate( 
			const char * SopClass,
			const Dataset & attributes,
			Dataset * created = nullptr,
			quint16 * status = nullptr 
		);
		QByteArray nCreate( 
			const QSopClass & SopClass,
			const Dataset & attributes,
			Dataset * created = nullptr,
			quint16 * status = nullptr 
		);

		/**
		 * Performs N-DELETE operation. Returns \c true on success.
		 */
		bool nDelete(
			const char * Class,
			const char * Instance,
			quint16 * status = nullptr
		);
		bool nDelete(
			const QSopClass & Class,
			const char * Instance,
			quint16 * status = nullptr
		);

		/**
		 * Performs a N-SET operation. Returns affected SOP Instance UID
		 * and a dataset of \a modified attributes (when applicable).
		 */
		QByteArray nSet( 
			const char * Class,
			const char * Instance,
			const Dataset & attributes = Dataset(),
			Dataset * modified = nullptr,
			quint16 * status = nullptr 
		);


	private :
		/**
		 */
		Dataset processNEventReport(
			const T_DIMSE_Message &, unsigned char &
		);

		/**
		 * Validates a C-ECHO \a response which was received after issuing the
		 * \a request. Method throws an exception if any abnormality is found.
		 */
		void validateCEchoResponse(
			const T_DIMSE_Message & response,
			const T_DIMSE_Message & request
		);
		
		/**
		 * Validates a C-MOVE \a response which was received after sending the
		 * \a request. Throws an exception if any abnormality is found.
		 *
		 * Returns \c true if \a response was final.
		 */
		bool validateCFindResponse(
			const T_DIMSE_Message & response,
			const T_DIMSE_Message & request
		);

		/**
		 * Validates a C-MOVE \a response which was received after sending the
		 * \a request. Throws an exception if any abnormality is found.
		 *
		 * Returns \c true if \a response was final.
		 */
		bool validateCMoveResponse(
			const T_DIMSE_Message & response,
			const T_DIMSE_Message & request
		);

		/**
		 * Validates a C-STORE \a response which was received after issuing the
		 * \a request. Method throws an exception if any abnormality is found.
		 */
		void validateCStoreResponse(
			const T_DIMSE_Message & response,
			const T_DIMSE_Message & request
		);

		/**
		 * Validates a N-ACTION \a response to \a request. This method throws
		 * an exception if response status indicates a fatal error. In case of
		 * less severe errors, only a warning message is printed.
		 */
		void validateNActionResponse(
			const T_DIMSE_Message & response,
			const T_DIMSE_Message & request
		);

		/**
		 * Validates a N-CREATE \a response to \a request. This method throws
		 * an exception if response status indicates a fatal error. In case of
		 * less severe errors, only a warning message is printed.
		 */
		void validateNCreateResponse(
			const T_DIMSE_Message & response,
			const T_DIMSE_Message & request
		);


		/**
		 * Validates a N-DELETE \a response to \a request. This method throws
		 * an exception if response status indicates a fatal error. In case of
		 * less severe errors, only a warning message is printed.
		 */
		void validateNDeleteResponse(
			const T_DIMSE_Message & response,
			const T_DIMSE_Message & request
		);

		/**
		 * Validates a N-SET \a response to \a request. This method throws
		 * an exception if response status indicates a fatal error. In case of
		 * less severe errors, only a warning message is printed.
		 */
		void validateNSetResponse(
			const T_DIMSE_Message & response,
			const T_DIMSE_Message & request
		);		
};

}; // Namespace DICOM ends here.


#endif
