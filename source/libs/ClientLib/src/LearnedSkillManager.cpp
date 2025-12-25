#include "LearnedSkillManager.h"
#include <cstdio>

// ============================================================
// LearnedSkillManager Implementation
// Uses IDA-verified offsets to traverse skill tree
// ============================================================

DWORD LearnedSkillManager::GetPlayerPointer() {
    return *(DWORD*)ADDR_PLAYER;
}

DWORD LearnedSkillManager::GetLearnedSkillStructure() {
    DWORD pPlayer = GetPlayerPointer();
    if (!pPlayer) return 0;
    return *(DWORD*)(pPlayer + OFF_LEARNED_SKILLS);
}

void LearnedSkillManager::TraverseSkillTree(DWORD pNode, DWORD pHead, std::vector<DWORD>& skills, int depth) {
    if (depth > 100 || !pNode || pNode == pHead) return;
    
    // Traverse left
    DWORD pLeft = *(DWORD*)(pNode + OFF_NODE_LEFT);
    TraverseSkillTree(pLeft, pHead, skills, depth + 1);
    
    // Get skill entry from current node
    DWORD pEntry = *(DWORD*)(pNode + OFF_NODE_ENTRY);
    if (pEntry) {
        DWORD dwSkillID = *(DWORD*)(pEntry + OFF_ENTRY_SKILLID);
        if (dwSkillID > 0 && dwSkillID < 100000) {
            skills.push_back(dwSkillID);
        }
    }
    
    // Traverse right
    DWORD pRight = *(DWORD*)(pNode + OFF_NODE_RIGHT);
    TraverseSkillTree(pRight, pHead, skills, depth + 1);
}

std::vector<DWORD> LearnedSkillManager::GetAllLearnedSkillIDs() {
    std::vector<DWORD> skills;
    
    DWORD pLearnedSkills = GetLearnedSkillStructure();
    if (!pLearnedSkills) return skills;
    
    DWORD pTreeHead = *(DWORD*)(pLearnedSkills + OFF_SKILL_TREE);
    if (!pTreeHead) return skills;
    
    DWORD pRoot = *(DWORD*)(pTreeHead + 0x04);
    if (!pRoot || pRoot == pTreeHead) return skills;
    
    TraverseSkillTree(pRoot, pTreeHead, skills, 0);
    return skills;
}

bool LearnedSkillManager::IsSkillLearned(DWORD dwSkillID) {
    std::vector<DWORD> skills = GetAllLearnedSkillIDs();
    for (size_t i = 0; i < skills.size(); i++) {
        if (skills[i] == dwSkillID) return true;
    }
    return false;
}

std::string LearnedSkillManager::GetSkillIconPath(DWORD dwSkillID) {
    typedef DWORD (__thiscall *GetSkillObject_t)(void* pSkillMgr, DWORD dwSkillID);
    static GetSkillObject_t fnGetSkillObj = (GetSkillObject_t)FUNC_GET_SKILL_OBJ;
    
    void* pSkillMgr = (void*)ADDR_SKILL_MANAGER;
    DWORD pSkillObj = fnGetSkillObj(pSkillMgr, dwSkillID);
    if (!pSkillObj) return "";
    
    // SkillData is INLINE at SkillObject + 0x10C
    DWORD pSkillData = pSkillObj + OFF_SKILL_DATA;
    
    // DDJ path is at SkillData + 0xD4
    // MSVC std::string layout: if capacity >= 16, first 4 bytes is pointer
    DWORD pDdjStr = pSkillData + OFF_DDJ_PATH;
    DWORD strCap = *(DWORD*)(pDdjStr + 0x14);
    
    const char* ddjStr;
    if (strCap >= 16) {
        ddjStr = *(char**)(pDdjStr);  // Long string
    } else {
        ddjStr = (char*)(pDdjStr);    // Short string inline
    }
    
    if (!ddjStr || ddjStr[0] == '\0') return "";
    return std::string(ddjStr);
}
