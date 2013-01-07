/***************************************************************************
 *   Copyright © 2012-2013 by Flux Inc.                                    *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#ifndef	QTDICOM_QASSOCIATION_HPP
#define QTDICOM_QASSOCIATION_HPP

#include <QtCore/QObject>
#include <QtCore/QRunnable>

#include <QtDicom/ConnectionParameters.hpp>
#include <QtDicom/Globals.hpp>
#include <QtDicom/UidList.hpp>
#include <QtDicom/QDcmtkResult>
#include <QtDicom/QPresentationContextList>


class QHostAddress;

struct T_ASC_Association;
struct T_ASC_Network;
struct T_ASC_Parameters;

/**
 * The \em QAssociation class proivdes methods allowing to establish 
 * connections between DICOM nodes.
 *
 * DICOM entities connect to each other and exchange data over the network 
 * providing services during communication sessions called the associations. 
 * An association object -- object of the \em QAssociation class -- enables 
 * just that; to connect to some remote DICOM host (to request association),
 * be used as a carrier for DICOM messages, and in the end be disconnected
 * (released) from the host.
 *
 * A \em QAssociation object can be used in two modes: as \em Requestor
 * (client) or \em Acceptor (server). Interface of this class, however, is 
 * mostly designed to support the former. To accept incoming DICOM associations
 * one should use the \ref QDicomServer class instead.
 *
 * Every \em QAssociation object starts living in the \en Unconnected state.
 * After creating an object, user can connect to other DICOM node using the 
 * \ref request() slot. Prior to that, however, it is necessary to provide 
 * information about what Presentation Contexts should be negotiated and 
 * the connection parameters of the DICOM server.
 * 
 * Each Presentation Context is a combination of Service-Object Pair Class 
 * (SOP Class), e.g. US Image Storage, and one or more Transfer Syntaxes, e.g.
 * Little Endian, RLE. After association is being requested, DICOM server scans 
 * provided list of presentation contexts and accepts those containing 
 * supported SOP Classes, and then for each SOP Class it selects a single 
 * Transfer Syntax to be used during communication.
 *
 * You can provide and query the list of presentation context using the \ref 
 * setPresentationContexs() and \ref presentationContexts() methods, 
 * respectively. It is also possible to set the Presentation Contexts when 
 * calling an overloaded \ref request( QPresentationContextList ) method.
 *
 * DICOM server connection parameters can be provided and queried in the 
 * similar manner: using the \ref setConnectionParameters() and \ref
 * connectionParameters() methods.
 *
 * Once the list of presentation contexts is populated and connection 
 * parameters are valid, the \ref request() slot can be safely called. It 
 * immediately changes object's state to \em Requesting and starts the 
 * association process in the background. When the association is established,
 * object emits the \ref connected() signal and changes its state to \em
 * Established. In the event of an error, the \ref error() signal is emitted
 * instead and association goes back to the \em Unconnected state.
 *
 * After the \em connected() signal is received, association user can obtain
 * SOP Classe and Transfer Syntaxe pairs accepted by the DICOM server with the 
 * \ref acceptedPresentationContexts() method.
 *
 * In the \em Established state, a \em QAssociation object is supposed to be
 * used by DICOM Service Users to carry DICOM messages by means of DICOM 
 * Message Service Elements (DIMSE) protocol. Since QtDicom library is still
 * based on DCMTK, the tAscAssociation() accessor and \em T_ASC_Association
 * conversion operator were provided in order to expose internal DCMTK 
 * structure to other DCMTK-based methods that carry on the transmission. In
 * addition, each association tracks message IDs being carried through. To
 * obtain such ID, use the \ref nextMessageId() method.
 *
 * After user finishes exchanging messages, associations should be released 
 * with the \ref release() slot, which sginals association state changing back 
 * to \em Unconnected by the \ref disconnected() signal.
 * 
 * 
 *
 * \author Paweł Żak <pawel.zak@fluxinc.ca>
 */
class QDICOM_DLLSPEC QAssociation : public QObject {
	Q_OBJECT;

	public :
		enum State {
			Unconnected,
			Requesting,
			Established,
			Releasing,
			Aborting
		};

		enum Mode {
			Requestor,
			Acceptor
		};

	public :
		/**
		 * Default contructor, creates an association and sets its \a parent 
		 * Qt object, if provided.
		 */
		QAssociation( QObject * parent = 0 );

		/**
		 * Destroys the association. If association is destroyed in \em 
		 * Established state, a warning message is reported.
		 */
		virtual ~QAssociation();

		/**
		 * Returns internally stored pointer to DCMTK association structure. 
		 * The structure is a mandatory attribute for many DCMTK methods.
		 */
		operator T_ASC_Association *&() const;

		/**
		 * In \ref Requestor mode returns the list of presentation contexts 
		 * accepted by the DICOM server. This list is valid only when the 
		 * association has beeen successfully established (\ref isEstablished() 
		 * returns \c true).
		 */
		QPresentationContextList acceptedPresentationContexts() const;

		/**
		 * Returns the connection parameters used during last \a request().
		 */
		const Dicom::ConnectionParameters & connectionParameters() const;

		/**
		 * Returns the error message.
		 */
		const QString & errorMessage() const;

		/**
		 * Returns \c true if an error occured during last operation. Every 
		 * error is also signalled by the \ref error() signal. The \ref 
		 * errorMessage() provides further explanation.
		 *
		 * Error flag and message are cleared only at the beginning of \ref 
		 * request() slot.
		 */
		bool hasError() const;

		/**
		 * Returns \c true when association has been established successfully 
		 * and currently is in the \em Established state.
		 *
		 * \sa state()
		 */
		bool isEstablished() const;

		/**
		 * Returns local, that is calling in the \em Requestor or called in 
		 * the \em Acceptor modes, Application Entity title. This property is
		 * read from stored connection parameters.
		 *
		 * \sa connectionParameters()
		 */
		const QString & myAeTitle() const;

		/**
		 * Returns a maximum nunber of Protocol Data Units the association can
		 * handle during transmission. This property is read from stored 
		 * connection parameters.
		 *
		 * \sa connectionParameters()
		 */
		unsigned maxPdu() const;

		/**
		 * Returns the mode in which the association operates. Standalone 
		 * object of \em QAssociation class always work in the \em Requestor
		 * mode. To create \em Acceptor mode associations use the \ref 
		 * QAssociationServer class.
		 */
		Mode mode() const;

		/**
		 * Generates a new, unique for this association, message ID. The ID
		 * can be used by during DIMSE exchange.
		 */
		quint16 nextMessageId();

		/**
		 * Returns the port number used during transmission. This property
		 * is read from communication parameters stored via \ref
		 * setConnectionParameters().
		 */
		quint16 port() const;

		/**
		 * Returns the list of presentation contexts set by either the \ref
		 * setPresentationContexts() method or when requesting the association.
		 */
		const QPresentationContextList & presentationContexts() const;

		/**
		 * An overloaded method, provided for conveniance. Sets connection
		 * parameters for this association to \a parameters and then calls the
		 * \ref request() slot.
		 */
		void request(
			const Dicom::ConnectionParameters & parameters
		);

		/**
		 * An overloaded method, provided for conveniance. Sets presentation
		 * contexts for this association to \a contexts and then calls the
		 * \ref request() slot.
		 */
		void request(
			const QPresentationContextList & contexts
		);

		/**
		 * An overloaded method, provided for conveniance. Sets connection
		 * parameters for this association to \a parameters, presentation 
		 * contexts to \a contexts and then calls the \ref request() slot.
		 */
		void request(
			const Dicom::ConnectionParameters & parameters,
			const QPresentationContextList & contexts
		);

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
		 * Sets the list of Presentation Contexts used during the next time an
		 * association is requested to the \a list.
		 */
		void setPresentationContexts( const QPresentationContextList & list );

		/**
		 * Return current state of the association.
		 */
		const State & state() const;

		/**
		 * Returns internally stored pointer to DCMTK association structure. 
		 * The structure is a mandatory attribute for many DCMTK methods.
		 */
		T_ASC_Association *& tAscAssociation() const;

		/**
		 * Returns the amount of seconds used as a timeout value for network 
		 * operations. This property is read from connection parameters.
		 *
		 * \sa setConnectionParameters()
		 */
		int timeout() const;

	public slots :
		/**
		 * Aborts the association, sending the A-ABORT message.
		 *
		 * This method merely schedules the abort task and sets object state to
		 * \em Aborting, almost immediately returning to the caller.
		 * 
		 * The actuall business of aborting association is done in the 
		 * background. Association users are notified about the fact that the 
		 * task finished by the \ref disconnected() signal.
		 *
		 * Note, that even unsuccessfull, the abort operation does not indicate
		 * error through \ref hasError() flag, except for logging a warning 
		 * message.
		 */
		void abort();

		/**
		 * Requests an association, sending the A-ASSOCIATE-RQ message to other
		 * DICOM node specified by the connection parameters, using provided
		 * Presentation Contexts.
		 * 
		 * This slot can be called only when the association is in the \em 
		 * Unconnected state. The state is immediately changed to \em Requesting
		 * and control returns back to the caller. The process of requesting 
		 * association takes place in the background and is signalled by the
		 * \ref connected() signal, followed by another state change to \em
		 * Established.
		 *
		 * In case of any error, the \ref error() signal is used to report a
		 * problem. When it happens, the error flag is raised as well (\ref
		 * hasError() returns \c true).
		 */
		void request();

		/**
		 * Releases the association, sending the A-RELEASE-RQ message to 
		 * connected DICOM node.
		 *
		 * This slot can be called only when association has already been
		 * established (\ref isEstablished() returns \c true). It changes the
		 * state of association to \em Releasing and returns control back to 
		 * the caller. The release process is taken care of in the background
		 * and signalled by the \ref disconnected() signal. If error occurs
		 * during release, the \ref error() signal is NOT emitted and 
		 * association is immediately aborted, which also results in the \ref 
		 * disconnected() signal.
		 */
		void release();

	signals :
		/**
		 * Emmited after the \ref request() slot had been called and 
		 * association was successfully established.
		 */
		void connected();

		/**
		 * This signal is emitted whenever an established association is 
		 * released or aborted -- from client's perspective -- or when its 
		 * release has been acknowledged by the server.
		 *
		 * \sa abort(), release()
		 */
		void disconnected();

		/**
		 * Whenever an error occurs, this signal is emmited passing \a message 
		 * to explain the problem.
		 */
		void error( QString message );

	protected :		
		/**
		 * Raises an error flag and sets the error \a message.
		 */
		void raiseError( const QString & message );

	private :
		void fillAeTitles( T_ASC_Parameters *& parameters ) const;
		void fillConnectionSettings( T_ASC_Parameters *& parameters ) const;
		void fillPresentationContexts( T_ASC_Parameters *& parameters ) const;		

	private slots :
		void startAborting();
		void finishAborting( QDcmtkResult result );

		void startReleasing();
		void finishReleasing( QDcmtkResult result );

		void startRequesting();
		void finishRequesting( QDcmtkResult result );

	private :		
		const Mode Mode_;

		mutable T_ASC_Association * association_;
		void dropTAscAssociation();

		Dicom::ConnectionParameters connectionParameters_;

		QString errorMessage_;

		mutable T_ASC_Network * network_;
		inline T_ASC_Network *& tAscNetwork() const;
		void dropTAscNetwork();
		bool initializeTAscNetwork();

		QPresentationContextList presentationContexts_;

		State state_;
		inline void setState( State );
};

#endif
