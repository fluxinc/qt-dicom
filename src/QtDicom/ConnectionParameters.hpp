/***************************************************************************
 *   Copyright (C) 2011 by Flux Inc.                                       *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#ifndef DICOM_CONNECTIONPARAMETERS_HPP
#define DICOM_CONNECTIONPARAMETERS_HPP

#include <QtCore/QSharedDataPointer>

#include <QtDicom/ConnectionParameters_priv.hpp>
#include <QtDicom/Globals.hpp>


class QXmlStreamReader;
class QXmlStreamWriter;

namespace Dicom {

class QDICOM_DLLSPEC ConnectionParameters {
	public :
		enum Type {
			Client = 0,
			Server = 1
		};

		static ConnectionParameters fromXml(
			QXmlStreamReader & input, QString * errorMessage
		);

	public :
		ConnectionParameters();
		ConnectionParameters( Type type );
		ConnectionParameters( const ConnectionParameters & other );
		~ConnectionParameters();
		ConnectionParameters & operator = (
			const ConnectionParameters & other
		);

		const QHostAddress & hostAddress() const;
		bool isValid() const;
		unsigned maxPdu() const;
		const QString & myAeTitle() const;
		const QString & peerAeTitle() const;
		quint16 port() const;

		void setHostAddress( const QHostAddress & );
		void setMaxPdu( unsigned );
		void setMyAeTitle( const QString & );
		void setPeerAeTitle( const QString & );
		void setPort( quint16 );
		void setTimeout( int );
		void setType( Type type );

		int timeout() const;
		QString toString() const;
		void toXml( QXmlStreamWriter & stream ) const;
		Type type() const;
		const QString & typeString() const;

	public :
		static int defaultTimeout();
		static unsigned defaultMaxPdu();
		static const QString & defaultMyAeTitle();
		static Type typeFromString( const QString & type );
		static const QString & typeString( Type type );

	private :
		bool readXml( QXmlStreamReader & input, QString * errorMessage = 0 );

	private :
		QSharedDataPointer< ConnectionParameters_priv > d_;
};

}; // Namespace end.

#endif
