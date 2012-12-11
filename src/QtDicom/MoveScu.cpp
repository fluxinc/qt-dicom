/***************************************************************************
 *   Copyright (C) 2011 by Flux Inc.                                       *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#include "ConnectionParameters.hpp"
#include "Dataset.hpp"
#include "MoveScu.hpp"
#include "RequestorAssociation.hpp"

#include <QtCore/QDir>


namespace Dicom {

MoveScu::MoveScu() :
	ServiceUser()
{
}


MoveScu::~MoveScu() {
}


int MoveScu::move(
	const ConnectionParameters & Parameters,
	const char * AbstractSyntax,
	const Dataset & Dataset,
	const QString & DestinationAe,
	UidList * failedSopInstances,
	int * warned
) {
	int result = 0;

	const UidList AbstractSyntaxes = 
		UidList( AbstractSyntax )
	;

	RequestorAssociation a;
	setAssociation( &a );

	bool timedOut;
	const int Count = a.request( Parameters, AbstractSyntaxes, &timedOut );
	if ( Count > 0 ) {
		int failed;
		result = cMove( Dataset, AbstractSyntax, DestinationAe, &failed, failedSopInstances, warned );
		if ( failed != failedSopInstances->size() ) {
			qWarning(
				"Retrieved instances count (%d) differs from the status info (%d).",
				failedSopInstances->size(), failed
			);
		}
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
