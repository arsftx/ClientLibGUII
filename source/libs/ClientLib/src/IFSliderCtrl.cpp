
#include "IFSliderCtrl.h"

GFX_IMPLEMENT_DYNAMIC_EXISTING(CIFSliderCtrl, 0x9FE218)


void CIFSliderCtrl::SetSliderTexture(const char* leftButtonTexturePath, const char* rightButtonTexturePath,
                                     const char* midButtonTexturePath) {
    reinterpret_cast<void (__thiscall *)(CIFSliderCtrl *,
                                         const char*,
                                         const char*,
                                         const char*)>(0x00428060)(this, leftButtonTexturePath,
                                                                           rightButtonTexturePath,
                                                                           midButtonTexturePath);
}

void CIFSliderCtrl::FUN_006596f0(int param_1, int param_2, int param_3, undefined4 param_4, int param_5,
                                 undefined4 param_6) {
    reinterpret_cast<void (__thiscall *)(CIFSliderCtrl *, int, int, int, undefined4, int, undefined4)>(0x00428460)(this,
                                                                                                                   param_1,
                                                                                                                   param_2,
                                                                                                                   param_3,
                                                                                                                   param_4,
                                                                                                                   param_5,
                                                                                                                   param_6);
}

void CIFSliderCtrl::SetMaxButtonSlide(undefined4 param_1) {
    reinterpret_cast<void (__thiscall *)(CIFSliderCtrl *, undefined4)>(0x004285B0)(this, param_1);
}

void CIFSliderCtrl::FUN_006599e0(int param_2, int param_3, int param_4, int param_5) {
    reinterpret_cast<void (__thiscall *)(CIFSliderCtrl *, int, int, int, int)>(0x00428770)(this, param_2, param_3,
                                                                                           param_4, param_5);
}

void CIFSliderCtrl::SetEnabledState(bool bState) {
    reinterpret_cast<void (__thiscall *)(CIFSliderCtrl *, bool)>(0x006597d0)(this, bState);
}
