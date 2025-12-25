/**
 * @file CIItem.h
 * @brief Dropped Item Entity - ECSRO Version
 * 
 * Represents a dropped item in the game world.
 * Size: 0x1E0 (480 bytes) in ECSRO
 * 
 * Runtime Class: 0x00A0454C
 * Constructor chain: sub_660E70 -> sub_665310 -> sub_666DB0
 */

#pragma once
#include <Windows.h>

// ============================================
// ECSRO CIItem Offsets (from IDA analysis)
// ============================================

// ECSRO CIItem - Dropped item entity
// Size: 0x1E0 (480 bytes)
class CIItem {
public:
    // +0x00: vtable (off_94EEE8 / off_94EC30)
    DWORD vtable;                    // 0x0000
    
    // +0x04: Item state flags (from sub_647F80)
    // Flags: & 6 = invalid, & 1 = pickable, & 8 = gold, & 0x10 = equipment
    DWORD m_flags;                   // 0x0004
    
    char pad_0008[0x30];             // 0x0008
    
    // +0x38: Unknown DWORD
    DWORD m_unk38;                   // 0x0038
    
    char pad_003C[0x10];             // 0x003C
    
    // +0x4C: vtable2 (off_94EEE0 / off_94EC28)
    DWORD vtable2;                   // 0x004C
    
    char pad_0050[0x40];             // 0x0050
    
    // +0x90: Some object pointer
    DWORD m_pObject;                 // 0x0090
    
    char pad_0094[0x30];             // 0x0094
    
    // +0xC4: Sub-object with vtable (off_94EF58)
    char m_subObject[0x34];          // 0x00C4
    
    // +0xF8: List/array structure (NOT UniqueID like STLibrary!)
    char m_listStruct[0x0C];         // 0x00F8
    
    // +0x104: Sub-object initialized by sub_64B480
    char m_subObject2[0x5C];         // 0x0104
    
    // +0x160 - 0x16C: Various DWORD fields
    DWORD m_unk160;                  // 0x0160
    DWORD m_unk164;                  // 0x0164
    DWORD m_unk168;                  // 0x0168
    DWORD m_unk16C;                  // 0x016C
    
    char pad_0170[0x20];             // 0x0170
    
    // +0x190: float = 1000.0f
    float m_fUnk190;                 // 0x0190
    
    char pad_0194[0x08];             // 0x0194
    
    // +0x19C: DWORD = 0
    DWORD m_unk19C;                  // 0x019C
    
    char pad_01A0[0x04];             // 0x01A0
    
    // +0x1A4: BYTE = 0
    BYTE m_bUnk1A4;                  // 0x01A4
    
    char pad_01A5[0x03];             // 0x01A5
    
    // +0x1A8: DWORD = 0
    DWORD m_unk1A8;                  // 0x01A8
    
    char pad_01AC[0x14];             // 0x01AC
    
    // +0x1C0: WORD = 0
    WORD m_wUnk1C0;                  // 0x01C0
    
    char pad_01C2[0x06];             // 0x01C2
    
    // +0x1C8: DWORD = 0
    DWORD m_unk1C8;                  // 0x01C8
    
    char pad_01CC[0x04];             // 0x01CC
    
    // +0x1D0: Owner check (from sub_647F80: [esi+0x1D0])
    DWORD m_ownerCheck;              // 0x01D0
    
    // +0x1D4: List structure (12 bytes)
    char m_itemList[0x0C];           // 0x01D4
    
    // Total: 0x1E0 (480 bytes)
    
    // ============================================
    // IMPORTANT: RefObjID is at +0x160 (confirmed via debug)
    // Access via: *(DWORD*)(pCIItem + 0x160)
    // ============================================
    DWORD GetRefObjID() const { return m_unk160; }  // RefObjID = +0x160
    
public:
    // ============================================
    // FLAG DEFINITIONS (from sub_647F80)
    // ============================================
    bool IsInvalid() const { return (m_flags & 0x06) != 0; }
    bool IsPickable() const { return (m_flags & 0x01) != 0; }
    bool IsGoldItem() const { return (m_flags & 0x08) != 0; }
    bool IsEquipmentItem() const { return (m_flags & 0x10) != 0; }
    bool IsOtherItem() const { return (m_flags & 0x18) == 0; }
    bool HasOwner() const { return m_ownerCheck != 0; }
};

// Runtime class for IsSame() checks
#define GFX_RUNTIME_CLASS_CIITEM ((void*)0x00A0454C)

// IsSame function address
#define ADDR_IS_SAME_ITEM 0x00898FD0

