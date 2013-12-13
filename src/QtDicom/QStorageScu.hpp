/***************************************************************************
 *   Copyright (C) 2011-2012 by Flux Inc.                                  *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#ifndef QSTORAGESCU_HPP
#define QSTORAGESCU_HPP

#include <QtCore/QObject>
#include <QtCore/QList>

#include <QtDicom/Globals.hpp>
#include <QtDicom/ServiceUser.hpp>
#include <QtDicom/QPresentationContext>
#include <QtDicom/QTransferSyntax>
#include <QtDicom/QUid>


namespace Dicom {
	class ConnectionParameters;
	class RequestorAssociation;
}


/**
 * The \em QStorageScu class provides a simple DICOM storage client 
 * implementation.
 *
 *
 * \em QStorageScu class allows to transfer objects of \ref Dicom::Dataset type
 * to any DICOM node providing storage service. Before the store operations 
 * takes place, however, it is essential to let the SCU know what are the SOP 
 * classes to be transferred, and what is the preferred transfer syntax. This
 * can be accomplished with \ref setSopClasses() and \ref setTransferSyntax()
 * methods.
 *
 * DICOM association is requested with the call to \ref connectToAe() method.
 * This method returns almost immediately, changing object's internal state 
 * to \em Requesting, which triggers the connect process once the event loop 
 * reaches it. After the successfull connection is made, the \ref connected()
 * signal is emitted, internal state changes to \em Connected and SCU is ready 
 * to store.
 *
 * Store operations begins when the \ref store() method is invoked with valid
 * DICOM Data Set as a parameter. The Data Set must belong to one of SOP 
 * Classes specified before the connection began via the \ref setSopClasses().
 * The Data Set also must also be represented by selected transfer syntax or be
 * convertable to it. When above is false, the \ref store() method attempts to
 * decompress the Data Set to the default DICOM transfer syntax: the Little
 * Endian Implicit VR. When a Data Set is successfully stored, SCU emits the 
 * \ref stored() signal with Instance UID of stored object.
 *
 * After all Data Sets have been transferred, user can release the association
 * using the \ref disconnectFromAe() method. The SCU is ready again to connect
 * when the \ref disconnected() signal is emitted and object state goes back to
 * \em Disconnected.
 *
 *
 * The \em QStorageScu report errors using the \ref error() signals and by 
 * changing its state to \em Error. This releases association (if it was already
 * established) and nullify any further requests to store. Only another call to
 * \ref connectToAe() can reset the error flag. Note, that if error is reported
 * when SCU is connected to SCP, the \ref error() signal is immediately
 * followed by \ref disconnected().
 *
 * \author Paweł Żak <pawel.zak@fluxinc.ca>
 */
class QDICOM_DLLSPEC QStorageScu : public QObject {
	Q_OBJECT;

	public :
		/**
		 * The \em Error type represents error codes reported by \em 
		 * QStorageScu via the \ref error() function.
		 */
		enum Error {
			NoError = 0, /*< 
			  Everything went successfully thus far. */
			InvalidConnectionParameters, /*<
			  Either connection parameters provided by the \ref 
			  setConnectionParameters() method were invalid or no connection
			  parameters have been provided at all. */
			Timeout, /*<
			  Timeout occured during network communication. */
			SopClassNotSupported, /*<
			  SCP does not support all of the SOP classes provided via 
			  \ref setSopClasses() method. */
			AssociationError, /*<
			  Error occured during association negotiation. Further information
			  can be obtained using the \ref associationErrorString() method. */
			DimseError, /*<
			  An error occured during DICOM message exchange process. Furhter
			  information can be obtained from the \ref dimseErrorString() 
			  method. */
			InvalidSopClass, /*<
			  Either no SOP Class has been specified at all, or the Data Set 
			  requested to be transferred belongs to the SOP Class that wasn't
			  negotiated during association request (wasn't provided with the 
			  \ref setSopClasses() method). */
			InvalidTransferSyntax, /*<
			  The Data Set requested to be transferred has Transfer Syntax 
			  incompatible with both the preferred transfer syntax provided by 
			  the \ref setTransferSyntax() and the default DICOM TS: Little
			  Endian Explicit. */
			UnknownError /*<
			  An unknown or uncategorized error occured. */
		};

		enum State {
			Disconnected,
			Requesting,
			Connected,
			Sending
		};

	public :
		/**
		 * Creates an SCU and sets its \a parent.
		 */
		QStorageScu( QObject * parent = 0 );

		/**
		 * Destroys the SCU, disconnecting the AE if necessary.
		 */
		~QStorageScu();

		/**
		 * Returns error message reported by the association layer. The message
		 * is only meaningfull when the \ref error() gives \em AssociationError.
		 */
		QString associationErrorString() const;

		/**
		 * Returns error message reported by DIMSE layer. The message is
		 * only meaningfull when the \ref error() gives \em DimseError.
		 */
		QString dimseErrorString() const;

		/**
		 * An overloaded method. Sets the single \a SOP class, the transfer
		 * \a syntax and invokes the \ref connectToAe() slot.
		 */
		void connectToAe(
			const QUid & SOP, const QTransferSyntax & syntax = QTransferSyntax()
		);

		/**
		 * Returns error code of the error that occured during last operation.
		 */
		Error error() const;

		/**
		 * Returns string with message explaining the last error.
		 */
		QString errorString() const;

		/**
		 * Returns \c true when SCU state is equal to \em Error, i.e. when the 
		 * last operation hasn't completed successfully.
		 */
		bool hasError() const;

		/**
		 * Sets connection \a parameters. The parameters will be used next time
		 * the \ref connectToAe() method is called.
		 */
		void setConnectionParameters(
			const Dicom::ConnectionParameters & parameters
		);

		/**
		 * Sets the list of SOP classes to be used during next association 
		 * negotiation (invoked by \ref connectToAe()) to the UIDs.
		 */
		void setSopClasses( const QList< QUid > & UIDs );

		/**
		 * Sets preferred transfer syntax to \ref syntax. The SCU always propose
		 * this transfer syntax to the SCP for every specified SOP class during
		 * association negotiation.
		 */
		void setTransferSyntax( const QTransferSyntax & syntax );

		/**
		 * Returns current state of the SCU.
		 */
		State state() const;

	public slots :
		/**
		 * Connects to AE using connection paramters, SOP class(es) and 
		 * preferred transfer syntax pre-specified with \ref 
		 * setConnectionParameters(), \ref setSopClasses() and \ref 
		 * setTransferSyntax() methods, respectively.
		 */
		void connectToAe();

		/**
		 * Disconnects the AE, releasing the association. This method returns
		 * immediately, the \ref disconnected() signal is emitted when SCU 
		 * finishes releasing the association and is ready to make a new 
		 * connection.
		 */
		void disconnectFromAe();

		/**
		 * Sends \a dataset to AE. The SCU should be in \a Connected state and
		 * the \ref dataset should belong to one of the specified SOP classes.
		 * This methods returns immediately and the sore storage process is
		 * performed in the background.
		 */
		void store( Dicom::Dataset dataset );

	signals :
		/**
		 * Signals that the \ref connectToAe() method succeded: association 
		 * has been successfully established and all requested SOP classes are 
		 * supported by the server.
		 */
		void connected();

		/**
		 * Signals that the \ref disconnectFromAe() request was handled: the
		 * SCU released the association.
		 */
		void disconnected();

		/**
		 * Emitted when the \a error occurs. 
		 */
		void error( QStorageScu::Error error );

		/**
		 * Emitted when an error occurs. Instead of the error code this method
		 * provides a descriptive \a message explaining it.
		 */
		void error( QString message );

		/**
		 * Signal emitted when a dataset is successfully stored to SCP. Signal
		 * parameter is Instance UID of the dataset.
		 */
		void stored( QByteArray UID );

	private slots :
		void releaseAssociation();
		void requestAssociation();
		void storeDataset( Dicom::Dataset dataset );

	private :
		QList< QTransferSyntax > acceptedTransferSyntaxes(
			const QUid & SopClass
		) const;
		bool areAllSopClassesAccepted(
			const QList< QPresentationContext > & proposed
		) const;
		bool canConvert( const Dicom::Dataset & dataset ) const;
		QList< QPresentationContext > preparePresentationContexts() const;
		inline void setError( Error e );
		inline void setState( State s );

	private :
		inline Dicom::RequestorAssociation & association();
		inline const Dicom::RequestorAssociation & association() const;
		Dicom::RequestorAssociation * association_;

		Dicom::ServiceUser dimseClient_;
		Error error_;
		QList< QUid > sopClasses_;
		State state_;
		QTransferSyntax transferSyntax_;
};

#endif
