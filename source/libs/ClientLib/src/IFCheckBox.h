#pragma once
#include "IFWnd.h"

class CIFCheckBox : public CIFWnd {
	GFX_DECLARE_DYNAMIC_EXISTING(CIFCheckBox, 0x9FE118)
		GFX_DECLARE_MESSAGE_MAP(CIFCheckBox)
public:
    /// \brief Get checkbox checked state (offset 0x2B4)
    /// \address 004240A0
    bool GetCheckedState_MAYBE() const;

    /// \brief Set checkbox checked state (requires enabled flag at 0x2B5 to be set!)
    /// \address 00424040
    void FUN_00656d50(bool state);
    
    /// \brief Set enabled flag - MUST be called before SetChecked on dynamically created checkboxes!
    /// Offset 0x2B5 must be non-zero for SetChecked to work
    void SetEnabled(bool enabled) {
        *((BYTE*)this + 0x2B5) = enabled ? 1 : 0;
    }
    
    /// \brief Convenience wrapper - sets enabled and then sets checked state
    void SetChecked(bool checked) {
        SetEnabled(true);
        FUN_00656d50(checked);
    }
};

