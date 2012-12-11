/***************************************************************************
 *   Copyright © 2012 by Flux Inc.                                         *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#include "QPresentationContext.hpp"
#include "QPresentationContextData.hpp"

#include <QtCore/QStringList>

#include <dcmtk/dcmnet/assoc.h>


QPresentationContext::QPresentationContext( const QUid & AbstractSyntax ) :
	data_( new QPresentationContextData() )	
{
	data_->abstractSyntax_ = AbstractSyntax;
	data_->acceptedTransferSyntaxPosition_ = -1;
}


QPresentationContext::QPresentationContext( const QPresentationContext & Other ) :
	data_( Other.data_ )
{
}


QPresentationContext::~QPresentationContext() {
}


QPresentationContext & QPresentationContext::operator << (
	const QTransferSyntax & Ts
) {
	addTransferSyntax( Ts );

	return *this;
}


const QUid & QPresentationContext::abstractSyntax() const {
	return data_->abstractSyntax_;
}


void QPresentationContext::accept( const QTransferSyntax & Ts ) {
	data_->acceptedTransferSyntaxPosition_ = 
		data_->transferSyntaxes_.indexOf( Ts )
	;
	if ( data_->acceptedTransferSyntaxPosition_ == -1 ) {
		qWarning( __FUNCTION__": "
			"transfer syntax %s is not a part of this presentation context",
			qPrintable( Ts.toString() )
		);
	}
}


bool QPresentationContext::accepted() const {
	return data_->acceptedTransferSyntaxPosition_ > -1;
}


const QTransferSyntax & QPresentationContext::acceptedTransferSyntax() const {
	if ( accepted() ) {
		Q_ASSERT( data_->transferSyntaxes_.size() > data_->acceptedTransferSyntaxPosition_ );

		return data_->transferSyntaxes_.at( data_->acceptedTransferSyntaxPosition_ );
	}
	else {
		static const QTransferSyntax Dummy;
		return Dummy;
	}
}


void QPresentationContext::addTransferSyntax( const QTransferSyntax & Ts ) {
	if ( Ts.isValid() ) {
		if ( ! data_->transferSyntaxes_.contains( Ts ) ) {
			data_->transferSyntaxes_.append( Ts );
		}
		else {
			qWarning( __FUNCTION__": "
				"this presentation context includes %s transfer syntax already",
				qPrintable( Ts.toString() )
			);
		}
	}
	else {
		qWarning( __FUNCTION__": failed to add invalid transfer syntax" );
	}
}


QPresentationContext QPresentationContext::defaultFor( const QUid & As ) {
	QPresentationContext result( As );
	result 
		<< QTransferSyntax::LittleEndian 
		<< QTransferSyntax::BigEndian
		<< QTransferSyntax::LittleEndianImplicit
	;

	return result;
}


QPresentationContext QPresentationContext::fromTAscPresentationContext(
	const T_ASC_PresentationContext & dcmContext
) {
	QPresentationContext result;
	result.setAbstractSyntax( dcmContext.abstractSyntax );
	for ( int i = 0; i < dcmContext.transferSyntaxCount; ++i ) {
		const QTransferSyntax Ts = QTransferSyntax::fromUid(
			dcmContext.proposedTransferSyntaxes[ i ]
		);
		if ( Ts.isValid() ) {
			result << Ts;
		}
		else {
			qWarning( __FUNCTION__": "
				"couldn't detect Transfer Syntax from unknown UID: %s",
				dcmContext.proposedTransferSyntaxes[ i ]
			);
		}
	}

	if ( dcmContext.resultReason == ASC_P_ACCEPTANCE ) {
		const QTransferSyntax Ts = QTransferSyntax::fromUid(
			dcmContext.acceptedTransferSyntax
		);
		if ( Ts.isValid() ) {
			result.accept( Ts );
		}
		else {
			qWarning( __FUNCTION__": "
				"couldn't detect Transfer Syntax from unknown UID: %s",
				dcmContext.acceptedTransferSyntax
			);
		}
	}

	return result;
}


bool QPresentationContext::isNull() const {
	return data_->abstractSyntax_.isEmpty() || data_->transferSyntaxes_.isEmpty();
}


bool QPresentationContext::isValid() const {
	return ! isNull();
}


const QList< QTransferSyntax > 
	QPresentationContext::proposedTransferSyntaxes()
const {
	return data_->transferSyntaxes_;
}


void QPresentationContext::setAbstractSyntax( const QUid & Uid ) {
	data_->abstractSyntax_ = Uid;
}


QString QPresentationContext::toString() const {
	QStringList tsNames;
	foreach( const QTransferSyntax & ts, data_->transferSyntaxes_ ) {
		tsNames.append( ts.toString() );
	}
	
	return QString(
		"Abstract Syntax  : %1\n"
		"Transfer Syntaxes: %2\n"
		"Accepted         : %3"
	)
	.arg( dcmFindNameOfUID( data_->abstractSyntax_, data_->abstractSyntax_ ) )
	.arg( tsNames.join( ", " ) )
	.arg( accepted() ? acceptedTransferSyntax().toString() : QString( "<None>" ) );
}