#pragma once
#include "IFWnd.h"

/**
 * CIFRadioButton - Radio button helper class
 * 
 * Note: This is a utility class for reading radio button state from 
 * existing game radio button controls. It does not create new instances.
 * For creating radio button visuals, use CIFStatic with:
 *   - com_radiobutton_on.ddj (selected state)
 *   - com_radiobutton_off.ddj (unselected state)
 *
 * Member Layout (from offset 0x2B8):
 * +0x2B8: int m_nRadioButtonCount
 * +0x2E0: int m_nSelectedIndex
 */
class CIFRadioButton {
public:
    // Get selected index from existing game radio button control
    static int GetSelectedIndex(void* pRadioButton) {
        if (!pRadioButton) return -1;
        return *reinterpret_cast<int*>((char*)pRadioButton + 0x2E0);
    }
    
    // Set selected index using game's native function
    static void SetSelectedIndex(void* pRadioButton, int index) {
        if (!pRadioButton) return;
        typedef void (__thiscall *SetSelectedFn)(void*, int);
        SetSelectedFn fn = reinterpret_cast<SetSelectedFn>(0x423BB0);
        fn(pRadioButton, index);
    }
    
    // Get radio button count
    static int GetRadioCount(void* pRadioButton) {
        if (!pRadioButton) return 0;
        return *reinterpret_cast<int*>((char*)pRadioButton + 0x2B8);
    }
};
