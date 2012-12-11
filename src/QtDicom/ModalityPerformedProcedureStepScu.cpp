/***************************************************************************
 *   Copyright (C) 2011 by Flux Inc.                                       *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#include "ModalityPerformedProcedureStepScu.hpp"
#include "RequestorAssociation.hpp"

#include <dcmtk/dcmdata/dcuid.h>

namespace Dicom {

ModalityPerformedProcedureStepScu::ModalityPerformedProcedureStepScu() :
	ServiceUser()
{
}


ModalityPerformedProcedureStepScu::~ModalityPerformedProcedureStepScu() {
}


QByteArray ModalityPerformedProcedureStepScu::create(
	const ConnectionParameters & Parameters,
	const Dataset & Attributes,
	Dataset * affected
) {
	RequestorAssociation a;
	setAssociation( &a );

	bool timedOut;
	const int Count = a.request( 
		Parameters,
		UidList( UID_ModalityPerformedProcedureStepSOPClass ),
		&timedOut
	);

	if ( Count > 0 ) {
		const QByteArray AffectedSopInstance = nCreate(
			UID_ModalityPerformedProcedureStepSOPClass,
			Attributes,
			affected
		);
		a.release();

		Q_ASSERT( ! ( AffectedSopInstance.isEmpty() ^ hasError() ) );
		
		if ( ! hasError() && AffectedSopInstance.isEmpty() ) {
			raiseError( "Empty Affected SOP Instance retrieved." );
		}
		else if ( ! hasError() ) {
			return AffectedSopInstance;
		}
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

	return QByteArray();
}


void ModalityPerformedProcedureStepScu::set(
	const ConnectionParameters & Parameters,
	const QByteArray & AffectedSopInstanceUid,
	const Dataset & Attributes,
	Dataset * modified
) {
	RequestorAssociation a;
	setAssociation( &a );

	bool timedOut;
	const int Count = a.request( 
		Parameters,
		UidList( UID_ModalityPerformedProcedureStepSOPClass ),
		&timedOut
	);

	if ( Count > 0 ) {
		nSet(
			UID_ModalityPerformedProcedureStepSOPClass,
			AffectedSopInstanceUid.constBegin(),
			Attributes,
			modified
		);
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
}

}; // Namespace DICOM ends here.
