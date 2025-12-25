#pragma once

#include "IFSlot.h"
#include "SOItem.h"

// =====================================================================
// CIFSlotWithHelp - Slot window with help tooltip
// =====================================================================
// IDA Analysis (ECSRO):
//   RuntimeClass: 0x9FFD04
//   Size: 0x4B8 bytes (+0x204 bytes from CIFSlot)
//   Parent: CIFSlot (0x9FE4C0)
//   Constructor: sub_53DAF0
// =====================================================================
// Hierarchy:
//   CIFWnd (0x9FE5C0) - 0x2B4 bytes
//     └── CIFSlot (0x9FE4C0) - 0x2B4 bytes
//           └── CIFSlotWithHelp (0x9FFD04) - 0x4B8 bytes
// =====================================================================
// Used by: CIFInventory, CIFUnderBar for actual draggable slots
// NOT the same as CIFLattice (which is just visual grid background)
// =====================================================================

class CIFSlotWithHelp : public CIFSlot {
    GFX_DECLARE_DYNAMIC_EXISTING(CIFSlotWithHelp, 0x009FFD04) // ECSRO


public:
    // === CIFSlotWithHelp-specific functions ===
    
    // Item data functions (sub_6871D0)
    void SetSlotData(CSOItem *pItemSocket);
    
    // Item check byte at offset 0x2F4
    BYTE GetSlotItemCheck() const;
    
    // Item ID at offset 0x498
    int GetSlotItemID() const;
    void SetSlotItemID(int value);

    // Override GetSlotIndex to use proper offset (0x494)
    int GetSlotIndex() const;

    // Slot dimensions (defaults: 64x64 from constructor)
    int GetSlotWidth() const { return m_slotWidth; }
    int GetSlotHeight() const { return m_slotHeight; }
    void SetSlotSize(int width, int height);
    
    // Initialize drag-drop support
    // This sets up +0x6C icon VTable and +0x3CC vector that are missing
    // when using CGWnd::CreateInstance instead of IRM
    void InitDropSupport();
    
    // Set slot icon by DDJ path (calls native sub_4452F0)
    // Returns true on success, false on failure
    bool SetIcon(const char* ddjPath);
    
    // Get current icon DDJ path from +0x140 buffer
    const char* GetIconPath() const;
    
    // Clear the slot icon
    void ClearIcon();
    
    // Note: SetSlotIndex, SetSlotType, SetSlotEnabled, InitSlotIndex2, 
    // InitSlotParam, InitRenderData are inherited from CIFSlot


private:
    // CIFSlot base: 0x000 - 0x2B4 (size 0x2B4)
    // Secondary vtable at 0x6C for icon sub-object
    
    // Padding from CIFSlot end (0x2B4) to first CIFSlotWithHelp member
    char pad_02B4[0x10C];      // 0x2B4 - 0x3C0 (268 bytes)

    int m_slotWidth;          // 0x3C0 - default 0x40 (64)
    int m_slotHeight;         // 0x3C4 - default 0x40 (64)
    BYTE m_flag3C8;           // 0x3C8 - default 1
    char pad_03C9[0x3];       // 0x3C9 - 0x3CC
    
    // Vector/list at 0x3CC (12 bytes)
    void* m_vectorBegin;      // 0x3CC
    void* m_vectorEnd;        // 0x3D0
    void* m_vectorCapacity;   // 0x3D4
    
    BYTE m_flag3D8;           // 0x3D8
    BYTE m_flag3D9;           // 0x3D9
    BYTE m_flag3DA;           // 0x3DA
    char pad_03DB;            // 0x3DB
    int m_field3DC;           // 0x3DC - default -1
    BYTE m_flag3E0;           // 0x3E0
    
    char pad_03E1[0xAB];      // 0x3E1 - 0x48C

    int m_field48C;           // 0x48C
    BYTE m_flag490;           // 0x490
    char pad_0491[0x3];       // 0x491 - 0x494
    int m_slotIndex;          // 0x494 - slot index (default -1)
    int m_itemID;             // 0x498 - item ID (default -1)
    int m_field49C;           // 0x49C
    int m_field4A0;           // 0x4A0
    int m_field4A4;           // 0x4A4
    int m_field4A8;           // 0x4A8
    int m_field4AC;           // 0x4AC
    BYTE m_flag4B0;           // 0x4B0
    BYTE m_flag4B1;           // 0x4B1 - default 1
    char pad_04B2[0x6];       // 0x4B2 - 0x4B8

    // Size: 0x4B8
};