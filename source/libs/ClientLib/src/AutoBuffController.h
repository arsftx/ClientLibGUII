#pragma once

#include <windows.h>
#include <vector>
#include <set>

// ============================================================
// AutoBuffController
// Manages automatic buff reapplication based on user settings.
// 
// Flow:
//   1. Get selected buff skill IDs from IFAutoHuntSettings
//   2. Get currently active buffs from ActiveBuffManager
//   3. Determine which buffs are missing
//   4. (Phase 2) Cast missing buffs
// ============================================================

class AutoBuffController {
public:
    // Initialize the controller
    static void Initialize();
    
    // Main update loop - call periodically (e.g., every 1 second)
    static void Update();
    
    // Check for missing buffs and log them (Phase 1)
    static void CheckAndLogMissingBuffs();
    
    // Enable/Disable auto buff system
    static void SetEnabled(bool enabled);
    static bool IsEnabled();
    
    // Get list of missing buffs (selected but not active)
    static std::vector<DWORD> GetMissingBuffs();

private:
    static bool s_initialized;
    static bool s_enabled;
    static DWORD s_lastCheckTime;
    static DWORD s_checkIntervalMs;  // How often to check (default: 1000ms)
    
    // Previous missing buffs (for change detection in logs)
    static std::set<DWORD> s_previousMissingBuffs;
};
