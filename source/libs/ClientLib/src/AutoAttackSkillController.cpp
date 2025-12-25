#include "AutoAttackSkillController.h"
#include "IFAutoHuntSettings.h"
#include "IFMacroWindow.h"  // For g_pMacroWindow UI toggle
#include "ReturnToTownController.h"  // For death/low item detection
#include "AutoTargetController.h"
#include "AutoBuffController.h"
#include "AutoMoveController.h"  // For obstacle avoidance
#include "EquippedItemManager.h"  // For durability detection
#include "ActiveBuffManager.h"  // For imbue buff detection
#include <ClientNet/MsgStreamBuffer.h>
#include <NavMesh/NavMeshCollision.h>  // For path collision detection
#include "GlobalHelpersThatHaveNoHomeYet.h"
#include "ECSRO_Classes.h"  // For GetPlayerAddressRaw, GetLocationRaw
#include <cstdio>
#include <algorithm>

// Debug logging flag - comment out to disable verbose logs
// #define AUTOATTACK_DEBUG_LOG

#ifdef AUTOATTACK_DEBUG_LOG
#define AUTOATTACK_LOG(...) do { printf(__VA_ARGS__); fflush(stdout); } while(0)
#else
#define AUTOATTACK_LOG(...) ((void)0)
#endif

// ============================================================
// AutoAttackSkillController Implementation
// Automatic attack skill usage on current target
// ============================================================

// ============================================================
// Attack skill packet construction from DBBot Class16.smethod_1
// Packet: 0x7074 with skill ID and target ID
// ============================================================

// Send attack skill packet (DBBot format)
static void SendAttackSkillPacket(DWORD skillId, DWORD targetId) {
    // Format from DBBot Class16.smethod_1:
    // action=1, flag=4, skillId, 1, 1, targetId
    NEWMSG(0x7074)
    pReq << (BYTE)1;              // Action type (attack)
    pReq << (BYTE)4;              // Flag (4 = skill attack)
    pReq << skillId;              // Skill ID (4 bytes)
    pReq << (BYTE)1;              // Unknown (always 1)
    pReq << (BYTE)1;              // Unknown (always 1)  
    pReq << targetId;             // Target Unique ID (4 bytes)
    SENDMSG()
}

// Cast attack skill - sends raw 0x7074 packet with target
static bool CastAttackSkillWithTarget(DWORD skillId, DWORD targetId) {
    if (skillId == 0 || targetId == 0) {
        return false;
    }
    SendAttackSkillPacket(skillId, targetId);
    return true;
}


// Static member initialization
bool AutoAttackSkillController::s_initialized = false;
bool AutoAttackSkillController::s_enabled = false;  // OFF by default
DWORD AutoAttackSkillController::s_lastCastTime = 0;
DWORD AutoAttackSkillController::s_castIntervalMs = 500;  // Minimum 500ms between casts
int AutoAttackSkillController::s_currentSkillIndex = 0;
std::map<DWORD, DWORD> AutoAttackSkillController::s_skillLastCastTime;
DWORD AutoAttackSkillController::s_castingEndTime = 0;
std::set<DWORD> AutoAttackSkillController::s_previousSelectedSkills;

// Obstacle avoidance state
bool AutoAttackSkillController::s_isNavigatingToTarget = false;
DWORD AutoAttackSkillController::s_pendingTargetId = 0;
DWORD AutoAttackSkillController::s_pendingSkillId = 0;



void AutoAttackSkillController::Initialize() {
    if (s_initialized) return;
    
    s_initialized = true;
    s_lastCastTime = 0;
    s_currentSkillIndex = 0;
    s_previousSelectedSkills.clear();
}

void AutoAttackSkillController::SetEnabled(bool enabled) {
    s_enabled = enabled;
}

bool AutoAttackSkillController::IsEnabled() {
    return s_enabled;
}

std::vector<DWORD> AutoAttackSkillController::GetSelectedAttackSkills() {
    std::vector<DWORD> imbueSkills;
    std::vector<DWORD> regularSkills;
    
    // Get settings window
    if (!g_pCIFAutoHuntSettings) {
        return regularSkills;
    }
    
    // Get selected attack skill IDs from settings
    const DWORD* selectedSkillIds = g_pCIFAutoHuntSettings->GetSelectedAttackSkillIds();
    
    if (!selectedSkillIds) {
        return regularSkills;
    }
    
    // ========== IMBUE CHAIN IDs - PRIORITY SKILLS ==========
    // These skills are used FIRST before regular attack skills
    static const DWORD imbueChainIds[] = {
        237, 238, 239,  // Lightning imbue
        254, 255, 256,  // Cold imbue
        271, 272, 273,  // Fire imbue
        331, 334, 337   // EU imbue skills
    };
    static const int imbueChainCount = sizeof(imbueChainIds) / sizeof(imbueChainIds[0]);
    
    // Native function to get skill object for chain_id lookup
    typedef DWORD (__thiscall *GetSkillObj_t)(void*, DWORD);
    GetSkillObj_t GetSkillObj = (GetSkillObj_t)0x616790;
    void* pSkillMgr = (void*)0xA01010;
    
    // Separate skills into imbue (priority) and regular
    for (int i = 0; i < SKILL_SLOT_COUNT; i++) {
        DWORD skillId = selectedSkillIds[i];
        if (skillId == 0) continue;
        
        // Get chain_id from SkillData + 0x04
        DWORD chainId = 0;
        DWORD pSkillObj = GetSkillObj(pSkillMgr, skillId);
        if (pSkillObj) {
            chainId = *(DWORD*)(pSkillObj + 0x10C + 0x04);
        }
        
        // Check if imbue skill
        bool isImbue = false;
        for (int j = 0; j < imbueChainCount; j++) {
            if (chainId == imbueChainIds[j]) { isImbue = true; break; }
        }
        
        if (isImbue) {
            imbueSkills.push_back(skillId);
        } else {
            regularSkills.push_back(skillId);
        }
    }
    
    // Return imbue skills FIRST, then regular skills
    std::vector<DWORD> result;
    result.insert(result.end(), imbueSkills.begin(), imbueSkills.end());
    result.insert(result.end(), regularSkills.begin(), regularSkills.end());
    
    return result;
}

// Get skill cast time from SkillData +0x44 (animation time in ms)
DWORD AutoAttackSkillController::GetSkillCastTime(DWORD skillId) {
    typedef DWORD (__thiscall *GetSkillObj_t)(void*, DWORD);
    GetSkillObj_t GetSkillObj = (GetSkillObj_t)0x616790;
    void* pSkillMgr = (void*)0xA01010;
    
    DWORD pSkillObj = GetSkillObj(pSkillMgr, skillId);
    if (pSkillObj) {
        DWORD pSkillData = pSkillObj + 0x10C;
        return *(DWORD*)(pSkillData + 0x44);
    }
    return 0;
}

// Get skill cooldown from SkillData +0x48 (cooldown time in ms)
DWORD AutoAttackSkillController::GetSkillCooldown(DWORD skillId) {
    typedef DWORD (__thiscall *GetSkillObj_t)(void*, DWORD);
    GetSkillObj_t GetSkillObj = (GetSkillObj_t)0x616790;
    void* pSkillMgr = (void*)0xA01010;
    
    DWORD pSkillObj = GetSkillObj(pSkillMgr, skillId);
    if (pSkillObj) {
        DWORD pSkillData = pSkillObj + 0x10C;
        return *(DWORD*)(pSkillData + 0x48);
    }
    return 0;
}

// Check if skill is ready (cooldown has passed)
bool AutoAttackSkillController::IsSkillReady(DWORD skillId) {
    DWORD now = GetTickCount();
    
    // Check if this skill has been cast before
    std::map<DWORD, DWORD>::iterator it = s_skillLastCastTime.find(skillId);
    if (it == s_skillLastCastTime.end()) {
        return true;  // Never cast, ready to use
    }
    
    DWORD lastCast = it->second;
    DWORD cooldown = GetSkillCooldown(skillId);
    
    return (now >= lastCast + cooldown);
}

// Check if character is currently casting (animation in progress)
bool AutoAttackSkillController::IsCasting() {
    DWORD now = GetTickCount();
    return (s_castingEndTime > 0 && now < s_castingEndTime);
}

// Check if currently navigating around obstacle to target
bool AutoAttackSkillController::IsNavigatingToTarget() {
    return s_isNavigatingToTarget && AutoMoveController::GetState() == PATROL_AVOIDING;
}

// Legacy function - returns max(castTime, cooldown) for wait time
DWORD AutoAttackSkillController::GetSkillDuration(DWORD skillId) {
    DWORD castTime = GetSkillCastTime(skillId);
    DWORD cooldown = GetSkillCooldown(skillId);
    return (castTime > cooldown) ? castTime : cooldown;
}

// Get skills that are ready to use (not on cooldown)
std::vector<DWORD> AutoAttackSkillController::GetReadySkills() {
    std::vector<DWORD> readySkills;
    std::vector<DWORD> selectedSkills = GetSelectedAttackSkills();
    
    for (size_t i = 0; i < selectedSkills.size(); i++) {
        DWORD skillId = selectedSkills[i];
        if (IsSkillReady(skillId)) {
            readySkills.push_back(skillId);
        }
    }
    
    return readySkills;
}

// Get skills that are on cooldown
std::vector<DWORD> AutoAttackSkillController::GetCooldownSkills() {
    std::vector<DWORD> cooldownSkills;
    std::vector<DWORD> selectedSkills = GetSelectedAttackSkills();
    
    for (size_t i = 0; i < selectedSkills.size(); i++) {
        DWORD skillId = selectedSkills[i];
        if (!IsSkillReady(skillId)) {
            cooldownSkills.push_back(skillId);
        }
    }
    
    return cooldownSkills;
}



bool AutoAttackSkillController::CastAttackSkill(DWORD skillId, DWORD targetId) {
    if (targetId == 0) {
        return false; // No target
    }
    
    // 0x7074 packet now includes target ID directly (DBBot format)
    // No need for separate 0x7045 target select packet
    

    
    return CastAttackSkillWithTarget(skillId, targetId);
}

void AutoAttackSkillController::Update() {
    if (!s_initialized) {
        Initialize();
    }
    
    if (!s_enabled) return;
    
    DWORD now = GetTickCount();
    
    // ========== AUTO BERSERK CHECK ==========
    // Check if Hwan gauge is full (5) and Auto Berserk is enabled
    static DWORD s_lastBerserkCheckTime = 0;
    static DWORD s_lastBerserkUseTime = 0;
    
    if (now - s_lastBerserkCheckTime >= 500) {  // Check every 500ms
        s_lastBerserkCheckTime = now;
        
        // Check settings and player pointer
        if (g_pCIFAutoHuntSettings && g_pCIFAutoHuntSettings->IsAutoBerserkChecked()) {
            DWORD playerPtr = *(DWORD*)0xA0465C;
            if (playerPtr) {
                BYTE hwanPoint = *(BYTE*)(playerPtr + 0x6F7);  // Hwan gauge (0-5)
                
                // Log current Hwan point periodically
                static DWORD lastHwanLog = 0;
                if (now - lastHwanLog >= 5000 && hwanPoint > 0) {  // Log every 5s if > 0
                    printf("[AutoBerserk] Hwan Point: %d/5\n", hwanPoint);
                    fflush(stdout);
                    lastHwanLog = now;
                }
                
                // If Hwan is full (5) and we haven't used it recently (2s cooldown)
                if (hwanPoint == 5 && now - s_lastBerserkUseTime >= 2000) {
                    printf("[AutoBerserk] Hwan FULL! Using Berserk...\n");
                    fflush(stdout);
                    
                    // Send Hwan USE packet: opcode 0x70A7, data: 01
                    NEWMSG(0x70A7)
                    pReq << (BYTE)1;
                    SENDMSG()
                    
                    s_lastBerserkUseTime = now;
                    printf("[AutoBerserk] Berserk packet sent!\n");
                    fflush(stdout);
                }
            }
        }
    }
    
    // ========== RETURN TO TOWN CONTROLLER (Death & Low Item Detection) ==========
    ReturnToTownController::Update();
    
    // If player is dead, don't process any other logic
    if (ReturnToTownController::IsPlayerDead()) {
        return;
    }
    
    // BUFF PRIORITY: Don't do anything if buffs are missing
    std::vector<DWORD> missingBuffsEarly = AutoBuffController::GetMissingBuffs();
    if (!missingBuffsEarly.empty()) {
        static DWORD lastBuffLog = 0;
        if (now - lastBuffLog > 2000) {
            AUTOATTACK_LOG("[AutoAttack] BLOCKED: %d buffs missing\n", (int)missingBuffsEarly.size());
            lastBuffLog = now;
        }
        return; // Wait for all buffs first
    }
    
    // ========== IMBUE CHECK (BEFORE CASTING CHECK) ==========
    // Imbue can be cast AT ANY TIME, even during other skill animations

    static const DWORD imbueChainIds[] = {
        237, 238, 239, 254, 255, 256, 271, 272, 273, 331, 334, 337
    };
    static const int imbueChainCount = sizeof(imbueChainIds) / sizeof(imbueChainIds[0]);
    
    typedef DWORD (__thiscall *GetSkillObj_t)(void*, DWORD);
    GetSkillObj_t GetSkillObj = (GetSkillObj_t)0x616790;
    void* pSkillMgr = (void*)0xA01010;
    
    // Rate limit imbue check (200ms minimum)
    static DWORD s_lastImbueCheckTime = 0;
    static DWORD s_lastImbueCastTime = 0;  // Track when we last cast imbue
    
    if (now - s_lastImbueCheckTime >= 200) {
        s_lastImbueCheckTime = now;
        
        // Check if target exists first
        DWORD targetId = AutoTargetController::GetCurrentTargetID();
        if (targetId != 0) {
            // Check all selected skills for imbue needing cast
            std::vector<DWORD> selectedSkills = GetSelectedAttackSkills();
            for (size_t i = 0; i < selectedSkills.size(); i++) {
                DWORD skillId = selectedSkills[i];
                
                // Get chain_id
                DWORD chainId = 0;
                DWORD pSkillObj = GetSkillObj(pSkillMgr, skillId);
                if (pSkillObj) {
                    chainId = *(DWORD*)(pSkillObj + 0x10C + 0x04);
                }
                
                bool isImbue = false;
                for (int j = 0; j < imbueChainCount; j++) {
                    if (chainId == imbueChainIds[j]) { isImbue = true; break; }
                }
                
                if (isImbue) {
                    // Check if imbue buff is NOT active
                    bool imbueBuffActive = ActiveBuffManager::IsBuffActive(skillId);
                    
                    // Also check time-based lockout (1000ms after last imbue cast)
                    bool recentlycastImbue = (now - s_lastImbueCastTime < 1000);

                    
                    if (!imbueBuffActive && !recentlycastImbue) {
                        
                        if (CastAttackSkill(skillId, targetId)) {
                            AUTOATTACK_LOG("[AutoAttack] Casting IMBUE skill %d\n", skillId);
                            s_skillLastCastTime[skillId] = now;
                            s_lastImbueCastTime = now;
                            // Only set casting end time if not already casting
                            if (s_castingEndTime == 0 || now >= s_castingEndTime) {
                                s_castingEndTime = now + 100;
                            }
                            return;  // Don't cast regular skills in same Update
                        }
                        break;
                    }


                }
            }
        }
    }

    
    // CASTING CHECK: Don't cast regular skills while animation is playing
    // Also add safety timeout - max 5 seconds, something is wrong if longer
    static DWORD s_castingStartTime = 0;
    if (s_castingEndTime > 0 && now < s_castingEndTime) {
        // Track when casting started for timeout detection
        if (s_castingStartTime == 0) {
            s_castingStartTime = now;
        }
        
        // Safety timeout: If we've been "casting" for more than 5 seconds, something is wrong
        if (now - s_castingStartTime > 5000) {
            AUTOATTACK_LOG("[AutoAttack] WARNING: Casting stuck for 5s, resetting!\n");
            s_castingEndTime = 0;
            s_castingStartTime = 0;
        } else {
            static DWORD lastCastLog = 0;
            if (now - lastCastLog > 1000) {
                AUTOATTACK_LOG("[AutoAttack] BLOCKED: Still casting (ends in %dms)\n", s_castingEndTime - now);
                lastCastLog = now;
            }
            return; // Still casting/animating (but imbue was already checked above)
        }
    } else {
        // Not casting anymore, reset start time
        s_castingStartTime = 0;
    }
    
    // Rate limit regular casts (minimum interval between skills)
    if (now - s_lastCastTime < s_castIntervalMs) {
        return;
    }
    
    // BUFF PRIORITY: Don't attack if buffs are missing
    std::vector<DWORD> missingBuffs = AutoBuffController::GetMissingBuffs();
    if (!missingBuffs.empty()) {
        return; // Wait for buffs to be cast first
    }

    
    // Refresh target selection
    bool hasTarget = AutoTargetController::SelectClosestTarget();
    if (!hasTarget || !AutoTargetController::HasValidTarget()) {
        static DWORD lastNoTargetLog = 0;
        if (now - lastNoTargetLog > 3000) {
            AUTOATTACK_LOG("[AutoAttack] BLOCKED: No valid target\n");
            lastNoTargetLog = now;
        }
        s_isNavigatingToTarget = false;  // No target, stop navigation
        return; // No valid target
    }
    
    DWORD targetId = AutoTargetController::GetCurrentTargetID();
    
    // =========== OBSTACLE AVOIDANCE CHECK ===========
    // If currently navigating around obstacle, wait until done
    if (s_isNavigatingToTarget) {
        PatrolState state = AutoMoveController::GetState();
        if (state == PATROL_AVOIDING) {
            // Still navigating, don't cast skill yet
            return;
        } else {
            // Navigation complete, can cast skill now
            {
                FILE* fp = fopen("navmeshlog.txt", "a");
                if (fp) { fprintf(fp, "[AutoAttack] Nav complete\n"); fflush(fp); fclose(fp); }
            }
            s_isNavigatingToTarget = false;
        }
    }
    
    // Find target entity position using GetNearbyMonsters
    float targetX = 0, targetY = 0, targetZ = 0;
    std::vector<MonsterInfo> monsters = AutoTargetController::GetNearbyMonsters();
    for (size_t i = 0; i < monsters.size(); i++) {
        if (monsters[i].uniqueID == targetId) {
            targetX = monsters[i].posX;
            targetY = monsters[i].posY;
            targetZ = monsters[i].posZ;
            break;
        }
    }
    
    // Check if path to target is blocked
    if (targetX != 0 || targetZ != 0) {
        DWORD playerAddr = GetPlayerAddressRaw();
        if (playerAddr) {
            D3DVECTOR playerPos = GetLocationRaw(playerAddr);
            D3DVECTOR targetPos;
            targetPos.x = targetX;
            targetPos.y = targetY;
            targetPos.z = targetZ;
            
            D3DVECTOR objCenter;
            float objRadius;
            
            if (NavMeshCollision::GetBlockingObject(playerPos, targetPos, objCenter, objRadius)) {
                // Path is blocked! Start navigation around obstacle
                // printf("[AutoAttack] Blocked at (%.0f,%.0f) r=%.0f, navigating\n", objCenter.x, objCenter.z, objRadius);
                
                s_isNavigatingToTarget = true;
                s_pendingTargetId = targetId;
                
                // Use AutoMoveController to navigate around
                AutoMoveController::MoveToWithAvoidance(targetX, targetZ);
                return;  // Don't cast skill, wait for navigation
            }
        }
    }

    // Get selected attack skills
    std::vector<DWORD> selectedSkills = GetSelectedAttackSkills();
    
    if (selectedSkills.empty()) {
        static bool loggedEmpty = false;
        if (!loggedEmpty) {
            AUTOATTACK_LOG("[AutoAttack] No skills selected!\n");
            fflush(stdout);
            loggedEmpty = true;
        }
        return;
    }
    
    // Log skill changes
    std::set<DWORD> currentSet(selectedSkills.begin(), selectedSkills.end());
    if (currentSet != s_previousSelectedSkills) {
        AUTOATTACK_LOG("[AutoAttack] Skills (%d): ", (int)selectedSkills.size());
        for (size_t i = 0; i < selectedSkills.size(); i++) {
            printf("%d ", selectedSkills[i]);
        }
        printf("\n");
        fflush(stdout);
        s_previousSelectedSkills = currentSet;
    }
    
    // ========== REGULAR SKILL ROTATION ==========
    // (Imbue is already handled above, before casting check)
    DWORD skillToCast = 0;
    for (size_t i = 0; i < selectedSkills.size(); i++) {
        int idx = (s_currentSkillIndex + (int)i) % (int)selectedSkills.size();
        DWORD skillId = selectedSkills[idx];
        
        // Skip imbue skills here (they're handled above)
        typedef DWORD (__thiscall *GetSkillObj_t)(void*, DWORD);
        GetSkillObj_t GetSkillObj = (GetSkillObj_t)0x616790;
        void* pSkillMgr = (void*)0xA01010;
        
        DWORD chainId = 0;
        DWORD pSkillObj = GetSkillObj(pSkillMgr, skillId);
        if (pSkillObj) {
            chainId = *(DWORD*)(pSkillObj + 0x10C + 0x04);
        }
        
        static const DWORD imbueChainIds[] = {
            237, 238, 239, 254, 255, 256, 271, 272, 273, 331, 334, 337
        };
        static const int imbueChainCount = sizeof(imbueChainIds) / sizeof(imbueChainIds[0]);
        
        bool isImbue = false;
        for (int j = 0; j < imbueChainCount; j++) {
            if (chainId == imbueChainIds[j]) { isImbue = true; break; }
        }
        
        if (isImbue) continue;  // Skip imbues
        
        if (IsSkillReady(skillId)) {
            skillToCast = skillId;
            s_currentSkillIndex = (idx + 1) % (int)selectedSkills.size();
            break;
        }
    }

    
    if (skillToCast == 0) {
        return; // All skills on cooldown
    }

    
    // Cast the skill
    if (CastAttackSkill(skillToCast, targetId)) {
        s_lastCastTime = now;
        
        // Record when this skill was cast (for cooldown tracking)
        s_skillLastCastTime[skillToCast] = now;
        
        // Set casting end time ONLY if not already casting (prevent accumulation)
        if (s_castingEndTime == 0 || now >= s_castingEndTime) {
            DWORD castTime = GetSkillCastTime(skillToCast);
            if (castTime > 0) {
                s_castingEndTime = now + castTime + 200;
            } else {
                s_castingEndTime = now + 200;  // 200ms minimum for no cast time skills
            }
        }
    }
}


