/***************************************************************************
 *   Copyright © 2012 by Flux Inc.                                         *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#ifndef QTDICOM_QTRANSFERSYTNAX_HPP
#define QTDICOM_QTRANSFERSYTNAX_HPP

#include <QtCore/QByteArray>
#include <QtCore/QString>

#include <QtDicom/Globals.hpp>

class QUid;

/**
 * The \em QTransferSyntax class allows to identify DICOM Transfer Syntax used
 * during association negotiation.
 *
 * \author Paweł Żak <pawel.zak@fluxinc.ca>
 */
class QDICOM_DLLSPEC QTransferSyntax {
	friend uint qHash( const QTransferSyntax & );

	public :
		enum Id {
			Unknown = 0,

			LittleEndianImplicit, /*< 
			  Implicit VR Little Endian: Default Transfer Syntax for DICOM */

			LittleEndian,         /*< Explicit VR Little Endian */
			LittleEndianDeflated, /*< Deflated Explicit VR Little Endian */
			BigEndian,            /*< Explicit VR Big Endian */

			JpegProcess1, /*<
			  JPEG Baseline (Process 1): Default Transfer Syntax for Lossy JPEG 8 Bit
			  Image Compression */
			JpegProcess2_4, /*<
			  JPEG Extended (Process 2 & 4): Default Transfer Syntax for Lossy JPEG 12 Bit
			  Image Compression (Process 4 only) */
			JpegProcess3_5, /*<
			  JPEG Extended (Process 3 & 5) (Retired) */
			JpegProcess6_8, /*<
			  JPEG Spectral Selection, Non-Hierarchical (Process 6 & 8) (Retired) */
			JpegProcess7_9, /*<
			  JPEG Spectral Selection, Non-Hierarchical (Process 7 & 9) (Retired) */
			JpegProcess10_12, /*<
			  JPEG Full Progression, Non-Hierarchical (Process 10 & 12) (Retired) */
			JpegProcess11_13, /*<
			  JPEG Full Progression, Non-Hierarchical (Process 11 & 13) (Retired) */
			JpegProcess14, /*<
			  JPEG Lossless, Non-Hierarchical (Process 14) */
			JpegProcess15, /*<
			  JPEG Lossless, Non-Hierarchical (Process 15) (Retired) */
			JpegProcess16_18, /*<
			  JPEG Extended, Hierarchical (Process 16 & 18) (Retired) */
			JpegProcess17_19, /*<
			  JPEG Extended, Hierarchical (Process 17 & 19) (Retired) */
			JpegProcess20_22, /*<
			  JPEG Spectral Selection, Hierarchical (Process 20 & 22) (Retired) */
			JpegProcess21_23, /*<
			  JPEG Spectral Selection, Hierarchical (Process 21 & 23) (Retired) */
			JpegProcess24_26, /*<
			  JPEG Full Progression, Hierarchical (Process 24 & 26) (Retired) */
			JpegProcess25_27, /*<
			  JPEG Full Progression, Hierarchical (Process 25 & 27) (Retired) */
			JpegProcess28, /*<
			  JPEG Lossless, Hierarchical (Process 28) (Retired) */
			JpegProcess29, /*<
			  JPEG Lossless, Hierarchical (Process 29) (Retired) */
			JpegProcess14Sv1, /*<
			  JPEG Lossless, Non-Hierarchical, First-Order Prediction (Process 14
			  [Selection Value 1]): Default Transfer Syntax for Lossless JPEG Image
			  Compression */

			JpegLsLossless, /*<
			  JPEG-LS Lossless Image Compression */
			JpegLsLossy, /*<
			  JPEG-LS Lossy (Near-Lossless) Image Compression */

			Jpeg2000Lossless, /*<
			  JPEG 2000 Image Compression (Lossless Only) */
			Jpeg2000Lossy, /*<
			  JPEG 2000 Image Compression */
			Jpeg2000P2Lossless, /*<
			  JPEG 2000 Part 2 Multi-component Image Compression (Lossless Only) */
			Jpeg2000P2Lossy, /*<
			  JPEG 2000 Part 2 Multi-component Image Compression */

			Jpip,         /*< JPIP Referenced */
			JpipDeflated, /*< JPIP Referenced Deflate */

			Mpeg2Main, /*< MPEG2 Main Profile @ Main Level */
			Mpeg2High, /*< MPEG2 Main Profile @ High Level */

			Mpeg4,   /*< MPEG-4 AVC/H.264 High Profile / Level 4.1 */
			Mpeg4Bd, /*< MPEG-4 AVC/H.264 BD-compatible High Profile / Level 4.1 */

			Rle, /*< RLE Lossless */

			Mime, /*< RFC 2557 MIME encapsulation */

			Xml /*< XML Encoding */
		};

	public :		
		static QTransferSyntax fromName( const QString & name );
		static QTransferSyntax fromUid( const char * UID );

	public :
		QTransferSyntax( Id id = Unknown );
		~QTransferSyntax();

		operator int() const;

		bool operator == ( const QTransferSyntax & TS ) const;
		bool operator != ( const QTransferSyntax & TS ) const;

		bool isCompressed() const;
		bool isNull() const;
		bool isRetired() const;
		bool isValid() const;
		const char * name() const;
		int toInt() const;
		QString toString() const;
		const QUid & uid() const;


	private :
		static const char * name( Id id );
		static const QUid & uid( Id id );

	private :
		Id id_;
};

#endif
