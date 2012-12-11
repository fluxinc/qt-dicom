/***************************************************************************
 *   Copyright (C) 2011 by Flux Inc.                                       *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#include <QtCore/QXmlStreamReader>

#include "ConnectionParameters.hpp"


namespace Dicom {

ConnectionParameters::ConnectionParameters() :
	d_( new ConnectionParameters_priv() )
{
	setMaxPdu( defaultMaxPdu() );
	setMyAeTitle( defaultMyAeTitle() );
	setTimeout( defaultTimeout() );
}


ConnectionParameters::ConnectionParameters( Type type ) :
	d_( new ConnectionParameters_priv() )
{
	setType( type );
}


ConnectionParameters::ConnectionParameters(
	const ConnectionParameters & Other 
) :
	d_( Other.d_ )
{
}
	

ConnectionParameters ConnectionParameters::fromXml( 
	QXmlStreamReader & input, QString * errorMessage
) {
	ConnectionParameters params;
	params.readXml( input, errorMessage );

	return params;
}


ConnectionParameters::~ConnectionParameters() {
}


ConnectionParameters & ConnectionParameters::operator = (
	const ConnectionParameters & Other 
) {
	if ( this != &Other ) {
		d_ = Other.d_;
	}

	return * this;
}


unsigned ConnectionParameters::defaultMaxPdu() {
	return 16384;
}


const QString & ConnectionParameters::defaultMyAeTitle() {
	static const QString TheTitle = "FLUX";

	return TheTitle;
}


int ConnectionParameters::defaultTimeout() {
	return 10;
}


const QHostAddress & ConnectionParameters::hostAddress() const {
	return d_->hostAddress_;	
}


bool ConnectionParameters::isValid() const {
	return
		( type() == Client && ! (
			hostAddress().isNull() ||
			maxPdu() == 0 ||
			myAeTitle().isEmpty() ||
			peerAeTitle().isEmpty() ||
			port() == 0
		) ) ||
		( type() == Server && ! (
			maxPdu() == 0 ||
			myAeTitle().isEmpty() ||
			port() == 0
		) )
	;
}


unsigned ConnectionParameters::maxPdu() const {
	return d_->maxPdu_;
}


const QString & ConnectionParameters::myAeTitle() const {
	return d_->myAeTitle_;	
}


const QString & ConnectionParameters::peerAeTitle() const {
	return d_->peerAeTitle_;
}


quint16 ConnectionParameters::port() const {
	return d_->port_;	
}


bool ConnectionParameters::readXml( 
	QXmlStreamReader & input, QString * errorMessage
) {
	Q_ASSERT( input.name() == "ConnectionParameters" );

	if ( input.name() == "ConnectionParameters" ) {
		const QXmlStreamAttributes Attributes = input.attributes();

		for (
			QXmlStreamAttributes::const_iterator i = Attributes.constBegin();
			i != Attributes.constEnd(); ++i
		) {
			if ( i->name() == "type" ) {
				setType( typeFromString( i->value().toString() ) );

				// We're setting default listening host address,
				// can be further overriden with the <HostAddress> element.
				if ( type() == Server ) {
					setHostAddress( QHostAddress::Any );
				}
			}
			else {
				input.raiseError(
					QString( 
						"Unexpected attribute `%1' in the "
						"`ConnectionParameters' element."
					)
					.arg( i->name().toString() )
				);
			}
		}
	}
	else {
		input.raiseError( 
			QString( "Unexpected element: `%1'" )
			.arg( input.name().toString() )
		);
	}

	while ( ! input.atEnd() ) {
		if ( ! input.readNextStartElement() ) {
			break;
		}

		const QStringRef Name = input.name();
		const QString Text = input.readElementText();
		if ( Name == "HostAddress" ) {
			const QHostAddress Address( Text );
			if ( ! Address.isNull() ) {
				setHostAddress( Address );
			}
			else {
				input.raiseError(
					QString( 
						"Failed to extract a valid IP address from the: `%1' value"
					)
					.arg( Text )
				);
			}
		}
		else if ( Name == "MaxPdu" ) {
			bool ok;
			const int MaxPdu = Text.toUInt( &ok );
			if ( ok ) {
				setMaxPdu( MaxPdu );
			}
			else {
				input.raiseError(
					QString( 
						"Failed to extract an integer from the: `%1' value "
						"when reading a maximum PDU"
					)
					.arg( Text )
				);
				setMaxPdu( 0 );
			}
		}
		else if ( Name == "MyAeTitle" ) {
			setMyAeTitle( Text );
		}
		else if ( Name == "PeerAeTitle" ) {
			setPeerAeTitle( Text );
		}
		else if ( Name == "Port" ) {
			bool ok;
			const quint16 Port = static_cast< quint16>( Text.toUInt( &ok ) );
			if ( ok ) {
				setPort( Port );
			}
			else {
				input.raiseError(
					QString( 
						"Failed to extract an integer from the: `%1' value "
						"when reading a port number"
					)
					.arg( Text )
				);
				setPort( 0 );
			}
		}
		else if ( Name == "Timeout" ) {
			bool ok;
			const int Timeout = Text.toInt( &ok );
			if ( ok ) {
				setTimeout( Timeout );
			}
			else {
				input.raiseError(
					QString(
						"Failed to extract an integer from the: `%1' value"
						" whren reading a timeout"
					)
					.arg( Text )
				);
			}
		}
		else {
			input.raiseError(
				QString( "Enexpected element: `%1'" )
				.arg( Name.toString() )
			);
		}
	}

	if ( ! input.hasError() ) {
		return true;
	}
	else {
		if ( errorMessage ) {
			*errorMessage = 
				QString( 
					"%1 at position %2:%3."
				)
				.arg( input.errorString() )
				.arg( input.lineNumber() )
				.arg( input.columnNumber() )
			;
		}
		return false;
	}
}


void ConnectionParameters::setHostAddress( const QHostAddress & Address ) {
	d_->hostAddress_ = Address;
}


void ConnectionParameters::setMaxPdu( unsigned Max ) {
	d_->maxPdu_ = Max;
}


void ConnectionParameters::setMyAeTitle( const QString & Title ) {
	d_->myAeTitle_ = Title;
}


void ConnectionParameters::setPeerAeTitle( const QString & Title ) {
	d_->peerAeTitle_ = Title;
}


void ConnectionParameters::setPort( quint16 port ) {
	d_->port_ = port;
}


void ConnectionParameters::setTimeout( int timeout ) {
	d_->timeout_ = timeout;
}


void ConnectionParameters::setType( Type type ) {
	d_->type_ = static_cast< int >( type );
}


int ConnectionParameters::timeout() const {
	return d_->timeout_;	
}


QString ConnectionParameters::toString() const {
	if ( type() == Client ) {
		return QString(
			"Host Address  : %1\n"
			"My AE Title   : %2\n"
			"Peer AE Title : %3\n"
			"Port          : %4\n"
			"Timeout       : %5\n"
			"Maximum PDU   : %6\n"
		)
		.arg( hostAddress().toString() )
		.arg( myAeTitle() )
		.arg( peerAeTitle() )
		.arg( port() )
		.arg( timeout() )
		.arg( maxPdu() );
	}
	else {
		return QString(
			"Host Address  : %1\n"
			"My AE Title   : %2\n"
			"Port          : %3\n"
			"Timeout       : %4\n"
			"Maximum PDU   : %5\n"
		)
		.arg( hostAddress().toString() )
		.arg( myAeTitle() )
		.arg( port() )
		.arg( timeout() )
		.arg( maxPdu() );
	}
}


void ConnectionParameters::toXml( QXmlStreamWriter & xml ) const {
	xml.writeStartElement( "ConnectionParameters" );

	xml.writeAttribute( "type", typeString() );

	xml.writeTextElement( "HostAddress", hostAddress().toString() );
	
	xml.writeTextElement( "MaxPdu", QString::number( maxPdu() ) );
	xml.writeTextElement( "MyAeTitle", myAeTitle() );
	
	xml.writeTextElement( "PeerAeTitle", peerAeTitle() );
	xml.writeTextElement( "Port", QString::number( port() ) );
	xml.writeTextElement( "Timeout", QString::number( timeout() ) );

	xml.writeEndElement(); // ConnectionParameters
}


ConnectionParameters::Type ConnectionParameters::type() const {
	return static_cast< Type >( d_->type_ );
}


ConnectionParameters::Type ConnectionParameters::typeFromString( 
	const QString & Value
) {
	static const QString & ServerString = typeString( Server );

	if ( Value.compare( ServerString, Qt::CaseInsensitive ) == 0 ) {
		return Server;
	}
	else {
		return Client;
	}
}


const QString & ConnectionParameters::typeString( Type type ) {
	if ( type == Server ) {
		static const QString TheValue = "Server";

		return TheValue;
	}
	else {
		static const QString TheValue = "Client";

		return TheValue;
	}
}


const QString & ConnectionParameters::typeString() const {
	return typeString( type() );
}

} // Namespace DICOM end.