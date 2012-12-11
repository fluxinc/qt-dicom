/***************************************************************************
 *   Copyright (C) 2011 by Flux Inc.                                       *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#include "UidList.hpp"

#include <QtCore/QSet>
#include <QtCore/QStringList>
#include <QtCore/QXmlStreamWriter>

#include <dcmtk/dcmdata/dcuid.h>
#include <dcmtk/dcmdata/dcxfer.h>


namespace Dicom {

UidList::UidList() {
}


UidList::UidList( const char * Uid ) {
	append( Uid );
}


UidList::UidList( const QStringList & List ) {
	for (
		QStringList::const_iterator i = List.constBegin();
		i != List.constEnd(); ++i
	) {
		append( i->toAscii() );
	}
}


UidList::~UidList() {
}


UidList UidList::echoSopClass() {
	static UidList theList;

	if ( ! theList.isEmpty() ) {
		return theList;
	}
	else {
		theList.append( UID_VerificationSOPClass );
		return theList;
	}
}


UidList UidList::queryRetrieveSopClasses() {
	static UidList theList;

	if ( ! theList.isEmpty() ) {
		return theList;
	}
	else {
		theList.append( UID_FINDPatientRootQueryRetrieveInformationModel );
		theList.append( UID_FINDStudyRootQueryRetrieveInformationModel );
		return theList;
	}
}


UidList UidList::storageSopClasses() {
	static UidList theList;

	if ( ! theList.isEmpty() ) {
		return theList;
	}
	else {
		for ( int i = 0; i < numberOfAllDcmStorageSOPClassUIDs; ++i ) {
			theList.append( dcmAllStorageSOPClassUIDs[ i ] );
		}
		return theList;
	}
}


UidList UidList::supportedTransferSyntaxes() {
	static UidList theList;

	if ( ! theList.isEmpty() ) {
		return theList;
	}
	else {
		theList.append( UID_JPEG2000TransferSyntax );
		theList.append( UID_JPEG2000LosslessOnlyTransferSyntax );
		theList.append( UID_JPEGProcess2_4TransferSyntax );
		theList.append( UID_JPEGProcess1TransferSyntax );
		theList.append( UID_JPEGProcess14SV1TransferSyntax );
		theList.append( UID_JPEGLSLossyTransferSyntax );
		theList.append( UID_JPEGLSLosslessTransferSyntax );
		theList.append( UID_RLELosslessTransferSyntax );
		theList.append( UID_MPEG2MainProfileAtMainLevelTransferSyntax );
		theList.append( UID_MPEG2MainProfileAtHighLevelTransferSyntax );
		theList.append( UID_DeflatedExplicitVRLittleEndianTransferSyntax );
		if ( gLocalByteOrder == EBO_LittleEndian ) {
			theList.append( UID_LittleEndianExplicitTransferSyntax );
			theList.append( UID_BigEndianExplicitTransferSyntax );
        } 
		else {
			theList.append( UID_BigEndianExplicitTransferSyntax );
			theList.append( UID_LittleEndianExplicitTransferSyntax );
        }
		theList.append( UID_LittleEndianImplicitTransferSyntax );

		return theList;
	}
}


const char ** UidList::toFlatArray() const {
	const char ** string = new const char * [ size() ];
	for ( int i = 0; i < size(); ++i ) {
		string[ i ] = at( i );
	}

	return string;
}


QSet< QByteArray > UidList::toSet() const {
	Q_ASSERT( 0 );

	return QSet< QByteArray >();
}


void UidList::writeXml( QXmlStreamWriter & output ) const {
	output.writeStartElement( "UidList" );
	output.writeAttribute( "count", QString::number( size() ) );
	for (
		UidList::const_iterator i = constBegin();
		i != constEnd(); ++i
	) {
		output.writeTextElement( "Uid", *i );
	}
	output.writeEndElement();
}


UidList operator+( const UidList & first, const UidList & second ) {
	UidList result( first );
	result.append( second );

	return result;
}

}; // Namespace DICOM ends here.
