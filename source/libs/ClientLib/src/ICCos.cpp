/**
 * @file ICCos.cpp
 * @brief CICCos class - ECSRO Pet Monitor System
 */
#include "ICCos.h"
#include <Windows.h>
#include <cstdio>
#include <ctime>
#include "GInterface.h"
#include "IFInventory.h"
#include "IFMainPopup.h"
#include "SOItem.h"
#include "ICPlayer.h"
#include <ClientNet/MsgStreamBuffer.h>
#include "GlobalHelpersThatHaveNoHomeYet.h"
#include "IFPetAutoPotion.h"
#include "ActiveBuffManager.h"

GFX_IMPLEMENT_DYNAMIC_EXISTING(CICCos, 0x00A01DD8)
CLASSLINK_STATIC_IMPL(CICCos)

// ============================================
// Pet Potion Item IDs (ORDERED: XLarge > Large > Small)
// ============================================
// HP Potions: 9008 (XLarge), 2144 (Large), 2143 (Small)
// Cure Potions: 9010 (Large), 9009 (Small)
// HGP Potions: 7553
// ============================================
static const int PET_HP_POTIONS[] = { 9008, 2144, 2143 };  // XLarge first
static const int PET_HP_POTION_COUNT = 3;

static const int PET_CURE_POTIONS[] = { 9010, 9009 };  // Large first
static const int PET_CURE_POTION_COUNT = 2;

static const int PET_HGP_POTIONS[] = { 7553 };
static const int PET_HGP_POTION_COUNT = 1;

// ============================================
// Pet Monitor - Real-time tracking
// ============================================

// Cached pet data for change detection
struct PetCache {
    DWORD address;
    DWORD refObjID;
    DWORD uniqueID;      // Pet's unique game ID for selection
    DWORD currentHP;
    DWORD maxHP;
    WORD  hgp;
    DWORD currentEXP;
    DWORD maxEXP;
    BYTE  petType;
    DWORD status;
    bool  isValid;
};

// 4 pet slots by type: 0=Horse, 1=Job, 2=Pick, 3=Attack
#define PET_SLOT_HORSE  0
#define PET_SLOT_JOB    1
#define PET_SLOT_PICK   2
#define PET_SLOT_ATTACK 3
#define MAX_PET_SLOTS   4

static PetCache s_petCache[MAX_PET_SLOTS] = {0};
static DWORD s_startupTime = 0;
static bool s_initialized = false;

// ========== STATUS EFFECT at CICCos+0x1E8 ==========
// Found via Cheat Engine: write at 0x6534BB, read at 0x64F5FC
// 0x00 = None, 0x01 = FREEZE, 0x02 = FROSTBITE, 0x04 = BURN, etc.
// 0x80 = STUN (cure doesn't work on stun)
static const char* GetStatusNameFromByte(BYTE status) {
    switch (status) {
        case 0x01: return "FREEZE";
        case 0x02: return "FROSTBITE";
        case 0x04: return "BURN";
        case 0x08: return "ELECTRIC_SHOCK";
        case 0x10: return "POISON";
        case 0x20: return "ZOMBIE";
        case 0x40: return "DARKNESS";
        case 0x80: return "STUN";
        default: return NULL;
    }
}

// Read status effect from CICCos+0x1E8
static BYTE ReadPetStatus(DWORD cicCosAddr) {
    __try {
        if (cicCosAddr == 0 || cicCosAddr < 0x10000) return 0;
        return *(BYTE*)(cicCosAddr + 0x1E8);
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
    }
    return 0;
}

// Console output helper - DISABLED (buff monitoring uses its own PrintBuff)
static void PrintPet(const char* format, ...) {
    // Pet logging disabled - using ActiveBuffManager for buff logs only
}

// Get pet name by type
static const char* GetPetTypeName(BYTE type) {
    switch (type) {
        case 0: return "Horse";
        case 1: return "Job";       // Job Pet (trading, thief, hunter)
        case 2: return "Pick";
        case 3: return "Attack";    // Attack Pet (fighter cos)
        default: return "Unknown";
    }
}

// Print pet stats in clean format
static void PrintPetStats(PetCache* pet, const char* petName = NULL) {
    PrintPet("\n========== PET: %s ==========\n", petName ? petName : "Unknown");
    PrintPet("Type: %s | HP: %u/%u\n", 
        GetPetTypeName(pet->petType),
        pet->currentHP, pet->maxHP);
    PrintPet("EXP: %u/%u", pet->currentEXP, pet->maxEXP);
    if (pet->maxEXP > 0) {
        PrintPet(" (%.2f%%)", (double)pet->currentEXP / pet->maxEXP * 100.0);
    }
    PrintPet("\n=====================================\n");
}


// ============================================
// PET POTION INVENTORY SEARCH
// ============================================

// Find pet potion in inventory, returns slot ID or -1 if not found
// PRIORITY: Uses array order (XLarge > Large > Small), then lowest quantity within same type
static int FindPetPotionInInventory(const int* potionIDs, int potionCount) {
    if (!g_pCGInterface || !g_pCICPlayer) return -1;
    
    CIFInventory* inventory = g_pCGInterface->GetMainPopup()->GetInventory();
    if (!inventory) return -1;
    
    int invCount = *(unsigned char*)((DWORD_PTR)g_pCICPlayer + 0x13B4);
    
    // Search in priority order (first ID in array = highest priority)
    for (int j = 0; j < potionCount; j++) {
        int targetPotionID = potionIDs[j];
        int bestSlot = -1;
        int lowestQuantity = INT_MAX;
        
        // Find lowest quantity stack of this specific potion type
        for (int i = 0; i < invCount; i++) {
            CSOItem* pItem = inventory->GetItemBySlot(i);
            if (!pItem) continue;
            
            int itemID = pItem->m_refObjItemId;
            if (itemID == 0) continue;
            
            if (itemID == targetPotionID) {
                int quantity = pItem->m_itemQuantity;
                if (quantity < lowestQuantity) {
                    lowestQuantity = quantity;
                    bestSlot = i;
                }
            }
        }
        
        // If we found this priority potion, use it
        if (bestSlot >= 0) {
            return bestSlot;
        }
    }
    
    return -1;
}

// Shortcut functions for each potion type
static int FindPetHPPotion() {
    return FindPetPotionInInventory(PET_HP_POTIONS, PET_HP_POTION_COUNT);
}

static int FindPetHGPPotion() {
    return FindPetPotionInInventory(PET_HGP_POTIONS, PET_HGP_POTION_COUNT);
}

static int FindPetCurePotion() {
    return FindPetPotionInInventory(PET_CURE_POTIONS, PET_CURE_POTION_COUNT);
}

// ============================================
// PET SELECTION FOR POTION USAGE
// ============================================

// Select a pet by its unique ID (for potion targeting)
// NOTE: Direct memory write may cause crash - disabled for now
static bool SelectPetByUniqueID(DWORD uniqueID) {
    // Temporarily disabled - direct memory write may crash
    // Need to find safe way to select pet programmatically
    return true; // Return true to allow potion use attempt
}

// Get currently selected pet ID
static DWORD GetSelectedPetID() {
    if (!g_pCICPlayer) return 0;
    
    CCOSDataMgr* pCosMgr = g_pCICPlayer->GetCosMgr();
    if (!pCosMgr) return 0;
    
    return pCosMgr->m_selectedPetUniqueId;
}


// Use potion on a specific pet by sending direct packet with pet uniqueID
// This fixes the issue where potion fails if a different pet is selected in UI
static bool UsePotionOnPet(int potionSlot, DWORD petUniqueID, const char* petName, const char* potionType) {
    if (potionSlot < 0 || !g_pCGInterface || !g_pCICPlayer || petUniqueID == 0) {
        return false;
    }
    
    CIFMainPopup* mainPopup = g_pCGInterface->GetMainPopup();
    if (!mainPopup) return false;
    
    CIFInventory* inventory = mainPopup->GetInventory();
    if (!inventory) return false;
    
    CSOItem* pItem = inventory->GetItemBySlot(potionSlot);
    if (!pItem) return false;
    
    int refObjId = pItem->m_refObjItemId;
    if (refObjId == 0) return false;
    
    // Map known pet potion refObjIds to their TypeIDs
    WORD itemTypeID = 0;
    switch (refObjId) {
        // HP Potions
        case 9008: itemTypeID = 0x20EC; break;  // HP X-Large
        case 2143: itemTypeID = 0x20EC; break;  // HP
        case 2144: itemTypeID = 0x20EC; break;  // HP
        // Cure Potions
        case 9009: itemTypeID = 0x396C; break;  // Cure Status
        case 9010: itemTypeID = 0x396C; break;  // Cure Status variant
        // HGP Potions
        case 7553: itemTypeID = 0x48EC; break;  // HGP
        default:
            // Fall back to UseItem method for unknown potions
            g_pCGInterface->UseItem(potionSlot, -1, -1);
            return true;
    }
    
    // Calculate slot ID for packet (slot + 0xD as per server protocol)
    BYTE packetSlotID = static_cast<BYTE>(potionSlot + 0xD);
    
    // Send potion packet
    NEWMSG(0x704C)
    pReq << packetSlotID << itemTypeID << static_cast<DWORD>(petUniqueID);
    SENDMSG()
    
    return true;
}

// Print change notification
static void PrintChange(const char* petName, const char* field, DWORD oldVal, DWORD newVal) {
    PrintPet("[%s] %s: %u -> %u\n", petName, field, oldVal, newVal);
}

// Get status effect name from type
static const char* GetStatusEffectName(WORD statusType) {
    switch (statusType) {
        case 1: return "POISON";
        case 2: return "BURN";
        case 3: return "FREEZE";
        case 4: return "FROSTBITE";
        case 5: return "ELECTRIC_SHOCK";
        case 9: return "ZOMBIE";
        default: return NULL;
    }
}

// Read pet status effect by looking up in status manager's map
// Manager at 0xA01010, map at manager+0x58
// Map node: +0x08=left, +0x0C=right, +0x10=key, +0x14=value (status ptr)
static WORD ReadPetStatusEffect(DWORD cosUniqueID) {
    __try {
        // Manager is at 0xA01010 (not a pointer, actual address)
        BYTE* pManager = (BYTE*)0xA01010;
        
        // Map root at manager+0x58 (this[22] in pseudocode)
        DWORD mapRoot = *(DWORD*)(pManager + 0x58);
        if (mapRoot == 0) return 0;
        
        // First node at mapRoot+0x04
        DWORD node = *(DWORD*)(mapRoot + 0x04);
        DWORD sentinel = mapRoot;
        DWORD foundNode = mapRoot;
        
        // Binary search in map
        while (node != 0) {
            DWORD nodeKey = *(DWORD*)(node + 0x10);
            if (nodeKey < cosUniqueID) {
                node = *(DWORD*)(node + 0x0C);  // Right child
            } else {
                foundNode = node;
                node = *(DWORD*)(node + 0x08);  // Left child
            }
        }
        
        // Check if found
        if (foundNode == sentinel) return 0;
        if (cosUniqueID < *(DWORD*)(foundNode + 0x10)) return 0;
        
        // Get status pointer from node+0x14
        DWORD statusPtr = *(DWORD*)(foundNode + 0x14);
        if (statusPtr == 0 || statusPtr < 0x10000) return 0;
        
        // Read status WORD
        return *(WORD*)statusPtr;
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
    }
    return 0;
}

// Read pet data from memory
static bool ReadPetData(DWORD cicCosAddr, PetCache* outCache) {
    __try {
        BYTE* base = reinterpret_cast<BYTE*>(cicCosAddr);
        
        // Get COSInfo pointer (CICCos + 0x544)
        DWORD pDataPtr = *(DWORD*)(base + 0x544);
        if (pDataPtr == 0 || pDataPtr < 0x10000) return false;
        
        DWORD refObjID = *(DWORD*)(pDataPtr + 0x04);  // RefObjID for matching
        DWORD cosMaxHP = *(DWORD*)(pDataPtr + 0x0FC); // MaxHP from COSInfo
        
        // Get SCOSInfo from CCOSDataMgr
        DWORD pPlayer = *(DWORD*)0xA0465C;
        if (pPlayer == 0) return false;
        
        typedef DWORD (__thiscall *GetCosMgr_t)(void* pPlayer);
        GetCosMgr_t GetCosMgr = (GetCosMgr_t)0x659D70;
        DWORD pCosMgr = GetCosMgr((void*)pPlayer);
        if (pCosMgr == 0) return false;
        
        // Search map for SCOSInfo
        DWORD* pMapRootPtr = (DWORD*)(pCosMgr + 0x04);
        DWORD mapRoot = *pMapRootPtr;
        if (mapRoot == 0) return false;
        
        BYTE* pSCOSInfo = NULL;
        DWORD scosUniqueID = 0;
        DWORD node = *(DWORD*)(mapRoot + 0x08);
        for (int i = 0; i < 10 && node != 0 && node != mapRoot; i++) {
            DWORD nodeVal = *(DWORD*)(node + 0x14);
            if (nodeVal > 0x10000) {
                DWORD scosRefObjID = *(DWORD*)(nodeVal + 0x00);
                if (scosRefObjID == refObjID) {
                    pSCOSInfo = (BYTE*)nodeVal;
                    scosUniqueID = *(DWORD*)(nodeVal + 0x04);  // Get UniqueID for status lookup
                    break;
                }
            }
            // Tree traversal
            DWORD right = *(DWORD*)(node + 0x0C);
            if (right != 0) {
                node = right;
                while (*(DWORD*)(node + 0x08) != 0) node = *(DWORD*)(node + 0x08);
            } else {
                DWORD parent = *(DWORD*)(node + 0x04);
                while (parent != 0 && node == *(DWORD*)(parent + 0x0C)) {
                    node = parent;
                    parent = *(DWORD*)(node + 0x04);
                }
                node = parent;
            }
        }
        
        if (pSCOSInfo == NULL) return false;
        
        // Fill cache
        outCache->address = cicCosAddr;
        outCache->refObjID = refObjID;
        outCache->uniqueID = scosUniqueID;  // Store unique ID for pet selection
        outCache->currentHP = *(DWORD*)(pSCOSInfo + 0x08);
        outCache->maxHP = cosMaxHP;
        outCache->hgp = *(WORD*)(pSCOSInfo + 0x10);
        outCache->petType = *(BYTE*)(pSCOSInfo + 0x14);
        outCache->currentEXP = *(DWORD*)(pSCOSInfo + 0x40);
        outCache->maxEXP = *(DWORD*)(pSCOSInfo + 0x48);
        
        // ========== STATUS LOOKUP from CICCos+0x1E8 ==========
        BYTE statusByte = ReadPetStatus(cicCosAddr);
        
        // Status will be checked for changes in LogAllPets
        
        outCache->status = statusByte;
        outCache->isValid = true;
        
        return true;
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

// Get cache slot by pet type (0=Horse, 1=Job, 2=Pick, 3=Attack)
static PetCache* GetCacheSlotByType(BYTE petType) {
    if (petType < MAX_PET_SLOTS) {
        return &s_petCache[petType];
    }
    return NULL;
}

// Find existing cache entry by address
static PetCache* FindCacheByAddress(DWORD address) {
    for (int i = 0; i < MAX_PET_SLOTS; i++) {
        if (s_petCache[i].address == address && s_petCache[i].isValid) {
            return &s_petCache[i];
        }
    }
    return NULL;
}

// Pending pets queue
#define MAX_PENDING_PETS 10
struct PendingPet { DWORD address; DWORD time; };
static PendingPet s_pendingPets[MAX_PENDING_PETS] = {0};
static int s_pendingPetCount = 0;

// Queue pet for delayed processing (3 seconds)
void CICCos::QueuePetForDelayedLog(DWORD address) {
    // Check if this address is already in pending queue (avoid duplicates)
    for (int i = 0; i < s_pendingPetCount; i++) {
        if (s_pendingPets[i].address == address) {
            return; // Already queued
        }
    }
    
    // Check if this address is already being tracked (any slot)
    for (int i = 0; i < MAX_PET_SLOTS; i++) {
        if (s_petCache[i].address == address && s_petCache[i].isValid) {
            return; // Already tracked
        }
    }
    
    if (s_pendingPetCount < MAX_PENDING_PETS) {
        s_pendingPets[s_pendingPetCount].address = address;
        s_pendingPets[s_pendingPetCount].time = GetTickCount();
        s_pendingPetCount++;
        PrintPet("[PetMonitor] New pet detected: 0x%08X (Pending: %d)\n", address, s_pendingPetCount);
    }
}

// Main update function - called every frame from EndScene
void CICCos::LogAllPets() {
    if (s_startupTime == 0) s_startupTime = GetTickCount();
    
    // Update buff monitoring system
    ActiveBuffManager::Update();
    
    // Initialize message
    if (!s_initialized && (GetTickCount() - s_startupTime) > 5000) {
        s_initialized = true;
    }
    
    DWORD currentTime = GetTickCount();
    
    // Process pending pets (3 second delay)
    for (int i = 0; i < s_pendingPetCount; i++) {
        if (s_pendingPets[i].address != 0 && (currentTime - s_pendingPets[i].time) > 3000) {
            DWORD addr = s_pendingPets[i].address;
            
            PetCache newData = {0};
            if (ReadPetData(addr, &newData)) {
                // Get cache slot by pet type
                PetCache* cache = GetCacheSlotByType(newData.petType);
                if (cache) {
                    // If slot already has a different address (teleport case), log it
                    if (cache->isValid && cache->address != addr) {
                        PrintPet("[PetMonitor] %s pet respawned: 0x%08X -> 0x%08X\n", 
                            GetPetTypeName(newData.petType), cache->address, addr);
                    }
                    // Update slot with new data
                    *cache = newData;
                    PrintPetStats(cache, GetPetTypeName(cache->petType));
                }
            }
            
            // Remove from queue
            for (int j = i; j < s_pendingPetCount - 1; j++) {
                s_pendingPets[j] = s_pendingPets[j + 1];
            }
            s_pendingPetCount--;
            i--;
        }
    }
    
    // ============================================
    // AUTO PET POTION SYSTEM
    // ============================================
    // Uses settings from CIFPetAutoPotion UI:
    // - Attack Pet: HP, HGP, Cure (each controlled by checkbox + slider)
    // - Transport Pet (Horse/Job): HP, Cure (no HGP)
    // ============================================
    
    static DWORD s_lastAutoPotionCheck = 0;
    static DWORD s_lastHPPotionTime[MAX_PET_SLOTS] = {0};
    static DWORD s_lastHGPPotionTime[MAX_PET_SLOTS] = {0};
    static DWORD s_lastCurePotionTime[MAX_PET_SLOTS] = {0};
    
    // Check every 500ms to avoid spam
    if (currentTime - s_lastAutoPotionCheck < 500) {
        goto skip_auto_potion;
    }
    s_lastAutoPotionCheck = currentTime;
    
    // Get PetAutoPotion instance for UI settings
    // Use the global g_pCIFPetAutoPotion defined in IFPetAutoPotion.h
    extern CIFPetAutoPotion* g_pCIFPetAutoPotion;
    CIFPetAutoPotion* pPetAutoPotion = g_pCIFPetAutoPotion;
    
    // Check if any pet auto potion settings are enabled
    if (!pPetAutoPotion) {
        // Silently skip - no warning spam when UI not loaded
        goto skip_auto_potion;
    }
    
    // Check all pet slots for auto potion needs
    for (int i = 0; i < MAX_PET_SLOTS; i++) {
        if (!s_petCache[i].isValid) continue;
        
        PetCache* pet = &s_petCache[i];
        const char* petName = GetPetTypeName(pet->petType);
        
        // Determine if this is an Attack Pet (type 3) or Transport Pet (Horse=0, Job=1)
        bool isAttackPet = (pet->petType == PET_SLOT_ATTACK);
        bool isTransportPet = (pet->petType == PET_SLOT_HORSE || pet->petType == PET_SLOT_JOB);
        
        // Calculate HP percentage
        int hpPercent = 0;
        if (pet->maxHP > 0) {
            hpPercent = (int)((pet->currentHP * 100) / pet->maxHP);
        }
        
        // Calculate HGP percentage (max is 10000)
        int hgpPercent = (int)(pet->hgp / 100); // HGP is 0-10000, so /100 = 0-100%
        
        // ===== HP CHECK =====
        // Attack Pet: uses IsAttackPetHPEnabled() and GetAttackPetHPThreshold()
        // Transport Pet: uses IsTransportPetHPEnabled() and GetTransportPetHPThreshold()
        // IMPORTANT: Do NOT use HP pot when ZOMBIE status is active (0x20) - pet will die!
        bool hpEnabled = false;
        int hpThreshold = 50;
        
        if (isAttackPet) {
            hpEnabled = pPetAutoPotion->IsAttackPetHPEnabled();
            hpThreshold = pPetAutoPotion->GetAttackPetHPThreshold();
        } else if (isTransportPet) {
            hpEnabled = pPetAutoPotion->IsTransportPetHPEnabled();
            hpThreshold = pPetAutoPotion->GetTransportPetHPThreshold();
        }
        
        // Check for zombie status (0x20) - DO NOT USE HP POT when zombie!
        bool hasZombie = ((pet->status & 0x20) != 0);
        
        if (hasZombie) {
            // Pet has zombie - skip HP pot (would kill pet), will attempt cure instead
        }
        
        if (hpEnabled && !hasZombie && hpPercent < hpThreshold && pet->currentHP > 0) {
            // Cooldown: 2 seconds between HP potions per pet
            if (currentTime - s_lastHPPotionTime[i] > 2000) {
                int hpPotionSlot = FindPetHPPotion();
                if (hpPotionSlot >= 0) {
                    UsePotionOnPet(hpPotionSlot, pet->uniqueID, petName, "HP");
                    s_lastHPPotionTime[i] = currentTime;
                } else {
                    s_lastHPPotionTime[i] = currentTime; // Cooldown even when no potion
                }
            }
        }
        
        // ===== HGP CHECK (Attack Pet only) =====
        // Uses IsAttackPetHGPEnabled() and GetAttackPetHGPThreshold()
        if (isAttackPet && pPetAutoPotion->IsAttackPetHGPEnabled()) {
            int hgpThreshold = pPetAutoPotion->GetAttackPetHGPThreshold();
            int hgpThresholdValue = hgpThreshold * 100; // Convert % to 0-10000 scale
            
            if (pet->hgp < hgpThresholdValue) {
                // Cooldown: 2 seconds between HGP potions
                if (currentTime - s_lastHGPPotionTime[i] > 2000) {
                    int hgpPotionSlot = FindPetHGPPotion();
                    if (hgpPotionSlot >= 0) {
                        UsePotionOnPet(hgpPotionSlot, pet->uniqueID, petName, "HGP");
                        s_lastHGPPotionTime[i] = currentTime;
                    } else {
                        s_lastHGPPotionTime[i] = currentTime; // Cooldown even when no potion
                    }
                }
            }
        }
        
        // ===== STATUS CHECK (Cure) =====
        // Attack Pet: uses IsAttackPetCureEnabled()
        // Transport Pet: uses IsTransportPetCureEnabled()
        // IMPORTANT: STUN (0x80) cannot be cured with potions - skip it
        bool cureEnabled = false;
        
        if (isAttackPet) {
            cureEnabled = pPetAutoPotion->IsAttackPetCureEnabled();
        } else if (isTransportPet) {
            cureEnabled = pPetAutoPotion->IsTransportPetCureEnabled();
        }
        
        // Check for STUN status (0x80) - cure doesn't work on stun!
        bool hasStun = ((pet->status & 0x80) != 0);
        // Get the status that needs curing (exclude STUN bit)
        BYTE curableStatus = pet->status & ~0x80;
        
        // If only STUN is active, no other status to cure - skip cure attempt
        
        // Only cure if there's a curable status (not just STUN)
        if (cureEnabled && curableStatus != 0) {
            // Cooldown: 1 second between cure potions
            if (currentTime - s_lastCurePotionTime[i] > 1000) {
                int curePotionSlot = FindPetCurePotion();
                const char* statusName = GetStatusNameFromByte(curableStatus);
                
                if (curePotionSlot >= 0) {
                    UsePotionOnPet(curePotionSlot, pet->uniqueID, petName, "CURE");
                    s_lastCurePotionTime[i] = currentTime;
                } else {
                    s_lastCurePotionTime[i] = currentTime; // Cooldown even when no potion
                }
            }
        }
    }
    
skip_auto_potion:
    
    // Check for changes in all cached pet slots (every frame)
    for (int i = 0; i < MAX_PET_SLOTS; i++) {
        if (!s_petCache[i].isValid) continue;
        
        PetCache newData = {0};
        if (ReadPetData(s_petCache[i].address, &newData)) {
            PetCache* old = &s_petCache[i];
            
            // Check for changes (HP/EXP logging disabled for now)
            // if (newData.currentHP != old->currentHP) {
            //     PrintPet("[%s] HP: %u/%u\n", GetPetTypeName(old->petType), newData.currentHP, newData.maxHP);
            // }
            // if (newData.currentEXP != old->currentEXP) {
            //     PrintPet("[%s] EXP: %u/%u\n", GetPetTypeName(old->petType), newData.currentEXP, newData.maxEXP);
            // }
            
            // Check for status effect changes
            if (newData.status != old->status) {
                const char* newEffect = GetStatusNameFromByte((BYTE)newData.status);
                const char* oldEffect = GetStatusNameFromByte((BYTE)old->status);
                
                // Always print the raw value for debugging
                PrintPet("[%s] STATUS CHANGE: 0x%02X -> 0x%02X\n", 
                    GetPetTypeName(old->petType), old->status, newData.status);
                
                if (newData.status != 0) {
                    if (newEffect) {
                        PrintPet("[%s] STATUS: %s!\n", GetPetTypeName(old->petType), newEffect);
                    } else {
                        PrintPet("[%s] STATUS: Unknown (0x%02X)\n", GetPetTypeName(old->petType), newData.status);
                    }
                } else if (old->status != 0) {
                    PrintPet("[%s] STATUS: CURED (%s removed)\n", GetPetTypeName(old->petType), oldEffect ? oldEffect : "unknown");
                }
            }
            
            // Update cache
            *old = newData;
        }
    }
}

// Debug functions (kept for compatibility)
void CICCos::LogDebugInfo() {}
void CICCos::OnPetSpawn(CICCos* pPet) {}
void CICCos::OnPetDespawn(CICCos* pPet) {}
void CICCos::TestPetAtAddress(DWORD address) {}

// ============================================
// ECSRO CICCos Spawn Hook
// Hook address: 0x64E31A (right after constructor call in factory)
// At this point, eax = new CICCos pointer
// ============================================

static DWORD s_returnAddress = 0x0064E325;

__declspec(naked) void CICCos_SpawnHook() {
    __asm {
        pushad
        pushfd
        push eax
        call CICCos::QueuePetForDelayedLog
        add esp, 4
        popfd
        popad
        mov ecx, [esp+4]
        mov dword ptr fs:[0], ecx
        jmp s_returnAddress
    }
}

void CICCos::InstallSpawnHook() {
    PrintPet("[PetMonitor] Installing spawn hook...\n");
    
    DWORD hookAddr = 0x0064E31A;
    DWORD hookFunc = (DWORD)CICCos_SpawnHook;
    DWORD relativeJump = hookFunc - hookAddr - 5;
    
    DWORD oldProtect;
    VirtualProtect((LPVOID)hookAddr, 11, PAGE_EXECUTE_READWRITE, &oldProtect);
    
    *(BYTE*)hookAddr = 0xE9;
    *(DWORD*)(hookAddr + 1) = relativeJump;
    
    for (int i = 5; i < 11; i++) {
        *(BYTE*)(hookAddr + i) = 0x90;
    }
    
    VirtualProtect((LPVOID)hookAddr, 11, oldProtect, &oldProtect);
    
    PrintPet("[PetMonitor] Hook installed!\n");
}

