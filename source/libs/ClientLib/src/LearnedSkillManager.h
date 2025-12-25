#pragma once

#include <vector>
#include <string>
#include <windows.h>

// ============================================================
// LearnedSkillManager - Access learned skills from game memory
// Based on IDA analysis of ECSRO client
// ============================================================

// Key addresses (ECSRO verified via IDA)
#define ADDR_PLAYER         0xA0465C     // g_pCICPlayer / g_pMyPlayerObj
#define ADDR_SKILL_MANAGER  0xA01010     // Global skill data manager

// Native function addresses
#define FUNC_GET_SKILL_OBJ  0x616790     // sub_616790(dwSkillID) -> SkillObject*
#define FUNC_GET_SKILL_DATA 0x601900     // sub_601900(pSkillObj) -> SkillData*

// Offsets
#define OFF_LEARNED_SKILLS  0x13BC       // Player -> LearnedSkillStructure*
#define OFF_SKILL_TREE      0x0C         // LearnedSkillStructure -> Tree root
#define OFF_SKILL_DATA      0x10C        // SkillObject -> SkillData
#define OFF_DDJ_PATH        0xD4         // SkillData -> DDJ icon path (std::string)

// Tree node offsets (std::map-like red-black tree)
#define OFF_NODE_LEFT       0x08         // TreeNode -> Left child
#define OFF_NODE_RIGHT      0x0C         // TreeNode -> Right child  
#define OFF_NODE_ENTRY      0x14         // TreeNode -> SkillEntry*

// SkillEntry offsets
#define OFF_ENTRY_SKILLID   0x00         // SkillEntry -> Skill ID

class LearnedSkillManager {
public:
    // Get all learned skill IDs for current character
    static std::vector<DWORD> GetAllLearnedSkillIDs();
    
    // Check if a specific skill is learned
    static bool IsSkillLearned(DWORD dwSkillID);
    
    // Get skill icon DDJ path from skill manager
    static std::string GetSkillIconPath(DWORD dwSkillID);
    
    // Get player pointer (0xA0465C dereferenced)
    static DWORD GetPlayerPointer();
    
    // Get LearnedSkillStructure pointer
    static DWORD GetLearnedSkillStructure();

private:
    // Recursive in-order tree traversal
    static void TraverseSkillTree(DWORD pNode, DWORD pHead, std::vector<DWORD>& skills, int depth = 0);
    
    // Debug logging
    static void Log(const char* format, ...);
};
