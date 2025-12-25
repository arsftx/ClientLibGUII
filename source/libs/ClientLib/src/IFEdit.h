#pragma once

#include "IFStatic.h"

class CIFEdit : public CIFStatic {
GFX_DECLARE_DYNAMIC_EXISTING(CIFEdit, 0x0CAF368)

public:
    /// \address 00634EA0
    bool HasFocus() const;

    /// \address 00635150
    void AddValue_404(undefined4 value);

    /// \address 00635070
    void SetValue_404(undefined4 value);

    /// \address 00635060
    undefined4 GetValue_404() const;

    /// \address 00635170
    bool IsSetValue_404(undefined4 value) const;

    /// \address 00634f80
    void SetMaxLength(int length);

    /// \address 006351b0
    void SetTextmode(undefined4 mode);

    const std::n_wstring &GetCurrentText() const;

private:
    HWND m_hEditBoxWnd; //0x0380
    char pad_0384[0xB0FC]; //0x0384 ????
private:
//BEGIN_FIXTURE()
//        ENSURE_SIZE(0xb480) // big size, ha
//        ENSURE_OFFSET(m_hEditBoxWnd, 0x0380)
//    END_FIXTURE()
//
//    RUN_FIXTURE(CIFEdit)
};
