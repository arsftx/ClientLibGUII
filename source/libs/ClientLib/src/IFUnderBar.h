#pragma once

#include "IFWnd.h"
#include "IFSlotWithHelp.h"
class CIFUnderBar : public CIFWnd {
	GFX_DECLARE_DYNAMIC_EXISTING(CIFUnderBar, 0x9FFD30)
public:
	FILE* SaveUnderbarData();
	FILE* LoadUnderbar();
	void LoadUnderbarData();
	bool OnCreateIMPL(long ln);
public:
	char pad_02B4[64];
	CIFSlotWithHelp* m_main_slot;
	CIFSlotWithHelp* m_totalslotunderbar[41];
	BEGIN_FIXTURE()
		ENSURE_SIZE(0x39c)
		ENSURE_OFFSET(m_main_slot, 0x02F4)
	END_FIXTURE()

	RUN_FIXTURE(CIFUnderBar)
};
