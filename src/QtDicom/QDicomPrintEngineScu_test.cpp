/***************************************************************************
 *   Copyright © 2013 by Flux Inc.                                         *
 *   Author: Pawel Zak <pawel.zak@fluxinc.ca>                              *
 **************************************************************************/

#ifdef _DEBUG

#include "QDicomPrintEngineScu.hpp"


extern bool TestQDicomPrintEngineScu_ConvertPixel();

extern bool TestQDicomPrintEngineScu() {
	return TestQDicomPrintEngineScu_ConvertPixel();
}

#endif // _DEBUG

 