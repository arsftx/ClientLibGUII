#pragma once

#include <windows.h>
#include <vector>

// ============================================================
// AutoTargetController
// Manages automatic target selection for auto-hunt system.
// 
// Features:
//   1. Save character start position when enabled
//   2. Find nearby monsters within range
//   3. Select closest valid target
//   4. Provide target ID for attack skills
//
// Range values from IFAutoHuntSettings: 10m, 30m, 50m, 70m, 100m
// Game unit conversion: 1 meter = 10 game units (approximately)
// ============================================================

struct MonsterInfo {
    DWORD uniqueID;       // Target ID for packets
    DWORD entityAddress;  // CICMonster pointer
    float posX;           // World X position
    float posY;           // World Y position  
    float posZ;           // World Z position
    float distance;       // Distance from start position
    bool isHostile;       // Can be attacked
};

class AutoTargetController {
public:
    // Initialize the controller
    static void Initialize();
    
    // Main update loop - call periodically
    static void Update();
    
    // Enable/Disable auto targeting
    static void SetEnabled(bool enabled);
    static bool IsEnabled();
    
    // Get current target ID (0 if no target) - from our tracking
    static DWORD GetCurrentTargetID();
    
    // Get game's actual selected target from memory
    static DWORD GetGameCurrentTarget();
    
    // Check if we have a valid target (game actually has target selected)
    static bool HasValidTarget();
    
    // Get player's start position (saved when enabled)
    static void GetStartPosition(float& x, float& y, float& z);
    
    // Get range setting from IFAutoHuntSettings (in game units)
    static float GetRangeInGameUnits();
    
    // Get time when target was last changed
    static DWORD GetLastTargetChangeTime();
    
    // Find all monsters within range
    static std::vector<MonsterInfo> GetNearbyMonsters();
    
    // Select the closest monster as target
    static bool SelectClosestTarget();
    
    // Send target select packet (0x7045) - use before skill cast
    static void SendTargetPacket(DWORD targetId);

private:
    // Save current player position as start position
    static void SaveStartPosition();
    
    // Read player's current position
    static bool GetPlayerPosition(float& x, float& y, float& z);
    
    // Read entity position from memory
    static bool GetEntityPosition(DWORD entityAddr, float& x, float& y, float& z);
    
    // Calculate distance between two points
    static float CalculateDistance(float x1, float y1, float z1, float x2, float y2, float z2);

private:
    static bool s_initialized;
    static bool s_enabled;
    static DWORD s_currentTargetID;
    static DWORD s_lastTargetSearchTime;
    static DWORD s_targetSearchIntervalMs;  // How often to search for targets
    
    // Start position (saved when auto-hunt is enabled)
    static float s_startPosX;
    static float s_startPosY;
    static float s_startPosZ;
    static bool s_hasStartPosition;
};
