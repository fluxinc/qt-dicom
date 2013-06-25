/***************************************************************************
 *   Copyright © 2013 by Flux Inc.                                         *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#ifndef QTDICOM_QDICOMATTRIBUTE_HPP
#define QTDICOM_QDICOMATTRIBUTE_HPP

#include <QtCore/QSharedData>
#include <QtCore/QString>
#include <QtCore/QStringList>

#include <QtDicom/Globals.hpp>
#include <QtDicom/QDicomTag>


class QDICOM_DLLSPEC QDicomAttribute {
	public :
		QDicomAttribute();
		QDicomAttribute(
			const QDicomTag & tag, const QStringList & values
		);
		QDicomAttribute(
			const QDicomTag & tag, const QString & value
		);
		~QDicomAttribute();
		
		bool isEmpty() const;
		bool isNull() const;
		bool isValid() const;
		int multiplicity() const;
		const QString & name() const;
		const QDicomTag & tag() const;
		QString value() const;
		const QStringList & values() const;

	private :
		struct Data : QSharedData {
			Data( const QDicomTag & Tag, const QStringList & Value ) :
				tag( Tag ), value( Value )
			{
			}

			QDicomTag tag;
			QStringList value;
		};

	private :
		inline const Data & data() const;
		inline Data & data();
		QSharedDataPointer< Data > data_;
		
};

#endif // ! QTDICOM_QDICOMATTRIBUTE_HPP
