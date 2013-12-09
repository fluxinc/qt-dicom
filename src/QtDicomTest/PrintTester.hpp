/***************************************************************************
 *   Copyright © 2013 by Flux Inc.                                         *
 *   Author: Pawel Zak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#ifndef PRINTTESTER_HPP
#define PRINTTESTER_HPP

#include <QtCore/QCoreApplication>


class PrintTester : public QCoreApplication {
	Q_OBJECT;

	public :
		PrintTester( int argc, char * argv[] );
		const QString & errorString() const;
		bool hasError() const;

	private :
		void raiseError( const QString &);

	private slots :
		void run();

	private :
		QString errorString_;
};

#endif // PRINTTESTER_HPP
 