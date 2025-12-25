#pragma once

#include <windows.h>
#include <vector>
#include <cstdio>

// ============================================================
// ReturnToTownController
// Handles automatic return to town on:
// - Player death (uses opcode 0x3053)
// - Low HP potions (<20) (uses Return Scroll)
// - Low MP potions (<20) (uses Return Scroll)
// - Low Pet HP potions (<20) (uses Return Scroll)
// ============================================================

// Return Scroll Item IDs (priority order: highest first)
#define RETURN_SCROLL_MALL_HIGH_SPEED   3769  // ITEM_MALL_RETURN_SCROLL_HIGH_SPEED (priority 1)
#define RETURN_SCROLL_03                2199  // ITEM_ETC_SCROLL_RETURN_03 (priority 2)
#define RETURN_SCROLL_02                2198  // ITEM_ETC_SCROLL_RETURN_02 (priority 3)
#define RETURN_SCROLL_01                61    // ITEM_ETC_SCROLL_RETURN_01 (priority 4)

// Arrow Item IDs
#define ARROW_ITEM_01                   62    // ITEM_ETC_AMMO_ARROW_01
#define ARROW_ITEM_01_DEF               3655  // ITEM_ETC_AMMO_ARROW_01_DEF
#define ARROW_ITEM_MALL_QUIVER          3823  // ITEM_MALL_QUIVER

// Thresholds
#define LOW_ITEM_THRESHOLD              20    // Potions threshold
#define LOW_ARROW_THRESHOLD             50    // Arrow threshold

// Slot info for return scroll
struct ReturnScrollInfo {
    int slot;       // Inventory slot (0-based)
    int itemTID;    // Item TypeID
    int count;      // Stack count
};

class ReturnToTownController {
public:
    // Enable/Disable the controller (follows bot state)
    static void SetEnabled(bool enabled);
    static bool IsEnabled();
    
    // Main update function - call from AutoAttackSkillController::Update()
    static void Update();
    
    // Potion count getters
    static int GetHPPotionCount();
    static int GetMPPotionCount();
    static int GetPetHPPotionCount();
    static int GetArrowCount();  // Arrow count
    
    // Return scroll detection
    static int GetReturnScrollCount();                    // Total return scrolls
    static ReturnScrollInfo FindBestReturnScroll();       // Find highest priority scroll
    static bool HasReturnScroll();                        // Quick check
    
    // State checks
    static bool IsPlayerDead();
    static bool ShouldReturnToTown();  // Check all conditions
    
    // Town return actions
    static void SendDeathReturnPacket();       // For death - opcode 0x3053
    static void UseReturnScroll(int slot);     // For low items - use scroll item
    static void DisableBotAndReturnOnDeath();  // Death return sequence
    static void DisableBotAndReturnOnLowItem();// Low item return sequence
    
    // Reset state (call when bot is toggled off manually)
    static void Reset();
    
    // Debug logging
    static void LogInventoryPotions();      // Log HP/MP potion counts
    static void LogReturnScrolls();         // Log return scroll inventory
    
private:
    static bool s_enabled;
    static bool s_initialized;
    
    // Death detection state
    static bool s_deathDetected;
    static DWORD s_deathDetectedTime;
    static bool s_deathPacketSent;
    
    // Low item detection state
    static bool s_lowItemDetected;
    static DWORD s_lowItemDetectedTime;
    static bool s_lowItemPacketSent;
    static int s_lowItemType;  // 1=HP, 2=MP, 3=PetHP
    
    // Timing
    static DWORD s_lastCheckTime;
    static DWORD s_lastLogTime;
    static DWORD s_lastDurabilityCheckTime;  // For durability check (15 seconds)
    static const int CHECK_INTERVAL_MS = 4000;  // Check every 4 seconds
    static const int DURABILITY_CHECK_INTERVAL_MS = 15000;  // Check durability every 15 seconds
    static const int TOWN_RETURN_DELAY_MS = 3000;  // Wait 3 seconds before returning
    static const int LOG_INTERVAL_MS = 5000;  // Log every 5 seconds (for testing)
    
    static void Initialize();
    
    // Internal inventory helpers
    static bool IsReturnScrollID(int itemTID);
    static int GetReturnScrollPriority(int itemTID);  // Lower = higher priority
};
