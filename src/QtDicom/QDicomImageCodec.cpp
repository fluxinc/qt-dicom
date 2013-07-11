/***************************************************************************
 *   Copyright © 2012 by Flux Inc.                                         *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#include "QDicomImageCodec.hpp"
#include "QTransferSyntax.hpp"

#include <dcmtk/dcmdata/dcrledrg.h> /* DcmRLEDecoderRegistration::registerCodecs */
#include <dcmtk/dcmdata/dcrleerg.h> /* DcmRLEEncoderRegistration::registerCodecs */
#include <dcmtk/dcmdata/dcrlerp.h>  /* DcmRLERepresentationParameter */

#include <dcmtk/dcmimage/diregist.h> /* To add support for color images */

#include <dcmtk/dcmjpeg/djencode.h> /* DJDecoderRegistration::registerCodecs */
#include <dcmtk/dcmjpeg/djdecode.h> /* DJEncoderRegistration::registerCodecs */
#include <dcmtk/dcmjpeg/djrplol.h>  /* DJ_RPLossless */
#include <dcmtk/dcmjpeg/djrploss.h> /* DJ_RPLossy */

#include <dcmtk/dcmjpls/djdecode.h> /* DJLSDecoderRegistration::registerCodecs */
#include <dcmtk/dcmjpls/djencode.h> /* DJLSEncoderRegistration::registerCodecs */
#include <dcmtk/dcmjpls/djrparam.h> /* DJLSRepresentationParameter */


QHash< QTransferSyntax, QDicomImageCodec > QDicomImageCodec::codecRegister_;


QDicomImageCodec::QDicomImageCodec() :
	family_( Unknown ),
	features_( None ),
	parameters_( nullptr )
{
}


QDicomImageCodec::QDicomImageCodec(
	ParametersFamily family, DcmRepresentationParameter * parameters,
	Features features
) :
	family_( family ),
	features_( features ),
	parameters_( parameters )
{
	Q_ASSERT( parameters != nullptr );
}


QDicomImageCodec::QDicomImageCodec( const QDicomImageCodec & Other ) :
	family_( Other.family_ ),
	features_( Other.features_ ),
	parameters_( Other.isNull() ? nullptr : Other.parameters().clone() )
{
}


QDicomImageCodec & QDicomImageCodec::operator = ( const QDicomImageCodec & Other ) {
	if ( this != &Other ) {
		clear();

		family_ = Other.family_;
		features_ = Other.features_;
		parameters_ = Other.isNull() ? nullptr : Other.parameters().clone();
	}

	return *this;
}


QDicomImageCodec::~QDicomImageCodec() {
	clear();
}


bool QDicomImageCodec::addCodec(
	const QTransferSyntax & Ts,	const QDicomImageCodec & Codec
) {
	Q_ASSERT( Ts.isValid() );
	Q_ASSERT( ! codecRegister_.contains( Ts ) ); 

	codecRegister_.insert( Ts, Codec );
	return true;
}


void QDicomImageCodec::cleanupRegister() {
	codecRegister_.clear();
	
	DcmRLEEncoderRegistration::cleanup();
	DcmRLEDecoderRegistration::cleanup();

	DJLSEncoderRegistration::cleanup();
	DJLSDecoderRegistration::cleanup();

	DJEncoderRegistration::cleanup();
	DJDecoderRegistration::cleanup();
}


void QDicomImageCodec::clear() {
	family_ = Unknown;
	features_ = None;
	if ( parameters_ != nullptr ) {
		delete parameters_;
		parameters_ = nullptr;
	}
}


const DcmRepresentationParameter & QDicomImageCodec::dcmParameters() const {
	return parameters();
}


inline const QDicomImageCodec::ParametersFamily & QDicomImageCodec::family() const {
	return family_;
}


inline const QDicomImageCodec::Features & QDicomImageCodec::features() const {
	return features_;
}


QDicomImageCodec QDicomImageCodec::forTransferSyntax(
	const QTransferSyntax & Ts
) {
	static QDicomImageCodec dummy;

	return codecRegister_.value( Ts, dummy );
}


bool QDicomImageCodec::hasFeature( Feature f ) const {
	return features().testFlag( f );
}


void QDicomImageCodec::initRegister() {
	DJDecoderRegistration::registerCodecs(
		EDC_photometricInterpretation, // color conversion
		EUC_never // Never change UID
	);
	DJEncoderRegistration::registerCodecs(
		ECC_lossyYCbCr, // Color space conversion
		EUC_never     // Never change UID
	);

	addCodec( QTransferSyntax::JpegProcess1, 
		QDicomImageCodec( JpegLossy, new DJ_RPLossy, Quality )
	);
	addCodec( QTransferSyntax::JpegProcess2_4, 
		QDicomImageCodec( JpegLossy, new DJ_RPLossy, Quality )
	);
	addCodec( QTransferSyntax::JpegProcess6_8, 
		QDicomImageCodec( JpegLossy, new DJ_RPLossy, Quality )
	);
	addCodec( QTransferSyntax::JpegProcess10_12, 
		QDicomImageCodec( JpegLossy, new DJ_RPLossy, Quality )
	);

	addCodec( QTransferSyntax::JpegProcess14,
		QDicomImageCodec( JpegLossless, new DJ_RPLossless )
	);
	addCodec( QTransferSyntax::JpegProcess14Sv1,
		QDicomImageCodec( JpegLossless, new DJ_RPLossless )
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
		QDicomImageCodec( JpegLsLossless, new DJLSRepresentationParameter( 2, true ) )
	);
	addCodec( QTransferSyntax::JpegLsLossy,
		QDicomImageCodec( JpegLsLossy, new DJLSRepresentationParameter( 2, false ) )
	);

	DcmRLEDecoderRegistration::registerCodecs( false );
	DcmRLEEncoderRegistration::registerCodecs( false );

	addCodec( QTransferSyntax::Rle,
		QDicomImageCodec( RleLossless, new DcmRLERepresentationParameter )
	);
}


bool QDicomImageCodec::isNull() const {
	return family_ == Unknown && features_ == None && parameters_ == nullptr;
}


bool QDicomImageCodec::isValid() const {
	return family_ != Unknown && parameters_ != nullptr;
}


inline const DcmRepresentationParameter & QDicomImageCodec::parameters() const {
	Q_ASSERT( parameters_ );

	return *parameters_;
}


int QDicomImageCodec::quality() const {
	if ( isValid() && hasFeature( Quality ) ) {
		Q_ASSERT( family() == JpegLossy );
		DJ_RPLossy * params = static_cast< DJ_RPLossy * >( parameters_ );

		return params->getQuality();
	}
	else {
		return -1;
	}
}


void QDicomImageCodec::setQuality( int value ) {
	if ( isValid() && hasFeature( Quality ) ) {
		Q_ASSERT( family() == JpegLossy );

		delete parameters_;
		parameters_ = new DJ_RPLossy( value );
	}
	else {
		qDebug( __FUNCTION__": "
			"codec is invalid or does not support the Quality feature"
		);
	}
}


QList< QTransferSyntax > QDicomImageCodec::supportedTransferSyntaxes() {
	return codecRegister_.keys();
}

