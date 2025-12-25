/**
 * @file IFEquipment.h
 * @brief Equipment window class - ECSRO
 * @note Offsets reverse engineered from ECSRO client via IDA (sub_453D90)
 */

#pragma once

#include "IFWnd.h"

// Equipment slot offsets within CIFEquipment (ECSRO)
// These point to CIFSlotWithHelp* for each slot
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

// Slot array containing all 13 equipment slots
#define EQUIP_SLOT_ARRAY_OFFSET     0x1110

class CIFEquipment : public CIFWnd
{
public:
    // 0x036C to 0x43C = padding before first slot
    char pad_0x036C[0xD0]; // 0x036C to 0x43C
    
    // Equipment slot pointers (CIFSlotWithHelp*)
    void* m_pSlotHelm;       // 0x43C
    void* m_pSlotMail;       // 0x440
    void* m_pSlotShoulder;   // 0x444
    void* m_pSlotGauntlet;   // 0x448
    void* m_pSlotPants;      // 0x44C
    void* m_pSlotBoots;      // 0x450
    void* m_pSlotWeapon;     // 0x454
    void* m_pSlotShield;     // 0x458
    void* m_pSlotSpecial;    // 0x45C
    void* m_pSlotEarring;    // 0x460
    void* m_pSlotNecklace;   // 0x464
    void* m_pSlotRingLeft;   // 0x468
    void* m_pSlotRingRight;  // 0x46C
    
    // Padding to slot array
    char pad_0x0470[0xCA0]; // 0x470 to 0x1110
    
    // Array of 13 slot pointers (same slots as above, different access pattern)
    void** m_pSlotArray;     // 0x1110
    
    // Remaining padding
    char pad_0x1114[0x15AC]; // 0x1114 to 0x26C0

}; // Size=0x26C0