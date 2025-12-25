#include "IFVerticalScroll.h"

// Link to native runtime class at 0x9FE238
// Registration: sub_4289A0
// Factory: sub_4289D0
// Constructor: sub_428B60
GFX_IMPLEMENT_DYNAMIC_EXISTING(CIFVerticalScroll, 0x9FE238)

CIFVerticalScroll::CIFVerticalScroll() {
    // Native constructor (sub_428B60) does:
    // 1. Call parent CIFScrollBar constructor (sub_427020)
    // 2. Set VTable 1 at +0x00 = 0x93C688
    // 3. Set VTable 2 at +0x6C = 0x93C640
    // 4. Initialize byte at +0x2F0 = 0 (orientation flag?)
    memset(m_vscrollData, 0, sizeof(m_vscrollData));
}

CIFVerticalScroll::~CIFVerticalScroll() {
}