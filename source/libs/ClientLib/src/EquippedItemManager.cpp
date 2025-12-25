/**
 * @file EquippedItemManager.cpp
 * @brief Equipped Item Detection for ECSRO
 * @note Offsets reverse engineered from ECSRO client via IDA (sub_453D90)
 */

#include "EquippedItemManager.h"
#include "GInterface.h"
#include "IFMainPopup.h"
#include "IFEquipment.h"
#include "SOItem.h"
#include <cstdio>

// Slot offset lookup table
static const int s_slotOffsets[EQUIP_SLOT_COUNT] = {
    EQUIP_OFFSET_HELM,       // 0x43C
    EQUIP_OFFSET_MAIL,       // 0x440
    EQUIP_OFFSET_SHOULDER,   // 0x444
    EQUIP_OFFSET_GAUNTLET,   // 0x448
    EQUIP_OFFSET_PANTS,      // 0x44C
    EQUIP_OFFSET_BOOTS,      // 0x450
    EQUIP_OFFSET_WEAPON,     // 0x454
    EQUIP_OFFSET_SHIELD,     // 0x458
    EQUIP_OFFSET_SPECIAL,    // 0x45C
    EQUIP_OFFSET_EARRING,    // 0x460
    EQUIP_OFFSET_NECKLACE,   // 0x464
    EQUIP_OFFSET_RING_LEFT,  // 0x468
    EQUIP_OFFSET_RING_RIGHT  // 0x46C
};

// Slot names for logging
static const char* s_slotNames[EQUIP_SLOT_COUNT] = {
    "Helm", "Mail", "Shoulder", "Gauntlet", "Pants", "Boots",
    "Weapon", "Shield", "Special", "Earring", "Necklace", "L.Ring", "R.Ring"
};

CIFEquipment* EquippedItemManager::GetEquipmentWindow() {
    if (!g_pCGInterface) return NULL;
    CIFMainPopup* mainPopup = g_pCGInterface->GetMainPopup();
    if (!mainPopup) return NULL;
    return mainPopup->GetEquipment();
}

int EquippedItemManager::GetSlotOffset(int slot) {
    if (slot < 0 || slot >= EQUIP_SLOT_COUNT) return 0;
    return s_slotOffsets[slot];
}

CSOItem* EquippedItemManager::GetEquippedItem(int slot) {
    CIFEquipment* equipment = GetEquipmentWindow();
    if (!equipment) return NULL;
    
    // Get slot array at CIFEquipment+0x1110
    // This is an array of 13 CIFSlotWithHelp* pointers
    DWORD* slotArray = *(DWORD**)((DWORD_PTR)equipment + EQUIP_SLOT_ARRAY_OFFSET);
    if (!slotArray) return NULL;
    
    if (slot < 0 || slot >= EQUIP_SLOT_COUNT) return NULL;
    
    // Get the slot object
    DWORD slotObj = slotArray[slot];
    if (!slotObj) return NULL;
    
    // CIFSlotWithHelp has m_pObject at +0x2C4 (from earlier IDA analysis: sub_5402E0)
    CSOItem* pItem = *(CSOItem**)((DWORD_PTR)slotObj + 0x2C4);
    return pItem;
}

EquippedItemInfo EquippedItemManager::GetEquippedItemInfo(int slot) {
    EquippedItemInfo info;
    memset(&info, 0, sizeof(info));
    info.slot = slot;
    info.isValid = false;
    
    CSOItem* pItem = GetEquippedItem(slot);
    if (!pItem) return info;
    
    // Check if item is valid (m_refObjItemId != 0)
    if (pItem->m_refObjItemId == 0) return info;
    
    info.isValid = true;
    info.itemTID = pItem->m_refObjItemId;
    info.currDurability = pItem->m_CurrDurability;
    info.maxDurability = pItem->m_MaxDurability;
    info.optLevel = pItem->m_OptLevel;
    info.phyAtkMin = pItem->m_PhyAtkPwrMin;
    info.phyAtkMax = pItem->m_PhyAtkPwrMax;
    info.magAtkMin = pItem->m_MagAtkPwrMin;
    info.magAtkMax = pItem->m_MagAtkPwrMax;
    info.phyDef = pItem->m_PhyDefPwrValue;
    info.magDef = pItem->m_MagDefPwrValue;
    
    return info;
}

int EquippedItemManager::GetDurability(int slot) {
    CSOItem* pItem = GetEquippedItem(slot);
    if (!pItem || pItem->m_refObjItemId == 0) return -1;
    return pItem->m_CurrDurability;
}

int EquippedItemManager::GetMaxDurability(int slot) {
    CSOItem* pItem = GetEquippedItem(slot);
    if (!pItem || pItem->m_refObjItemId == 0) return -1;
    return pItem->m_MaxDurability;
}

float EquippedItemManager::GetDurabilityPercent(int slot) {
    CSOItem* pItem = GetEquippedItem(slot);
    if (!pItem || pItem->m_refObjItemId == 0) return -1.0f;
    
    int max = pItem->m_MaxDurability;
    if (max <= 0) return -1.0f;
    
    return (float)pItem->m_CurrDurability / (float)max * 100.0f;
}

bool EquippedItemManager::HasLowDurabilityEquipment() {
    // Check armor and weapon slots (0-7), skip accessories (8-12)
    for (int i = 0; i <= EQUIP_SLOT_SHIELD; i++) {
        float percent = GetDurabilityPercent(i);
        if (percent >= 0.0f && percent < DURABILITY_LOW_PERCENT) {
            return true;
        }
    }
    return false;
}

void EquippedItemManager::LogAllEquippedItems() {
    printf("\n[EquippedItemManager] === ALL EQUIPPED ITEMS (ECSRO) ===\n");
    
    // Debug: Check each step
    if (!g_pCGInterface) {
        printf("[EquippedItemManager] ERROR: g_pCGInterface is NULL\n");
        fflush(stdout);
        return;
    }
    
    CIFMainPopup* mainPopup = g_pCGInterface->GetMainPopup();
    if (!mainPopup) {
        printf("[EquippedItemManager] ERROR: MainPopup is NULL\n");
        fflush(stdout);
        return;
    }
    
    CIFEquipment* equipment = mainPopup->GetEquipment();
    if (!equipment) {
        printf("[EquippedItemManager] ERROR: Equipment window is NULL\n");
        fflush(stdout);
        return;
    }
    printf("[EquippedItemManager] Equipment window: 0x%08X\n", (DWORD)equipment);
    
    // Try slot array access
    DWORD* slotArray = *(DWORD**)((DWORD_PTR)equipment + EQUIP_SLOT_ARRAY_OFFSET);
    printf("[EquippedItemManager] SlotArray (at +0x1110): 0x%08X\n", (DWORD)slotArray);
    
    if (!slotArray) {
        printf("[EquippedItemManager] ERROR: SlotArray is NULL - trying direct offsets\n");
        
        // Try direct slot offsets instead
        for (int i = 0; i < EQUIP_SLOT_COUNT; i++) {
            void* slotPtr = *(void**)((DWORD_PTR)equipment + s_slotOffsets[i]);
            printf("[EquippedItemManager] Slot[%d] %s offset 0x%X -> 0x%08X\n", 
                   i, s_slotNames[i], s_slotOffsets[i], (DWORD)slotPtr);
        }
        fflush(stdout);
        return;
    }
    
    for (int i = 0; i < EQUIP_SLOT_COUNT; i++) {
        CSOItem* pItem = GetEquippedItem(i);
        if (pItem && pItem->m_refObjItemId != 0) {
            printf("\n[%d] %s: TID=%d, +%d\n", i, s_slotNames[i], pItem->m_refObjItemId, pItem->m_OptLevel);
            printf("    Durability: %d / %d\n", pItem->m_CurrDurability, pItem->m_MaxDurability);
            printf("    PhyAtk: %d ~ %d, MagAtk: %d ~ %d\n", 
                   pItem->m_PhyAtkPwrMin, pItem->m_PhyAtkPwrMax,
                   pItem->m_MagAtkPwrMin, pItem->m_MagAtkPwrMax);
            printf("    PhyDef: %.1f, MagDef: %.1f\n", pItem->m_PhyDefPwrValue, pItem->m_MagDefPwrValue);
            printf("    Critical: %d, Parry: %.1f, Block: %d\n", 
                   pItem->m_CriticalValue, pItem->m_ParryRateValue, pItem->m_BlockingRateValue);
            printf("    Evasion: %d, HitRate: %d\n", pItem->m_EvasionRateValue, pItem->m_AttackRateValue);
            printf("    PhyReinf: %.1f%% ~ %.1f%%, MagReinf: %.1f%% ~ %.1f%%\n",
                   pItem->m_PhyReinforcementMin * 100.0f, pItem->m_PhyReinforcementMax * 100.0f,
                   pItem->m_MagReinforcementMin * 100.0f, pItem->m_MagReinforcementMax * 100.0f);
            printf("    PhyDefSpec: %.1f%%, MagDefSpec: %.1f%%, MagAbsorb: %.1f\n",
                   pItem->m_PhyDefSpecialize * 100.0f, pItem->m_MagDefSpecialize * 100.0f, pItem->m_MagAbsorption);
        } else {
            printf("[%d] %s: (empty)\n", i, s_slotNames[i]);
        }
    }
    
    printf("\n=====================================\n\n");
    fflush(stdout);
}

void EquippedItemManager::LogEquipmentDurability() {
    printf("[EquippedItemManager] === DURABILITY STATUS ===\n");
    
    bool hasLow = false;
    for (int i = 0; i <= EQUIP_SLOT_SHIELD; i++) {
        float percent = GetDurabilityPercent(i);
        if (percent >= 0.0f) {
            const char* status = "";
            if (percent < DURABILITY_LOW_PERCENT) {
                status = " [LOW!]";
                hasLow = true;
            } else if (percent < DURABILITY_REPAIR_PERCENT) {
                status = " [WARN]";
            }
            printf("  %s: %.1f%%%s\n", s_slotNames[i], percent, status);
        }
    }
    
    if (hasLow) {
        printf("  >>> LOW DURABILITY DETECTED! <<<\n");
    }
    printf("=====================================\n");
    fflush(stdout);
}
