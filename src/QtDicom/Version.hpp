/***************************************************************************
 *   Copyright (C) 2009-2012 by Flux Inc.                                  *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#ifndef VERSION_HPP
#define VERSION_HPP

#define COMPANY_NAME_STRING       "Flux Inc."
#define PRODUCT_NAME_STRING       "Qt DICOM"
#define LEGAL_COPYRIGHT_STRING     L"Copyright \x00a9 2010-2012 Flux Inc."
#ifndef _DEBUG
#	define ORIGINAL_FILENAME_STRING   "QtDicom4.dll"
#else
#	define ORIGINAL_FILENAME_STRING   "QtDicomd4.dll"
#endif
#define FILE_DESCRIPITION_STIRNG   PRODUCT_NAME_STRING " module"
#define FILE_VERSION               4,8,1,60
#define FILE_VERSION_STRING        "4.8.1.60"
#define PRODUCT_VERSION            4,8,1,60
#define PRODUCT_VERSION_STRING     "4.8.1.60"

#endif
