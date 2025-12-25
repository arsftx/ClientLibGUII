#pragma once

#include "IFWnd.h"

// =====================================================================
// CIFSlot - Base class for slot windows (inventory, quickslot, etc.)
// =====================================================================
// IDA Analysis (ECSRO):
//   RuntimeClass: 0x9FE4C0
//   Size: 0x2B4 (same as CIFWnd - no extra members, just vtable override)
//   Parent: CIFWnd (0x9FE5C0)
//   Constructor: sub_4411B0
//   Main VTable: 0x93D9C8
//   Secondary VTable (0x6C): 0x93D980 - Icon/Tile sub-object
// =====================================================================
// Hierarchy:
//   CIFWnd (0x9FE5C0) - 0x2B4 bytes
//     └── CIFSlot (0x9FE4C0) - 0x2B4 bytes
//           └── CIFSlotWithHelp (0x9FFD04) - 0x4B8 bytes
// =====================================================================

class CIFSlot : public CIFWnd {
    GFX_DECLARE_DYNAMIC_EXISTING(CIFSlot, 0x9FE4C0)

public:
    CIFSlot();
    virtual ~CIFSlot();

    // Slot index functions (sub_5425E0)
    void SetSlotIndex(int index);
    int GetSlotIndex() const;

    // Slot type (sub_5425A0) - 0x46 for inventory, 0x0C for quickslot
    void SetSlotType(int type);

    // Enable/Disable slot (sub_542B40)
    void SetSlotEnabled(bool enabled);
    bool IsSlotEnabled() const;

    // Init functions used by inventory
    void InitSlotIndex2(int index);   // sub_542B00
    void InitSlotParam(int param);    // sub_542750

    // Render rect init (sub_445260) - copies rect to offset 0x130
    void InitRenderData(int left, int top, int right, int bottom);

    // Get icon sub-object at offset 0x6C
    void* GetIconSubObject() const { return (void*)((char*)this + 0x6C); }

    BEGIN_FIXTURE()
        ENSURE_SIZE(0x2B4)
    END_FIXTURE()

    RUN_FIXTURE(CIFSlot)
};
