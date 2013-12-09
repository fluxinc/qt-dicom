/***************************************************************************
 *   Copyright © 2012 by Flux Inc.                                         *
 *   Author: Paweł Żak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#include <windows.h>

#include <QtDicom/QDicomImageCodec>


#ifdef _DEBUG

extern bool TestQDicomPrintEngineScu();
extern bool TestQDicomPrinterDriver();
extern bool TestQDicomTag();
extern bool TestQSopClass();
extern bool TestQTransferSyntax();

static bool RunTests() {
	return
		::TestQDicomPrintEngineScu() &&
		::TestQDicomPrinterDriver() &&
		::TestQDicomTag() &&
		::TestQSopClass() &&
		::TestQTransferSyntax()
	;
}
#else // ! _DEBUG
static bool RunTests() {
	return true;
} 
#endif // DEBUG


static void globalCleanup() {
	QDicomImageCodec::cleanupRegister();
}


static void globalInit() {
	QDicomImageCodec::initRegister();

	Q_ASSERT( ::RunTests() );
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
