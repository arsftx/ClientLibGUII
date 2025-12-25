#include "IFSlot.h"
#include <cstdio>
#include <cstdarg>

// Link to native runtime class at 0x9FE4C0
GFX_IMPLEMENT_DYNAMIC_EXISTING(CIFSlot, 0x9FE4C0)

// Debug logging helper
static void SlotDebugLog(const char* format, ...) {
    va_list args;
    va_start(args, format);
    char buffer[512];
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    FILE* f = fopen("clientlog.txt", "a");
    if (f) {
        fprintf(f, "[CIFSlot] %s\n", buffer);
        fclose(f);
    }
}

CIFSlot::CIFSlot() {
    // Native constructor (sub_4411B0) handles:
    // - Call parent constructor (sub_4449C0)
    // - Set main vtable at offset 0 = 0x93D9C8
    // - Set secondary vtable at offset 0x6C = 0x93D980
}

CIFSlot::~CIFSlot() {
}

// Set slot index (sub_5425E0)
void CIFSlot::SetSlotIndex(int index) {
    reinterpret_cast<void (__thiscall *)(CIFSlot *, int)>(0x005425E0)(this, index);
}

// Get slot index - stored at some offset within the class
int CIFSlot::GetSlotIndex() const {
    // This will be properly implemented when we know the exact offset
    // For CIFSlotWithHelp it's at 0x494, but CIFSlot may differ
    return -1;
}

// Set slot type (sub_5425A0)
// Type values: 0x46 = Inventory, 0x0C = QuickSlot, 0x50 = Skill
void CIFSlot::SetSlotType(int type) {
    reinterpret_cast<void (__thiscall *)(CIFSlot *, int)>(0x005425A0)(this, type);
}

// Enable/Disable slot (sub_542B40)
void CIFSlot::SetSlotEnabled(bool enabled) {
    reinterpret_cast<void (__thiscall *)(CIFSlot *, int)>(0x00542B40)(this, enabled ? 1 : 0);
}

bool CIFSlot::IsSlotEnabled() const {
    // Will need proper offset once analyzed
    return true;
}

// Init slot index 2 (sub_542B00)
void CIFSlot::InitSlotIndex2(int index) {
    reinterpret_cast<void (__thiscall *)(CIFSlot *, int)>(0x00542B00)(this, index);
}

// Init slot param (sub_542750)
void CIFSlot::InitSlotParam(int param) {
    reinterpret_cast<void (__thiscall *)(CIFSlot *, int)>(0x00542750)(this, param);
}

// Init render data (sub_445260) - copies 16-byte rect to offset 0x130
void CIFSlot::InitRenderData(int left, int top, int right, int bottom) {
    struct RectData {
        int left;
        int top;
        int right;
        int bottom;
    } rectData = { left, top, right, bottom };
    
    reinterpret_cast<void (__thiscall *)(CIFSlot *, void*)>(0x00445260)(this, &rectData);
}
