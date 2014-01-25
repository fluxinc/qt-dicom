/***************************************************************************
 *   Copyright © 2010-2014 by Flux Inc.                                    *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#ifndef QDCMTEST_HPP
#define QDCMTEST_HPP

#include <QtCore/QFileInfo>
#include <QtCore/QList>
#include <QtCore/QObject>
#include <QtCore/QSize>
#include <QtCore/QString>
#include <QtCore/QStringList>

class QImage;
class QStringList;

class QDcmPluginTest : public QObject {
	Q_OBJECT;

	public :
		QDcmPluginTest();
		~QDcmPluginTest();

	private :		
		struct DcmSampleProperties {
			QFileInfo File;
			QString TransferSyntax;
			QString Modality;
			QString ColorMode;
			unsigned BitDepth;
			QSize Size;
			unsigned FramesCount;
			QString Version;
		};

		QList< DcmSampleProperties > samples_;

	private :
		static const QStringList & dcmSampleFilesNames();
		bool displayImage( const QImage & image ) const;
		bool loadDcmSampleFilesProperites();
		QString imageText( const QImage & image ) const;

		/**
		 * Performs a test on the \a sample. This include:
		 * - checking format of the file,
		 * - validating number of frames,
		 * - for each frame:
		 *   - comparing reported and original pixel data dimensions,
		 *   - makeing sure image format matches colot mode.
		 *
		 * When the \a strict mode is disabled, method will only raise an error
		 * if file couldn't be loaded.
		 */
		void testSample( const DcmSampleProperties & sample, bool strict = true );
		

	private slots :
		void cleanupTestCase();

		void t01_check_if_dcm_fromat_is_available_for_reading();
		void t02_test_explicit_transfer_syntaxes();
		void t03_test_implicit_transfer_syntaxes();
		void t04_test_compressed_transfer_syntaxes();
		void t05_test_non_conformant_files();
};

#endif
