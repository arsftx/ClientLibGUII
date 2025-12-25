#pragma once

#include "IFWnd.h"
#include <Test/Test.h>

// =====================================================================
// CIFScrollBar - Base Scrollbar Control
// IDA Analysis (ECSRO):
//   Runtime Class: 0x9FE1F8
//   Size: 0x2F4 (756 bytes)
//   Parent: CIFWnd (0x9FE5C0)
//
//   Registration: sub_426F80
//   Factory:      sub_426FB0
//   Constructor:  sub_427020
//   CreateObject: sub_427010
//
//   VTable 1: 0x93C3F0 (at +0x00)
//   VTable 2: 0x93C3A8 (at +0x6C, secondary/inner class)
//
// Offset Layout (from sub_427020):
//   +0x000 - 0x2B3: CIFWnd base
//   +0x2B4: First CIFScrollBar member (initialized to 0)
//   +0x2B4 - 0x2F3: CIFScrollBar data (0x40 bytes = 64 bytes)
// =====================================================================

class CIFScrollBar : public CIFWnd {
    GFX_DECLARE_DYNAMIC_EXISTING(CIFScrollBar, 0x9FE1F8)

public:
    CIFScrollBar();
    ~CIFScrollBar();

    // =====================================================================
    // Offset-based accessors (IDA verified: +0x2B4 initialized to 0)
    // Scroll bar typically needs: min, max, position, step, etc.
    // These offsets need VTable function analysis to confirm exact meaning
    // =====================================================================
    
    // +0x2B4: Unknown DWORD (initialized to 0 in constructor)
    void SetScrollField_2B4(int value) {
        MEMUTIL_WRITE_BY_PTR_OFFSET(this, 0x2B4, int, value);
    }
    int GetScrollField_2B4() const {
        return MEMUTIL_READ_BY_PTR_OFFSET(this, 0x2B4, int);
    }

    // =====================================================================
    // Scrollbar Range/Position Control
    // Based on CIFHScroll_Option analysis: offsets 0x2E0-0x2F0 control scroll state
    // =====================================================================
    
    // Set scroll range (min, max) - based on SetHScrollBar pattern
    void SetScrollRange(int minVal, int maxVal, int step = 1) {
        MEMUTIL_WRITE_BY_PTR_OFFSET(this, 0x2E0, int, minVal);  // min value
        MEMUTIL_WRITE_BY_PTR_OFFSET(this, 0x2E4, int, maxVal);  // max value
        MEMUTIL_WRITE_BY_PTR_OFFSET(this, 0x2E8, int, step);    // step size
    }
    
    // Set current scroll position
    void SetScrollPosition(int position) {
        MEMUTIL_WRITE_BY_PTR_OFFSET(this, 0x2EC, int, position);
    }
    
    // Get current scroll position
    int GetScrollPosition() const {
        return MEMUTIL_READ_BY_PTR_OFFSET(this, 0x2EC, int);
    }
    
    // Full scrollbar setup (like SetHScrollBar)
    void SetScrollBar(int minVal, int maxVal, int current, int step) {
        MEMUTIL_WRITE_BY_PTR_OFFSET(this, 0x2E0, int, minVal);
        MEMUTIL_WRITE_BY_PTR_OFFSET(this, 0x2E4, int, maxVal);
        MEMUTIL_WRITE_BY_PTR_OFFSET(this, 0x2E8, int, step);
        MEMUTIL_WRITE_BY_PTR_OFFSET(this, 0x2EC, int, current);
    }

private:
    // CIFWnd ends at 0x2B4, CIFScrollBar adds 0x40 bytes (0x2F4 - 0x2B4 = 0x40)
    char m_scrollData[0x40];  // 0x2B4 to 0x2F4

    BEGIN_FIXTURE()
    ENSURE_SIZE(0x02F4)
    END_FIXTURE()

    RUN_FIXTURE(CIFScrollBar)
};
