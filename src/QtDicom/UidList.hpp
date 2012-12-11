/***************************************************************************
 *   Copyright (C) 2011 by Flux Inc.                                       *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#ifndef QDICOM_UIDLIST_HPP
#define QDICOM_UIDLIST_HPP

#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QSet>
#include <QtCore/QVector>

#include "QtDicom/Globals.hpp"

class QStringList;
class QXmlStreamWriter;

namespace Dicom {

class QDICOM_DLLSPEC UidList : public QList< QByteArray > {
	public :
		UidList();
		UidList( const char * UID );
		UidList( const QStringList & list );
		~UidList();

		QSet< QByteArray > toSet() const;

		/**
		 * De-allocate later!.
		 */
		const char ** toFlatArray() const;

		void writeXml( QXmlStreamWriter & output ) const;

	public :
		static UidList echoSopClass();
		static UidList queryRetrieveSopClasses();
		static UidList supportedTransferSyntaxes();
		static UidList storageSopClasses();
};

UidList operator+( const UidList & first, const UidList & second );

}; // Namesapce DICOM ends here.

#endif
