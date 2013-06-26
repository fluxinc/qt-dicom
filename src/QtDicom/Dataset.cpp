/***************************************************************************
 *   Copyright © 2011-2013 by Flux Inc.                                    *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#include <QtCore/QDateTime>
#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QMutex>
#include <QtCore/QRegExp>
#include <QtCore/QSet>
#include <QtCore/QStringList>
#include <QtCore/QTime>
#include <QtCore/QXmlStreamReader>
#include <QtCore/QXmlStreamWriter>

#include <QtDicom/QDicomImageCodec>
#include <QtDicom/QDicomTag>
#include <QtDicom/QTransferSyntax>
#include <QtDicom/QUid>

#include <dcmtk/dcmdata/dcsequen.h>
#include <dcmtk/dcmdata/dcelem.h>
#include <dcmtk/dcmdata/dcfilefo.h>

#include <dcmtk/dcmimage/diregist.h>

#include <dcmtk/dcmnet/diutil.h>

#include "Dataset.hpp"
#include "DatasetConstIterator.hpp"


static QDate dateFromString( const QString & String );
static QDateTime dateTimeFromString( const QString & String );
static QTime timeFromString( const QString & String );

static bool matchElement_Range( 
	const QString & Pattern, const QString & Value, const DcmEVR & Vr
);
static bool matchElement_SingleValue( 
	const QString & Pattern, const QString & Value, const DcmEVR & Vr
);
static bool matchElement_WildCard( 
	const QString & Pattern, const QString & Value, const DcmEVR & Vr
);

const QString WildCardCharacters( "*?" );
const QString RangeCharacters( "-" );
const QString ReservedCharacters( "\\" );
const QSet< DcmEVR > TimeVrs = QSet< DcmEVR >() 
	<< EVR_DA << EVR_DT << EVR_TM
;
const QSet< DcmEVR > WildCardAllowedVrs = QSet< DcmEVR >()
	<< EVR_AE << EVR_CS << EVR_LT << EVR_LO 
	<< EVR_SH << EVR_PN << EVR_ST << EVR_UT
;


namespace Dicom {

Dataset::Dataset() :
	d_( new Dataset_priv() )
{
}


Dataset::Dataset( const Dataset & Other ) :
	d_( Other.d_ )
{
}


Dataset::Dataset( const DcmDataset & otherDataset ) :
	d_( new Dataset_priv() )
{
	setDcmDataset( otherDataset );
}


Dataset::~Dataset() {
}


Dataset & Dataset::operator = ( const Dataset & Other ) {
	if ( this != &Other ) {
		d_ = Other.d_;
	}

	return * this;
}


QDicomAttribute Dataset::attribute( const QDicomTag & Tag, bool * exists ) const {
	OFString value;
	const OFCondition Result = unconstDcmDataSet().findAndGetOFStringArray( 
		DcmTagKey( Tag.group(), Tag.element() ), value
	);
	if ( Result.good() ) {
		if ( exists )  {
			*exists = true;
		}
		return QDicomAttribute( Tag, QString( value.c_str() ).split( '\\' ) );
	}
	else {
		if ( exists )  {
			*exists = false;
		}
		return QDicomAttribute();
	}
}


bool Dataset::canConvertToTransferSyntax( 
	const QTransferSyntax & DstTs
) const {
	static const QList< QTransferSyntax > SupportedTs = 
		QDicomImageCodec::supported()
	;

	const QTransferSyntax SrcTs = syntax();
	return 
		( ( ! SrcTs.isCompressed() ) || SupportedTs.contains( SrcTs ) ) &&
		( ( ! DstTs.isCompressed() ) || SupportedTs.contains( DstTs ) )
	;
}


Dataset::const_iterator Dataset::constBegin() const {
	return Dataset::ConstIterator( *this );
}


Dataset::const_iterator Dataset::constEnd() const {
	return Dataset::ConstIterator( *this, true );
}


bool Dataset::containsTag( const DcmTagKey & key ) const {
	return unconstDcmDataSet().tagExists( key );
}


bool Dataset::containsTag( quint16 group, quint16 element ) const {
	return containsTag( DcmTagKey( group, element ) );
}


Dataset Dataset::convertedToTransferSyntax( const QTransferSyntax & DstTs ) const {
	const QTransferSyntax & SrcTs = syntax();

	if ( DstTs != SrcTs ) {
		Dataset result;

		if ( canConvertToTransferSyntax( DstTs ) ) {
			DcmDataset newDset = dcmDataset();

			QDicomImageCodec * codec = 
				QDicomImageCodec::codecForTransferSyntax( DstTs )
			;
			if ( codec != NULL ) {
				const OFCondition Converted = newDset.chooseRepresentation( 
					DcmXfer( DstTs.uid() ).getXfer(),
					codec->dcmParameters()
				);

				if ( Converted.good() ) {
					newDset.removeAllButCurrentRepresentations();
					result.setDcmDataset( newDset );

					Q_ASSERT( result.syntax() == DstTs );
					return result;
				}
				else {
					qCritical( __FUNCTION__": "
						"failed to convert from %s to %s; %s",
						SrcTs.name(), DstTs.name(), Converted.text()
					);
				}
			}
			else {
				qCritical( __FUNCTION__": "
					"no codec for %s transfer syntax",
					DstTs.name()
				);
			}
		}
		else {
			qCritical( __FUNCTION__": "
				"conversion from %s to %s is unsupported",
				SrcTs.name(), DstTs.name()
			);
		}

		return result;
	}
	else {
		return Dataset( dcmDataset() );
	}	
}


const DcmDataset & Dataset::dcmDataset() const {
	return  d_->dcmDataSet();
}


DcmDataset & Dataset::dcmDataset() {
	return d_->dcmDataSet();
}


Dataset Dataset::fromDicomFile( const QString & Path, QString * errorMessage ) {
	DcmFileFormat file;
	const OFCondition Result = file.loadFile( Path.toUtf8().constData() );
	if ( Result.bad() ) {
		if ( errorMessage ) {
			*errorMessage = 
				QString( "Failed to load a DICOM file `%1'; %2." )
				.arg( QDir::toNativeSeparators( Path ) )
				.arg( Result.text() )
			;
		}
		return Dataset();
	}

	return Dataset( *file.getDataset() );
}


Dataset Dataset::fromFile( const QString & Path, QString * errorMessage ) {
	DcmDataset dataSet;
	const OFCondition Result = dataSet.loadFile( Path.toUtf8().constData() );
	if ( Result.bad() ) {
		if ( errorMessage ) {
			*errorMessage = 
				QString( "Failed to load a raw Data Set from file `%1'; %2." )
				.arg( QDir::toNativeSeparators( Path ) )
				.arg( Result.text() )
			;
		}
		return Dataset();
	}

	return Dataset( dataSet );
}


Dataset Dataset::fromXmlStream( QXmlStreamReader & input, QString * errorMessage ) {
	Dicom::Dataset dataSet;
	const bool Result = dataSet.readXml( input, errorMessage );
	if ( Result ) {
		return dataSet;
	}
	else {
		return Dataset();
	}
}


bool Dataset::isEmpty() const {
	return unconstDcmDataSet().isEmpty();
}


Dataset Dataset::match( const Dataset & Mask ) const {
	Dataset result;

	const bool Status = matchItem( 
		const_cast< Dataset & >( Mask ).dcmDataset(),
		unconstDcmDataSet(),
		result.dcmDataset()
	);

	if ( Status ) {
		return result;
	}
	else {
		return Dataset();
	}
}


bool Dataset::matchItem( 
	DcmItem & mask, DcmItem & identifier, DcmItem & result
) const {
	DcmElement * element = 0;

	for (
		ConstIterator i = ConstIterator( mask ); 
		i != ConstIterator( mask, true ); ++i
	) {
		element = 0;
		if ( i.atElement() || i.atSequence() ) {
			const OFCondition Result = identifier.findAndGetElement( 
				i->getTag(), element
			);
			if ( element ) {
				element = i.atElement() ?
					matchElement( i.element(), *element ) :
					matchSequence(
						i.sequence(), *reinterpret_cast< DcmSequenceOfItems * >( element )
					)
				;
				if ( element ) {
					const OFCondition Result = result.insert( element, true );
					if ( Result.bad() ) {
						qWarning() << QString( "Failed to insert `%1'; %2." )
										.arg( DcmTag( i->getTag() ).getTagName() )
										.arg( Result.text() );
					}
				}
				else {
					return false;
				}
			}
		}
		else {
			Q_ASSERT( 0 );
		}
	}

	return true;
}


DcmElement * Dataset::matchElement( DcmElement & mask, DcmElement & identifier ) const {
	Q_ASSERT( mask.getTag() == identifier.getTag() );

	OFString pattern;
	mask.getOFStringArray( pattern );

	DcmElement * result = NULL;

	// If the value specified for a Key Attribute in a request is zero length, then all entities shall match
	// this Attribute. An Attribute which contains a Universal Match specification in a C-FIND request
	// provides a mechanism to request the selected Attribute value be returned in corresponding C-
	// FIND responses.
	if ( pattern.size() == 0 ) { // Zero length
		return reinterpret_cast< DcmElement * >( identifier.clone() );
	}

	const QString Pattern( pattern.c_str() );
	const DcmEVR Vr = mask.getVR();
	const unsigned long Vm = mask.getVM();

	OFString value;
	identifier.getOFStringArray( value );
	const QString Value( value.c_str() );

	enum Matching {
		Unknown = 0,
		SingleValue,
		ListOfUid,
		WildCard,
		Range
	};

	Matching matching = Unknown;

	{
		// If the value specified for a Key Attribute in a request is non-zero length and if it is:
		// a) not a date or time or datetime, contains no wild card characters
		// b) a date or time or datetime, contains a single date or time or datetime with no “-“
		// then single value matching shall be performed. 
		const bool IsSingleValue = (
			! TimeVrs.contains( Vr ) ?
			! Pattern.contains( QRegExp(
				QString( "[%1%2]" )
				.arg( QRegExp::escape( WildCardCharacters ) )
				.arg( QRegExp::escape( ReservedCharacters ) )
			) ) :
			! Pattern.contains( QRegExp(
				QString( "[%1]" )
				.arg( QRegExp::escape( RangeCharacters ) )
			) )
		);
		if ( IsSingleValue ) {
			matching = SingleValue;
		}
		else if ( ( Vr == EVR_UI ) && ( Vm > 1 ) ) {
			//  A list of single values is encoded exactly as a VR of UI and a VM of Multiple
			matching = ListOfUid;
		}
		else {
			// If the Attribute is not a date, time, signed long, signed short, unsigned short, unsigned long,
			// floating point single, floating point double, other byte string, other word string, unknown, attribute
			// tag, decimal string, integer string, age string or UID and the value specified in the request
			// contains any occurrence of an “*” or a “?”
			const bool IsWildCard = WildCardAllowedVrs.contains( Vr ) && 
				Pattern.contains( QRegExp(
					QString( "[%1]" ).arg( QRegExp::escape( WildCardCharacters ) )
				) )
			;
			if ( IsWildCard ) {
				matching = WildCard;
			}
			else {
				const bool IsRange = TimeVrs.contains( Vr ) &&
					Pattern.contains( QRegExp(
						QString( "[%1]" ).arg( QRegExp::escape( RangeCharacters ) )
					) )
				;
				matching = Range;
			}
		}
	}

	switch ( matching ) {

	case SingleValue :
		if ( matchElement_SingleValue( Pattern, Value, Vr ) ) {
			result = &identifier;
		}
		break;

	case ListOfUid :
		if ( Pattern.split( '\\' ).contains( Value ) ) {
			result = &identifier;
		}
		break;

	case WildCard :
		if ( matchElement_WildCard( Pattern, Value, Vr ) ) {
			result = &identifier;
		}
		break;

	case Range :
		if ( matchElement_Range( Pattern, Value, Vr ) ) {
			result = &identifier;
		}
		break;

	default :
		qWarning(
			"Unrecognized matching type for pattern: `%s'", qPrintable( Pattern )
		);
	}

	return result ? reinterpret_cast< DcmElement * >( result->clone() ) : result;
}


DcmSequenceOfItems * Dataset::matchSequence( 
	DcmSequenceOfItems & mask, DcmSequenceOfItems & identifier
) const {
	if ( mask.card() < 1 ) {
		qWarning() << QString( 
				"An invalid Sequence Key Attribute with no Items found when "
				"reading: `%1'."
			)
			.arg( DcmTag( mask.getTag() ).getTagName() )
		;

		return 0;
	}

	DcmSequenceOfItems * result = new DcmSequenceOfItems( mask.getTag() );
	DcmItem * maskItem = mask.getItem( 0 );
	for ( unsigned long i = 0; i < identifier.card(); ++i ) {
		DcmItem * resultItem = new DcmItem();
		if ( matchItem( *maskItem, *identifier.getItem( i ), *resultItem ) ) {
			result->insert( resultItem );
		}
		else {
			delete resultItem;
		}
	}

	if ( result->card() > 0 ) {
		return result;
	}
	else {
		delete result;
		return 0;
	}
}


void Dataset::readContainerItems(
	QXmlStreamReader & input, DcmItem & container 
) const {
	while ( ! input.atEnd() ) {
		if ( ! input.readNextStartElement() ) {
			break;
		}

		const QStringRef Name = input.name();
		if ( Name == "Sequence" ) {
			readSequence( input, container );
		}
		else if ( Name == "Tag" ) {
			readTag( input, container );
		}
		else {
			input.raiseError(
				QString( "Unexpected element: `%1'" )
				.arg( Name.toString() )
			);
		}
	}
}


QPair< int, int > Dataset::readGroupAndElementAttributes( 
	QXmlStreamReader & input
) const {
	QPair< int, int > result = qMakePair( -1, -1 );

	const QXmlStreamAttributes Attributes = input.attributes();
	bool ok;

	const QString GroupString = Attributes.value( "group" ).toString();
	const quint16 Group = GroupString.toUShort( &ok, 16 );

	if ( ! ok ) {
		input.raiseError( 
			GroupString.isEmpty() ?
			"No `group' attribute in the `Tag' element" :
			QString( 
				"Failed to extract an integer from the: `%1' value "
				"when reading tag group designator" 
			)
			.arg( GroupString )
		);		
		return result;
	}


	const QString ElementString = Attributes.value( "element" ).toString();
	const quint16 Element = ElementString.toUShort( &ok, 16 );

	if ( ! ok ) {
		input.raiseError( 
			ElementString.isEmpty() ?
			"No `element' attribute in the `Tag' element" :
			QString( 
				"Failed to extract an integer from the: `%1' value "
				"when reading tag element designator" 
			)
			.arg( GroupString )
		);
		return result;
	}

	return qMakePair< int, int >( Group, Element );
}


void Dataset::readItem(
	QXmlStreamReader & input, DcmSequenceOfItems & sequence
) const {
	Q_ASSERT( input.name() == "Item" );

	DcmItem * item = new DcmItem();
	readContainerItems( input, *item );
	const OFCondition Result = sequence.insert( item );
	if ( Result.bad() ) {
		input.raiseError( Result.text() );
	}
}


void Dataset::readSequence(
	QXmlStreamReader & input, DcmItem & container
) const {
	Q_ASSERT( input.name() == "Sequence" );

	const QPair< int, int > Designator = readGroupAndElementAttributes( input );
	if ( input.hasError() ) {
		return;
	}

	DcmTag t( Designator.first, Designator.second );
	if ( t.error().bad() ) {
		input.raiseError( t.error().text() );
		return;
	}

	const DcmEVR Vr = t.getVR().getEVR();
	Q_ASSERT( Vr == EVR_SQ );
	if ( Vr != EVR_SQ ) {
		input.raiseError(
			QString( "Element: (%1,%2) is not a sequence" )
			.arg( Designator.first, 4, 16, QChar( '0' ) )
			.arg( Designator.second, 4, 16, QChar( '0' ) )
		);

		return;
	}

	DcmSequenceOfItems * sequence = new DcmSequenceOfItems( t );
	while ( ! input.atEnd() ) {
		if ( ! input.readNextStartElement() ) {
			break;
		}

		const QStringRef Name = input.name();
		if ( Name == "Item" ) {
			readItem( input, *sequence );
		}
		else {
			input.raiseError(
				QString( "Unexpected element: `%1'" )
				.arg( Name.toString() )
			);
		}
	}
	if ( ! input.hasError() ) {
		OFCondition Result = container.insert( sequence );
		if ( Result.bad() ) {
			input.raiseError( Result.text() );
		}
	}
}


void Dataset::readTag( QXmlStreamReader & input, DcmItem & item ) const {
	Q_ASSERT( input.name() == "Tag" );

	const QPair< int, int > Designator = readGroupAndElementAttributes( input );
	if ( input.hasError() ) {
		return;
	}

	DcmTag t( Designator.first, Designator.second );
	if ( t.error().bad() ) {
		input.raiseError( t.error().text() );
		return;
	}

	const DcmEVR Vr = t.getVR().getEVR();
	Q_ASSERT( Vr != EVR_SQ && Vr != EVR_item && Vr != EVR_dataset );

	if ( Vr != EVR_SQ && Vr != EVR_item && Vr != EVR_dataset ) {
		const bool Raw = 
			input.attributes().value( "encoding" ) != "Base64"
		;

		const QString Text = Raw ?
			input.readElementText() :
			QByteArray::fromBase64( input.readElementText().toAscii() )
		;
		const OFString Bytes = Text.toStdString().c_str();
		const OFCondition Result = item.putAndInsertOFStringArray(
			t, Bytes
		);

		if ( Result.bad() ) {
			input.raiseError( Result.text() );
		}
	}
	else {
		input.raiseError(
			QString( "Element: (%1,%2) is not a leaf" )
			.arg( Designator.first, 4, 16, QChar( '0' ) )
			.arg( Designator.second, 4, 16, QChar( '0' ) )
		);
	}
}


bool Dataset::readXml( QXmlStreamReader & input, QString * errorMessage ) {
	Q_ASSERT( input.name() == "Dataset" );

	if ( input.name() != "Dataset" ) {
		input.raiseError( 
			QString( "Unexpected element: `%1'" )
			.arg( input.name().toString() )
		);
	}

	readContainerItems( input, dcmDataset() );

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


bool Dataset::toDicomFile( const QString & FilePath, QString * message ) const {
	const std::string Path = QDir::toNativeSeparators( FilePath ).toStdString();

	const E_TransferSyntax CurrentSyntax = dcmDataset().getCurrentXfer();	
	const E_TransferSyntax FileSyntax =
		CurrentSyntax != EXS_Unknown ?
		CurrentSyntax : EXS_LittleEndianExplicit
	;


	DcmFileFormat file( &unconstDcmDataSet() );

	const OFCondition Result = file.saveFile(
		OFString( Path.data(), Path.size() ), FileSyntax
	);

	if ( Result.good() ) {
		return true;
	}
	else if ( message != nullptr ) {
		*message = Result.text();
	}

	return false;
}


void Dataset::setAttribute( const QDicomTag & Tag, const QStringList & Values ) {
	const std::string Value = Values.join( "\\" ).toStdString();

	const OFCondition Status = dcmDataset().putAndInsertOFStringArray(
		DcmTag( Tag.group(), Tag.element() ), Value.c_str()
	);
	if ( Status.good() ) {
		return;
	}
	else {
		qWarning( __FUNCTION__": "
			"failed to set %s %s attribute to `%s'; %s",
			qPrintable( Tag.keyword() ), qPrintable( Tag.toString() ), Value.c_str(),
			Status.text()
		);
	}
}


void Dataset::setAttribute( const QDicomTag & Tag, const QString & Value ) {
	setAttribute( QDicomAttribute( Tag, Value ) );
}


void Dataset::setAttribute( const QDicomAttribute & Attribute ) {
	setAttribute( Attribute.tag(), Attribute.values() );
}


void Dataset::setDcmDataset( const DcmDataset & Dataset ) {
	d_->dcmDataSet() = Dataset;
}


void Dataset::setSopInstanceUid( const QByteArray & Value ) {
	setAttribute( QDicomTag::SopInstanceUid, Value.constData() );
}


QByteArray Dataset::sopClassUid() const {
	DIC_UI sopClass, sopInstance;

	const bool SopClassPresent = DU_findSOPClassAndInstanceInDataSet( 
		&unconstDcmDataSet(), sopClass, sopInstance
	);
	if ( SopClassPresent ) {
		return sopClass;
	}
	else {
		return QByteArray();
	}
}


QByteArray Dataset::sopInstanceUid() const {
	DIC_UI sopClass, sopInstance;

	const bool SopClassPresent = DU_findSOPClassAndInstanceInDataSet( 
		&unconstDcmDataSet(), sopClass, sopInstance
	);
	if ( SopClassPresent ) {
		return sopInstance;
	}
	else {
		return QByteArray();
	}
}


QTransferSyntax Dataset::syntax() const {
	return QTransferSyntax::fromUid( 
		DcmXfer( dcmDataset().getCurrentXfer() ).getXferID()
	);
}


QString Dataset::tagValue( const DcmTagKey & Key, bool * exists ) const {
	OFString value;
	const OFCondition Result = unconstDcmDataSet().findAndGetOFStringArray( 
		Key, value
	);
	if ( Result.good() ) {
		if ( exists )  {
			*exists = true;
		}
		return QString( value.c_str() );
	}
	else {
		if ( exists )  {
			*exists = false;
		}
		return QString();
	}
}


QString Dataset::tagValue( quint16 group, quint16 element, bool * exists ) const {
	return tagValue( DcmTagKey( group, element ), exists );
}


QString Dataset::toString() const {
	std::ostringstream os;
	unconstDcmDataSet().print( os, DCMTypes::PF_shortenLongTagValues | DCMTypes::PF_convertToMarkup );
	std::string tmp = os.str();
	return QString( tmp.c_str() );
}


DcmDataset & Dataset::unconstDcmDataSet() const {
	return const_cast< DcmDataset & >( dcmDataset() );
}


void Dataset::writeContainerItems(
	QXmlStreamWriter & output,
	DcmItem & item
) const {
	DcmObject * o = 0;
	while ( o = item.nextInContainer( o ) ) {
		if ( o->isLeaf() ) {
			DcmElement * element =
				reinterpret_cast< DcmElement * >( o )
			;
			writeTag( output, *element );		
		}
		else if ( o->ident() == EVR_SQ ) {
			DcmSequenceOfItems * sequence = 
				reinterpret_cast< DcmSequenceOfItems * >( o )
			;
			writeSequence( output, *sequence );
		}
		else {
			qWarning() << 
				QString( "Unexpetect element (%1,%2) with EVR: `%3'." )
				.arg( o->getGTag(), 4, 16, QChar( '0' ) )
				.arg( o->getETag(), 4, 16, QChar( '0' ) )
				.arg( static_cast< int >( o->ident() ) )
			;
		}
	}
}

void Dataset::writeGroupAndElementAttributes(
	QXmlStreamWriter & output, DcmElement & element
) const {
	output.writeAttribute( 
		"group", 
		QString( "%1" ).arg( element.getGTag(), 4, 16, QChar( '0' ) )
	);
	output.writeAttribute( 
		"element", 
		QString( "%1" ).arg( element.getETag(), 4, 16, QChar( '0' ) )
	);
}


void Dataset::writeItem(
	QXmlStreamWriter & output, DcmItem & item
) const {
	Q_ASSERT( item.ident() == EVR_item );
	output.writeStartElement( "Item" );

	writeContainerItems( output, item );

	output.writeEndElement(); // </Item>
}


void Dataset::writeSequence( 
	QXmlStreamWriter & output, DcmSequenceOfItems & sequence
) const {
	Q_ASSERT( sequence.ident() == EVR_SQ );

	output.writeStartElement( "Sequence" );
	writeGroupAndElementAttributes( output, sequence );

	DcmObject * o = 0;
	while ( o = sequence.nextInContainer( o ) ) {
		Q_ASSERT( o->ident() == EVR_item );

		DcmItem * item = reinterpret_cast< DcmItem * >( o );
		writeItem( output, *item );
	}

	output.writeEndElement(); // </Sequence>
}


void Dataset::writeTag(
	QXmlStreamWriter & output, DcmElement & element
) const {
	Q_ASSERT( element.isLeaf() );

	output.writeStartElement( "Tag" );
	writeGroupAndElementAttributes( output, element );

	OFString tmp;
	const OFCondition Result = element.getOFStringArray( tmp );
	if ( Result.good() ) {
		const bool Encode = 
			containsTag( 0x0008, 0x0005 ) && (
				element.getVR() == EVR_SH ||
				element.getVR() == EVR_LO ||
				element.getVR() == EVR_ST ||
				element.getVR() == EVR_LT ||
				element.getVR() == EVR_PN ||
				element.getVR() == EVR_UT
		);

		const QString Value = QString::fromStdString( tmp.c_str() );
		if ( ! Encode ) {
			output.writeCharacters( Value );
		}
		else {
			output.writeAttribute( "encoding", "Base64" );
			output.writeCharacters( Value.toAscii().toBase64() );
		}
	}
	else {
		qWarning() << 
			QString( 
				"Failed to read value of a DICOM element: (%1,%2). "
				"Internal error description:\n%3"
			)
			.arg( element.getGTag(), 4, 16, QChar( '0' ) )
			.arg( element.getETag(), 4, 16, QChar( '0' ) )
			.arg( Result.text() )
		;
	}

	output.writeEndElement(); // </Tag>
}


void Dataset::writeXml( QXmlStreamWriter & output ) const {
#ifdef _DEBUG
	output.setAutoFormatting( true );
	output.setAutoFormattingIndent( -1 );
#endif

	DcmDataset & dataset = unconstDcmDataSet();

	Q_ASSERT( dataset.ident() == EVR_dataset );
	output.writeStartElement( "Dataset" );
	
	writeContainerItems( output, dataset );

	output.writeEndElement(); // </Dataset>
}


}; // Namespace DICOM ends here.



QDateTime dateTimeFromString( const QString & String ) {
	static const QRegExp Format(
		//YYYY (1)          MM (2)   DD (3)  
		"(\\d{4})[-./\\\\]?(\\d{2})?[-./\\\\]?(\\d{2})?T?"
		//HH (4)   MM (5)   SS (6)  
		"(\\d{2})?:?(\\d{2})?:?(\\d{2})?"
		//  F+ (7)        
		"(?:\\.(\\d+))?"
		// ZZ (8)      XX (9)
		"([+-]\\d{2})?(\\d{2})?"
	);

	Q_ASSERT( Format.isValid() );
	Q_ASSERT( Format.indexIn( "2012" ) > -1 );
	Q_ASSERT( Format.indexIn( "201207" ) > -1 );
	Q_ASSERT( Format.indexIn( "20120721" ) > -1 );
	Q_ASSERT( Format.indexIn( "2012072116" ) > -1 );
	Q_ASSERT( Format.indexIn( "201207211632" ) > -1 );
	Q_ASSERT( Format.indexIn( "20120721163208" ) > -1 );
	Q_ASSERT( Format.indexIn( "20120721163208.123" ) > -1 );
	Q_ASSERT( Format.indexIn( "20120721163208.123456+02" ) > -1 );
	Q_ASSERT( Format.indexIn( "20120721163208.123456+0200" ) > -1 );
	Q_ASSERT( Format.indexIn( "20120721163208.123456-1100" ) > -1 );

	if ( Format.indexIn( String ) < 0 ) {
		qWarning( "Invalid DT : `%s", qPrintable( String ) );
		return QDateTime();
	}

	// Convert time to ISO 8601 supported by Qt
	const QString Iso = QString( "%1-%2-%3T%4:%5:%6%7:%8" )
#define CAP( N, D, L ) \
		! Format.cap( N ).isEmpty() ? Format.cap( N ) : QString( "%1" ).arg( D, L, 10, QChar( '0' ) )
		.arg( CAP( 1, 100, 4 ) ) // year
		.arg( CAP( 2,   1, 2 ) ) // month
		.arg( CAP( 3,   1, 2 ) ) // day
		.arg( CAP( 4,   0, 2 ) ) // hour
		.arg( CAP( 5,   0, 2 ) ) // minute
		.arg( CAP( 6,   0, 2 ) ) // second
#undef CAP
		.arg( // Offset (hours)
			! Format.cap( 8 ).isEmpty() ?
			Format.cap( 8 ) :
			QString( "+00" )
		)
		.arg( // Offset (minutes)
			! Format.cap( 9 ).isEmpty() ?
			Format.cap( 9 ) :
			QString( "00" )
		)
	;

	const QString FractionString = Format.cap( 7 );
	const int Msecs = 
		FractionString.isEmpty() ? 0 :
		qRound( 1000.0 * FractionString.toUInt() / pow( 10.0, FractionString.size() ) )
	;
	Q_ASSERT( Msecs >= 0 );

	QDateTime result = QDateTime::fromString( Iso, Qt::ISODate );
	result.setTimeSpec( Qt::OffsetFromUTC );
	result = result.toUTC();
	result = result.addMSecs( Msecs );
#ifdef _DEBUG
	const QString Result = result.toString();
#endif

	return result;
}


QDate dateFromString( const QString & String ) {
	static const QRegExp Format(
		//YYYY (1)          MM (2)   DD (3)  
		"(\\d{4})[-./\\\\]?(\\d{2})?[-./\\\\]?(\\d{2})?"
	);

	Q_ASSERT( Format.isValid() );
	Q_ASSERT( Format.indexIn( "2012" ) > -1 );
	Q_ASSERT( Format.indexIn( "201207" ) > -1 );
	Q_ASSERT( Format.indexIn( "2012-07-20" ) > -1 );

	if ( Format.indexIn( String ) < 0 ) {
		qWarning( "Invalid DA : `%s", qPrintable( String ) );

		return QDate();
	}

	// Convert time to ISO 8601 supported by Qt
	const QString Iso = QString( "%1-%2-%3" )
#define CAP( N, P ) \
		! Format.cap( N ).isEmpty() ? Format.cap( N ) : QString( P, '0' )
		.arg( CAP( 1, 4 ) ) // year
		.arg( CAP( 2, 2 ) ) // month
		.arg( CAP( 3, 2 ) ) // day
#undef CAP
	;

	return QDate::fromString( Iso, Qt::ISODate );
}


bool matchElement_Range( 
	const QString & Pattern, const QString & Value, const DcmEVR & Vr
) {	
	Q_ASSERT( TimeVrs.contains( Vr ) );

	if ( Value.size() == 0 ) {
		return false;
	}

	const QStringList RangePoints = Pattern.split( '-', QString::KeepEmptyParts );
	if ( RangePoints.size() != 2 ) {
		qWarning( 
			"Invalid date or/and time range pattern: `%s'",
			qPrintable( Pattern )
		);

		return false;
	}

	bool result = false;

	const QString & FromString = RangePoints.at( 0 );
	const QString & ToString = RangePoints.at( 1 );

	switch ( Vr ) {
		case EVR_DA : {
			const QDate Current = dateFromString( Value );
			if ( Current.isValid() ) {
				const QDate From = 
					FromString.isEmpty() ? 
					QDate() : dateFromString( FromString )
				;
				const QDate To =
					ToString.isEmpty() ? 
					QDate() : dateFromString( ToString )
				;

				if ( From.isValid() && To.isValid() ) {
					result = ( ( Current >= From ) && ( Current <= To ) );
				}
				else if ( From.isValid() ) {
					result = Current >= From;
				}
				else if ( To.isValid() ) {
					result = Current <= To;
				}
			}
			break;
		}

		 case EVR_DT : {
			const QDateTime Current = dateTimeFromString( Value );
			if ( Current.isValid() ) {
				const QDateTime From = 
					FromString.isEmpty() ? 
					QDateTime() : dateTimeFromString( FromString )
				;
				const QDateTime To =
					ToString.isEmpty() ? 
					QDateTime() : dateTimeFromString( ToString )
				;

				if ( From.isValid() && To.isValid() ) {
					result = ( ( Current >= From ) && ( Current <= To ) );
				}
				else if ( From.isValid() ) {
					result = Current >= From;
				}
				else if ( To.isValid() ) {
					result = Current <= To;
				}
			}
			break;
		}

		case EVR_TM : {
			const QTime Current = timeFromString( Value );
			if ( Current.isValid() ) {
				const QTime From = 
					FromString.isEmpty() ? 
					QTime() : timeFromString( FromString )
				;
				const QTime To =
					ToString.isEmpty() ? 
					QTime() : timeFromString( ToString )
				;

				if ( From.isValid() && To.isValid() ) {
					result = ( ( Current >= From ) && ( Current <= To ) );
				}
				else if ( From.isValid() ) {
					result = Current >= From;
				}
				else if ( To.isValid() ) {
					result = Current <= To;
				}
			}
			break;
		}
	}
		
	return result;
}


bool matchElement_SingleValue( 
	const QString & Pattern, const QString & Value, const DcmEVR & Vr
) {
	const QStringList Values = Value.split( '\\' );
	bool result = false;

	if ( ! TimeVrs.contains( Vr ) ) {
		result = Values.contains( 
			Pattern, Vr != EVR_PN ? Qt::CaseSensitive : Qt::CaseInsensitive
		);
	}
	else {
		switch ( Vr ) {
			case EVR_DT : {
				const QDateTime ValueDate = dateTimeFromString( Value );
				const QDateTime PatternDate = dateTimeFromString( Pattern );

				if ( ValueDate.isValid() && PatternDate.isValid() ) {
					result = ValueDate == PatternDate;
				}
				break;
			}
			case EVR_DA : {
				const QDate ValueDate = dateFromString( Value );
				const QDate PatternDate = dateFromString( Pattern );

				if ( ValueDate.isValid() && PatternDate.isValid() ) {
					result = ValueDate == PatternDate;
				}
				break;
			}
			case EVR_TM : {
				const QTime ValueTime = timeFromString( Value );
				const QTime PatternTime = timeFromString( Pattern );

				if ( ValueTime.isValid() && PatternTime.isValid() ) {
					result = ValueTime == PatternTime;
				}
				break;
			}
			default :
				Q_ASSERT( 0 );
		};
	}

	return result;
}


bool matchElement_WildCard( 
	const QString & Pattern, const QString & Value, const DcmEVR & Vr
) {
	const QStringList Values( Value.split( '\\' ) );
	const QRegExp PatternExp( 
		Pattern,
		Vr != EVR_PN ? Qt::CaseSensitive : Qt::CaseInsensitive,
		QRegExp::Wildcard
	);

	if ( ! PatternExp.isValid() ) {
		qWarning( "Invliad wildcard pattern: `%s'.", qPrintable( Pattern ) );

		return false;
	}			

	for (
		QStringList::const_iterator i = Values.constBegin();
		i != Values.constEnd(); ++i
	) {
		if ( PatternExp.indexIn( *i ) > -1 ) {
			return true;
		}
	}

	return false;
}


QTime timeFromString( const QString & String ) {
	static const QRegExp Format(
		//HH (1)   MM (2)   SS (3)  
		"(\\d{2})?:?(\\d{2})?:?(\\d{2})?"
		//  F+ (4)        
		"(?:\\.(\\d+))?"
	);

	Q_ASSERT( Format.isValid() );
	Q_ASSERT( Format.indexIn( "2012" ) > -1 );
	Q_ASSERT( Format.indexIn( "201207" ) > -1 );
	Q_ASSERT( Format.indexIn( "20120721" ) > -1 );
	Q_ASSERT( Format.indexIn( "2012072116" ) > -1 );
	Q_ASSERT( Format.indexIn( "201207211632" ) > -1 );
	Q_ASSERT( Format.indexIn( "20120721163208" ) > -1 );
	Q_ASSERT( Format.indexIn( "20120721163208.123" ) > -1 );
	Q_ASSERT( Format.indexIn( "20120721163208.123456+02" ) > -1 );
	Q_ASSERT( Format.indexIn( "20120721163208.123456+0200" ) > -1 );
	Q_ASSERT( Format.indexIn( "20120721163208.123456-1100" ) > -1 );

	if ( Format.indexIn( String ) < 0 ) {
		qWarning( "Invalid TM : `%s", qPrintable( String ) );

		return QTime();
	}

	// Convert time to ISO 8601 supported by Qt
	const QString Iso = QString( "%1:%2:%3" )
#define CAP( N ) \
		! Format.cap( N ).isEmpty() ? Format.cap( N ) : QString( "00" )
		.arg( CAP( 1 ) ) // hour
		.arg( CAP( 2 ) ) // minute
		.arg( CAP( 3 ) ) // second
#undef CAP
	;

	const QString FractionString = Format.cap( 4 );
	const int Msecs = 
		FractionString.isEmpty() ? 0 :
		qRound( 1000.0 * FractionString.toUInt() / pow( 10.0, FractionString.size() ) )
	;
	Q_ASSERT( Msecs >= 0 );

	return QTime::fromString( Iso, Qt::ISODate ).addMSecs( Msecs );
}
