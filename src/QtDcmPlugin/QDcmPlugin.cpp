/***************************************************************************
 *   Copyright © 2010-2014 by Flux Inc.                                    *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#include "QDcmHandler.hpp"
#include "QDcmPlugin.hpp"

#include <dcmtk/config/osconfig.h>

#include <dcmtk/dcmdata/dcrledrg.h>

#include <dcmtk/dcmjpeg/djdecode.h>


QDcmPlugin::QDcmPlugin() {
	DcmRLEDecoderRegistration::registerCodecs();
	DJDecoderRegistration::registerCodecs();
}


QDcmPlugin::~QDcmPlugin() {
	DJDecoderRegistration::cleanup();
	DcmRLEDecoderRegistration::cleanup();
}


QStringList QDcmPlugin::keys() const {
	static const QStringList Keys = QStringList() << "dcm";

    return Keys;
}


QImageIOPlugin::Capabilities QDcmPlugin::capabilities(
	QIODevice * device, const QByteArray & Format
) const {
	bool Readable = 
		Format == "dcm" ||
		( device && device->isReadable() && QDcmHandler::canRead( device ) )
	;
    if ( Readable ) {
        return Capabilities( CanRead );
	}
	else {
		return 0;
	}
}

QImageIOHandler * QDcmPlugin::create(
	QIODevice * device, const QByteArray & Format
) const {
    return new QDcmHandler( device, Format );
}


Q_EXPORT_PLUGIN2( qdcm,  QDcmPlugin )
