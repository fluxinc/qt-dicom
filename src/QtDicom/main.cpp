/***************************************************************************
 *   Copyright © 2012 by Flux Inc.                                         *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#include <windows.h>

#include <QtDicom/QDicomImageCodec>


void cleanup() {
	QDicomImageCodec::cleanup();
}


void init() {
	QDicomImageCodec::init();
}


BOOL WINAPI DllMain(
	_In_ HINSTANCE dllHandle,
	_In_ DWORD reason,
	_In_ LPVOID reserved
) {
	switch ( reason ) {

	case DLL_PROCESS_ATTACH :
		init();
		break;

	case DLL_PROCESS_DETACH :
		cleanup();
		break;
	};

	return TRUE;
}
