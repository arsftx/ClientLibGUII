#include "IFFrame.h"
#include <BSLib/Debug.h>

GFX_IMPLEMENT_DYNAMIC_EXISTING(CIFFrame, 0x09FFE94)

bool CIFFrame::OnCreate(long ln) {
    return reinterpret_cast<bool(__thiscall *)(CIFFrame *, long)>(0x00564620)(this, ln);//ecsro
}

void CIFFrame::RenderMyself() {
    reinterpret_cast<void(__thiscall *)(CIFFrame *)>(0x00564640)(this);
}

void CIFFrame::Func_49(std::n_string str) {
    BS_DEBUG_LOW("%s (\"%s\")", __FUNCTION__, str.c_str());
    reinterpret_cast<void(__thiscall *)(CIFFrame *, std::n_string)>(0x005658f0)(this, str);
}
