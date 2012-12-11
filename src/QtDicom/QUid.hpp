/***************************************************************************
 *   Copyright © 2012 by Flux Inc.                                         *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#ifndef QTDICOM_QUID_HPP
#define QTDICOM_QUID_HPP

#include <QtCore/QByteArray>

#include <QtDicom/Globals.hpp>

class QDICOM_DLLSPEC QUid : public QByteArray {
	public :
		QUid();
		QUid( const char * UID );
		QUid( const QByteArray & UID );
		explicit QUid( const QString & UID );
		explicit QUid( const QUid & UID );
		~QUid();

		QUid & operator = ( const char * UID );

		bool operator == ( const QUid & UID ) const;
		bool operator != ( const QUid & UID ) const;


		operator const char * () const;
};


#endif
