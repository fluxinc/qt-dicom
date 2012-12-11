/***************************************************************************
 *   Copyright © 2012 by Flux Inc.                                         *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#ifndef QTDICOM_QPRESENTATIONCONTEXTDATA_HPP
#define QTDICOM_QPRESENTATIONCONTEXTDATA_HPP

#include <QtCore/QList>
#include <QtCore/QSharedData>

#include <QtDicom/QTransferSyntax>
#include <QtDicom/QUid>

class QPresentationContext;


class QDICOM_DLLSPEC QPresentationContextData : public QSharedData {
	friend QPresentationContext;

	public :
		QPresentationContextData();
		QPresentationContextData( const QPresentationContextData & other );
		~QPresentationContextData();

	private :
		QUid abstractSyntax_;
		int acceptedTransferSyntaxPosition_;
		QList< QTransferSyntax > transferSyntaxes_;
};


#endif
