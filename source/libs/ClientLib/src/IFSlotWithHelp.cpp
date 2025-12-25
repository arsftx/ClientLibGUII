
#include "IFSlotWithHelp.h"
#include <cstdio>

GFX_IMPLEMENT_DYNAMIC_EXISTING(CIFSlotWithHelp, 0x009FFD04) // ECSRO

// =====================================================================
// CIFSlotWithHelp-specific implementations
// =====================================================================

void CIFSlotWithHelp::SetSlotData(CSOItem *pItemSocket) {
    reinterpret_cast<void (__thiscall *)(CIFSlotWithHelp *, CSOItem *)>(0x006871d0)(this, pItemSocket);
}

BYTE CIFSlotWithHelp::GetSlotItemCheck() const {
    return MEMUTIL_READ_BY_PTR_OFFSET(this, 0x2F4, BYTE);
}

int CIFSlotWithHelp::GetSlotItemID() const {
    return MEMUTIL_READ_BY_PTR_OFFSET(this, 0x498, int);
}

void CIFSlotWithHelp::SetSlotItemID(int value) {
    MEMUTIL_WRITE_BY_PTR_OFFSET(this, 0x498, int, value);
}

int CIFSlotWithHelp::GetSlotIndex() const {
    return MEMUTIL_READ_BY_PTR_OFFSET(this, 0x494, int);
}

void CIFSlotWithHelp::SetSlotSize(int width, int height) {
    MEMUTIL_WRITE_BY_PTR_OFFSET(this, 0x3C0, int, width);
    MEMUTIL_WRITE_BY_PTR_OFFSET(this, 0x3C4, int, height);
}

void CIFSlotWithHelp::InitDropSupport() {
    // Set Icon sub-object VTable at +0x6C
    DWORD* vtablePtr = (DWORD*)((char*)this + 0x6C);
    if (*vtablePtr == 0) {
        *vtablePtr = 0x94AEB8;
    }
    
    // Initialize vector at +0x3CC (drop data buffer)
    DWORD* vecPtr = (DWORD*)((char*)this + 0x3CC);
    if (vecPtr[0] == 0) {
        typedef void* (*Alloc_t)(size_t);
        static Alloc_t fnAlloc = (Alloc_t)0x4064A0;
        void* buffer = fnAlloc(8);
        
        if (buffer) {
            vecPtr[0] = (DWORD)buffer;
            vecPtr[1] = (DWORD)buffer;
            vecPtr[2] = (DWORD)buffer + 8;
        }
    }
    
    // Set default size to 32x32
    if (MEMUTIL_READ_BY_PTR_OFFSET(this, 0x3C0, int) == 0) {
        MEMUTIL_WRITE_BY_PTR_OFFSET(this, 0x3C0, int, 32);
        MEMUTIL_WRITE_BY_PTR_OFFSET(this, 0x3C4, int, 32);
    }
    MEMUTIL_WRITE_BY_PTR_OFFSET(this, 0x3C8, BYTE, 1);
    
    // Set drop flags at +0x12C
    DWORD oldFlags = MEMUTIL_READ_BY_PTR_OFFSET(this, 0x12C, DWORD);
    DWORD newFlags = (oldFlags & ~0x20) | 0x02;
    MEMUTIL_WRITE_BY_PTR_OFFSET(this, 0x12C, DWORD, newFlags);
}

bool CIFSlotWithHelp::SetIcon(const char* ddjPath) {
    if (!ddjPath || ddjPath[0] == '\0') return false;
    
    typedef char (__thiscall *SetSlotIcon_t)(void*, const char*);
    static SetSlotIcon_t fnSetIcon = (SetSlotIcon_t)0x4452F0;
    
    return fnSetIcon(this, ddjPath) != 0;
}

const char* CIFSlotWithHelp::GetIconPath() const {
    DWORD* vecPtr = (DWORD*)((char*)this + 0x140);
    if (vecPtr[0] == 0 || vecPtr[0] == vecPtr[1]) return NULL;
    return (const char*)vecPtr[0];
}

void CIFSlotWithHelp::ClearIcon() {
    typedef void (__thiscall *ClearSlotIcon_t)(void*, const char*);
    static ClearSlotIcon_t fnClear = (ClearSlotIcon_t)0x446C70;
    
    void* iconSubObj = (void*)((char*)this + 0x6C);
    fnClear(iconSubObj, NULL);
}