/***************************************************************************
 *   Copyright (C) 2012 by Flux Inc.                                       *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#ifndef EXCEPTIONS_HPP
#define EXCEPTIONS_HPP

#include <exception>
#include <stdexcept>

#include <QtCore/QString>


class GenericException : public std::runtime_error {
	public :
		GenericException( const QString & message );
		virtual ~GenericException();
};


class OperationFailedException : public GenericException {
	public :
		OperationFailedException( const QString & message );
};


#endif

