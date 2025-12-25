#pragma once

#include "ifwnd.h"
#include "IFMainFrame.h"
class CIFSliderCtrl: public CIFMainFrame
{
GFX_DECLARE_DYNAMIC_EXISTING(CIFSliderCtrl, 0x9FE218)
public:
    
    void SetSliderTexture(const char* leftButtonTexturePath, const char* rightButtonTexturePath,const char* midButtonTexturePath);
    void FUN_006596f0(int param_1, int param_2, int param_3, undefined4 param_4, int param_5,undefined4 param_6);
    void SetMaxButtonSlide(undefined4 param_1);
    void FUN_006599e0(int param_2, int param_3, int param_4, int param_5);
    void SetEnabledState(bool bState);

};

