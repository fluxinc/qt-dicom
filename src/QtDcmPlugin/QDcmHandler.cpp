/***************************************************************************
 *   Copyright © 2010-2014 by Flux Inc.                                    *
 *   Author: Pawel Zak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#include "Exceptions.hpp"
#include "QDcmHandler.hpp"

#include <sstream>

#include <QtCore/QDebug>
#include <QtCore/QtEndian>
#include <QtCore/QList>
#include <QtCore/QMutex>
#include <QtCore/QMutexLocker>
#include <QtCore/QSize>
#include <QtCore/QVector>

#include <QtGui/QImage>

#include <dcmtk/config/osconfig.h>

#include <dcmtk/dcmdata/dcdeftag.h>
#include <dcmtk/dcmdata/dcfilefo.h>
#include <dcmtk/dcmdata/dcistrmb.h>

#include <dcmtk/dcmimgle/dcmimage.h>

#include <dcmtk/dcmimage/diregist.h>

#include <dcmtk/ofstd/ofcond.h>
#include <dcmtk/ofstd/ofconfig.h>

#include <dcmtk/dcmdata/dcdict.h>


QDcmHandler::QDcmHandler( QIODevice * device, const QByteArray & format ) :
	currentFrame_( 0 ),
	dicomFile_( 0 )
{
	if ( ! dcmDataDict.isDictionaryLoaded() ) {
		qCritical( __FUNCTION__": data dictionary is not loaded" );

		return;
	}

	setDevice( device );
	setFormat( format );

	loadDicomFile();
}


QDcmHandler::~QDcmHandler() {
	if ( dicomFile_ ) {
		delete dicomFile_;
		dicomFile_ = 0;
	}
}


bool QDcmHandler::canRead() const {
	if ( device() && canRead( device() ) ) {
		setFormat( "dcm" );

		return true;
	}
	else {
		return false;
	}
}


bool QDcmHandler::canRead( QIODevice * device ) {
	Q_ASSERT( device );

	QByteArray prefix, preamble;
	if ( ! device->isSequential() ) {
		if ( device->seek( 128 ) ) {
			prefix = device->peek( 4 );
			if ( ! device->seek( 0 ) ) {
				qCritical( 
					__FUNCTION__": failed to seek back to 0 offset"
				);
				return false;
			}
		}
		else {
			qDebug(
				__FUNCTION__": failed to seek the 128 byte"
			);
			return false;
		}
	}
	else {
		preamble = device->peek( 128 );
		if ( preamble.size() != 128 ) {
			qDebug(
				__FUNCTION__": failed to peek 128 bytes"
			);
			return false;
		}
		prefix = device->peek( 4 );
	}

	static const QByteArray DicomPrefix = "DICM";

	return ( prefix == DicomPrefix );
}


int QDcmHandler::currentImageNumber() const {
	return currentFrame_;
}


inline DcmFileFormat & QDcmHandler::dicomFile() const {
	Q_ASSERT( dicomFile_ );
	Q_ASSERT( isDicomFileLoaded() );

	return *dicomFile_;
}


inline DcmDataset & QDcmHandler::dicomFileDataset() const {
	Q_ASSERT( isDicomFileLoaded() );

	return *dicomFile().getDataset();
}


const QVector< QRgb > & QDcmHandler::grayscaleColorTable() {
	static QVector< QRgb > colorTable;

	if ( colorTable.isEmpty() ) {
		colorTable.reserve( 256 );
		colorTable.resize( 256 );
		for ( int i = 0; i < 256; ++i ) {
			colorTable[ i ] = qRgb( i, i, i );
		}
	}

	return colorTable;
}


int QDcmHandler::imageCount() const {
	static const int ErrorValue = 0;
	static const int DefaultValue = 1;

	if ( ! isDicomFileLoaded() ) {
		return ErrorValue;
	}

	static const QString CacheKey = "ImageCount";
	if ( cache_.contains( CacheKey ) ) {
		Q_ASSERT( cache_.object( CacheKey ) );
		
#ifndef _DEBUG
		return cache_.object( CacheKey )->toInt();
#else 
		bool ok;
		const int TheCount = cache_.object( CacheKey )->toInt( &ok );
		Q_ASSERT( ok );
		return TheCount;
#endif
	}

	int theCount = DefaultValue;

	const QString Value = tagValue( DCM_NumberOfFrames, true );
	if ( ! Value.isEmpty() ) {
		bool ok;
		theCount = Value.toInt( &ok );
		if ( ! ok ) {
			qWarning() <<
				QString( 
					__FUNCTION__": "
					"failed to extract integer value from the: `%1' string"
				)
				.arg( Value )
			;
			theCount = ErrorValue;
		}
	}

	cache_.insert( CacheKey, new QVariant( theCount ) );
	return theCount;
}


QSize QDcmHandler::imageResolution() const {
	Q_ASSERT( isDicomFileLoaded() );

	static const QString CacheKey = "ImageResolution";
	if ( cache_.contains( CacheKey ) ) {
		Q_ASSERT( cache_.object( CacheKey ) );

		return cache_.object( CacheKey )->toSize();
	}


	// It validates as non-conformant values as you can possibly imagine.
	static const QRegExp SpacingFormat( 
		"(\\d+(?:\\.\\d+)?)\\s*(?:mm)?\\s*(?:[\\\\/]s*(\\d+(?:\\.\\d+)?)\\s*(?:mm)?)?"
	);
	Q_ASSERT( SpacingFormat.isValid() );

	static const QList< DcmTagKey > PixelSpacingTags = QList< DcmTagKey >()
		<< DCM_PixelSpacing
		<< DCM_ImagerPixelSpacing
		<< DCM_NominalScannedPixelSpacing
	;

	QSize resolution( -1, -1 );
	for (
		QList< DcmTagKey >::const_iterator i = PixelSpacingTags.constBegin();
		i != PixelSpacingTags.constEnd(); ++i
	) {
		const QString Value = tagValue( *i, true );
		if ( ! Value.isEmpty() ) {
			if ( SpacingFormat.indexIn( Value ) > -1 ) {
				bool ok = true;

				resolution.rwidth() = 
					qRound( 1000.0 / SpacingFormat.cap( 1 ).toDouble( &ok ) )
				;
				Q_ASSERT( ok );

				// I have to store this one, as it can be missing.
				const QString SecondGroup = SpacingFormat.cap( 2 );
				resolution.rheight() = 
					SecondGroup.isEmpty() ?
					resolution.width() :
					qRound( 1000.0 / SecondGroup.toDouble( &ok ) )
				;
				Q_ASSERT( ok );
			}
			else {
				qWarning() <<
					QString( 
						__FUNCTION__": "
						"unable to parse pixel spacing value from the: `%1' string" 
					)
					.arg( Value )
				;
			}
		}
	}

	cache_.insert( CacheKey, new QVariant( resolution ) );

	return resolution;
}


QSize QDcmHandler::imageSize() const {
	static const QSize Default( -1, -1 );

	if ( ! isDicomFileLoaded() ) {
		return Default;
	}

	static const QString CacheKey = "ImageSize";

	if ( cache_.contains( CacheKey ) ) {
		Q_ASSERT( cache_.object( CacheKey ) );

		return cache_.object( CacheKey )->toSize();
	}

	QSize size( -1, -1 );

	bool ok = false;
	QString value = tagValue( DCM_Rows, false );
	if ( ! value.isEmpty() ) {
		size.setHeight( value.toInt( &ok ) );
		if ( ! ok ) {
			qWarning() <<
				QString( 
					__FUNCTION__": "
					"failed to extract integer value from the: `%1'"
				)
				.arg( value )
			;
			size = Default;
		}
	}
	else {
		size = Default;
	}

	value = tagValue( DCM_Columns, false );
	if ( ok && ( ! value.isEmpty() ) ) {
		bool ok;
		size.setWidth( value.toInt( &ok ) );
		if ( ! ok ) {
			qWarning() <<
				QString( 
					__FUNCTION__": "
					"failed to extract integer value from the: `%1'"
				)
				.arg( value )
			;
			size = Default;
		}
	}
	else {
		size = Default;
	}

	cache_.insert( CacheKey, new QVariant( size ) );
	return size;
}


inline bool QDcmHandler::isDicomFileLoaded() const {
	return ( dicomFile_ != 0 );
}


bool QDcmHandler::jumpToImage( int Frame ) {
	if ( isDicomFileLoaded() && Frame < imageCount() ) {
		currentFrame_ = Frame;

		return true;
	}
	else {
		return false;
	}
}


bool QDcmHandler::jumpToNextImage() {
	if ( isDicomFileLoaded() && ( currentFrame_ < imageCount() - 1 ) ) {
		++currentFrame_;
		
		return true;
	}
	else {
		return false;
	}
}


void QDcmHandler::loadDicomFile() {
	// Start try block.
	try {

	const QByteArray Bytes = device()->readAll();

	DcmInputBufferStream in;
	in.setBuffer( Bytes, Bytes.size() );
	in.setEos();
	OFCondition result = in.status();

	if ( result.bad() ) {
		throw OperationFailedException(
			QString(
				__FUNCTION__": "
				"failed to set input buffer stream with the "
				"contents of the device: `%1'"
			)
			.arg( result.text() )
		);
	}

	dicomFile_ = new DcmFileFormat();
	result = dicomFile_->read( in );

	if ( result.bad() ) {
		throw OperationFailedException(
			QString(
				__FUNCTION__": "
				"failed to load DICOM file from the input buffer. Reason: `%1'"
			)
			.arg( result.text() )
		);
	}

	result = dicomFile_->chooseRepresentation( EXS_LittleEndianExplicit, NULL );
	if ( result.bad() ) {
		throw OperationFailedException(
			QString(
				__FUNCTION__": "
				"failed to choose Little Endian Implicit representation. "
				"Reason: `%1'"
			)
			.arg( result.text() )
		);
	}

	setFormat( "dcm" );
	return;

	// End of try block.
	} 
	catch ( OperationFailedException & e ) {
		qCritical() << e.what();
	}
	catch ( std::exception & e ) {
		// Although we throw only OperationFailedException, we can catch also 
		// the others, originated by the DCMTK.
		qCritical() << "Unknown exception caught in "__FUNCTION__": " << e.what();
	}

	if ( dicomFile_ ) {
		delete dicomFile_;
		dicomFile_ = 0;
	}
}


int QDcmHandler::loopCount() const {
	return 1;
}


inline const QString & QDcmHandler::multipleValuesSeparator() {
	static const QString TheSeparator = '\\';

	return TheSeparator;
}


int QDcmHandler::nextImageDelay() const {
	// Set a second default.
	static const int DefaultValue = 1000;
	static const int ErrorValue = 0;

	if ( ! isDicomFileLoaded() ) {
		return ErrorValue;
	}

	static const QString CacheKey = "NextImageDelay";
	if ( cache_.contains( CacheKey ) ) {
		Q_ASSERT( cache_.object( CacheKey ) );
#ifndef _DEBUG
		return cache_.object( CacheKey )->toInt();
#else
		bool ok;
		const int TheDelay = cache_.object( CacheKey )->toInt( &ok );
		Q_ASSERT( ok );
		return TheDelay;
#endif
	}

	int theDelay = DefaultValue;

	const QString Value = tagValue( DCM_FrameTime, true );
	if ( ! Value.isEmpty() ) {
		bool ok;
		theDelay = Value.toInt( &ok );
		if ( ! ok ) {
			qWarning() <<
				QString( 
					__FUNCTION__": "
					"Failed to extract integer value from the: `%1'"
				)
				.arg( Value )
			;
			theDelay = ErrorValue;
		}
	}

	cache_.insert( CacheKey, new QVariant( theDelay ) );
	return theDelay;
}


QVariant QDcmHandler::option( QImageIOHandler::ImageOption option ) const {
	if ( ! isDicomFileLoaded() ) {
		return QVariant();
	}

	switch ( option ) {
		case QImageIOHandler::Size :
			return imageSize();
		case QImageIOHandler::Description : {
			static const QString CacheKey = "ImageText";

			if ( cache_.contains( CacheKey ) ) {
				return cache_.object( CacheKey )->toString();
			}
			else {
				const QHash< QString, QVariant > TagsValues = tagsValues();
				QString tmp;
				for (
					QHash< QString, QVariant >::const_iterator i 
						= TagsValues.constBegin()
					;
					i != TagsValues.constEnd(); ++i
				) {
					tmp += 
						QString( "%1: %2\n\n" )
						.arg( i.key() )
						.arg( i.value().toStringList().join( 
							multipleValuesSeparator()
						) )
					;
				}

				cache_.insert( CacheKey, new QVariant( tmp ) );

				return tmp;
			}
		}
		case QImageIOHandler::Animation :
			return true;
		default :
			return QVariant();
	}

	return QVariant();
}


bool QDcmHandler::read( QImage * image ) {
	if ( ! isDicomFileLoaded() ) {
		return false;
	}

	const QImage Frame = readFrame( currentFrame_ );
	if ( ! Frame.isNull() ) {
		*image = Frame;

		return true;
	}	
	else {
		return false;
	}
}


QImage QDcmHandler::readFrame( int FrameNumber ) {
	if ( ! isDicomFileLoaded() ) {
		return QImage();
	}

	Q_ASSERT( FrameNumber < imageCount() );

	if ( frames_.contains( FrameNumber ) ) {
		Q_ASSERT( frames_.object( FrameNumber ) );
		const QImage Frame = *frames_.object( FrameNumber );
		Q_ASSERT( ! Frame.isNull() );

		return Frame;
	}

	DicomImage * dicomImage = 0;

	// Start try block.
	try {

	static const unsigned long Flags = 
		CIF_MayDetachPixelData | 
		CIF_UsePartialAccessToPixelData |
		CIF_AcrNemaCompatibility
	;

	const E_TransferSyntax TransferSyntax = dicomFileDataset().getOriginalXfer();
	dicomImage = new DicomImage( 
		& dicomFile(), TransferSyntax, Flags, FrameNumber, 1
	);

	if ( dicomImage->getStatus() != EIS_Normal ) {
		throw OperationFailedException(
			QString(
				__FUNCTION__": "
				"failed to create DICOM image object. Status: `%1'"
			)
			.arg( DicomImage::getString( dicomImage->getStatus() ) )
		);
	}

	const int Height = dicomImage->getHeight();
	const int Width = dicomImage->getWidth();

	Q_ASSERT( QSize( Width, Height ) == imageSize() );

	// Disable active VOI LUT and use generic MinMax.
	dicomImage->setMinMaxWindow( 0 );

	const unsigned char * Data = static_cast< const unsigned char * >(
		dicomImage->getOutputData( 8, 0 )
	);

	if ( ! Data ) {
		throw OperationFailedException( 
			__FUNCTION__": "
			"NULL pixel data retrieved"
		);
	}

	QImage image;
	if ( dicomImage->isMonochrome() ) {
		QImage tmp( Width, Height, QImage::Format_Indexed8 );

		for ( int i = 0; i < Height; ++i ) {
			unsigned char * line = tmp.scanLine( i );
			for ( int j = 0; j < Width; ++j ) {
				line[ j ] = Data[ i * Width + j ];
			}
		}

		tmp.setColorCount( grayscaleColorTable().size() );
		tmp.setColorTable( grayscaleColorTable() );

		image = tmp;
	}
	else {
		QImage tmp( Width, Height, QImage::Format_ARGB32 );

		for ( int i = 0; i < Height; ++i ) {
			QRgb * line = reinterpret_cast< QRgb * >( tmp.scanLine( i ) );
			for ( int j = 0; j < Width; ++j ) {
				const int Offset = ( i * Width + j ) * 3;
				line[ j ] = qRgb( 
					Data[ Offset     ],
					Data[ Offset + 1 ],
					Data[ Offset + 2 ]
				);
			}
		}

		image = tmp;
	}

	dicomImage->deleteOutputData();
	delete dicomImage;

	const bool IsValid = ! image.isNull();

	if ( IsValid ) {
		const QSize Resolution = imageResolution();
		if ( Resolution.isValid() ) {
			image.setDotsPerMeterX( Resolution.width() );
			image.setDotsPerMeterY( Resolution.height() );
		}

		const QHash< QString, QVariant > TagsValues = tagsValues();
		for ( 
			QHash< QString, QVariant >::const_iterator i = TagsValues.constBegin();
			i != TagsValues.constEnd(); ++i
		) {
			image.setText( 
				i.key(), 
				i.value().toStringList().join( multipleValuesSeparator() )
			);
		}

		frames_.insert( FrameNumber, new QImage( image ) );

		return image;
	}
	else {
		throw OperationFailedException( 
			__FUNCTION__": "
			"failed to create valid QImage from retrieved pixel data"
		);
	}

	// Try block ends here.
	}
	catch ( OperationFailedException & e ) {
		qCritical( e.what() );
	}
	catch ( std::exception & e ) {
		qCritical()
			<< "Unknown exception caught in "__FUNCTION__": " << e.what()
		;
	}

	if ( dicomImage ) {
		delete dicomImage;
		dicomImage = 0;
	}

	// We'll get here only if exception occured so return null image.
	frames_.insert( FrameNumber, new QImage() );
	return QImage();
}


bool QDcmHandler::supportsOption( QImageIOHandler::ImageOption option ) const {
	switch ( option ) {
		case QImageIOHandler::Size :
		case QImageIOHandler::Description :
		case QImageIOHandler::Animation :
			return true;
		default :
			return false;
	}
	
}


QString QDcmHandler::tagValue(
	const DcmTagKey & Tag, bool Suppress
) const {
	static QMutex serializer;
	QMutexLocker locker( &serializer );

	QString value;
	if ( isDicomFileLoaded() ) {
		OFString tmp;
		OFCondition result = 
			dicomFileDataset().findAndGetOFStringArray( Tag, tmp, true )
		;
		if ( result.good() ) {
			value = QString::fromLocal8Bit( tmp.c_str() );
		}
		else {
			DcmTag t( Tag );
			
			if ( ! Suppress ) {
				qDebug() <<
					QString(
						__FUNCTION__": "
						"failed to read %1 (%2,%3) tag value. Reason: `%4'"
					)
					.arg( t.getTagName() )
					.arg( t.getGroup(), 4, 16, QChar( '0' ) )
					.arg( t.getElement(), 4, 16, QChar( '0' ) )
					.arg( result.text() )
				;
			}
		}
	}

	return value;
}


QHash< QString, QVariant > QDcmHandler::tagsValues() const {
	static const QString CacheKey = "TagsValues";

	if ( cache_.contains( CacheKey ) ) {
		const QHash< QString, QVariant > Tags = 
			cache_.object( CacheKey )->toHash()
		;

		return Tags;
	}

	QHash< QString, QVariant > tagsRead;
	DcmStack stack;
	OFCondition status = dicomFile().nextObject( stack, OFTrue );

	while ( status.good() ) {
		DcmObject * object = stack.top();

		const DcmEVR TagVr = object->ident();

		const bool IsDisplayable =
			object->isLeaf() && 
			TagVr != EVR_OB &&
			TagVr != EVR_OF &&
			TagVr != EVR_OW &&
			TagVr != EVR_PixelData &&
			TagVr != EVR_pixelItem &&
			TagVr != EVR_na &&
			TagVr != EVR_xs
		;

		if ( IsDisplayable ) {
			DcmElement * currentElement = ( DcmElement * )object;
			DcmTag Tag( currentElement->getTag() );
			const QString TagDescriptor =
				QString( "(%1,%2)" )
				.arg( Tag.getGroup(),   4, 16, QChar( '0' ) )
				.arg( Tag.getElement(), 4, 16, QChar( '0' ) )
			;
			const QString TagName = Tag.getTagName();

			OFString value;
			OFCondition result = currentElement->getOFStringArray( value );
			if ( result.bad() ) {
				qCritical() <<
					QString( 
						__FUNCTION__": "
						"failed to read tag: `%1' %2 value. Reason: `%3'"
					)
					.arg( TagName ).arg( TagDescriptor ).arg( result.text() )
				;

				status = dicomFile().nextObject( stack, OFTrue );
				continue;
			}
			
			const QStringList TagValues = 
				QString( value.c_str() )
				.remove( '\n' )
				.remove( '\r' )
				.split( multipleValuesSeparator() )
			;

			QString prefix;
			{ 
				// Generate prefix from sequence elements below the current one 
				// on the stack.
				const unsigned long Size = stack.card();
				for ( unsigned long i = 1; i < Size; ++i ) {
					DcmObject * o = stack.elem( i );
					if ( o->ident() == EVR_SQ ) {
						const QString TagDescriptor =
							QString( "(%1,%2)" )
							.arg( o->getGTag(), 4, 16, QChar( '0' ) )
							.arg( o->getETag(), 4, 16, QChar( '0' ) )
						;
						prefix.prepend( TagDescriptor );
					}
				}
			}

			const QString ImageTextKey =
				QString( "%1%2" )
				.arg( prefix ).arg( TagDescriptor )
			;

			if ( ! tagsRead.contains( ImageTextKey ) ) {
				tagsRead.insert( ImageTextKey, QVariant( TagValues ) );
			}
			else {
				qWarning() <<
					QString(
						__FUNCTION__": "
						"tag: `%1' %2 was read already. "
						"Value(s) read already: `%3'. New value(s): `%4'"
					)
					.arg( TagName ).arg( TagDescriptor )
					.arg( tagsRead.value( ImageTextKey ).toStringList().join( "', " ) )
					.arg( TagValues.join( ", " ) )
				;
			}
		}

		status = dicomFile().nextObject( stack, OFTrue );
	}


	cache_.insert( CacheKey, new QVariant( tagsRead ) );

	return tagsRead;
}