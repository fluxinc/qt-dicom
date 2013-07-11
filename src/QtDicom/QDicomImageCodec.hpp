/***************************************************************************
 *   Copyright © 2012 by Flux Inc.                                         *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#ifndef QTDICOM_QDICOMIMAGECODEC_HPP
#define QTDICOM_QDICOMIMAGECODEC_HPP

#include <QtCore/QHash>

#include <QtDicom/Globals.hpp>

class DcmRepresentationParameter;
class QTransferSyntax;


class QDICOM_DLLSPEC QDicomImageCodec {
	friend void globalCleanup();
	friend void globalInit();

	public :
		static QDicomImageCodec forTransferSyntax( 
			const QTransferSyntax & TS
		);
		static QList< QTransferSyntax > supportedTransferSyntaxes();

	public :
		enum Feature {
			None = 0,
			Quality = 1
		};
		Q_DECLARE_FLAGS( Features, Feature );

	public :
		QDicomImageCodec( const QDicomImageCodec & codec );
		~QDicomImageCodec();
		QDicomImageCodec & operator = ( const QDicomImageCodec & );

		/**
		 * Returns DCMTK's structure responsible for holding codec parameters.
		 *
		 * \note This method must be called only on valid codecs.
		 */
		const DcmRepresentationParameter & dcmParameters() const;

		bool hasFeature( Feature f ) const;

		bool isNull() const;
		bool isValid() const;

		int quality() const;
		void setQuality( int value );

	private :
		enum ParametersFamily {
			Unknown = 0,
			JpegLossless,
			JpegLossy,
			JpegLsLossless,
			JpegLsLossy,
			RleLossless
		};

	private :
		static bool addCodec( 
			const QTransferSyntax & , const QDicomImageCodec &
		);
		static void initRegister();
		static void cleanupRegister();

	private :
		QDicomImageCodec();
		QDicomImageCodec(
			ParametersFamily family, DcmRepresentationParameter * parameters,
			Features features = None
		);
		void clear();

	private :
		static QHash< QTransferSyntax, QDicomImageCodec > codecRegister_;

	private :
		const ParametersFamily & family() const;
		ParametersFamily family_;

		const Features & features() const;
		Features features_;

		const DcmRepresentationParameter & parameters() const;
		DcmRepresentationParameter * parameters_;
};

#endif
