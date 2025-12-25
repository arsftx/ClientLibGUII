#pragma once

#include "IFFrame.h"
#include "IFVerticalScroll.h"

// =====================================================================
// CIFScrollManager - Scroll Manager Control (ECSRO)
// IDA Analysis:
//   RuntimeClass: 0x9FFE54
//   Size: 0x6A4 (1700 bytes)
//   VTable 1: 0x94BD30 (at +0x00)
//   VTable 2: 0x94BCE8 (at +0x6C)
//
//   Registration: sub_561B60
//   Factory:      sub_561B90
//   Constructor:  sub_561C90
//   OnCreate:     sub_561FB0
//
// Key Functions:
//   sub_5626C0 - SetScrollSize(offsetX, offsetY) - controls scroll height
//   sub_428DE0 - SetScrollBarRange (called on CIFVerticalScroll)
//
// Offset Layout:
//   +0x150: IRM (resource manager)
//   +0x678: int - flag (init to 1)
//   +0x67C: BYTE - flag (init to 1)
//   +0x67D: BYTE - flag (init to 0)
//   +0x67E: BYTE - flag (init to 0)
//   +0x680: int - scroll X offset
//   +0x684: int - scroll Y offset
//   +0x688: int - additional Y offset
//   +0x68C: int - content height/row count? (sub_562760 writes here)
//   +0x690: int - unknown (init to 0)
//   +0x694: int - step value (init to 5)
//   +0x698: int - unknown (init to 0)
//   +0x69C: CIFVerticalScroll* - scrollbar pointer
//   +0x6A0: void* - list node pointer
// =====================================================================

class CIFScrollManager : public CIFFrame
{
    GFX_DECLARE_DYNAMIC_EXISTING(CIFScrollManager, 0x9FFE54)

public:
    // =====================================================================
    // Scrollbar access
    // =====================================================================
    CIFVerticalScroll* GetVerticalScroll() const {
        return MEMUTIL_READ_BY_PTR_OFFSET(this, 0x69C, CIFVerticalScroll*);
    }

    // =====================================================================
    // Scroll offset control (IDA: sub_5626C0)
    // These offsets affect scrollbar position and height calculation
    // =====================================================================
    int GetScrollOffsetX() const {
        return MEMUTIL_READ_BY_PTR_OFFSET(this, 0x680, int);
    }
    void SetScrollOffsetX(int value) {
        MEMUTIL_WRITE_BY_PTR_OFFSET(this, 0x680, int, value);
    }

    int GetScrollOffsetY() const {
        return MEMUTIL_READ_BY_PTR_OFFSET(this, 0x684, int);
    }
    void SetScrollOffsetY(int value) {
        MEMUTIL_WRITE_BY_PTR_OFFSET(this, 0x684, int, value);
    }

    int GetAdditionalYOffset() const {
        return MEMUTIL_READ_BY_PTR_OFFSET(this, 0x688, int);
    }
    void SetAdditionalYOffset(int value) {
        MEMUTIL_WRITE_BY_PTR_OFFSET(this, 0x688, int, value);
    }

    // =====================================================================
    // sub_5626C0 - SetScrollSize
    // This function calculates and sets the scrollbar height based on
    // window dimensions and offset values.
    // Height formula: windowHeight - (2 * offsetY + 48)
    // =====================================================================
    void SetScrollSize(int offsetX, int offsetY) {
        typedef void(__thiscall* SetScrollSize_t)(void*, int, int);
        static SetScrollSize_t func = (SetScrollSize_t)0x5626C0;
        func(this, offsetX, offsetY);
    }

    // =====================================================================
    // Step control
    // =====================================================================
    int GetStep() const {
        return MEMUTIL_READ_BY_PTR_OFFSET(this, 0x694, int);
    }
    void SetStep(int value) {
        MEMUTIL_WRITE_BY_PTR_OFFSET(this, 0x694, int, value);
    }

    // =====================================================================
    // Max index control (for scroll range)
    // =====================================================================
    void SetMaxIndexScroll(int value) {
        // Update scrollbar max value through the scroll pointer
        CIFVerticalScroll* scroll = GetVerticalScroll();
        if (scroll) {
            scroll->SetMaxValue(value);
        }
    }

    // =====================================================================
    // Content height/row count (IDA: sub_562760)
    // SkillBoard calls this with value 55 BEFORE SetScrollSize!
    // =====================================================================
    void SetContentHeight(int value) {
        typedef void(__thiscall* SetContentHeight_t)(void*, int);
        static SetContentHeight_t func = (SetContentHeight_t)0x562760;
        func(this, value);
    }
    
    int GetContentHeight() const {
        return MEMUTIL_READ_BY_PTR_OFFSET(this, 0x68C, int);
    }

}; //Size=0x6A4