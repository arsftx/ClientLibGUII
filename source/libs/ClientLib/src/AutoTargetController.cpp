#include "AutoTargetController.h"
#include "IFAutoHuntSettings.h"
#include "ICPlayer.h"
#include "EntityManagerClient.h"
#include "IGIDObject.h"
#include "ECSRO_Classes.h"
#include <ClientNet/MsgStreamBuffer.h>
#include <cstdio>
#include <cmath>
#include <cstring>

// Debug logging flag - comment out to disable verbose logs
// #define AUTOTARGET_DEBUG_LOG

#ifdef AUTOTARGET_DEBUG_LOG
#define AUTOTARGET_LOG(...) do { printf(__VA_ARGS__); fflush(stdout); } while(0)
#else
#define AUTOTARGET_LOG(...) ((void)0)
#endif

// ============================================================
// AutoTargetController Implementation
// Automatic monster targeting for auto-hunt system
// Using STLibrary approach with g_pGfxEttManager->entities
// ============================================================

// Opcode for selecting target
#define OPCODE_SELECT_TARGET 0x7045

// Send select target packet (internal helper)
static void SendSelectTargetPacket(DWORD targetUniqueID) {
    NEWMSG(OPCODE_SELECT_TARGET)
    pReq << targetUniqueID;
    SENDMSG()
}

// Public function to send target packet - call before skill cast
void AutoTargetController::SendTargetPacket(DWORD targetId) {
    if (targetId == 0) return;
    SendSelectTargetPacket(targetId);
}

// Helper function for readable timestamp (HH:MM:SS.mmm)
static const char* GetTimeStr() {
    static char buf[16];
    DWORD tick = GetTickCount();
    DWORD ms = tick % 1000;
    DWORD sec = (tick / 1000) % 60;
    DWORD min = (tick / 60000) % 60;
    DWORD hr = (tick / 3600000) % 24;
    sprintf(buf, "%02lu:%02lu:%02lu.%03lu", hr, min, sec, ms);
    return buf;
}

// Static member initialization
bool AutoTargetController::s_initialized = false;
bool AutoTargetController::s_enabled = false;
DWORD AutoTargetController::s_currentTargetID = 0;
DWORD AutoTargetController::s_lastTargetSearchTime = 0;
DWORD AutoTargetController::s_targetSearchIntervalMs = 10;  // Search every 10ms (instant targeting)
static DWORD s_lastTargetChangeTime = 0;  // Track when target was selected

float AutoTargetController::s_startPosX = 0.0f;
float AutoTargetController::s_startPosY = 0.0f;
float AutoTargetController::s_startPosZ = 0.0f;
bool AutoTargetController::s_hasStartPosition = false;

void AutoTargetController::Initialize() {
    if (s_initialized) return;
    
    s_initialized = true;
    s_currentTargetID = 0;
    s_lastTargetSearchTime = 0;
    s_hasStartPosition = false;
}

void AutoTargetController::SetEnabled(bool enabled) {
    if (enabled && !s_enabled) {
        SaveStartPosition();
    } else if (!enabled && s_enabled) {
        s_hasStartPosition = false;
        s_currentTargetID = 0;
    }
    s_enabled = enabled;
}

bool AutoTargetController::IsEnabled() {
    return s_enabled;
}

DWORD AutoTargetController::GetCurrentTargetID() {
    return s_currentTargetID;
}

// Game Manager address and target offset (from IDA analysis of sub_48B0A0)
#define ECSRO_ADDR_GAME_MANAGER 0xC5DD24
#define ECSRO_OFFSET_TARGET_UID 0x68C

// Debug logging for target
static DWORD s_lastLoggedGameTarget = 0;
static DWORD s_lastLoggedOurTarget = 0;

DWORD AutoTargetController::GetGameCurrentTarget() {
    // Read game manager pointer
    DWORD* pManager = (DWORD*)ECSRO_ADDR_GAME_MANAGER;
    if (!pManager || IsBadReadPtr(pManager, sizeof(DWORD))) {
        return 0;
    }
    
    DWORD manager = *pManager;
    if (!manager || IsBadReadPtr((void*)(manager + ECSRO_OFFSET_TARGET_UID), sizeof(DWORD))) {
        return 0;
    }
    
    // Read current target unique ID from manager + 0x68C
    DWORD targetUID = *(DWORD*)(manager + ECSRO_OFFSET_TARGET_UID);
    return targetUID;
}

// Grace period for server confirmation (ms)
#define TARGET_CONFIRM_GRACE_MS 500

bool AutoTargetController::HasValidTarget() {
    // No target selected by us
    if (s_currentTargetID == 0) {
        return false;
    }
    
    // Check game's actual target
    DWORD gameTarget = GetGameCurrentTarget();
    
    // We have a tracked target - trust it
    return true;
}

void AutoTargetController::GetStartPosition(float& x, float& y, float& z) {
    x = s_startPosX;
    y = s_startPosY;
    z = s_startPosZ;
}

DWORD AutoTargetController::GetLastTargetChangeTime() {
    return s_lastTargetChangeTime;
}

// Static to track last range for change detection
static float s_lastRange = -1.0f;

float AutoTargetController::GetRangeInGameUnits() {
    // Get range from AutoHuntSettings if available
    float range = 100.0f; // Default 100 game units
    
    if (g_pCIFAutoHuntSettings) {
        range = g_pCIFAutoHuntSettings->GetRangeValue();
    }
    
    if (range != s_lastRange) {
        s_lastRange = range;
    }
    
    return range;
}

void AutoTargetController::SaveStartPosition() {
    if (!g_pCICPlayer) {
        s_hasStartPosition = false;
        return;
    }
    
    DWORD playerAddr = (DWORD)g_pCICPlayer;
    s_startPosX = *(float*)(playerAddr + 0x74);
    s_startPosY = *(float*)(playerAddr + 0x78);
    s_startPosZ = *(float*)(playerAddr + 0x7C);
    s_hasStartPosition = true;
}

bool AutoTargetController::GetPlayerPosition(float& x, float& y, float& z) {
    if (!g_pCICPlayer) return false;
    
    // Use direct offset 0x74 for location (ECSRO)
    DWORD playerAddr = (DWORD)g_pCICPlayer;
    x = *(float*)(playerAddr + 0x74);
    y = *(float*)(playerAddr + 0x78);
    z = *(float*)(playerAddr + 0x7C);
    return true;
}


bool AutoTargetController::GetEntityPosition(DWORD entityAddr, float& x, float& y, float& z) {
    if (!entityAddr || entityAddr < 0x10000) return false;
    
    // Check if memory is readable
    if (IsBadReadPtr((void*)(entityAddr + 0x74), 12)) return false;
    
    // ECSRO CIObject::location offset = 0x74
    x = *(float*)(entityAddr + 0x74);
    y = *(float*)(entityAddr + 0x74 + 4);
    z = *(float*)(entityAddr + 0x74 + 8);
    return true;
}

// 2D Distance calculation (DBBot style - horizontal plane only, no height)
// SRO coords: X = horizontal X, Y = height, Z = horizontal Y
// So 2D distance = sqrt(dx*dx + dz*dz), NOT dy!
float AutoTargetController::CalculateDistance(float x1, float y1, float z1, float x2, float y2, float z2) {
    float dx = x2 - x1;
    float dz = z2 - z1;  // Use Z, not Y! Y is height.
    return sqrtf(dx*dx + dz*dz);
}

// Static variable to track last monster count for change detection
static int s_lastMonsterCount = -1;

std::vector<MonsterInfo> AutoTargetController::GetNearbyMonsters() {
    std::vector<MonsterInfo> monsters;
    
    // Must have start position (set when enabled)
    if (!s_hasStartPosition) {
        return monsters;
    }
    
    // Get player position using ECSRO raw memory access
    DWORD playerAddr = GetPlayerAddressRaw();
    if (!playerAddr) {
        return monsters;
    }
    
    D3DVECTOR playerLoc = GetLocationRaw(playerAddr);
    float playerX = playerLoc.x;
    float playerY = playerLoc.y;
    float playerZ = playerLoc.z;
    
    float range = GetRangeInGameUnits();
    
    // Get all entities using ECSRO raw memory access
    std::vector<EntityInfo> allEntities = GetAllEntitiesRaw();
    
    int monsterCount = 0;
    for (size_t i = 0; i < allEntities.size(); i++) {
        EntityInfo& ent = allEntities[i];
        
        // Validate entity pointer
        if (!ent.address || IsBadReadPtr((void*)ent.address, 0x80)) {
            continue;
        }
        
        // Get runtime class to check if monster
        DWORD vtable = *(DWORD*)ent.address;
        if (!vtable || IsBadReadPtr((void*)vtable, 4)) {
            continue;
        }
        
        // Call GetRuntimeClass (first virtual function)
        typedef CGfxRuntimeClass* (__thiscall *GetRuntimeClassFn)(void*);
        GetRuntimeClassFn getRuntimeClass = *(GetRuntimeClassFn*)vtable;
        
        if (IsBadReadPtr((void*)getRuntimeClass, 4)) {
            continue;
        }
        
        CGfxRuntimeClass const* rtClass = getRuntimeClass((void*)ent.address);
        
        if (!rtClass || IsBadReadPtr((void*)rtClass, 4)) {
            continue;
        }
        
        const char* className = rtClass->m_lpszClassName;
        if (!className || IsBadReadPtr((void*)className, 1)) {
            continue;
        }
        
        // Check if CICMonster
        if (strcmp(className, "CICMonster") == 0) {
            // Read UniqueID first
            DWORD uniqueID = *(DWORD*)(ent.address + 0xE0);
            
            // Skip monsters with invalid UniqueID (0 = dying/invalid)
            if (uniqueID == 0) {
                continue;
            }
            
            // Check if monster is dead (skip dead monsters)
            // ActionState offset 0x1AF: 2 = dead (from 0x30BF packet handler)
            // State offset 0x1E6: 4 = dead (alternate flag)
            // Flag offset 0x63C: non-zero = invalid
            BYTE actionState = *(BYTE*)(ent.address + 0x1AF);
            BYTE state = *(BYTE*)(ent.address + 0x1E6);
            BYTE invalidFlag = *(BYTE*)(ent.address + 0x63C);
            
            if (actionState == 2 || state == 4 || invalidFlag != 0) {
                continue; // Skip dead/invalid monsters (no spam log)
            }
            
            monsterCount++;
            
            // Get monster location
            D3DVECTOR monsterLoc = GetLocationRaw(ent.address);
            float mx = monsterLoc.x;
            float my = monsterLoc.y;
            float mz = monsterLoc.z;
            
            // Calculate distances
            float distanceFromPlayer = CalculateDistance(playerX, playerY, playerZ, mx, my, mz);
            float distanceFromStart = CalculateDistance(s_startPosX, s_startPosY, s_startPosZ, mx, my, mz);
            
            // Filter: Only include monsters within range of CENTER
            // But sort by: Distance from PLAYER (DBBot style)
            if (distanceFromStart <= range) {
                MonsterInfo info;
                info.uniqueID = uniqueID;
                info.entityAddress = ent.address;
                info.posX = mx;
                info.posY = my;
                info.posZ = mz;
                info.distance = distanceFromPlayer;  // Use PLAYER distance for sorting (DBBot style)
                info.isHostile = true;
                
                monsters.push_back(info);
            }
        }
    }
    
    s_lastMonsterCount = (int)monsters.size();
    
    return monsters;
}


bool AutoTargetController::SelectClosestTarget() {
    std::vector<MonsterInfo> monsters = GetNearbyMonsters();
    
    if (monsters.empty()) {
        if (s_currentTargetID != 0) {
            AUTOTARGET_LOG("[AutoTarget] No monsters in range, clearing target %d\n", s_currentTargetID);
            s_currentTargetID = 0;
        }
        return false;
    }
    
    // Find current target in list and check if still valid
    bool currentTargetValid = false;
    float currentTargetDist = 999999.0f;
    
    if (s_currentTargetID != 0) {
        for (size_t i = 0; i < monsters.size(); i++) {
            if (monsters[i].uniqueID == s_currentTargetID) {
                currentTargetValid = true;
                currentTargetDist = monsters[i].distance;
                break;
            }
        }
        
        // If current target moved out of range, drop it
        if (!currentTargetValid) {
            AUTOTARGET_LOG("[AutoTarget] Target %d lost, searching new\n", s_currentTargetID);
            s_currentTargetID = 0;
        }
    }
    
    // If current target still valid and in range, keep it
    if (currentTargetValid) {
        return true;
    }
    
    // Current target is dead or no target - find closest monster
    MonsterInfo* closest = NULL;
    float minDistance = 999999.0f;
    
    for (size_t i = 0; i < monsters.size(); i++) {
        if (monsters[i].distance < minDistance) {
            minDistance = monsters[i].distance;
            closest = &monsters[i];
        }
    }
    
    if (closest) {
        DWORD oldTarget = s_currentTargetID;
        s_currentTargetID = closest->uniqueID;
        
        if (oldTarget != s_currentTargetID) {
            AUTOTARGET_LOG("[AutoTarget] Selected target %d (dist=%.0f)\n", s_currentTargetID, minDistance);
            SendSelectTargetPacket(s_currentTargetID);
        }
        return true;
    }
    
    return false;
}

void AutoTargetController::Update() {
    if (!s_initialized) {
        Initialize();
    }
    
    if (!s_enabled) return;
    
    // Rate limit target searches
    DWORD now = GetTickCount();
    if (now - s_lastTargetSearchTime < s_targetSearchIntervalMs) {
        return;
    }
    s_lastTargetSearchTime = now;
    
    // Always search for closest target
    SelectClosestTarget();
}
