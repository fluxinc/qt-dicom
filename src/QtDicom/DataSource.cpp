/***************************************************************************
 *   Copyright (C) 2011 by Flux Inc.                                       *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#include "DataSource.hpp"
#include "DataSourceFactory.hpp"
#include "FileSystemDataSource.hpp"

#include <QtCore/QXmlStreamReader>

namespace Dicom {


DataSource::DataSource( const QString & TypeName, QObject * parent ) :
	TypeName_( TypeName ),
	QObject( parent )
{
	static const bool FsRegistered = FileSystemDataSource::isRegistered();
	if ( ! FsRegistered ) {
		qWarning( "Failed to register file system data source." );
	}
}


DataSource::DataSource( const DataSource & Other ) :
	TypeName_( Other.TypeName_ ),
	cache_( Other.cache_ )
{
}


DataSource::~DataSource() {
}


QHash< int, Dataset > & DataSource::cache() const {
	return cache_;
}


void DataSource::clearCache() const {
	cache().clear();
}


Dataset DataSource::dataset( int num ) const {
	if ( cache().contains( num ) ) {
		return cache().value( num );
	}
	else {
		Dataset dset = readDataset( num );
		cache().insert( num, dset );
		return dset;
	}
}


DataSource * DataSource::fromXml(
	QXmlStreamReader & input, QObject * parent, QString * errorMessage
) {
	Q_ASSERT( input.isStartElement() );
	Q_ASSERT( input.name() == "DataSource" );
	Q_ASSERT( ! input.attributes().value( "type" ).isEmpty() );

	const QString TypeName = input.attributes().value( "type" ).toString();
	if ( TypeName.isEmpty() ) {
		input.raiseError( "Empty `type' attribute" );
		return 0;
	}

	DataSource * object = DataSourceFactory::create( TypeName, parent );
	if ( ! object ) {
		input.raiseError( 
			QString( "Unknown or unregistered data source type: `%1'." )
			.arg( TypeName )
		);
		return 0;
	}

	while ( ! input.atEnd() ) {
		if ( ! input.readNextStartElement() ) {
			break;
		}

		if ( input.name() == "Parameter" ) {
			object->readXmlParameter( input );
		}
		else {
			input.raiseError(
				QString( "Unexpected element found: `%1'" )
				.arg( input.name().toString() )
			);
		}
	}

	if ( ! input.hasError() ) {
		return object;
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
		delete object;
		return 0;
	}
}



QMultiHash< QString, QString > DataSource::parameters() const {
	static const QMultiHash< QString, QString > Empty;

	return Empty;
}


Dataset DataSource::readDataset( int num ) const {
	return Dataset();
}


void DataSource::readXmlParameter( QXmlStreamReader & input ) {
	Q_ASSERT( input.name() == "Parameter" );

	QString name;
	QString value;

	while ( ! input.atEnd() ) {
		if ( ! input.readNextStartElement() ) {
			break;
		}

		const QStringRef & Element = input.name();
		const QString Text = input.readElementText(
			QXmlStreamReader::ErrorOnUnexpectedElement
		);

		if ( Element == "Name" ) {
			name = Text;
		}
		else if ( Element == "Value" ) {
			value = Text;
		}
		else {
			input.raiseError(
				QString( "Unexpected element: `%1'" )
				.arg( Element.toString() )
			);
			return;
		}
	}

	if ( ! name.isEmpty() ) {
		setParameter( name, value );
	}
	else {
		input.raiseError( "Empty <Name> element" );
	}
}


void DataSource::refresh() {
	cache().clear();
}


void DataSource::setParameter( const QString & Name, const QString & Value ) {
}


int DataSource::size() const {
	return 0;
}


void DataSource::toXml( QXmlStreamWriter & xml ) const {
	xml.writeStartElement( "DataSource" );
	xml.writeAttribute( "type", typeName() );

	const QMultiHash< QString, QString >  Parameters = parameters();
	for (
		QMultiHash< QString, QString >::const_iterator i = Parameters.constBegin();
		i != Parameters.constEnd(); ++i
	) {
		xml.writeStartElement( "Parameter" );
		xml.writeTextElement( "Name", i.key() );
		xml.writeTextElement( "Value", i.value() );
		xml.writeEndElement(); // Parameter
	}

	xml.writeEndElement(); // DataSource
}


const QString & DataSource::typeName() const {
	return TypeName_;
}

}; // Namespace DICOM ends here.