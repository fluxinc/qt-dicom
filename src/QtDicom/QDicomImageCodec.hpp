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
	public :
		static void cleanup();
		static QDicomImageCodec * codecForTransferSyntax( 
			const QTransferSyntax & TS
		);
		static void init();
		static QList< QTransferSyntax > supported();

	public :
		~QDicomImageCodec();

		/**
		 * Returns DCMTK's structure responsible for holding codec parameters.
		 */
		DcmRepresentationParameter * dcmParameters();

	private :
		static bool addCodec( 
			const QTransferSyntax & , QDicomImageCodec *
		);

	private :
		QDicomImageCodec( DcmRepresentationParameter * dcmParameters = NULL );

	private :
		static QHash< QTransferSyntax, QDicomImageCodec * > codecRegister_;

	private :
		DcmRepresentationParameter * dcmParameters_;
};

#endif
