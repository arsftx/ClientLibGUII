#pragma once

#include "IFWnd.h"
#include "IFTileWnd.h"
class CIFGGMainSlot : public CIFWnd {
GFX_DECLARE_DYNAMIC_EXISTING(CIFGGMainSlot, 0x0CB0E50)
public:
	bool OnCreateGuideSlotIMPL(long ln);
	int OnPressGGMainSlot();
public:
	char pad_00[0x14];
//BEGIN_FIXTURE()
//        ENSURE_SIZE(0x380)
//    END_FIXTURE()
//
//    RUN_FIXTURE(CIFGGMainSlot)
};

