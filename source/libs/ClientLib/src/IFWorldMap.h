#pragma once

#include "IFWnd.h"
#include "IFMainFrame.h"
#include <IFScrollManager.h>
#include "IFTileWnd.h"
class CIFWorldMap : public CIFMainFrame {
GFX_DECLARE_DYNAMIC_EXISTING(CIFWorldMap, 0x00CAF9A4)
public:
	bool OnCreateIMPL(long ln);
	void DisableManualButton();
public:
	char pad_00[0x5a74];
//
//BEGIN_FIXTURE()
//        ENSURE_SIZE(0x6230)
//	//ENSURE_OFFSET(m_scrollmanager, 0x7E0)
//    END_FIXTURE()
//
//    RUN_FIXTURE(CIFWorldMap)
};

