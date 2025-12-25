#pragma once

#include <windows.h>
#include <vector>
#include <set>
#include <string>

// ============================================================
// ActiveBuffManager - Monitor active buffs on character
// Based on IDA analysis of ECSRO client (sub_653620)
// 
// Structure:
//   g_pCICPlayer + 0x1C4 → BuffList* (linked list head)
//       ListNode + 0x00 → Next node
//       ListNode + 0x08 → BuffEntry*
//           BuffEntry + 0x54 → Skill ID
// ============================================================

// Player address (ECSRO)
#define ADDR_PLAYER         0xA0465C     // g_pCICPlayer

class ActiveBuffManager {
public:
    // Initialize the buff monitor
    static void Initialize();
    
    // Update buff monitoring - call each frame
    static void Update();
    
    // Check if a specific buff skill is currently active
    static bool IsBuffActive(DWORD skillId);
    
    // Get all currently active buff skill IDs
    static std::vector<DWORD> GetActiveBuffs();
    
    // Get skill name from ID (placeholder)
    static std::string GetSkillName(DWORD skillId);

private:
    // Get player pointer
    static DWORD GetPlayerPointer();
    
    // Get buff list head pointer
    static DWORD GetBuffTreeBase();
    
    // Deprecated - kept for API compatibility
    static void TraverseBuffTree(DWORD pNode, DWORD pHead, std::vector<DWORD>& buffs, int depth = 0);
    
    // Previously active buffs (for change detection)
    static std::set<DWORD> s_previousBuffs;
    
    // Is system initialized
    static bool s_initialized;
};
