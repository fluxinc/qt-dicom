/***************************************************************************
 *   Copyright © 2012 by Flux Inc.                                         *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#include "QDicomImageCodec.hpp"
#include "QTransferSyntax.hpp"

#include <dcmtk/dcmdata/dcrledrg.h> /* DcmRLEDecoderRegistration::registerCodecs */
#include <dcmtk/dcmdata/dcrleerg.h> /* DcmRLEEncoderRegistration::registerCodecs */
#include <dcmtk/dcmdata/dcrlerp.h>  /* DcmRLERepresentationParameter */

#include <dcmtk/dcmjpeg/djencode.h> /* DJDecoderRegistration::registerCodecs */
#include <dcmtk/dcmjpeg/djdecode.h> /* DJEncoderRegistration::registerCodecs */
#include <dcmtk/dcmjpeg/djrplol.h>  /* DJ_RPLossless */
#include <dcmtk/dcmjpeg/djrploss.h> /* DJ_RPLossy */

#include <dcmtk/dcmjpls/djdecode.h> /* DJLSDecoderRegistration::registerCodecs */
#include <dcmtk/dcmjpls/djencode.h> /* DJLSEncoderRegistration::registerCodecs */
#include <dcmtk/dcmjpls/djrparam.h> /* DJLSRepresentationParameter */


QHash< QTransferSyntax, QDicomImageCodec * > QDicomImageCodec::codecRegister_;


QDicomImageCodec::QDicomImageCodec( DcmRepresentationParameter * param ) :
	dcmParameters_( param )
{
}


QDicomImageCodec::~QDicomImageCodec() {
}


bool QDicomImageCodec::addCodec(
	const QTransferSyntax & Ts,	QDicomImageCodec * codec
) {
	Q_ASSERT( Ts.isValid() );
	Q_ASSERT( ! codecRegister_.contains( Ts ) ); 

	codecRegister_.insert( Ts, codec );
	return true;
}


QDicomImageCodec * QDicomImageCodec::codecForTransferSyntax(
	const QTransferSyntax & Ts
) {
	static QDicomImageCodec dummy;

	return codecRegister_.value( Ts, &dummy );
}


void QDicomImageCodec::cleanup() {
	for (
		QHash< QTransferSyntax, QDicomImageCodec * >::iterator i = codecRegister_.begin();
		i != codecRegister_.end(); ++i
	) {
		if ( i.value() ) {
			if ( i.value()->dcmParameters() ) {
				delete i.value()->dcmParameters_;
				i.value()->dcmParameters_ = NULL;
			}
			delete i.value();
			i.value() = NULL;
		}
	}

	codecRegister_.clear();

	
	DcmRLEEncoderRegistration::cleanup();
	DcmRLEDecoderRegistration::cleanup();

	DJLSEncoderRegistration::cleanup();
	DJLSDecoderRegistration::cleanup();

	DJEncoderRegistration::cleanup();
	DJDecoderRegistration::cleanup();
}


DcmRepresentationParameter * QDicomImageCodec::dcmParameters() {
	return dcmParameters_;
}


void QDicomImageCodec::init() {
	DJDecoderRegistration::registerCodecs(
		EDC_always, // color conversion
		EUC_never // Never change UID
	);
	DJEncoderRegistration::registerCodecs(
		ECC_lossyRGB, // Color space conversion
		EUC_never     // Never change UID
	);

	addCodec( QTransferSyntax::JpegProcess1, 
		new QDicomImageCodec( new DJ_RPLossy )
	);
	addCodec( QTransferSyntax::JpegProcess2_4, 
		new QDicomImageCodec( new DJ_RPLossy )
	);
	addCodec( QTransferSyntax::JpegProcess6_8, 
		new QDicomImageCodec( new DJ_RPLossy )
	);
	addCodec( QTransferSyntax::JpegProcess10_12, 
		new QDicomImageCodec( new DJ_RPLossy )
	);

	addCodec( QTransferSyntax::JpegProcess14,
		new QDicomImageCodec( new DJ_RPLossless )
	);
	addCodec( QTransferSyntax::JpegProcess14Sv1,
		new QDicomImageCodec( new DJ_RPLossless )
	);


	DJLSDecoderRegistration::registerCodecs(
		EJLSUC_never // Do not change SOP instance
	);
	DJLSEncoderRegistration::registerCodecs(
		OFFalse, // Default arguments
		3, 7, 21,
		64, 0,
		OFTrue, 0, OFTrue,
		EJLSUC_never // Do not change SOP instance
	);

	addCodec( QTransferSyntax::JpegLsLossless,
		new QDicomImageCodec( new DJLSRepresentationParameter( 2, true ) )
	);
	addCodec( QTransferSyntax::JpegLsLossy,
		new QDicomImageCodec( new DJLSRepresentationParameter( 2, false ) )
	);

	DcmRLEDecoderRegistration::registerCodecs( false );
	DcmRLEEncoderRegistration::registerCodecs( false );

	addCodec( QTransferSyntax::Rle,
		new QDicomImageCodec( new DcmRLERepresentationParameter )
	);
}


QList< QTransferSyntax > QDicomImageCodec::supported() {
	return codecRegister_.keys();
}
