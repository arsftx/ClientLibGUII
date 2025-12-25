#include "ActiveBuffManager.h"
#include <algorithm>

// ============================================================
// ActiveBuffManager Implementation
// ECSRO Buff List Offset: 0x1C4 (IDA confirmed via sub_653620)
// 
// Structure (from sub_653620):
//   g_pCICPlayer + 0x1C4 → BuffList* (linked list head)
//       ListNode + 0x00 → Next node
//       ListNode + 0x08 → BuffEntry*
//           BuffEntry + 0x54 → Skill ID!
// ============================================================

// Static member initialization
std::set<DWORD> ActiveBuffManager::s_previousBuffs;
bool ActiveBuffManager::s_initialized = false;

// ECSRO Buff List Offset (IDA confirmed via sub_653620: this[113] = this+0x1C4)
#define OFF_BUFF_LIST        0x1C4

// Node structure offsets (from sub_653620)
#define OFF_NODE_NEXT        0x00    // Node → Next node pointer
#define OFF_NODE_ENTRY       0x08    // Node → Entry pointer (v4[2] = Node+0x08)

// Buff Entry internal offset (from sub_653620)
#define OFF_ENTRY_SKILLID    0x54    // Entry → Skill ID (84 decimal)

void ActiveBuffManager::Initialize() {
    if (s_initialized) return;
    s_previousBuffs.clear();
    s_initialized = true;
}

DWORD ActiveBuffManager::GetPlayerPointer() {
    return *(DWORD*)ADDR_PLAYER;
}

DWORD ActiveBuffManager::GetBuffTreeBase() {
    DWORD pPlayer = GetPlayerPointer();
    if (!pPlayer) return 0;
    return *(DWORD*)(pPlayer + OFF_BUFF_LIST);
}

// Deprecated - kept for API compatibility
void ActiveBuffManager::TraverseBuffTree(DWORD pNode, DWORD pHead, std::vector<DWORD>& buffs, int depth) {
}

std::vector<DWORD> ActiveBuffManager::GetActiveBuffs() {
    std::vector<DWORD> buffs;
    
    DWORD pPlayer = GetPlayerPointer();
    if (!pPlayer) return buffs;
    
    // Get buff list head (Player + 0x1C4)
    DWORD pListHead = *(DWORD*)(pPlayer + OFF_BUFF_LIST);
    if (!pListHead || pListHead < 0x10000) return buffs;
    
    // Get first node (dereference list head)
    if (IsBadReadPtr((void*)pListHead, 4)) return buffs;
    DWORD pCurrentNode = *(DWORD*)pListHead;
    
    // Traverse linked list: while (node != listHead)
    std::set<DWORD> visited;
    int maxIter = 50;
    
    while (pCurrentNode && pCurrentNode != pListHead && pCurrentNode > 0x10000 && maxIter-- > 0) {
        if (visited.find(pCurrentNode) != visited.end()) break;
        visited.insert(pCurrentNode);
        
        if (IsBadReadPtr((void*)pCurrentNode, 0x10)) break;
        
        // Get entry pointer: Node+0x08
        DWORD pEntry = *(DWORD*)(pCurrentNode + OFF_NODE_ENTRY);
        
        if (pEntry && pEntry > 0x10000 && !IsBadReadPtr((void*)pEntry, 0x58)) {
            // Get skill ID: Entry+0x54
            DWORD skillId = *(DWORD*)(pEntry + OFF_ENTRY_SKILLID);
            
            if (skillId > 0 && skillId < 100000) {
                buffs.push_back(skillId);
            }
        }
        
        // Move to next node: Node+0x00
        pCurrentNode = *(DWORD*)(pCurrentNode + OFF_NODE_NEXT);
    }
    
    return buffs;
}

bool ActiveBuffManager::IsBuffActive(DWORD skillId) {
    std::vector<DWORD> buffs = GetActiveBuffs();
    for (size_t i = 0; i < buffs.size(); i++) {
        if (buffs[i] == skillId) return true;
    }
    return false;
}

std::string ActiveBuffManager::GetSkillName(DWORD skillId) {
    char buf[32];
    sprintf(buf, "Skill_%d", skillId);
    return std::string(buf);
}

void ActiveBuffManager::Update() {
    if (!s_initialized) {
        Initialize();
    }
    
    DWORD pPlayer = GetPlayerPointer();
    if (!pPlayer) return;
    
    std::vector<DWORD> currentBuffs = GetActiveBuffs();
    std::set<DWORD> currentSet(currentBuffs.begin(), currentBuffs.end());
    
    // Update previous buffs for change tracking
    s_previousBuffs = currentSet;
}
