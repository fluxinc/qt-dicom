/***************************************************************************
 *   Copyright (C) 2012 by Flux Inc.                                       *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#include "Exceptions.hpp"


GenericException::GenericException( const QString & Message ) :
	   std::runtime_error( Message.toStdString() )
{
}


GenericException::~GenericException() {
}


OperationFailedException::OperationFailedException( const QString & Message ) :
	   GenericException( Message )
{
}

