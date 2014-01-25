/***************************************************************************
 *   Copyright © 2010-2014 by Flux Inc.                                    *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#include "QDcmPluginTest.hpp"

#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QRegExp>
#include <QtCore/QStringList>
#include <QtCore/QTemporaryFile>

#include <QtGui/QDialogButtonBox>
#include <QtGui/QImage>
#include <QtGui/QImageReader>
#include <QtGui/QGraphicsView>
#include <QtGui/QHBoxLayout>
#include <QtGui/QMessageBox>
#include <QtGui/QPushButton>
#include <QtGui/QTextEdit>
#include <QtGui/QVBoxLayout>

#include <QtTest/QTest>

QDcmPluginTest::QDcmPluginTest() {
	loadDcmSampleFilesProperites();
}


QDcmPluginTest::~QDcmPluginTest() {
}


void QDcmPluginTest::cleanupTestCase() {
	qDebug( "Cleaning up." );
}


bool QDcmPluginTest::displayImage( const QImage & Image ) const {
	QDialog * dialog = new QDialog( 0, Qt::WindowTitleHint | Qt::WindowSystemMenuHint );
	connect( 
		dialog, SIGNAL( finished( int ) ), 
		dialog, SLOT( deleteLater() )
	);

	static const unsigned HOffset = 40;
	static const unsigned VOffset = 60;
	static const unsigned TextViewWidth = 320;

	dialog->setWindowModality( Qt::ApplicationModal );
	dialog->resize( Image.width() + TextViewWidth + HOffset, Image.height() + VOffset );

	QGraphicsScene * scene = new QGraphicsScene( dialog );
	scene->addPixmap( QPixmap::fromImage( Image ) );
	
	QGraphicsView * view = new QGraphicsView( scene, dialog );
	view->resize( Image.width(), Image.height() );

	QTextEdit * edit = new QTextEdit( dialog );
	edit->setFontFamily( "Courier New" );
	edit->setReadOnly( true );
	edit->setLineWrapMode( QTextEdit::NoWrap );
	edit->resize( TextViewWidth, Image.height() );
	edit->setPlainText( imageText( Image ) );	

	QPushButton * acceptButton = new QPushButton( "Accept" );
	QPushButton * rejectButton = new QPushButton( "Reject" );

	QDialogButtonBox * buttons = new QDialogButtonBox( Qt::Horizontal, dialog );
	buttons->addButton( acceptButton, QDialogButtonBox::AcceptRole );
	buttons->addButton( rejectButton, QDialogButtonBox::RejectRole );
	connect( buttons, SIGNAL( accepted() ), dialog, SLOT( accept() ) );
	connect( buttons, SIGNAL( rejected() ), dialog, SLOT( reject() ) );

	QHBoxLayout * h = new QHBoxLayout();
	h->addWidget( view );
	h->addWidget( edit );

	QVBoxLayout * layout = new QVBoxLayout( dialog );
	layout->addLayout( h );
	layout->addWidget( buttons );
	
	dialog->setLayout( layout );

	dialog->show();
	return ( dialog->exec() == QDialog::Accepted );
}

bool QDcmPluginTest::loadDcmSampleFilesProperites() {
	if ( ! samples_.isEmpty() ) {
		return true;
	}

	const QStringList & Names = dcmSampleFilesNames();

	for (
		QStringList::const_iterator i = Names.constBegin();
		i != Names.constEnd(); ++i
	) {
		qDebug() << "Loading:" << *i;

		const QFileInfo SampleFile( *i );
		Q_ASSERT( SampleFile.exists() );

		static const QRegExp NameFormat(
			//         1               2             3    
			//   TransferSyntax   -Modality-     Colormode       -
			"(\\w+(?:_[\\w\\d]+)?)-(\\w{2})-(\\w+(?:_[\\w\\d]+)?)-"
			//    4           5 6             7                 8
			//  Bits   -     Size         (Frames)      -  (Version)
			"(\\d{1,2})-(\\d+)x(\\d+)(?:x(\\d{1,2}))?(?:-([\\w\\d]+))?"
		);
		Q_ASSERT( NameFormat.isValid() );

		const bool Matches = 
			( NameFormat.indexIn( SampleFile.completeBaseName() ) > -1 )
		;
		Q_ASSERT( Matches );

		DcmSampleProperties p;
		p.File = SampleFile;

		p.TransferSyntax = NameFormat.cap( 1 );
		Q_ASSERT( ! p.TransferSyntax.isEmpty() );

		p.Modality = NameFormat.cap( 2 );
		Q_ASSERT( ! p.Modality.isEmpty() );

		p.ColorMode = NameFormat.cap( 3 );
		Q_ASSERT( ! p.ColorMode.isEmpty() );

		bool ok = true;
		p.BitDepth = NameFormat.cap( 4 ).toUInt( &ok );
		Q_ASSERT( ok );

		p.Size = QSize( 
			NameFormat.cap( 5 ).toUInt( &ok ),
			NameFormat.cap( 6 ).toUInt( &ok )
		);
		Q_ASSERT( ok );

		const QString FramesCountStr = NameFormat.cap( 7 );
		p.FramesCount = 
			FramesCountStr.isEmpty() ? 
			1 : FramesCountStr.toUInt( &ok )
		;
		Q_ASSERT( ok );

		const QString Version = NameFormat.cap( 8 );
		p.Version = Version.isEmpty() ? "DICOM" : Version;
		Q_ASSERT( ! p.Version.isEmpty() );

		samples_.append( p );
	}

	return true;
}


const QStringList & QDcmPluginTest::dcmSampleFilesNames() {
	static const QStringList TheNames = QStringList()
		<< ":/Samples/Explicit_Big-US-RGB_Plane-8-640x480.dcm"
		<< ":/Samples/Explicit_Little-CT-MONOCHROME_2-16-512x512.dcm"
		<< ":/Samples/Explicit_Little-MR-MONOCHROME_2-8-256x256x16.dcm"
		<< ":/Samples/Explicit_Little-NM-MONOCHROME_2-16-64x64x13.dcm"
		<< ":/Samples/Explicit_Little-US-MONOCHROME_2-8-128x120x8.dcm"
		<< ":/Samples/Explicit_Little-US-RGB-8-256x120.dcm"
		<< ":/Samples/Implicit_Little-CR-MONOCHROME_1-10-440x440-RAW.dcm"
		<< ":/Samples/Implicit_Little-CT-MONOCHROME_2-8-512x512.dcm"
		<< ":/Samples/Implicit_Little-CT-MONOCHROME_2-12-512x512-AN2.dcm"
		<< ":/Samples/Implicit_Little-CT-MONOCHROME_2-16-512x512.dcm"
		<< ":/Samples/Implicit_Little-MR-MONOCHROME_2-12-256x256-AN1.dcm"
		<< ":/Samples/Implicit_Little-MR-MONOCHROME_2-12-256x256-AN2.dcm"
		<< ":/Samples/Implicit_Little-MR-MONOCHROME_2-16-256x256.dcm"
		<< ":/Samples/Implicit_Little-OT-MONOCHROME_2-8-512x512-RAW.dcm"
		<< ":/Samples/Implicit_Little-OT-RGB_Palette-8-640x480-RAW.dcm"
		<< ":/Samples/JPEG_57-MR-MONOCHROME_2-12-1024x1024.dcm"
		<< ":/Samples/JPEG_70-CT-MONOCHROME_2-16-512x400.dcm"
		<< ":/Samples/JPEG_70-XA-MONOCHROME_2-8-512x512x12.dcm"
		<< ":/Samples/RLE_Lossless-US-RGB_Palette-8-600x430x10.dcm"
	;

	return TheNames;
}


QString QDcmPluginTest::imageText( const QImage & Image ) const {
	const QStringList Keys = Image.textKeys();
	QString text;
	for (
		QStringList::const_iterator i = Keys.constBegin();
		i != Keys.constEnd(); ++i
	) {
		text += QString( "%1: %2\n" ).arg( *i ).arg( Image.text( *i ) );
	}

	return text;
}


void QDcmPluginTest::t01_check_if_dcm_fromat_is_available_for_reading() {
	QVERIFY( QImageReader::supportedImageFormats().contains( "dcm" ) );
}


void QDcmPluginTest::t02_test_explicit_transfer_syntaxes() {
	Q_ASSERT( loadDcmSampleFilesProperites() );

	for (
		QList< DcmSampleProperties >::const_iterator i = samples_.constBegin();
		i != samples_.constEnd(); ++i 
	) {
		if ( 
			i->TransferSyntax.startsWith( "Explicit" ) &&
			i->Version == "DICOM"
		) {
			testSample( *i );
		}
	}
}


void QDcmPluginTest::t03_test_implicit_transfer_syntaxes() {
	Q_ASSERT( loadDcmSampleFilesProperites() );

	for (
		QList< DcmSampleProperties >::const_iterator i = samples_.constBegin();
		i != samples_.constEnd(); ++i 
	) {
		if ( 
			i->TransferSyntax.startsWith( "Implicit" ) &&
			i->Version == "DICOM"
		) {
			testSample( *i );
		}
	}
}


void QDcmPluginTest::t04_test_compressed_transfer_syntaxes() {
	Q_ASSERT( loadDcmSampleFilesProperites() );

	for (
		QList< DcmSampleProperties >::const_iterator i = samples_.constBegin();
		i != samples_.constEnd(); ++i 
	) {
		if ( 
			! i->TransferSyntax.contains( QRegExp( "(?:Implicit|Explicit)" ) ) &&
			i->Version == "DICOM"
		) {
			testSample( *i );
		}
	}
}


void QDcmPluginTest::t05_test_non_conformant_files() {
	Q_ASSERT( loadDcmSampleFilesProperites() );

	for (
		QList< DcmSampleProperties >::const_iterator i = samples_.constBegin();
		i != samples_.constEnd(); ++i 
	) {
		if ( i->Version != "DICOM" ) {
			testSample( *i, false );
		}
	}
}


void QDcmPluginTest::testSample( const QDcmPluginTest::DcmSampleProperties & P, bool strict ) {
	qDebug() <<
		QString( "Testing %1 file: `%2' in %3 mode." )
		.arg( P.Version )
		.arg( QDir::toNativeSeparators( P.File.absoluteFilePath() ) )
		.arg( strict ? "strict" : "normal" )
	;

	QImageReader r( P.File.absoluteFilePath() );

#define TEST( COND, MSG ) \
	if ( strict ) { \
		QVERIFY2( ( COND ), ( MSG.toLocal8Bit().constData() ) ); \
	} \
	else { \
		if ( ! ( COND ) ) { \
			qWarning() << MSG; \
		} \
	}

	const QString Format = r.format();
	TEST(
		Format == "dcm",
		QString( "Detected file format: `%1' differs from the original: `dcm'." )
		.arg( Format )
	);
	TEST( 
		r.supportsAnimation(),
		QString( "Detected file format: `%1' doesn't support animations." )
		.arg( Format )
	);

	const int FramesCount = r.imageCount();
	TEST( 
		FramesCount != 0,
		QString( "Frames count is 0. Handler couldn't read the number of frames." )
	);

	TEST(
		FramesCount == P.FramesCount,
		QString( "Number of frames don't match the original (%1 vs %2)." )
		.arg( FramesCount ).arg( P.FramesCount )
	);
	
	TEST( 
		r.size() == P.Size,
		QString( "Reported size doesn't match the original (%1x%2 vs %3x%4)." )
		.arg( r.size().width() ).arg( r.size().height() )
		.arg( P.Size.width() ).arg( P.Size.height() )
	);

	TEST(
		r.text( "(0008,0060)" ) == P.Modality,
		QString( "Image text contains invalid modality tag (%1 vs %2)." )
		.arg( r.text( "(0008,0060)" ) ).arg( P.Modality )
	);

	for ( int i = 0; i < FramesCount; ++i ) {
		QImage img;
		TEST(
			r.jumpToImage( i ),
			QString( "Failed to select %1 frame." )
			.arg( i + 1 )
		);
		TEST( 
			r.read( &img ),
			QString( "Failed to read frame %1." )
			.arg( i + 1 )
		);
		TEST(
			( P.ColorMode.startsWith( "MONOCHROME" ) 
				&& img.format() == QImage::Format_Indexed8 ) ||
			( P.ColorMode.startsWith( "RGB" ) 
				&& img.format() == QImage::Format_ARGB32 ),
			QString( 
				"Pixel data format don't match original data representation (%1 vs %2)."
			)
			.arg( img.format() ).arg( P.ColorMode )
		);
		TEST(
			displayImage( img ),
			QString( "Image doesn't look so good." )
		);
	}

#undef TEST
}
