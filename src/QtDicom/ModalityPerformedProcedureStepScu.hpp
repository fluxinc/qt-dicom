/***************************************************************************
 *   Copyright (C) 2011 by Flux Inc.                                       *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#ifndef DICOM_MODALITYPERFORMEDPROCEDURESTEP_HPP
#define DICOM_MODALITYPERFORMEDPROCEDURESTEP_HPP

#include "QtDicom/ConnectionParameters.hpp"
#include "QtDicom/Globals.hpp"
#include "QtDicom/ServiceUser.hpp"

namespace Dicom {

class Dataset;

class QDICOM_DLLSPEC ModalityPerformedProcedureStepScu : public ServiceUser {
	public :
		ModalityPerformedProcedureStepScu();
		~ModalityPerformedProcedureStepScu();

		QByteArray create( 
			const ConnectionParameters & parameters,
			const Dataset & dataset,
			Dataset * attributes = 0
		);
		void set( 
			const ConnectionParameters & parameters, 
			const QByteArray & instance,
			const Dataset & dataset,
			Dataset * attributes = 0
		);
};

}; // Namepsace DICOM ends here.

#endif
