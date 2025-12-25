#pragma once

#include "IFWnd.h"
#include "IFFrame.h"
#include <IFScrollManager.h>
#include "IFTileWnd.h"
class CIFGGMenu : public CIFFrame {
GFX_DECLARE_DYNAMIC_EXISTING(CIFGGMenu, 0x0CB0E70)
public:
	bool OnCreateGGMenuIMPL(long ln);
	int OnPressTab(int a1);
	CIFScrollManager* GetScrollManager() const;
	int TabGGMenu();
public:
	char pad_00[48];
	CIFScrollManager* m_scrollmanager;
//BEGIN_FIXTURE()
//        ENSURE_SIZE(0x7E4)
//	ENSURE_OFFSET(m_scrollmanager, 0x7E0)
//    END_FIXTURE()
//
//    RUN_FIXTURE(CIFGGMenu)
};
