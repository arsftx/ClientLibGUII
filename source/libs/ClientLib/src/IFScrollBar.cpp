#include "IFScrollBar.h"

// Link to native runtime class at 0x9FE1F8
// Registration: sub_426F80
// Factory: sub_426FB0
// Constructor: sub_427020
GFX_IMPLEMENT_DYNAMIC_EXISTING(CIFScrollBar, 0x9FE1F8)

CIFScrollBar::CIFScrollBar() {
    // Native constructor (sub_427020) does:
    // 1. Call parent CIFWnd constructor (sub_4449C0)
    // 2. Set VTable 1 at +0x00 = 0x93C3F0
    // 3. Set VTable 2 at +0x6C = 0x93C3A8
    // 4. Initialize +0x2B4 = 0
    memset(m_scrollData, 0, sizeof(m_scrollData));
}

CIFScrollBar::~CIFScrollBar() {
}
