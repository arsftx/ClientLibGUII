#include "IFCheckBox.h"

bool CIFCheckBox::GetCheckedState_MAYBE() const {
    return reinterpret_cast<bool (__thiscall *)(const CIFCheckBox *)>(0x004240A0)(this); //ECSRO
}

void CIFCheckBox::FUN_00656d50(bool state) {
    reinterpret_cast<void (__thiscall *)(CIFCheckBox *, bool)>(0x00424040)(this, state);//ECSRO
}
