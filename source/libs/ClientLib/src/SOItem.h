#pragma once

#include "Data/ItemData.h"
#include <BSLib/BSLib.h>
#include <ghidra/undefined.h>

// Magic Attribute Types (Blue options) - ECSRO
// These are stored in the magic attribute list at CSOItem + 0x84
enum Blue : short {
    BLUE_STR = 0x000b,
    BLUE_INT = 0x0005,
    BLUE_DURABILITY = 0x0050,
    BLUE_LUCKY = 0x003d,
    BLUE_LUCKY2 = 0x003b,
    BLUE_MP = 0x0098,
    BLUE_HP = 0x008C,
    BLUE_PARRY = 0x0080,
    BLUE_REPAIR_INVALID = 0x0041,
    BLUE_STEADY = 0x0035
};

// ============================================================
// CSOItem - ECSRO Structure (Verified from IDA sub_4C3300)
// 
// Key accessor functions:
//   sub_5F5E40: returns this + 0x90 (Stats structure base)
//   sub_5F7640: returns this + 0x84 (Magic attribute list)
// ============================================================
class CSOItem {
public:
    /// \address 008ba440
    const SItemData *GetItemData() const;

    /// \address 008ba460
    int GetQuantity() const;

public:
    // ============== BASE FIELDS (0x00 - 0x83) ==============
    char pad_0000[0x28];              // 0x0000 - 0x0027
    undefined1 field_28;              // 0x0028 - Used by GInterface for validity check
    char pad_0029[0x3];               // 0x0029 - 0x002B
    int m_refObjItemId;               // 0x002C - Item Type ID (TID)
    char pad_0030[0x38];              // 0x0030 - 0x0067
    int m_CurrDurability;             // 0x0068 - Current Durability (verified: sub_5F5E30 returns [ecx+68h])
    int m_itemQuantity;               // 0x006C - Stack count (arrows, potions etc.)
    char pad_0070[0x14];              // 0x0070 - 0x0083
    
    // ============== MAGIC ATTRIBUTE LIST (0x84) ==============
    // Blue options (STR, INT, HP, MP, etc.) stored here
    // Accessed via sub_5F7640: lea eax, [ecx+84h]
    void* m_MagicAttrList;            // 0x0084 - Pointer to magic attributes list
    char pad_0088[0x4];               // 0x0088 - 0x008B
    BYTE m_OptLevel;                  // 0x008C - Enhancement level (+1, +2, etc.)
    char pad_008D[0x3];               // 0x008D - 0x008F
    
    // ============== STATS STRUCTURE (0x90+) ==============
    // Accessed via sub_5F5E40: lea eax, [ecx+90h]
    // All offsets below verified from sub_4C3300 IDA analysis
    
    int m_PhyAtkPwrMax;               // 0x0090 (stats+0x00) - NATTR_PA Max (game shows as first value)
    int m_PhyAtkPwrMin;               // 0x0094 (stats+0x04) - NATTR_PA Min (game shows as second value)
    int m_MagAtkPwrMax;               // 0x0098 (stats+0x08) - NATTR_MA Max
    int m_MagAtkPwrMin;               // 0x009C (stats+0x0C) - NATTR_MA Min
    int m_BlockingRateValue;          // 0x00A0 (stats+0x10) - NATTR_BR
    int m_MaxDurability;              // 0x00A4 (stats+0x14) - NATTR_DUR (Max Durability)
    float m_PhyDefPwrValue;           // 0x00A8 (stats+0x18) - NATTR_PD
    char pad_00AC[0x4];               // 0x00AC (stats+0x1C) - Unknown
    float m_ParryRateValue;           // 0x00B0 (stats+0x20) - NATTR_PAR
    char pad_00B4[0x4];               // 0x00B4 (stats+0x24) - Unknown
    int m_CriticalValue;              // 0x00B8 (stats+0x28) - NATTR_CRITICAL
    float m_MagDefPwrValue;           // 0x00BC (stats+0x2C) - NATTR_MD
    float m_MagAbsorption;            // 0x00C0 (stats+0x30) - NATTR_MAR
    float m_PhyReinforcementMin;      // 0x00C4 (stats+0x34) - Physical Reinforce Min %
    float m_PhyReinforcementMax;      // 0x00C8 (stats+0x38) - Physical Reinforce Max %
    float m_MagReinforcementMin;      // 0x00CC (stats+0x3C) - Magic Reinforce Min %
    float m_MagReinforcementMax;      // 0x00D0 (stats+0x40) - Magic Reinforce Max %
    float m_PhyDefSpecialize;         // 0x00D4 (stats+0x44) - NATTR_PDSTR
    float m_MagDefSpecialize;         // 0x00D8 (stats+0x48) - NATTR_MDINT
    int m_EvasionRateValue;           // 0x00DC (stats+0x4C) - NATTR_ER
    int m_AttackRateValue;            // 0x00E0 (stats+0x50) - NATTR_HR
    
    char pad_00E4[0x8];               // 0x00E4 - 0x00EB (padding for struct alignment)

    // FIXTURE CHECKS - ECSRO offsets verified from IDA
    BEGIN_FIXTURE()
        // Base fields
        ENSURE_OFFSET(m_refObjItemId, 0x2C)
        ENSURE_OFFSET(m_CurrDurability, 0x0068)
        ENSURE_OFFSET(m_itemQuantity, 0x006C)
        ENSURE_OFFSET(m_MagicAttrList, 0x0084)
        ENSURE_OFFSET(m_OptLevel, 0x008C)
        
        // Stats structure (base 0x90)
        ENSURE_OFFSET(m_PhyAtkPwrMax, 0x0090)
        ENSURE_OFFSET(m_PhyAtkPwrMin, 0x0094)
        ENSURE_OFFSET(m_MagAtkPwrMax, 0x0098)
        ENSURE_OFFSET(m_MagAtkPwrMin, 0x009C)
        ENSURE_OFFSET(m_BlockingRateValue, 0x00A0)
        ENSURE_OFFSET(m_MaxDurability, 0x00A4)
        ENSURE_OFFSET(m_PhyDefPwrValue, 0x00A8)
        ENSURE_OFFSET(m_ParryRateValue, 0x00B0)
        ENSURE_OFFSET(m_CriticalValue, 0x00B8)
        ENSURE_OFFSET(m_MagDefPwrValue, 0x00BC)
        ENSURE_OFFSET(m_MagAbsorption, 0x00C0)
        ENSURE_OFFSET(m_PhyReinforcementMin, 0x00C4)
        ENSURE_OFFSET(m_PhyReinforcementMax, 0x00C8)
        ENSURE_OFFSET(m_MagReinforcementMin, 0x00CC)
        ENSURE_OFFSET(m_MagReinforcementMax, 0x00D0)
        ENSURE_OFFSET(m_PhyDefSpecialize, 0x00D4)
        ENSURE_OFFSET(m_MagDefSpecialize, 0x00D8)
        ENSURE_OFFSET(m_EvasionRateValue, 0x00DC)
        ENSURE_OFFSET(m_AttackRateValue, 0x00E0)
    END_FIXTURE()

    RUN_FIXTURE(CSOItem)
};
