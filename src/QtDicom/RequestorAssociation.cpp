/***************************************************************************
 *   Copyright (C) 2011 by Flux Inc.                                       *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#include "Exceptions.hpp"
#include "RequestorAssociation.hpp"
#include "RequestorAssociation.moc.inl"

#include <QtDicom/QPresentationContext>
#include <QtDicom/QTransferSyntax>

#include <QtNetwork/QHostAddress>
#include <QtNetwork/QHostInfo>

#include <dcmtk/dcmnet/assoc.h>
#include <dcmtk/dcmnet/dimse.h>


namespace Dicom {


RequestorAssociation::RequestorAssociation( QObject * parent ) :
	Association( parent )
{
}

RequestorAssociation::RequestorAssociation(
	const ConnectionParameters & Parameters, QObject * parent 
) :
	Association( Parameters, parent )
{
}


RequestorAssociation::~RequestorAssociation() {
}


QList< QPresentationContext > 
	RequestorAssociation::acceptedPresentationContexts()
const {
	Q_ASSERT( tAscAssociation() );

	if ( isEstablished() ) {
		T_ASC_Parameters * params = tAscAssociation()->params;
		T_ASC_PresentationContext dcmContext;
		QList< QPresentationContext > contexts;

		const int Count = ASC_countPresentationContexts( params );
		for ( int i = 0; i < Count; ++i ) {
			OFCondition status = ASC_getPresentationContext(
				params, i, &dcmContext
			);
			if ( status.good() ) {
				QPresentationContext context = 
					QPresentationContext::fromTAscPresentationContext( dcmContext )
				;
				if ( context.accepted() ) {
					contexts.append( context );
				}
			}
			else {
				qCritical( __FUNCTION__": "
					"unable to read presentation context #%d; %s",
					i, status.text()
				);
			}
		}

		return contexts;
	}
	else {
		qWarning( __FUNCTION__": "
			"the list can be obtained only when association is established"
		);
		return QList< QPresentationContext >();
	}
}


QString RequestorAssociation::parametersText() const {
	OFString tmp;
	ASC_dumpParameters( tmp, tAscAssociation()->params, ASC_ASSOC_AC );

	return QString( tmp.c_str() );
}


bool RequestorAssociation::release() {	
	if ( ! ( tAscAssociation() && isEstablished() ) ) {
		qWarning( "Association hasn't been established yet." );

		return false;
	}

	const OFCondition Result = ASC_releaseAssociation( tAscAssociation() );
	setState( Disconnected );

	if ( Result.good() ) {
		return true;
	}
	else {
		raiseError(
			QString( 
				"Error occured when releasing association. "
				"Internal error description:\n%1"
			)
			.arg( Result.text() )
		);
		return false;
	}
}


int RequestorAssociation::request(
	const QList< QPresentationContext > & contexts,
	bool * timedOut
) {
#define RETURN( VALUE, TIMEOUT ) \
	if ( timedOut ) { \
		*timedOut = TIMEOUT; \
	} \
	return VALUE

	T_ASC_Parameters * parameters = 0;

	try {

	if ( ! ( tAscNetwork() || initializeTAscNetwork( -1 ) ) ) {
		raiseError( "Failed to initialize a network." );
		RETURN( -1, false );
	}

	OFCondition result = ASC_createAssociationParameters(
		& parameters, connectionParameters().maxPdu()
	);
	if ( result.bad() ) {
		throw OperationFailedException(
			QString( "Failed to create association parameters; %1." )
			.arg( result.text() )
		);
	}

	try {

	setAeTitles( parameters,
		connectionParameters().myAeTitle(), connectionParameters().peerAeTitle()
	);
	setConnectionSettings( parameters,
		connectionParameters().hostAddress(), connectionParameters().port()
	);
	setPresentationContexts( parameters, contexts );
	
	}
	catch ( ... ) {
		ASC_destroyAssociationParameters( & parameters );
		parameters = 0;
		throw;
	}

	result = ASC_requestAssociation(
		tAscNetwork(), parameters, &tAscAssociation()
	);

	if ( result.good() ) {
		setState( Established );

#ifdef _DEBUG
		const QList< QPresentationContext > contexts = acceptedPresentationContexts();
		foreach( const QPresentationContext & Context, contexts ) {
			qDebug( __FUNCTION__": "
				"accepted presentation context:\n%s",
				qPrintable( Context.toString() )
			);
		}
#endif

		const int Count = ASC_countAcceptedPresentationContexts( parameters );
		RETURN( Count, false );		
	}
	else if ( result == DUL_ASSOCIATIONREJECTED ) {
		QString rejectParamsString;
		T_ASC_RejectParameters rejectParameters;

		result = ASC_getRejectParameters( parameters, & rejectParameters );
		if ( result.good() ) {
			OFString tmp;
			ASC_printRejectParameters( tmp, & rejectParameters );
			rejectParamsString = 
				QString( "Rejection parameters:\n%1" ).arg( tmp.c_str() )
			;
		}
		else {
			rejectParamsString = 
				QString( "Failed to retrieve rejection parameters; %1." )
				.arg( result.text() )
			;
		}

		throw OperationFailedException(
			QString( "Association rejected. %1" ).arg( rejectParamsString )
		);
	}
	else if ( result == DUL_READTIMEOUT ) {
		RETURN( 0, true );
	}
	else {
		throw OperationFailedException(
			QString( "Failed to request an association; %1." )
			.arg( result.text() )
		);
	}

	// try block ends here
	}
	catch ( std::exception & e ) {
		raiseError( e.what() );
	}
	catch ( ... ) {
		raiseError( "Unknown exception occured." );
	}

	RETURN( -1, false );
#undef RETURN
}


int RequestorAssociation::request(
	const UidList & AbstractSyntaxes,
	const UidList & TransferSyntaxes,
	bool * timedOut
) {
	QList< QPresentationContext > contexts;

	foreach( const QByteArray & Abstract, AbstractSyntaxes ) {
		QPresentationContext context( Abstract );

		foreach( const QByteArray & Transfer, TransferSyntaxes ) {
			context << QTransferSyntax::fromUid( Transfer );
		}

		contexts.append( context );
	}

	return request( contexts, timedOut );
}


int RequestorAssociation::request(
	const ConnectionParameters & Parameters,
	const UidList & AbstractSyntaxes,
	const UidList & TransferSyntaxes,
	bool * timedOut
) {
	setConnectionParameters( Parameters );
	return request( AbstractSyntaxes, TransferSyntaxes, timedOut );
}


int RequestorAssociation::request(
	const QByteArray & AbstractSyntax, const QByteArray & TransferSyntax,
	bool * timedOut
) {
	return request( UidList( AbstractSyntax ), UidList( TransferSyntax ), timedOut );
}


int RequestorAssociation::request(
	const ConnectionParameters & Parameters,
	const UidList & AbstractSyntaxes,
	bool * timedOut
) {
	return request( Parameters, AbstractSyntaxes, UidList(), timedOut );
}


void RequestorAssociation::setAeTitles(
	T_ASC_Parameters *& parameters,
	const QString & MyAeTitle,
	const QString & HostAeTitle
) const {
	const QByteArray My = MyAeTitle.toAscii();
	const QByteArray Host = HostAeTitle.toAscii();

	const OFCondition Result = ASC_setAPTitles( parameters, My, Host, 0 );

	if ( Result.bad() ) {
		throw OperationFailedException(
			QString(
				"Failed to set AE titles. "
				"Internal error description:\n%1"
			)
			.arg( Result.text() )
		);
	}
}


void RequestorAssociation::setConnectionSettings(
	T_ASC_Parameters *& parameters,
	const QHostAddress & HostAddress,
	quint16 port
) const {
	const QByteArray LocalHostName = QHostInfo::localHostName().toAscii();
	const QByteArray RemoteHostAddress = 
		QString( "%1:%2" ).arg( HostAddress.toString() ).arg( port ).toAscii()
	;

	OFCondition result = ASC_setPresentationAddresses(
		parameters, LocalHostName.constData(), RemoteHostAddress.constData()
	);
	if ( result.bad() ) {
		throw OperationFailedException(
			QString( 
				"Failed to set presentation addresses. "
				"Internal error description:\n%1" )
			.arg( result.text() )
		);
	}

	result = ASC_setTransportLayerType( parameters, false );
	if ( result.bad() ) {
		throw OperationFailedException(
			QString( 
				"Failed to set transport layer. "
				"Internal error description:\n%1"
			)
			.arg( result.text() )
		);
	}
}


void RequestorAssociation::setPresentationContexts(
	T_ASC_Parameters *& parameters,
	const QList< QPresentationContext > & Contexts
) const {
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
		if ( Result.bad() ) {
			throw OperationFailedException(
				QString( 
					"Failed to add a presentation context. "
					"Internal error description:\n%1"
				)
				.arg( Result.text() )
			);
		}

	}
}


} // Namesapce DICOM ends here.
