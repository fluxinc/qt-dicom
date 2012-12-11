/***************************************************************************
 *   Copyright (C) 2012 by Flux Inc.                                       *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#ifndef QDICOM_GLOBALS_HPP
#define QDICOM_GLOBALS_HPP

#include <QtCore/QtGlobal>

#ifdef QDICOM_EXPORTS
# define QDICOM_DLLSPEC Q_DECL_EXPORT
#else
# define QDICOM_DLLSPEC Q_DECL_IMPORT
#endif

#endif