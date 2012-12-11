/***************************************************************************
 *   Copyright (C) 2011 by Flux Inc.                                       *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#include "AcceptorAssociation.hpp"
#include "AcceptorAssociation.moc.inl"

#include <dcmtk/dcmnet/assoc.h>


namespace Dicom {

AcceptorAssociation::AcceptorAssociation( QObject * parent ) :
	Association( parent )
{
}


AcceptorAssociation::AcceptorAssociation( 
	const ConnectionParameters & Parameters, QObject * parent
) :
	Association( Parameters, parent )
{
}


AcceptorAssociation::~AcceptorAssociation() {
}


bool AcceptorAssociation::accept(
	const UidList & AbstractSyntaxes,
	const UidList & TransferSyntaxes
) {
	if ( hasError() ) {
		return false;
	}

	if ( ! isEstablished() ) {
		raiseError( "None association request have been received yet." );
		return false;
	}
	if ( AbstractSyntaxes.isEmpty() ) {
		raiseError( "Empty abstract syntaxes list." );
		return false;
	}
	if ( TransferSyntaxes.isEmpty() ) {
		raiseError( "Empty transfer syntaxes list." );
		return false;
	}

	OFCondition result;
	{
		const char ** AS = AbstractSyntaxes.toFlatArray();
		const char ** TS = TransferSyntaxes.toFlatArray();

		result = ASC_acceptContextsWithPreferredTransferSyntaxes(
			tAscAssociation()->params,
			AS, AbstractSyntaxes.size(),
			TS, TransferSyntaxes.size()
		);

		delete [] AS;
		delete [] TS;
	}

	if ( result.bad() ) {
		raiseError(
			QString(
				"Failed to accept presentation contexts. "
				"Internal error description:\n%1"
			)
			.arg( result.text() )
		);
		return false;
	}

	if ( myAeTitle().isEmpty() ) {
		raiseError( "Empty AE title." );
		return false;
	}

	result = ASC_setAPTitles(
		tAscAssociation()->params, NULL, NULL, myAeTitle().toAscii()
	);
	if ( result.bad() ) {
		raiseError(
			QString(
				"Failed to set AE titles. "
				"Internal error description:\n%1"
			)
			.arg( result.text() )
		);
		return false;
	}

	QByteArray buffer( 129, '\0' );
	result = ASC_getApplicationContextName( 
		tAscAssociation()->params, buffer.data()
	);
	if ( 
		result.bad() || 
		buffer.simplified() != QString( UID_StandardApplicationContext )
	) {
		T_ASC_RejectParameters rejection = {
			ASC_RESULT_REJECTEDPERMANENT,
			ASC_SOURCE_SERVICEUSER,
			ASC_REASON_SU_APPCONTEXTNAMENOTSUPPORTED
		};

		result = ASC_rejectAssociation( tAscAssociation(), &rejection );

		if ( result.good() ) {
			raiseError(
				QString( 
					"Association rejected. "
					"Bad application context name: `%1'."
				)
				.arg( buffer.constData() )
			);
		}
		else {
			raiseError(
				QString( 
					"Failed to reject association due to a "
					"bad application context name: `%1'. "
					"Internal error description:\n%2"
				)
				.arg( buffer.constData() )
				.arg( result.text() )
			);
		}
		return false;
	}

	result = ASC_acknowledgeAssociation( tAscAssociation() );
	if ( result.bad() ) {
		raiseError(
			QString( 
				"Failed to acknowledge an association. "
				"Internal error description:\n%1"
			)
			.arg( result.text() )
		);
		return false;
	}

	return true;
}


QString AcceptorAssociation::calledAeTitle() const {
	if ( tAscAssociation() ) {
		QByteArray ae( 32, '\0' );
		ASC_getAPTitles( tAscAssociation()->params, 0, ae.data(), 0 );

		return QString( ae.simplified() );
	}
	else {
		return QString();
	}
}


QString AcceptorAssociation::callingAeTitle() const {
	if ( tAscAssociation() ) {
		QByteArray ae( 32, '\0' );
		ASC_getAPTitles( tAscAssociation()->params, ae.data(), 0, 0 );

		return QString( ae.simplified() );
	}
	else {
		return QString();
	}
}


bool AcceptorAssociation::confirmRelease() {
	const OFCondition Result = ASC_acknowledgeRelease(
		tAscAssociation()
	);
	setState( Disconnected );

	if ( Result.good() ) {
		return true;
	}
	else {
		raiseError(
			QString( 
				"Failed to acknowledge association release. "
				"Internal error description:\n%1"
			)
			.arg( Result.text() )
		);
		return false;
	}
}


QString AcceptorAssociation::parametersText() const {
	OFString tmp;
	ASC_dumpParameters( tmp, tAscAssociation()->params, ASC_ASSOC_RQ );

	return QString( tmp.c_str() );
}


bool AcceptorAssociation::receive( 
	const ConnectionParameters & Parameters,
	bool * timedOut
) { 
#define RETURN( RESULT, TIMEOUT ) \
	if ( timedOut ) { \
		*timedOut = TIMEOUT; \
	} \
	return RESULT

	setConnectionParameters( Parameters );

	if ( ! ( tAscNetwork() || initializeTAscNetwork( 1 ) ) ) {
		RETURN( false, false );
	}

	OFCondition result = ASC_receiveAssociation(
		tAscNetwork(), &tAscAssociation(),
		connectionParameters().maxPdu(),
		NULL, NULL, false, DUL_NOBLOCK,
		timeout()
	);

	if ( result.good() ) {
		setState( Established );

		RETURN( true, false );
	}
	else if ( 
		result == DUL_NOASSOCIATIONREQUEST &&
		timeout() >= 0
	) {
		RETURN( false, true );
	}
	else {
		raiseError(
			QString( 
				"Failed to receive an association. "
				"Internal error description:\n%1"
			)
			.arg( result.text() )
		);

		RETURN( false, false );
	}
#undef RETURN
}

		
bool AcceptorAssociation::receive( bool * timedOut ) {
	return receive( connectionParameters(), timedOut );
}

}; // Namespace DICOM ends here.