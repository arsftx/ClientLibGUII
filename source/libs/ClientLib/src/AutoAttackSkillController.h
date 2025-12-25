#pragma once

#include <windows.h>
#include <vector>
#include <set>
#include <map>

// ============================================================
// AutoAttackSkillController
// Manages automatic attack skill usage based on user settings.
// 
// Flow:
//   1. Get selected attack skill IDs from IFAutoHuntSettings
//   2. Check if path to target is clear (obstacle avoidance)
//   3. If blocked, navigate around using waypoints
//   4. Cast skill on current target when in range
// ============================================================

class AutoAttackSkillController {
public:
    // Initialize the controller
    static void Initialize();
    
    // Main update loop - call periodically
    static void Update();
    
    // Enable/Disable auto attack skill system
    static void SetEnabled(bool enabled);
    static bool IsEnabled();
    
    // Get list of selected attack skills
    static std::vector<DWORD> GetSelectedAttackSkills();
    
    // Cast attack skill on target
    static bool CastAttackSkill(DWORD skillId, DWORD targetId);
    
    // Get skill cast time from SkillData +0x44 (animation time)
    static DWORD GetSkillCastTime(DWORD skillId);
    
    // Get skill cooldown from SkillData +0x48 
    static DWORD GetSkillCooldown(DWORD skillId);
    
    // Check if skill is ready (not on cooldown)
    static bool IsSkillReady(DWORD skillId);
    
    // Check if character is currently casting (animation in progress)
    static bool IsCasting();
    
    // Check if currently navigating around obstacle to target
    static bool IsNavigatingToTarget();

    // Get skills that are ready to use (not on cooldown)
    static std::vector<DWORD> GetReadySkills();
    
    // Get skills that are on cooldown
    static std::vector<DWORD> GetCooldownSkills();
    
    // Legacy function - returns max(castTime, cooldown)
    static DWORD GetSkillDuration(DWORD skillId);


private:
    static bool s_initialized;
    static bool s_enabled;
    static DWORD s_lastCastTime;
    static DWORD s_castIntervalMs;  // Minimum interval between any skill casts
    static int s_currentSkillIndex;  // Round-robin through selected skills
    
    // Per-skill cooldown tracking
    static std::map<DWORD, DWORD> s_skillLastCastTime;  // skillId -> last cast timestamp
    
    // Global casting state
    static DWORD s_castingEndTime;  // When current skill animation finishes
    
    // Previous state for change detection in logs
    static std::set<DWORD> s_previousSelectedSkills;
    
    // Obstacle avoidance state
    static bool s_isNavigatingToTarget;  // Currently navigating around obstacle
    static DWORD s_pendingTargetId;      // Target to attack after navigation
    static DWORD s_pendingSkillId;       // Skill to cast after navigation
};
