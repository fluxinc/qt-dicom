/***************************************************************************
 *   Copyright (C) 2011 by Flux Inc.                                       *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#ifndef DICOM_CONNECTIONPARAMETERS_PRIV_HPP
#define DICOM_CONNECTIONPARAMETERS_PRIV_HPP

#include <QtCore/QSharedData>

#include <QtNetwork/QHostAddress>

namespace Dicom {

class ConnectionParameters;

class ConnectionParameters_priv : public QSharedData {
	friend class ConnectionParameters;

	public :
		ConnectionParameters_priv();
		ConnectionParameters_priv( const ConnectionParameters_priv & other );
		~ConnectionParameters_priv();
		ConnectionParameters_priv & operator = (
			const ConnectionParameters_priv & other
		);

	private :
		QHostAddress hostAddress_;
		unsigned maxPdu_;
		QString myAeTitle_;
		QString peerAeTitle_;
		quint16 port_;
		int timeout_;
		int type_;
};

}; // Namespace end.

#endif