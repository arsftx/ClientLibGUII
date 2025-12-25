#pragma once

#include "CTextBoard_ECSRO.h"

// CIFWnd_ECSRO - ECSRO-specific layout
// Based on IDA analysis of constructor at 0x004449C0
// Size: 0x02B4 (692 bytes)
// VTable: 0x0093E0B0

class CIFWnd_ECSRO {
public:
    // Raw padding - includes CGWnd + CTextBoard at offset 0x6C
    char pad_0000[0x2B4];  // 0x0000 - full CIFWnd size

public:
    CIFWnd_ECSRO() {
        reinterpret_cast<void(__thiscall*)(CIFWnd_ECSRO*)>(0x004449C0)(this);
    }
    
    virtual void OnUpdate() {
        reinterpret_cast<void(__thiscall*)(void*)>(0x004456B0)(this);
    }
    
    virtual void RenderMyself() {
        reinterpret_cast<void(__thiscall*)(void*)>(0x004456D0)(this);
    }
    
    virtual void OnCIFReady() {
        reinterpret_cast<void(__thiscall*)(void*)>(0x00445A70)(this);
    }
    
    virtual void ShowGWnd(bool bVisible) {
        reinterpret_cast<void(__thiscall*)(void*, bool)>(0x00446330)(this, bVisible);
    }
    
    virtual void BringToFront() {
        reinterpret_cast<void(__thiscall*)(void*)>(0x0089F190)(this);
    }
    
}; // Size: 0x02B4 (692 bytes)
