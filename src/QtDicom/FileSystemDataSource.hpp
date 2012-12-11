/***************************************************************************
 *   Copyright (C) 2011 by Flux Inc.                                       *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#ifndef DICOM_FILESYSTEMDATASOURCE_HPP
#define DICOM_FILESYSTEMDATASOURCE_HPP

#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QHash>
#include <QtCore/QString>
#include <QtCore/QStringList>

#include <QtDicom/DataSource.hpp>


namespace Dicom {

class FileSystemDataSource : public DataSource {
	Q_OBJECT;

	public :
		enum FileType {
			Unknown = 0,
			Dcm,
			Xml
		};

	public :
		FileSystemDataSource( QObject * parent = 0 );
		FileSystemDataSource( const FileSystemDataSource & other );
		~FileSystemDataSource();

		void addPath( const QString & path );

		static bool isRegistered();

		const QHash< FileType, QStringList > & nameFilters() const;
		QStringList nameFilters( FileType type ) const;

		const QStringList & paths() const;

		void refresh();

		void setNameFilters( FileType type, const QStringList & filters );

		int size() const;

	private :
		static FileType fileTypeFromString( const QString & string );
		static const QString & fileTypeString( FileType type );		
		static const QString & typeName();

		static const bool Registered_;

	private :
		void addDirectory( const QDir & dir ) const;
		void addDirectoryPath( const QString & dir ) const;

		void addFile( const QFileInfo & file, FileType type = Unknown ) const;
		void addFilePath( const QString & file, FileType type = Unknown ) const;

		QMultiHash< QString, QString > parameters() const;
		void parseNameFilters( const QString & value );
		Dicom::Dataset readDataset( int num ) const;
		void setParameter( const QString & name, const QString & value );

	private :
		QHash< FileType, QFileInfoList > & files() const;
		mutable QHash< FileType, QFileInfoList > files_;

		QHash< FileType, QStringList > & nameFilters();
		QStringList nameFilters( FileType type );
		QHash< FileType, QStringList > nameFilters_;

		QStringList & paths();
		QStringList paths_;
};

}; // Namespace DICOM ends here.

inline uint qHash( const QFileInfo & File ) {
	return qHash( File.absoluteFilePath() );
}

#endif
