/***************************************************************************
 *   Copyright © 2013 by Flux Inc.                                         *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#include "QDicomAttribute.hpp"
#include "QSopClass.hpp"


QDicomAttribute::QDicomAttribute() {
}


QDicomAttribute::QDicomAttribute(
	const QDicomTag & Tag, const QStringList & Values
) :
	data_( new Data( Tag, Values ) )
{
}
	

QDicomAttribute::QDicomAttribute( 
	const QDicomTag & Tag, const QString & Value
) :
	data_( new Data( Tag, Value.split( '\\' ) ) )
{
}


QDicomAttribute::QDicomAttribute(
	const QDicomTag & Tag, const QSopClass & SopClass 
) :
	data_( new Data( Tag, QStringList() << QString::fromAscii( SopClass.uid() ) ) )
{
}


QDicomAttribute::~QDicomAttribute() {
}


const QDicomAttribute::Data & QDicomAttribute::data() const {
	return *data_;
}


QDicomAttribute::Data & QDicomAttribute::data() {
	return *data_;
}


bool QDicomAttribute::isEmpty() const {
	const QStringList & List = values();

	return isNull() || ( [&List] () -> bool {
		for (
			QStringList::const_iterator i = List.constBegin();
			i != List.constEnd(); ++i
		) {
			if ( i->size() > 0 ) {
				return false;
			}
		}

		return true;
	} )();
}


bool QDicomAttribute::isNull() const {
	return tag().isNull() && value().isEmpty();
}


bool QDicomAttribute::isValid() const {
	return tag().isValid();
}


int QDicomAttribute::multiplicity() const {
	return value().size();
}


const QString & QDicomAttribute::name() const {
	return tag().keyword();
}


const QDicomTag & QDicomAttribute::tag() const {
	return data().tag;
}


QString QDicomAttribute::value() const {
	return values().join( "\\" );
}


const QStringList & QDicomAttribute::values() const {
	return data().value;
}
