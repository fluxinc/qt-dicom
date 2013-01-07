/***************************************************************************
 *   Copyright © 2012-2013 by Flux Inc.                                    *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#include "ConnectionParameters.hpp"

#include "QAssociation.hpp"
#include "QAssociation.moc.inl"
#include "QDcmtkTask.hpp"

#include <QtCore/QString>
#include <QtCore/QtConcurrentRun>

#include <QtNetwork/QHostInfo>

#include <dcmtk/dcmnet/assoc.h>
#include <dcmtk/dcmnet/dimse.h>

typedef Dicom::ConnectionParameters QConnectionParameters;


// These wrappers are used to invoke similar ASC_* method -- they were created 
// in order to minimize the amount of parameters being passed by \ref QDcmtkTask
// functor
inline static OFCondition ASC_initializeNetworkWrapper(
	T_ASC_NetworkRole role,
	int acceptorPort,
	int timeout,
	T_ASC_Network ** network
);
inline static OFCondition ASC_requestAssociationWrapper(
	T_ASC_Network * network,
	T_ASC_Parameters * parameters,
	T_ASC_Association ** association,
	int timeout
);


QAssociation::QAssociation( QObject * parent ) :
	QObject( parent ),
	Mode_( Requestor ),
	association_( NULL ),
	network_( NULL ),
	state_( Unconnected )
{
}


QAssociation::~QAssociation() {
	if ( isEstablished() ) {
		qWarning( "Destroying an established association" );
	}
	dropTAscNetwork();
}


void QAssociation::abort() {
	if ( state_ == Established ) {
		Q_ASSERT( tAscAssociation() );

		setState( Aborting );
		QMetaObject::invokeMethod( this, "startAborting", Qt::QueuedConnection );
	}
	else {
		qDebug( __FUNCTION__": "
			"association is not in the Established state"
		);
	}
}


QPresentationContextList QAssociation::acceptedPresentationContexts() const {
	Q_ASSERT( tAscAssociation() );

	if ( isEstablished() ) {
		T_ASC_Parameters * params = tAscAssociation()->params;
		T_ASC_PresentationContext dcmContext;

		QPresentationContextList contexts;
		const int Count = ASC_countPresentationContexts( params );
		for ( int i = 0; i < Count; ++i ) {
			const OFCondition Status = ASC_getPresentationContext(
				params, i, &dcmContext
			);
			if ( Status.good() ) {
				QPresentationContext context = 
					QPresentationContext::fromTAscPresentationContext( dcmContext )
				;
				if ( context.accepted() ) {
					contexts.append( context );
				}
			}
			else {
				qWarning( __FUNCTION__": "
					"unable to read presentation context #%d; %s",
					i, Status.text()
				);
			}
		}

		return contexts;
	}
	else {
		qDebug( __FUNCTION__": "
			"the list can be obtained only when association is established"
		);
		return QPresentationContextList();
	}
}


const QConnectionParameters & QAssociation::connectionParameters() const {
	return connectionParameters_;
}


void QAssociation::dropTAscAssociation() {
	if ( tAscAssociation() ) {
		const OFCondition Result = ASC_destroyAssociation( &tAscAssociation() );
		if ( Result.good() ) {
			qDebug( __FUNCTION__": destroyed DCMTK association object" );
		}
		else {
			qWarning( __FUNCTION__": "
				"error occured when destroying an association; %s",
				Result.text()
			);
		}

		association_ = 0;
	}
}


void QAssociation::dropTAscNetwork() {
	dropTAscAssociation();

	if ( tAscNetwork() ) {
		const OFCondition Result = ASC_dropNetwork( & network_ );
		network_ = NULL;
		if ( Result.good() ) {
			qDebug( __FUNCTION__": destroyed DCMTK network object" );
		}
		else {
			qWarning( __FUNCTION__": "
				"error occured when dropping network; %s", Result.text()
			);
		}
	}
}


const QString & QAssociation::errorMessage() const {
	return errorMessage_;
}


void QAssociation::fillAeTitles( T_ASC_Parameters *& parameters ) const {
	const QByteArray My = connectionParameters_.myAeTitle().toAscii();
	const QByteArray Host = connectionParameters_.peerAeTitle().toAscii();

	const OFCondition Result = ASC_setAPTitles( parameters, My, Host, NULL );
	if ( Result.good() ) {
		qDebug( __FUNCTION__": "
			"setting AE titles:\n"
			"\tlocal : `%s'\n"
			"\tremote: `%s'",
			My.constData(), Host.constData()
		);
	}
	else {
		throw QString(
			"Failed to set AE titles. "
			"Internal error description:\n%1"
		)
		.arg( Result.text() );
	}
}


void QAssociation::fillConnectionSettings(
	T_ASC_Parameters *& parameters
) const {
	const QByteArray LocalHostName = QHostInfo::localHostName().toAscii();
	const QHostAddress HostAddress = connectionParameters_.hostAddress();

	const QByteArray RemoteHostAddress = 
		QString( "%1:%2" ).arg( HostAddress.toString() ).arg( port() ).toAscii()
	;

	OFCondition result = ASC_setPresentationAddresses(
		parameters, LocalHostName, RemoteHostAddress
	);
	if ( result.good() ) {
		qDebug( __FUNCTION__": "
			"setting presentation addresses:\n"
			"\tlocal : `%s'\n"
			"\tremote: `%s'",
			LocalHostName.constData(),
			RemoteHostAddress.constData()
		);
	}
	else {
		throw QString( 
			"Failed to set presentation addresses. "
			"Internal error description:\n%1" )
		.arg( result.text() );
	}

	result = ASC_setTransportLayerType( parameters, false );
	if ( result.bad() ) {
		qDebug( "Selected transport layer" );
	}
	else {
		throw QString( 
			"Failed to set transport layer. "
			"Internal error description:\n%1"
		)
		.arg( result.text() );
	}
}


void QAssociation::fillPresentationContexts(
	T_ASC_Parameters *& parameters
) const {
	const QPresentationContextList & Contexts = presentationContexts_;

	const int PcCount = Contexts.size();

	static const int MaxTssCount = 255;
	const char * tss[ MaxTssCount ];

	for ( int i = 0; i < PcCount; ++i ) {
		const QPresentationContext & Pc = Contexts.at( i );
		const QList< QTransferSyntax > & Tss = Pc.proposedTransferSyntaxes();
		const int TssCount = Tss.size();

		Q_ASSERT( TssCount <= MaxTssCount );

		const char * As = Pc.abstractSyntax().constData();
	
		for ( int j = 0; j < TssCount; ++j ) {
			tss[ j ] = Tss.at( j ).uid().constData();
		}

		const OFCondition Result = ASC_addPresentationContext(
			parameters, i * 2 + 1, As, tss, TssCount
		);
		if ( Result.good() ) {
			qDebug( __FUNCTION__": "
				"adding presentation context:\n%s",
				qPrintable( Pc.toString() )
			);
		}
		else {
			throw QString( 
				"Failed to add a presentation context. "
				"Internal error description:\n%1"
			)
			.arg( Result.text() );
		}

	}
}


void QAssociation::finishAborting( QDcmtkResult result ) {
	Q_ASSERT( state_ == Aborting );

	if ( result.ofCondition().good() ) {
		qDebug( __FUNCTION__": association aborted successfully" );
	}
	else {
		qWarning( __FUNCTION__": "
			"failed to abort association; %s", result.ofCondition().text()
		);
	}

	dropTAscAssociation();

	setState( Unconnected );
	emit disconnected();
}


void QAssociation::finishAcquiringNetwork( QDcmtkResult result ) {
	Q_ASSERT( state_ == AcquiringNetwork );

	if ( result.ofCondition().good() ) {
		qDebug( __FUNCTION__": "
			"network object created:\n"
			"\tport    : %d\n"
			"\ttimeout : %d s",
			port(), timeout()
		);

		setState( Requesting );
		startRequesting();		
	}
	else {
		raiseError(
			QString( 
				"Failed to initialize network. "
				"Internal error description:\n%1"
			)
			.arg( result.ofCondition().text() )
		);

		setState( Unconnected );
	}
}


void QAssociation::finishReleasing( QDcmtkResult result ) {
	if ( result.ofCondition().good() ) {
		qDebug( __FUNCTION__": association released successfully" );

		dropTAscAssociation();

		setState( Unconnected );
		emit disconnected();
	}
	else {
		qWarning( __FUNCTION__": "
			"failed to rlease association; %s", result.ofCondition().text()
		);

		setState( Established );
		abort();
	}
}


void QAssociation::finishRequesting( QDcmtkResult result ) {
	Q_ASSERT( state_ == Requesting );

	OFCondition status = result.ofCondition();
	if ( status.good() ) {
		qDebug( __FUNCTION__": association requested successfully" );

#ifdef _DEBUG
		const QPresentationContextList Contexts = acceptedPresentationContexts();
		for (
			QPresentationContextList::const_iterator i = Contexts.constBegin();
			i != Contexts.constEnd(); ++i
		) {
			qDebug( __FUNCTION__": "
				"accepted presentation context:\n%s",
				qPrintable( i->toString() )
			);
		}
#endif

		setState( Established );
		emit connected();
		return;
	}
	else if ( status == DUL_ASSOCIATIONREJECTED ) {
		QString rejectParamsString;
		T_ASC_RejectParameters rejectParameters;

		status = ASC_getRejectParameters( tAscAssociation()->params, & rejectParameters );
		if ( status.good() ) {
			OFString tmp;
			ASC_printRejectParameters( tmp, & rejectParameters );
			rejectParamsString = 
				QString( "Rejection parameters:\n%1" ).arg( tmp.c_str() )
			;
		}
		else {
			rejectParamsString = 
				QString( "Failed to retrieve rejection parameters; %1." )
				.arg( status.text() )
			;
		}

		raiseError(
			QString( "Association rejected. %1" ).arg( rejectParamsString )
		);
	}
	else if ( status == DUL_READTIMEOUT ) {
		raiseError( "Timeout occured" );
	}
	else {
		raiseError(
			QString( "Failed to request an association; %1." )
			.arg( status.text() )
		);
	}

	dropTAscAssociation();
	setState( Unconnected );
}


bool QAssociation::hasError() const {
	return errorMessage_.size() > 0;
}


bool QAssociation::initializeTAscNetwork() {
	if ( port() != 0 ) {
		Q_ASSERT( ! tAscNetwork() );

		const int Port = 
			( Mode_ == Requestor ? 0 : static_cast< int >( port() ) )
		;
		const int Timeout = timeout();
		const OFCondition Result = ASC_initializeNetwork(
			Mode_ == Requestor ? NET_REQUESTOR : NET_ACCEPTOR,
			Port, Timeout,
			&tAscNetwork()
		);

		if ( Result.good() ) {
			qDebug( __FUNCTION__": "
				"network object created for %s mode:\n"
				"\tport    : %d\n"
				"\ttimeout : %d s",
				Mode_ == Requestor ? "requestor" : "acceptor",
				Port, Timeout
			);
			return true;
		}
		else {
			raiseError(
				QString( 
					"Failed to initialize network. "
					"Internal error description:\n%1"
				)
				.arg( Result.text() )
			);
		}
	}
	else {
		raiseError( "Invliad port number" );
	}

	return false;
}


bool QAssociation::isEstablished() const {
	return state() == Established;
}


unsigned QAssociation::maxPdu() const {
	return connectionParameters().maxPdu();
}


QAssociation::Mode QAssociation::mode() const {
	return Mode_;
}


const QString & QAssociation::myAeTitle() const {
	return connectionParameters().myAeTitle();
}


quint16 QAssociation::nextMessageId() {
	if ( tAscAssociation() ) {
		return tAscAssociation()->nextMsgID++;
	}
	else {
		Q_ASSERT( tAscAssociation() );

		return 0;
	}
}


quint16 QAssociation::port() const {
	return connectionParameters().port();
}


void QAssociation::raiseError( const QString & Message ) {
	if ( errorMessage_.isEmpty() ) {
		errorMessage_ = Message;
		emit error( Message );
	}
}


void QAssociation::release() {	
	if ( state_ == Established ) {
		Q_ASSERT( tAscAssociation() );

		setState( Releasing );
		QMetaObject::invokeMethod( 
			this, "startReleasing", Qt::QueuedConnection
		);		
	}
	else {
		qDebug( __FUNCTION__": "
			"association is not established; ignoring"
		);
	}
}


void QAssociation::request( const QConnectionParameters & Parameters ) {
	setConnectionParameters( Parameters );
	request();
}


void QAssociation::request( const QPresentationContextList & Contexts ) {
	setPresentationContexts( Contexts );
	request();
}


void QAssociation::request(
	const QConnectionParameters & Parameters,
	const QPresentationContextList & Contexts
) {
	setConnectionParameters( Parameters );
	setPresentationContexts( Contexts );
	request();
}


void QAssociation::request() {
	if ( state_ == Unconnected ) {
		errorMessage_.clear();

		if ( tAscNetwork() ) {
			setState( Requesting );

			QMetaObject::invokeMethod(
				this, "startRequesting", Qt::QueuedConnection
			);
		}
		else {
			setState( AcquiringNetwork );

			QMetaObject::invokeMethod(
				this, "startAcquiringNetwork", Qt::QueuedConnection
			);
		}
	}
	else {
		raiseError(
			"Attempting to request an association while the previous one "
			"remains established"
		);
	}
}


void QAssociation::setConnectionParameters(
	const QConnectionParameters & Parameters
) {
	if (
		connectionParameters().port()    != Parameters.port() ||
		connectionParameters().timeout() != Parameters.timeout()
	)
	{
		// Network parameters changed, drop current network.
		dropTAscNetwork();
	}

	connectionParameters_ = Parameters;
}


void QAssociation::setState( State s ) {
	state_ = s;
}


void QAssociation::startAborting() {
	QDcmtkTask * task = QDcmtkTask::create( 
		::ASC_abortAssociation, tAscAssociation()
	);

	connect( 
		task, SIGNAL( finished( QDcmtkResult ) ),
		SLOT( finishAborting( QDcmtkResult ) )
	);
	connect(
		task, SIGNAL( finished( QDcmtkResult ) ),
		task, SLOT( deleteLater() )
	);

	task->start();
}


void QAssociation::startAcquiringNetwork() {
	QDcmtkTask * task = QDcmtkTask::create( 
		::ASC_initializeNetworkWrapper,
		NET_REQUESTOR, static_cast< int >( port() ), timeout(), &tAscNetwork()
	);

	connect( 
		task, SIGNAL( finished( QDcmtkResult ) ),
		SLOT( finishAcquiringNetwork( QDcmtkResult ) )
	);
	connect(
		task, SIGNAL( finished( QDcmtkResult ) ),
		task, SLOT( deleteLater() )
	);

	task->start();
}


void QAssociation::startReleasing() {
	QDcmtkTask * task = QDcmtkTask::create( 
		::ASC_abortAssociation, tAscAssociation()
	);

	connect( 
		task, SIGNAL( finished( QDcmtkResult ) ),
		SLOT( finishReleasing( QDcmtkResult ) )
	);
	connect(
		task, SIGNAL( finished( QDcmtkResult ) ),
		task, SLOT( deleteLater() )
	);

	task->start();
}


void QAssociation::startRequesting() {
	T_ASC_Parameters * parameters = 0;

	try {

	OFCondition result = ASC_createAssociationParameters( 
		&parameters, maxPdu()
	);
	if ( result.good() ) {
		qDebug( __FUNCTION__": "
			"created association parameters with max PDU: %d",
			maxPdu()
		);
	}
	else {
		throw 
			QString( "Failed to create association parameters; %1." )
			.arg( result.text() )
		;
	}

	try { // Nested try block for parameters

	fillAeTitles( parameters );
	fillConnectionSettings( parameters );
	fillPresentationContexts( parameters );
	
	}
	catch ( ... ) {
		ASC_destroyAssociationParameters( &parameters );
		parameters = NULL;

		// The parameters structure is freed, pass the exception further
		throw;
	}

	QDcmtkTask * task = QDcmtkTask::create( 
		::ASC_requestAssociationWrapper,
		tAscNetwork(), parameters, &tAscAssociation(), timeout()
	);

	connect( 
		task, SIGNAL( finished( QDcmtkResult ) ),
		SLOT( finishRequesting( QDcmtkResult ) )
	);
	connect(
		task, SIGNAL( finished( QDcmtkResult ) ),
		task, SLOT( deleteLater() )
	);

	task->start();

	}
	catch ( QString & Msg ) {
		raiseError( Msg );

		setState( Unconnected );
	}
}


const QAssociation::State & QAssociation::state() const {
	return state_;
}


T_ASC_Association *& QAssociation::tAscAssociation() const {
	return association_;
}


T_ASC_Network *& QAssociation::tAscNetwork() const {
	return network_;
}


int QAssociation::timeout() const {
	return connectionParameters().timeout();
}


OFCondition ASC_initializeNetworkWrapper(
	T_ASC_NetworkRole role, int port, int timeout, T_ASC_Network ** network
) {
	return ::ASC_initializeNetwork(
		role, port, timeout, network
	);
}


OFCondition ASC_requestAssociationWrapper(
	T_ASC_Network * network,
	T_ASC_Parameters * parameters,
	T_ASC_Association ** association,
	int timeout
) {
	return ::ASC_requestAssociation(
		network, parameters, association, NULL, NULL, DUL_NOBLOCK, timeout
	);
}
