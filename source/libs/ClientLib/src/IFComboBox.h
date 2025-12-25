//
// Created by ISRO on 4/1/2022.
//
#pragma once

#include "IFStatic.h"

class CIFComboBox : public CIFStatic {
GFX_DECLARE_DYNAMIC_EXISTING(CIFComboBox, 0x09FE298)

public:
    bool OnCreate(long ln) override;

    void RenderMyself() override;

    void Func_15() override;

public:
    void SetMaxSlotInList(int nCount);

    size_t GetTextureListSize() const;

    int GetSelectedSlotId() const;
	void SetSelectedSlotId(int value);
    void SelectSlotList(int nSlotId);

    void EraseSlotFromList(size_t nSlotId);

    void InsertSlotToList(int nSlotId, const char *wstr);

public:
	unsigned int m_nMaxSlotInList; //0x0394
	unsigned int m_nSelectedSlotNum; //0x0398
};
