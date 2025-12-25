#include "ReturnToTownController.h"
#include "IFAutoHuntSettings.h"
#include "IFMacroWindow.h"
#include "IFAutoPotion.h"
#include "AutoAttackSkillController.h"
#include "AutoTargetController.h"
#include "AutoBuffController.h"
#include "AutoMoveController.h"
#include "EquippedItemManager.h"  // For equipped item durability detection
#include <ClientNet/MsgStreamBuffer.h>
#include "GlobalHelpersThatHaveNoHomeYet.h"  // For SendMsg
#include "GInterface.h"       // For g_pCGInterface
#include "IFMainPopup.h"      // For GetInventory()
#include "IFInventory.h"      // For CIFInventory
#include "SOItem.h"           // For CSOItem
#include "ICPlayer.h"         // For g_pCICPlayer, invCount

// ============================================================
// ReturnToTownController Implementation
// ============================================================

// Static member initialization
bool ReturnToTownController::s_enabled = false;
bool ReturnToTownController::s_initialized = false;

// Death detection state
bool ReturnToTownController::s_deathDetected = false;
DWORD ReturnToTownController::s_deathDetectedTime = 0;
bool ReturnToTownController::s_deathPacketSent = false;

// Low item detection state
bool ReturnToTownController::s_lowItemDetected = false;
DWORD ReturnToTownController::s_lowItemDetectedTime = 0;
bool ReturnToTownController::s_lowItemPacketSent = false;
int ReturnToTownController::s_lowItemType = 0;

// Timing
DWORD ReturnToTownController::s_lastCheckTime = 0;
DWORD ReturnToTownController::s_lastLogTime = 0;
DWORD ReturnToTownController::s_lastDurabilityCheckTime = 0;

void ReturnToTownController::Initialize() {
    if (s_initialized) return;
    Reset();
    s_initialized = true;
    printf("[ReturnToTown] Controller initialized\n");
    fflush(stdout);
}

void ReturnToTownController::SetEnabled(bool enabled) {
    s_enabled = enabled;
    if (enabled) {
        // Reset flags when bot is enabled (clears s_lowItemPacketSent etc.)
        Reset();
        printf("[ReturnToTown] Controller ENABLED - flags reset\n");
        fflush(stdout);
        
        // Log equipped items for testing
        EquippedItemManager::LogAllEquippedItems();
        
        // Log inventory state when bot is enabled
        LogInventoryPotions();
        LogReturnScrolls();
    } else {
        printf("[ReturnToTown] Controller DISABLED\n");
        fflush(stdout);
    }
}

bool ReturnToTownController::IsEnabled() {
    return s_enabled;
}

void ReturnToTownController::Reset() {
    s_deathDetected = false;
    s_deathDetectedTime = 0;
    s_deathPacketSent = false;
    s_lowItemDetected = false;
    s_lowItemDetectedTime = 0;
    s_lowItemPacketSent = false;
    s_lowItemType = 0;
    s_lastCheckTime = 0;
    s_lastLogTime = 0;
    s_lastDurabilityCheckTime = 0;
}

bool ReturnToTownController::IsPlayerDead() {
    DWORD playerPtr = *(DWORD*)0xA0465C;
    if (!playerPtr) return false;
    
    BYTE actionState = *(BYTE*)(playerPtr + 0x1AF);
    return actionState == 0x02;
}

// Check if an item ID is a return scroll
bool ReturnToTownController::IsReturnScrollID(int itemTID) {
    return itemTID == RETURN_SCROLL_MALL_HIGH_SPEED ||
           itemTID == RETURN_SCROLL_03 ||
           itemTID == RETURN_SCROLL_02 ||
           itemTID == RETURN_SCROLL_01;
}

// Get priority of return scroll (lower = higher priority)
int ReturnToTownController::GetReturnScrollPriority(int itemTID) {
    switch (itemTID) {
        case RETURN_SCROLL_MALL_HIGH_SPEED: return 1;  // Highest priority
        case RETURN_SCROLL_03: return 2;
        case RETURN_SCROLL_02: return 3;
        case RETURN_SCROLL_01: return 4;  // Lowest priority
        default: return 99;  // Not a return scroll
    }
}

int ReturnToTownController::GetHPPotionCount() {
    int count = 0;
    
    if (!g_pCGInterface || !g_pCICPlayer) return 0;
    CIFMainPopup* mainPopup = g_pCGInterface->GetMainPopup();
    if (!mainPopup) return 0;
    CIFInventory* inventory = mainPopup->GetInventory();
    if (!inventory) return 0;
    
    // Get dynamic inventory count from player (same as ICCos.cpp)
    int invCount = *(unsigned char*)((DWORD_PTR)g_pCICPlayer + 0x13B4);
    
    for (int i = 0; i < invCount; i++) {
        CSOItem* pItem = inventory->GetItemBySlot(i);
        if (!pItem) continue;
        
        int itemID = pItem->m_refObjItemId;
        if (itemID == 0) continue;
        
        // Check if this is an HP potion
        for (std::list<int>::iterator it = CIFAutoPotion::Potion_ObjID_HP_Potion_List.begin();
             it != CIFAutoPotion::Potion_ObjID_HP_Potion_List.end(); ++it) {
            if (*it == itemID) {
                int quantity = pItem->m_itemQuantity;
                count += (quantity > 0) ? quantity : 1;
                break;
            }
        }
    }
    
    return count;
}

int ReturnToTownController::GetMPPotionCount() {
    int count = 0;
    
    if (!g_pCGInterface || !g_pCICPlayer) return 0;
    CIFMainPopup* mainPopup = g_pCGInterface->GetMainPopup();
    if (!mainPopup) return 0;
    CIFInventory* inventory = mainPopup->GetInventory();
    if (!inventory) return 0;
    
    int invCount = *(unsigned char*)((DWORD_PTR)g_pCICPlayer + 0x13B4);
    
    for (int i = 0; i < invCount; i++) {
        CSOItem* pItem = inventory->GetItemBySlot(i);
        if (!pItem) continue;
        
        int itemID = pItem->m_refObjItemId;
        if (itemID == 0) continue;
        
        // Check if this is an MP potion
        for (std::list<int>::iterator it = CIFAutoPotion::Potion_ObjID_MP_Potion_List.begin();
             it != CIFAutoPotion::Potion_ObjID_MP_Potion_List.end(); ++it) {
            if (*it == itemID) {
                int quantity = pItem->m_itemQuantity;
                count += (quantity > 0) ? quantity : 1;
                break;
            }
        }
    }
    
    return count;
}

int ReturnToTownController::GetPetHPPotionCount() {
    // Pet HP Potion IDs from ICCos.cpp: 9008 (XLarge), 2144 (Large), 2143 (Small)
    static const int PET_HP_POTIONS[] = { 9008, 2144, 2143 };
    static const int PET_HP_POTION_COUNT = 3;
    
    int count = 0;
    
    if (!g_pCGInterface || !g_pCICPlayer) return 0;
    CIFMainPopup* mainPopup = g_pCGInterface->GetMainPopup();
    if (!mainPopup) return 0;
    CIFInventory* inventory = mainPopup->GetInventory();
    if (!inventory) return 0;
    
    int invCount = *(unsigned char*)((DWORD_PTR)g_pCICPlayer + 0x13B4);
    
    for (int i = 0; i < invCount; i++) {
        CSOItem* pItem = inventory->GetItemBySlot(i);
        if (!pItem) continue;
        
        int itemID = pItem->m_refObjItemId;
        if (itemID == 0) continue;
        
        // Check if this is a Pet HP potion
        for (int j = 0; j < PET_HP_POTION_COUNT; j++) {
            if (itemID == PET_HP_POTIONS[j]) {
                int quantity = pItem->m_itemQuantity;
                count += (quantity > 0) ? quantity : 1;
                break;
            }
        }
    }
    
    return count;
}

int ReturnToTownController::GetArrowCount() {
    // Arrow Item IDs: 62 (ARROW_01), 3655 (ARROW_01_DEF), 3823 (MALL_QUIVER)
    static const int ARROW_ITEMS[] = { ARROW_ITEM_01, ARROW_ITEM_01_DEF, ARROW_ITEM_MALL_QUIVER };
    static const int ARROW_ITEM_COUNT = 3;
    
    int count = 0;
    
    if (!g_pCGInterface || !g_pCICPlayer) return 0;
    CIFMainPopup* mainPopup = g_pCGInterface->GetMainPopup();
    if (!mainPopup) return 0;
    CIFInventory* inventory = mainPopup->GetInventory();
    if (!inventory) return 0;
    
    int invCount = *(unsigned char*)((DWORD_PTR)g_pCICPlayer + 0x13B4);
    
    for (int i = 0; i < invCount; i++) {
        CSOItem* pItem = inventory->GetItemBySlot(i);
        if (!pItem) continue;
        
        int itemID = pItem->m_refObjItemId;
        if (itemID == 0) continue;
        
        // Check quantity - skip empty/stale slots
        int quantity = pItem->m_itemQuantity;
        if (quantity <= 0) continue;
        
        // Check if this is an Arrow
        for (int j = 0; j < ARROW_ITEM_COUNT; j++) {
            if (itemID == ARROW_ITEMS[j]) {
                count += quantity;
                break;
            }
        }
    }
    
    return count;
}

int ReturnToTownController::GetReturnScrollCount() {
    int count = 0;
    
    if (!g_pCGInterface || !g_pCICPlayer) return 0;
    CIFMainPopup* mainPopup = g_pCGInterface->GetMainPopup();
    if (!mainPopup) return 0;
    CIFInventory* inventory = mainPopup->GetInventory();
    if (!inventory) return 0;
    
    int invCount = *(unsigned char*)((DWORD_PTR)g_pCICPlayer + 0x13B4);
    
    for (int i = 0; i < invCount; i++) {
        CSOItem* pItem = inventory->GetItemBySlot(i);
        if (!pItem) continue;
        
        int itemID = pItem->m_refObjItemId;
        if (itemID == 0) continue;
        
        // Check quantity - skip empty/stale slots
        int quantity = pItem->m_itemQuantity;
        if (quantity <= 0) continue;
        
        if (IsReturnScrollID(itemID)) {
            count += quantity;
        }
    }
    
    return count;
}

ReturnScrollInfo ReturnToTownController::FindBestReturnScroll() {
    ReturnScrollInfo best = {-1, 0, 0};
    int bestPriority = 99;
    
    if (!g_pCGInterface || !g_pCICPlayer) return best;
    CIFMainPopup* mainPopup = g_pCGInterface->GetMainPopup();
    if (!mainPopup) return best;
    CIFInventory* inventory = mainPopup->GetInventory();
    if (!inventory) return best;
    
    int invCount = *(unsigned char*)((DWORD_PTR)g_pCICPlayer + 0x13B4);
    
    for (int i = 0; i < invCount; i++) {
        CSOItem* pItem = inventory->GetItemBySlot(i);
        if (!pItem) continue;
        
        int itemID = pItem->m_refObjItemId;
        if (itemID == 0) continue;
        
        // IMPORTANT: Check quantity first - empty slots may have stale itemID
        int quantity = pItem->m_itemQuantity;
        if (quantity <= 0) continue;  // Skip empty/stale slots
        
        if (IsReturnScrollID(itemID)) {
            int priority = GetReturnScrollPriority(itemID);
            if (priority < bestPriority) {
                bestPriority = priority;
                best.slot = i;
                best.itemTID = itemID;
                best.count = quantity;
            }
        }
    }
    
    return best;
}

bool ReturnToTownController::HasReturnScroll() {
    return FindBestReturnScroll().slot >= 0;
}

void ReturnToTownController::LogInventoryPotions() {
    // Commented out - too verbose for testing
    /*
    int hpCount = GetHPPotionCount();
    int mpCount = GetMPPotionCount();
    int petHpCount = GetPetHPPotionCount();
    int arrowCount = GetArrowCount();
    
    printf("[ReturnToTown] === INVENTORY STATUS ===\n");
    printf("[ReturnToTown] HP Potions: %d", hpCount);
    if (hpCount < LOW_ITEM_THRESHOLD) {
        printf(" LOW! (<20)");
    }
    printf("\n");
    
    printf("[ReturnToTown] MP Potions: %d", mpCount);
    if (mpCount < LOW_ITEM_THRESHOLD) {
        printf(" LOW! (<20)");
    }
    printf("\n");
    
    printf("[ReturnToTown] Pet HP Potions: %d", petHpCount);
    if (petHpCount < LOW_ITEM_THRESHOLD) {
        printf(" LOW! (<20)");
    }
    printf("\n");
    
    printf("[ReturnToTown] Arrows: %d", arrowCount);
    if (arrowCount < LOW_ARROW_THRESHOLD) {
        printf(" LOW! (<50)");
    }
    printf("\n");
    
    fflush(stdout);
    */
}

void ReturnToTownController::LogReturnScrolls() {
    // Commented out - too verbose for testing
    /*
    printf("[ReturnToTown] === RETURN SCROLL SEARCH ===\n");
    
    // Debug: Show ALL return scrolls found in inventory
    if (!g_pCGInterface || !g_pCICPlayer) {
        printf("[ReturnToTown] ERROR: Cannot access inventory\n");
        return;
    }
    CIFMainPopup* mainPopup = g_pCGInterface->GetMainPopup();
    if (!mainPopup) return;
    CIFInventory* inventory = mainPopup->GetInventory();
    if (!inventory) return;
    
    int invCount = *(unsigned char*)((DWORD_PTR)g_pCICPlayer + 0x13B4);
    int totalScrolls = 0;
    
    printf("[ReturnToTown] Scanning %d inventory slots...\n", invCount);
    for (int i = 0; i < invCount; i++) {
        CSOItem* pItem = inventory->GetItemBySlot(i);
        if (!pItem) continue;
        
        int itemID = pItem->m_refObjItemId;
        if (itemID == 0) continue;
        
        // Check quantity - skip empty/stale slots
        int quantity = pItem->m_itemQuantity;
        if (quantity <= 0) continue;
        
        if (IsReturnScrollID(itemID)) {
            totalScrolls += quantity;
            printf("[ReturnToTown]   -> Slot %d: ItemID=%d, Qty=%d\n", i, itemID, quantity);
        }
    }
    
    printf("[ReturnToTown] Total Return Scrolls: %d\n", totalScrolls);
    
    ReturnScrollInfo best = FindBestReturnScroll();
    if (best.slot >= 0) {
        const char* scrollName = "Unknown";
        switch (best.itemTID) {
            case RETURN_SCROLL_MALL_HIGH_SPEED: scrollName = "MALL_HIGH_SPEED (Priority 1)"; break;
            case RETURN_SCROLL_03: scrollName = "SCROLL_03 (Priority 2)"; break;
            case RETURN_SCROLL_02: scrollName = "SCROLL_02 (Priority 3)"; break;
            case RETURN_SCROLL_01: scrollName = "SCROLL_01 (Priority 4)"; break;
        }
        printf("[ReturnToTown] BEST: %s (Slot=%d, TID=%d, Qty=%d)\n", scrollName, best.slot, best.itemTID, best.count);
    } else {
        printf("[ReturnToTown] NO RETURN SCROLL FOUND!\n");
    }
    printf("[ReturnToTown] ========================\n");
    fflush(stdout);
    */
}

void ReturnToTownController::SendDeathReturnPacket() {
    printf("[ReturnToTown] Sending DEATH return packet (0x3053)...\n");
    fflush(stdout);
    
    NEWMSG(0x3053)
    SENDMSG()
}

void ReturnToTownController::UseReturnScroll(int slot) {
    if (!g_pCGInterface || !g_pCICPlayer) {
        printf("[ReturnToTown] ERROR: g_pCGInterface or g_pCICPlayer is NULL\n");
        fflush(stdout);
        return;
    }
    
    CIFMainPopup* mainPopup = g_pCGInterface->GetMainPopup();
    if (!mainPopup) return;
    CIFInventory* inventory = mainPopup->GetInventory();
    if (!inventory) return;
    CSOItem* pItem = inventory->GetItemBySlot(slot);
    if (!pItem) return;
    
    int refObjId = pItem->m_refObjItemId;
    
    // Map refObjItemId to packet TypeID (observed from client packets)
    // Mall scroll packet: ED 09 = 0x09ED = 2541
    // Normal scroll packet: EC 09 = 0x09EC = 2540
    WORD packetTypeID = 0;
    switch (refObjId) {
        case RETURN_SCROLL_MALL_HIGH_SPEED:  // 3769
            packetTypeID = 0x09ED;  // Mall high-speed scroll (ED 09)
            break;
        case RETURN_SCROLL_03:  // 2199
        case RETURN_SCROLL_02:  // 2198
        case RETURN_SCROLL_01:  // 61
        default:
            packetTypeID = 0x09EC;  // Normal scrolls (EC 09)
            break;
    }
    
    // Calculate packet slot ID (slot + 0xD as per server protocol)
    BYTE packetSlotID = static_cast<BYTE>(slot + 0xD);
    
    printf("[ReturnToTown] Using return scroll: Slot=%d (Packet=0x%02X), RefObjID=%d, PacketTypeID=0x%04X\n", 
           slot, packetSlotID, refObjId, packetTypeID);
    fflush(stdout);
    
    // Send use item packet (0x704C)
    NEWMSG(0x704C)
    pReq << packetSlotID << packetTypeID;
    SENDMSG()
    
    printf("[ReturnToTown] Return scroll packet sent!\n");
    fflush(stdout);
}

void ReturnToTownController::DisableBotAndReturnOnDeath() {
    // For death - use death return packet (0x3053)
    SendDeathReturnPacket();
    
    // Disable all controllers
    AutoAttackSkillController::SetEnabled(false);
    AutoTargetController::SetEnabled(false);
    AutoMoveController::SetEnabled(false);
    AutoBuffController::SetEnabled(false);
    SetEnabled(false);
    
    // Update UI toggle
    extern CIFMacroWindow *g_pMacroWindow;
    if (g_pMacroWindow) {
        g_pMacroWindow->m_autoAttackEnabled = false;
    }
    
    printf("[ReturnToTown] Bot disabled (DEATH). Returning to town via death packet.\n");
    fflush(stdout);
}

void ReturnToTownController::DisableBotAndReturnOnLowItem() {
    // For low items - use return scroll
    ReturnScrollInfo scroll = FindBestReturnScroll();
    
    if (scroll.slot >= 0) {
        const char* scrollName = "Unknown";
        switch (scroll.itemTID) {
            case RETURN_SCROLL_MALL_HIGH_SPEED: scrollName = "MALL_HIGH_SPEED"; break;
            case RETURN_SCROLL_03: scrollName = "SCROLL_03"; break;
            case RETURN_SCROLL_02: scrollName = "SCROLL_02"; break;
            case RETURN_SCROLL_01: scrollName = "SCROLL_01"; break;
        }
        printf("[ReturnToTown] Using return scroll: %s (TID=%d, Slot=%d)\n", scrollName, scroll.itemTID, scroll.slot);
        fflush(stdout);
        
        UseReturnScroll(scroll.slot);
    } else {
        printf("[ReturnToTown] ERROR: No return scroll found! Cannot return to town.\n");
        fflush(stdout);
        return;  // Don't disable bot if we can't return
    }
    
    // Disable all controllers
    AutoAttackSkillController::SetEnabled(false);
    AutoTargetController::SetEnabled(false);
    AutoMoveController::SetEnabled(false);
    AutoBuffController::SetEnabled(false);
    SetEnabled(false);
    
    // Update UI toggle
    extern CIFMacroWindow *g_pMacroWindow;
    if (g_pMacroWindow) {
        g_pMacroWindow->m_autoAttackEnabled = false;
    }
    
    printf("[ReturnToTown] Bot disabled (LOW ITEM). Returning to town via scroll.\n");
    fflush(stdout);
}

bool ReturnToTownController::ShouldReturnToTown() {
    extern CIFAutoHuntSettings *g_pCIFAutoHuntSettings;
    if (!g_pCIFAutoHuntSettings) return false;
    
    // Check low HP potions
    if (g_pCIFAutoHuntSettings->IsTownOnLowHPChecked()) {
        int hpCount = GetHPPotionCount();
        if (hpCount < LOW_ITEM_THRESHOLD) {
            s_lowItemType = 1;
            return true;
        }
    }
    
    // Check low MP potions
    if (g_pCIFAutoHuntSettings->IsTownOnLowMPChecked()) {
        int mpCount = GetMPPotionCount();
        if (mpCount < LOW_ITEM_THRESHOLD) {
            s_lowItemType = 2;
            return true;
        }
    }
    
    // Check low Pet HP potions
    if (g_pCIFAutoHuntSettings->IsTownOnLowPetHPChecked()) {
        int petHpCount = GetPetHPPotionCount();
        if (petHpCount < LOW_ITEM_THRESHOLD) {
            s_lowItemType = 3;
            return true;
        }
    }
    
    // Check low Arrows
    if (g_pCIFAutoHuntSettings->IsTownOnLowArrowChecked()) {
        int arrowCount = GetArrowCount();
        if (arrowCount < LOW_ARROW_THRESHOLD) {
            s_lowItemType = 4;
            return true;
        }
    }
    
    return false;
}

void ReturnToTownController::Update() {
    if (!s_initialized) {
        Initialize();
    }
    
    if (!s_enabled) return;
    
    DWORD now = GetTickCount();
    
    // ========== PERIODIC LOGGING ==========
    if (now - s_lastLogTime >= LOG_INTERVAL_MS) {
        s_lastLogTime = now;
        LogInventoryPotions();
        LogReturnScrolls();
    }
    
    // ========== DEATH DETECTION ==========
    if (IsPlayerDead()) {
        extern CIFAutoHuntSettings *g_pCIFAutoHuntSettings;
        
        if (!s_deathDetected) {
            s_deathDetected = true;
            s_deathDetectedTime = now;
            printf("[ReturnToTown] PLAYER DIED! (waiting 3s before action)\n");
            fflush(stdout);
        }
        
        // After 3 second delay, send town return
        if (s_deathDetected && !s_deathPacketSent && (now - s_deathDetectedTime >= TOWN_RETURN_DELAY_MS)) {
            if (g_pCIFAutoHuntSettings && g_pCIFAutoHuntSettings->IsReturnToTownOnDeathChecked()) {
                DisableBotAndReturnOnDeath();  // Use death packet
            }
            s_deathPacketSent = true;
        }
        return;  // Don't process other logic while dead
    } else {
        // Reset death flags when alive
        s_deathDetected = false;
        s_deathPacketSent = false;
    }
    
    // ========== LOW ITEM DETECTION ==========
    // Only check if we haven't already sent the packet
    if (!s_lowItemPacketSent) {
        // Check every 4 seconds
        if (now - s_lastCheckTime >= CHECK_INTERVAL_MS) {
            s_lastCheckTime = now;
            
            if (ShouldReturnToTown() && !s_lowItemDetected) {
                s_lowItemDetected = true;
                s_lowItemDetectedTime = now;
                
                // Log which condition triggered (only once)
                const char* reason = "Unknown";
                switch (s_lowItemType) {
                    case 1: reason = "HP Potions"; break;
                    case 2: reason = "MP Potions"; break;
                    case 3: reason = "Pet HP Potions"; break;
                    case 4: reason = "Arrows"; break;
                    case 5: reason = "Durability"; break;
                }
                printf("[ReturnToTown] Low %s detected! (waiting 3s before using return scroll)\n", reason);
                fflush(stdout);
            }
        }
        
        // After 3 second delay, use return scroll for low items
        if (s_lowItemDetected && (now - s_lowItemDetectedTime >= TOWN_RETURN_DELAY_MS)) {
            DisableBotAndReturnOnLowItem();  // Use return scroll
            s_lowItemPacketSent = true;
        }
    }
    
    // ========== LOW DURABILITY DETECTION ==========
    // Check every 15 seconds (DURABILITY_CHECK_INTERVAL_MS)
    extern CIFAutoHuntSettings *g_pCIFAutoHuntSettings;
    bool durCheckEnabled = g_pCIFAutoHuntSettings && g_pCIFAutoHuntSettings->IsTownOnLowDurabilityChecked();
    
    if (durCheckEnabled) {
        if (now - s_lastDurabilityCheckTime >= DURABILITY_CHECK_INTERVAL_MS) {
            s_lastDurabilityCheckTime = now;
            
            printf("[ReturnToTown] Checking durability (every 15s)...\n");
            
            // Check slots 0-7: Head, Shoulder, Chest, Pants, Hands, Feet, Weapon, Shield
            for (int slot = 0; slot <= 7; slot++) {
                CSOItem* pEquip = EquippedItemManager::GetEquippedItem(slot);
                if (pEquip && pEquip->m_refObjItemId != 0 && pEquip->m_MaxDurability > 0) {
                    printf("  Slot %d: Dura %d/%d\n", slot, pEquip->m_CurrDurability, pEquip->m_MaxDurability);
                    
                    // Check if current durability is less than 5
                    if (pEquip->m_CurrDurability < 5) {
                        printf("[ReturnToTown] LOW DURABILITY detected on slot %d! (%d/%d)\n", 
                               slot, pEquip->m_CurrDurability, pEquip->m_MaxDurability);
                        fflush(stdout);
                        
                        // Trigger low item return (type 5 = durability)
                        s_lowItemType = 5;
                        s_lowItemDetected = true;
                        s_lowItemDetectedTime = now;
                        break;  // Only need to detect one low durability item
                    }
                }
            }
            fflush(stdout);
        }
    }
}
