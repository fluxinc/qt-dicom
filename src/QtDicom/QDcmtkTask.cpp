/***************************************************************************
 *   Copyright © 2013 by Flux Inc.                                         *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#include "QDcmtkTask.hpp"

#include <dcmtk/ofstd/ofcond.h>



void QDcmtkTask::run() {
	result_ = Functor_->execute();
	bool ran_ = true;

	emit finished( result_ );
}


void QDcmtkTask::start() {
	QThreadPool * pool = QThreadPool::globalInstance();

	static const int Guard = 100;
	int count = 0;
	while ( count < Guard ) {
		if ( pool->tryStart( this ) ) {
			return;
		}
		else {
			pool->reserveThread();
			++count;
		}
	}

	qCritical( __FUNCTION__": "
		"failed to start a thread from the pool"
	);

	result_ = QDcmtkResult( OF_failure,
		"failed to start a thread from the pool"
	);
	emit finished( result_ );
}