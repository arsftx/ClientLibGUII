/**
 * @file EquippedItemManager.h
 * @brief Equipped Item Detection for ECSRO
 * @note Offsets reverse engineered from ECSRO client via IDA (sub_453D90)
 */

#pragma once

#include <Windows.h>

// Forward declaration
class CSOItem;
class CIFEquipment;

// Equipment Slot Indices (ECSRO)
enum EquipmentSlot {
    EQUIP_SLOT_HELM         = 0,
    EQUIP_SLOT_MAIL         = 1,   // Chest armor
    EQUIP_SLOT_SHOULDER     = 2,
    EQUIP_SLOT_GAUNTLET     = 3,
    EQUIP_SLOT_PANTS        = 4,
    EQUIP_SLOT_BOOTS        = 5,
    EQUIP_SLOT_WEAPON       = 6,
    EQUIP_SLOT_SHIELD       = 7,
    EQUIP_SLOT_SPECIAL      = 8,   // Special dress/costume
    EQUIP_SLOT_EARRING      = 9,
    EQUIP_SLOT_NECKLACE     = 10,
    EQUIP_SLOT_RING_LEFT    = 11,
    EQUIP_SLOT_RING_RIGHT   = 12,
    EQUIP_SLOT_COUNT        = 13
};

// Equipment slot offsets within CIFEquipment (ECSRO - reversed from IDA sub_453D90)
// These are CIFSlotWithHelp* pointers, not CSOItem* directly
#define EQUIP_OFFSET_HELM           0x43C
#define EQUIP_OFFSET_MAIL           0x440
#define EQUIP_OFFSET_SHOULDER       0x444
#define EQUIP_OFFSET_GAUNTLET       0x448
#define EQUIP_OFFSET_PANTS          0x44C
#define EQUIP_OFFSET_BOOTS          0x450
#define EQUIP_OFFSET_WEAPON         0x454
#define EQUIP_OFFSET_SHIELD         0x458
#define EQUIP_OFFSET_SPECIAL        0x45C
#define EQUIP_OFFSET_EARRING        0x460
#define EQUIP_OFFSET_NECKLACE       0x464
#define EQUIP_OFFSET_RING_LEFT      0x468
#define EQUIP_OFFSET_RING_RIGHT     0x46C

// Equipment slot array at CIFEquipment+0x1110 (13 slots)
#define EQUIP_SLOT_ARRAY_OFFSET     0x1110

// Durability thresholds
#define DURABILITY_LOW_PERCENT      20    // Return to town at 20%
#define DURABILITY_REPAIR_PERCENT   50    // Use repair hammer at 50%

/**
 * @brief Equipment item info structure
 */
struct EquippedItemInfo {
    int slot;               // Equipment slot (EquipmentSlot enum)
    DWORD itemTID;          // Item TypeID (m_refObjItemId)
    int currDurability;     // Current durability
    int maxDurability;      // Max durability
    int optLevel;           // Enhancement level (+1, +2, etc.)
    int phyAtkMin;          // Physical attack min
    int phyAtkMax;          // Physical attack max
    int magAtkMin;          // Magic attack min
    int magAtkMax;          // Magic attack max
    float phyDef;           // Physical defense
    float magDef;           // Magic defense
    bool isValid;           // True if item exists in slot
};

/**
 * @brief Manager class for detecting equipped items and their properties
 * @note ECSRO-specific implementation
 */
class EquippedItemManager {
public:
    // Get equipped item raw pointer by slot
    static CSOItem* GetEquippedItem(int slot);
    
    // Get full item info for a slot
    static EquippedItemInfo GetEquippedItemInfo(int slot);
    
    // Convenience getters for weapon
    static CSOItem* GetWeapon() { return GetEquippedItem(EQUIP_SLOT_WEAPON); }
    static CSOItem* GetShield() { return GetEquippedItem(EQUIP_SLOT_SHIELD); }
    
    // Durability helpers (returns -1 if no item)
    static int GetDurability(int slot);
    static int GetMaxDurability(int slot);
    static float GetDurabilityPercent(int slot);  // 0.0 - 100.0
    
    // Check if any equipment is low durability (excluding accessories)
    static bool HasLowDurabilityEquipment();
    
    // Debug logging
    static void LogAllEquippedItems();
    static void LogEquipmentDurability();
    
private:
    // Get slot offset within CIFEquipment
    static int GetSlotOffset(int slot);
    
    // Get CIFEquipment pointer
    static CIFEquipment* GetEquipmentWindow();
};
