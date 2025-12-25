#include "AutoBuffController.h"
#include "ActiveBuffManager.h"
#include "IFAutoHuntSettings.h"
#include <cstdio>
#include <algorithm>

// ============================================================
// AutoBuffController Implementation
// Phase 1: Detect missing buffs and log to console
// Phase 2: Cast missing buffs using native sub_56BE40
// ============================================================

// Native skill usage function address
// sub_56BE40: Sends 0x7074 opcode with skill ID and action type 4 (buff)
// Signature: void __stdcall sub_56BE40(DWORD skillId)
// Packet format: 01 04 [skillId 4 bytes] 00 = 7 bytes
#define ADDR_UseSkill 0x56BE40

typedef void (__stdcall *fn_UseSkill)(DWORD skillId);

// Cast a buff skill using native sub_56BE40 function
static bool CastBuffSkill(DWORD skillId) {
    fn_UseSkill UseSkill = (fn_UseSkill)ADDR_UseSkill;
    UseSkill(skillId);
    return true;
}


// Static member initialization
bool AutoBuffController::s_initialized = false;
bool AutoBuffController::s_enabled = false;  // OFF by default - user must enable via Macro Window
DWORD AutoBuffController::s_lastCheckTime = 0;
DWORD AutoBuffController::s_checkIntervalMs = 2000;  // Check every 2 seconds
std::set<DWORD> AutoBuffController::s_previousMissingBuffs;

void AutoBuffController::Initialize() {
    if (s_initialized) return;
    
    s_initialized = true;
    s_lastCheckTime = 0;
    s_previousMissingBuffs.clear();
}

void AutoBuffController::SetEnabled(bool enabled) {
    s_enabled = enabled;
}

bool AutoBuffController::IsEnabled() {
    return s_enabled;
}

std::vector<DWORD> AutoBuffController::GetMissingBuffs() {
    std::vector<DWORD> missingBuffs;
    
    // Get settings window
    if (!g_pCIFAutoHuntSettings) {
        return missingBuffs;
    }
    
    // Get selected buff skill IDs from settings
    const DWORD* selectedBuffIds = g_pCIFAutoHuntSettings->GetSelectedBuffSkillIds();
    int selectedCount = g_pCIFAutoHuntSettings->GetSelectedBuffSkillCount();
    
    if (!selectedBuffIds) {
        return missingBuffs;
    }

    
    // Get currently active buffs on character
    std::vector<DWORD> activeBuffs = ActiveBuffManager::GetActiveBuffs();
    std::set<DWORD> activeSet(activeBuffs.begin(), activeBuffs.end());
    
    // Check each selected buff
    for (int i = 0; i < SKILL_SLOT_COUNT; i++) {
        DWORD buffId = selectedBuffIds[i];
        if (buffId == 0) continue;  // Empty slot
        
        // Check if this buff is currently active
        if (activeSet.find(buffId) == activeSet.end()) {
            // Buff is selected but NOT active - it's missing!
            missingBuffs.push_back(buffId);
        }
    }
    
    return missingBuffs;
}

void AutoBuffController::CheckAndLogMissingBuffs() {
    std::vector<DWORD> missingBuffs = GetMissingBuffs();
    std::set<DWORD> currentMissing(missingBuffs.begin(), missingBuffs.end());
    s_previousMissingBuffs = currentMissing;
}

void AutoBuffController::Update() {
    if (!s_initialized) {
        Initialize();
    }
    
    if (!s_enabled) return;
    
    // Rate limit checks - cast one buff every 3 seconds
    DWORD now = GetTickCount();
    if (now - s_lastCheckTime < s_checkIntervalMs) {
        return;
    }
    s_lastCheckTime = now;
    
    // Get missing buffs
    std::vector<DWORD> missingBuffs = GetMissingBuffs();
    
    // Track changes
    std::set<DWORD> currentMissing(missingBuffs.begin(), missingBuffs.end());
    s_previousMissingBuffs = currentMissing;
    
    // Cast first missing buff (one at a time)
    if (!missingBuffs.empty()) {
        DWORD buffToCast = missingBuffs[0];
        CastBuffSkill(buffToCast);
    }

}

