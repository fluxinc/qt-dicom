/***************************************************************************
 *   Copyright (C) 2011 by Flux Inc.                                       *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#include "ConnectionParameters.hpp"
#include "Dataset.hpp"
#include "QueryScu.hpp"
#include "RequestorAssociation.hpp"

#include <QtCore/QDir>


namespace Dicom {

QueryScu::QueryScu() :
	ServiceUser()
{
}


QueryScu::~QueryScu() {
}


QList< Dataset > QueryScu::query(
	const ConnectionParameters & Parameters,
	const char * AbstractSyntax,
	const Dataset & Dataset
) {
	QList< Dicom::Dataset > result;

	const UidList AbstractSyntaxes = 
		UidList( AbstractSyntax )
	;

	RequestorAssociation a;
	setAssociation( &a );

	bool timedOut;
	const int Count = a.request( Parameters, AbstractSyntaxes, &timedOut );
	if ( Count > 0 ) {
		result = cFind( Dataset, AbstractSyntax );
		a.release();
	}
	else if ( timedOut ) {
		raiseError( "Connection timed out." );
	}
	else if ( Count == 0 ) {
		raiseError( "None of proposed presentation contexts were supported." );
	}
	else {
		raiseError( association()->errorMessage() );
	}

	return result;
}

}; // Namespace DICOM ends here.
