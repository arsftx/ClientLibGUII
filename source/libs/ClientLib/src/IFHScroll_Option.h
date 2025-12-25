#pragma once

#include "IFStatic.h"
#include "IFScrollBar.h"
#include "IFButton.h"


class CIFHScroll_Option : public CIFScrollBar {
    GFX_DECLARE_DYNAMIC_EXISTING(CIFHScroll_Option, 0x9FEA24)

    GFX_DECLARE_MESSAGE_MAP(CIFHScroll_Option)
public:
	int Set2E0(int value);
	int Set2E4(int value);
	int Set2E8(int value);
	int Set2EC(int value);
	int Get2EC();
	int Set2F0(int value);
	CIFButton* Get2F4();
	CIFButton* Get2F8();
	CIFButton* Get2FC();
	int Set350(int value);
	void SetHScrollBar(int value1,int value2, int value3,int value4);
	void SetHCorrectScrollBar(int value1);
};