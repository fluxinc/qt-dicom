/***************************************************************************
 *   Copyright (C) 2011 by Flux Inc.                                       *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#include "DataSourceCreator.hpp"
#include "DataSourceFactory.hpp"
#include "FileSystemDataSource.hpp"

#include <QtCore/QFile>
#include <QtCore/QFileInfoList>
#include <QtCore/QRegExp>
#include <QtCore/QXmlStreamReader>

const bool Dicom::FileSystemDataSource::Registered_ = 
	DataSourceFactory::registerType(
		FileSystemDataSource::typeName(), 
		new DataSourceCreator< FileSystemDataSource >()
	);


namespace Dicom {

FileSystemDataSource::FileSystemDataSource( QObject * parent ) :
	DataSource( typeName(), parent )
{
	setNameFilters( Dcm, QStringList() << "*.dcm" );
}


FileSystemDataSource::FileSystemDataSource(
	const FileSystemDataSource & Other
) :
	DataSource( Other ),
	files_( Other.files_ ),
	nameFilters_( Other.nameFilters_ )
{
}


FileSystemDataSource::~FileSystemDataSource() {
}


void FileSystemDataSource::addDirectory( const QDir & Directory ) const {
	if ( ! Directory.exists() ) {
		qWarning(
			"Non-existing directory: `%s'",
			qPrintable( QDir::toNativeSeparators( Directory.absolutePath() ) )
		);
		return;
	}

	for (
		QHash< FileType, QStringList >::const_iterator i = nameFilters().constBegin();
		i != nameFilters().constEnd(); ++i
	) {
		const FileType & Type = i.key();

		const QFileInfoList TheList = Directory.entryInfoList( 
			nameFilters( Type ),
			QDir::Files | QDir::Readable,
			QDir::Name | QDir::Time
		);

		for (
			QFileInfoList::const_iterator j = TheList.constBegin();
			j != TheList.constEnd(); ++j
		) {
			addFile( *j, Type );
		}
	}
}


void FileSystemDataSource::addDirectoryPath( const QString & Path ) const {
	addDirectory( QDir( Path ) );
}


void FileSystemDataSource::addFile( const QFileInfo & File, FileType type ) const {
	if ( type != Unknown ) {
		files()[ type ].append( File );
		return;
	}
	else {
		for ( 
			QHash< FileType, QStringList >::const_iterator i = nameFilters().constBegin();
			i != nameFilters().constEnd(); ++i
		) {
			const QStringList & Filters = i.value();

			for ( 
				QStringList::const_iterator j = Filters.constBegin();
				j != Filters.constEnd(); ++j
			) {
				const QRegExp Filter( *j, Qt::CaseInsensitive, QRegExp::Wildcard );
				if ( Filter.exactMatch( File.fileName() ) ) {
					files()[ i.key() ].append( File );
					return;
				}
			}
		}
	}

	qWarning(
		"None filters applie to: `%s'",
		qPrintable( QDir::fromNativeSeparators( File.absoluteFilePath() ) )
	);
}


void FileSystemDataSource::addFilePath( const QString & Path, FileType type ) const {
	addFile( QFileInfo( Path ), type );
}


void FileSystemDataSource::addPath( const QString & Path ) {
	const QFileInfo Info( Path );

	if ( ! paths().contains( Path ) ) {
		paths().append( Path );

		if ( Info.isFile() ) {
			addFilePath( Path );
		}
		else if ( Info.isDir() ) {
			addDirectoryPath( Path );
		}
	}
	else {
		qWarning(
			"Path: `%s' has already been added",
			qPrintable( QDir::toNativeSeparators( Path ) )
		);
	}	
}


FileSystemDataSource::FileType 
FileSystemDataSource::fileTypeFromString( const QString & Value )
{
	static const QString & DcmString = fileTypeString( Dcm );
	static const QString & XmlString = fileTypeString( Xml );

	if ( Value.compare( DcmString, Qt::CaseInsensitive ) == 0 ) {
		return Dcm;
	}
	else if ( Value.compare( XmlString, Qt::CaseInsensitive ) == 0 ) {
		return Xml;
	}
	else {
		return Unknown;
	}
}


const QString & FileSystemDataSource::fileTypeString( FileType type ) {
	switch ( type ) {
#define CASE( TYPE, VALUE ) \
		case TYPE: { static const QString S = VALUE; return S; } break

		CASE( Dcm, "DCM" );
		CASE( Xml, "XML" );

		default : {
			Q_ASSERT( 0 );
			static const QString TheValue = "Unknown";
			return TheValue;
		}
	};
}


QHash< FileSystemDataSource::FileType, QFileInfoList > & 
FileSystemDataSource::files() const
{
	return files_;
}


bool FileSystemDataSource::isRegistered() {
	return Registered_;
}


const QHash< FileSystemDataSource::FileType, QStringList > & 
FileSystemDataSource::nameFilters() const
{
	return nameFilters_;
}


QHash< FileSystemDataSource::FileType, QStringList > & 
FileSystemDataSource::nameFilters()
{
	return nameFilters_;
}


QStringList FileSystemDataSource::nameFilters( FileType type ) const {
	if ( nameFilters().contains( type ) ) {
		return nameFilters().value( type );
	}
	else  {
		return QStringList();
	}
}


QStringList FileSystemDataSource::nameFilters( FileType type ) {
	if ( nameFilters().contains( type ) ) {
		return nameFilters().value( type );
	}
	else  {
		return QStringList();
	}
}


QMultiHash< QString, QString > FileSystemDataSource::parameters() const {
	QMultiHash< QString, QString > result;

	static const QString P1Name = "NameFilters";
	for (
		QHash< FileType, QStringList >::const_iterator i = nameFilters_.constBegin();
		i != nameFilters_.constEnd(); ++i
	) {
		const QString Filter = QString( "%1:%2" )
			.arg( fileTypeString( i.key() ) )
			.arg( i.value().join( "|" ) )
		;
		result.insert( P1Name, Filter );
	}

	static const QString P2Name = "Path";
	for (
		QStringList::const_iterator i = paths_.constBegin();
		i != paths_.constEnd(); ++i
	) {
		result.insert( P2Name, *i );
	}

	return result;
}


void FileSystemDataSource::parseNameFilters( const QString & Value ) {
	//                              xml:*.xml
	//                              DCM:*.dcm|*.dicom
	static const QRegExp Format( "^(XML|DCM):((?:[^|]+\\|)*[^|]+)$", Qt::CaseInsensitive );
	Q_ASSERT( Format.isValid() );
	Q_ASSERT( Format.indexIn( "xml:*.xml" ) > -1 );
	Q_ASSERT( Format.indexIn( "DCM:*.dcm|*.dicom" ) > -1 );
	Q_ASSERT( Format.indexIn( "DCM:" ) == -1 );

	if ( Format.indexIn( Value ) > -1 ) {
		const FileType Type = fileTypeFromString( Format.cap( 1 ) );
		Q_ASSERT( Type != Unknown );
		
		setNameFilters( Type, Format.cap( 2 ).split( '|' ) );
	}
	else {
		qWarning(
			"Filter: `%s' doesn't match format: `%s', e.g.: `%s', `%s'",
			qPrintable( Value ), qPrintable( Format.pattern() ),
			"XML:*.xml", "DCM:*.dcm|*.dicom"
		);
	}
}


const QStringList & FileSystemDataSource::paths() const {
	return paths_;
}


QStringList & FileSystemDataSource::paths() {
	return paths_;
}


Dicom::Dataset FileSystemDataSource::readDataset( int offset ) const {
	const int FilesCount = size();

	Q_ASSERT( offset < FilesCount && offset >= 0 );
	if ( offset >= FilesCount || offset < 0 ) {
		return Dicom::Dataset();
	}

	FileType type = FileSystemDataSource::Unknown;
	QFileInfoList & list = QFileInfoList();

	for (
		QHash< FileType, QFileInfoList >::const_iterator i = files().constBegin();
		i != files().constEnd(); ++i
	) {
		list = i.value();
		if ( offset >= list.size() ) {
			offset -= list.size();
		}
		else {
			type = i.key();
			break;
		}
	}

	Dicom::Dataset dset;
	QString errorMessage;
	const QString Path = list.at( offset ).absoluteFilePath();
	switch ( type ) {
		case Dcm :
			qDebug( "Loading Data Set from DCM file: `%s'", Path.toUtf8().constBegin() );

			dset = Dataset::fromDicomFile( Path, &errorMessage );
			break;
		case Xml : {
			qDebug( "Loading Data Set from XML file: `%s'", Path.toUtf8().constBegin() );

			QFile f( Path );
			if ( f.open( QIODevice::ReadOnly ) ) {
				QXmlStreamReader input( &f );
				if ( input.readNextStartElement() ) {
					dset.readXml( input, &errorMessage );
				}
			}
			else {
				errorMessage = QString( "Failed to open file: `%1'" )
					.arg( QDir::toNativeSeparators( Path ) )
				;
			}
			break;			
		}
		case Unknown :
		default :
			Q_ASSERT( 0 );
			errorMessage = "Unknown file type";
	}

	if ( dset.isEmpty() ) {
		if ( ! errorMessage.isEmpty() ) {
			qWarning( qPrintable( errorMessage ) );
		}
		else {
			qWarning(
				"File: `%s' contains an empty dataset.",
				qPrintable( QDir::toNativeSeparators( Path ) )
			);
		}
	}

	return dset;
}


void FileSystemDataSource::refresh() {
	clearCache();
	files().clear();

	for (
		QStringList::const_iterator i = paths().constBegin();
		i != paths().constEnd(); ++i
	) {
		addPath( *i );
	}
}


void FileSystemDataSource::setNameFilters( 
	FileType type, const QStringList & Filters
) {
	nameFilters_.insert( type, Filters );
}


void FileSystemDataSource::setParameter( 
	const QString & Name, const QString & Value
) {
	if ( Name == "Path" ) {
		addPath( Value );
	}
	else if ( Name == "NameFilters" ) {
		parseNameFilters( Value );
	}
	else {
		Q_ASSERT( 0 );

		qWarning(
			"Unsupported data source parameter: `%s'",
			qPrintable( Name )
		);
	}
}


int FileSystemDataSource::size() const {
	int size = 0;
	for (
		QHash< FileType, QFileInfoList >::const_iterator i = files().constBegin();
		i != files().constEnd(); ++i
	) {
		size += i.value().size();
	}
	return size;
}


const QString & FileSystemDataSource::typeName() {
	static const QString TheName = "FileSystem";

	return TheName;
}

}; // Namespace DICOM ends here.
