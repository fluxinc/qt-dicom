/***************************************************************************
 *   Copyright © 2012 by Flux Inc.                                         *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#include <windows.h>

#include <QtDicom/QDicomImageCodec>


static void globalCleanup() {
	QDicomImageCodec::cleanupRegister();
}


static void globalInit() {
	QDicomImageCodec::initRegister();
}


BOOL WINAPI DllMain(
	_In_ HINSTANCE dllHandle,
	_In_ DWORD reason,
	_In_ LPVOID reserved
) {
	switch ( reason ) {

	case DLL_PROCESS_ATTACH :
		globalInit();
		break;

	case DLL_PROCESS_DETACH :
		globalCleanup();
		break;
	};

	return TRUE;
}
