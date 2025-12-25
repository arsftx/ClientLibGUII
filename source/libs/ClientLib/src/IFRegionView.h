#pragma once

#include "IFWnd.h"
#include "IFTileWnd.h"
class IFRegionView : public CIFWnd {
GFX_DECLARE_DYNAMIC_EXISTING(IFRegionView, 0x0CAF8D0)

GFX_DECLARE_DYNCREATE_FN(IFRegionView)

GFX_DECLARE_MESSAGE_MAP(IFRegionView)
public:
	bool OnCreateIMPL(long ln);

	char pad_00[0x24];

//BEGIN_FIXTURE()
//        ENSURE_SIZE(0x390)
//    END_FIXTURE()
//
//    RUN_FIXTURE(IFRegionView)
};

