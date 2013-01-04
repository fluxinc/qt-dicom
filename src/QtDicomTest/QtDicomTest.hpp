/***************************************************************************
 *   Copyright © 2012-2013 by Flux Inc.                                    *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#ifndef	QTDICOMTEST_HPP
#define QTDICOMTEST_HPP

#include <QtCore/QObject>


class QtDicomTest : public QObject {
	Q_OBJECT;

	public :
	public slots :
		void testRequestorAssociation();
};

#endif
