#pragma once

#include <IFWnd.h>

// Hook class for CIFPlayerMiniInfo to disable native rendering
class CIFPlayerMiniInfo_Hook : public CIFWnd {
public:
    // Replacement render function - must match original signature
    // Original: sub_519080 ends with retn 0Ch = 3 params (12 bytes)
    // Signature: bool __thiscall Render(int a2, int a3, int a4)
    bool RenderHook(int a2, int a3, int a4) {
        return true;  // Skip native rendering
    }
};

// Install the hook (call from Util.cpp Setup())
void InstallPlayerMiniInfoHook();

// Move native PlayerMiniInfo off-screen (call each frame)
void HideNativePlayerMiniInfo();
