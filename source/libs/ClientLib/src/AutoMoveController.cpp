#include "AutoMoveController.h"
#include "AutoTargetController.h"
#include "AutoBuffController.h"  // For buff check
#include "AutoAttackSkillController.h"  // For IsCasting check
#include "IFAutoHuntSettings.h"

#include "ECSRO_Classes.h"  // For GetPlayerAddressRaw, GetLocationRaw
#include <ClientNet/MsgStreamBuffer.h>
#include "GlobalHelpersThatHaveNoHomeYet.h"
#include <cstdio>
#include <cstdarg>
#include <cmath>

// Debug logging flag - comment out to disable verbose logs
// #define AUTOMOVE_DEBUG_LOG

#ifdef AUTOMOVE_DEBUG_LOG
#define AUTOMOVE_LOG(...) printf(__VA_ARGS__)
#else
#define AUTOMOVE_LOG(...) ((void)0)
#endif

// ============================================================
// AutoMoveController Implementation
// Walk packet sending and patrol logic
// ============================================================

// Walk opcode
#define OPCODE_WALK 0x7021

// Native function to send walk packet
// sub_668530: Sends 0x7021 with position data
#define ADDR_SendWalkPacket 0x668530

// Convert world position to sector format
static void WorldToSector(float worldX, float worldZ, WORD& region, WORD& xOffset, WORD& zOffset) {
    // Silkroad world coordinate system:
    // World coords are in game units (approx 10 units = 1 meter)
    // Each region is 1920x1920 units
    // Region ID = (sectorX << 8) | sectorZ where sector = worldPos / 1920
    
    int sectorX = (int)(worldX / 1920.0f);
    int sectorZ = (int)(worldZ / 1920.0f);
    
    // Clamp to valid range
    if (sectorX < 0) sectorX = 0;
    if (sectorZ < 0) sectorZ = 0;
    if (sectorX > 255) sectorX = 255;
    if (sectorZ > 255) sectorZ = 255;
    
    // Region ID format: (sectorZ << 8) | sectorX
    region = (WORD)((sectorZ << 8) | sectorX);
    
    // Offset within region (0-1919)
    xOffset = (WORD)fmod(worldX, 1920.0f);
    zOffset = (WORD)fmod(worldZ, 1920.0f);
    
    // Handle negative coords
    if (worldX < 0) xOffset = (WORD)(1920.0f + fmod(worldX, 1920.0f));
    if (worldZ < 0) zOffset = (WORD)(1920.0f + fmod(worldZ, 1920.0f));
}

// Static member initialization
bool AutoMoveController::s_initialized = false;
bool AutoMoveController::s_enabled = false;
PatrolState AutoMoveController::s_state = PATROL_IDLE;

D3DVECTOR AutoMoveController::s_targetPosition = {0, 0, 0};
D3DVECTOR AutoMoveController::s_centerPosition = {0, 0, 0};
float AutoMoveController::s_huntRange = 1000.0f;

int AutoMoveController::s_currentWaypointIndex = 0;

DWORD AutoMoveController::s_lastMoveTime = 0;
DWORD AutoMoveController::s_moveIntervalMs = 1000;  // Check every 1 second
DWORD AutoMoveController::s_waitStartTime = 0;
DWORD AutoMoveController::s_waitDurationMs = 3000;  // Wait 3 seconds at each waypoint

// Obstacle avoidance waypoint queue
std::vector<PathWaypoint> AutoMoveController::s_avoidanceWaypoints;
int AutoMoveController::s_currentAvoidanceIndex = 0;
D3DVECTOR AutoMoveController::s_finalTarget = {0, 0, 0};

// Stuck detection for avoidance
static D3DVECTOR s_lastAvoidancePos = {0, 0, 0};
static DWORD s_lastAvoidanceMoveTime = 0;
static int s_stuckCounter = 0;

// Log helper
void AutoMoveController::Log(const char* format, ...) {
    // Commented out for performance - uncomment when debugging needed
    /*
    char buffer[512];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    // Console output
    printf("[AutoMove] %s\n", buffer);
    */
}

void AutoMoveController::Initialize() {
    if (s_initialized) return;
    
    s_initialized = true;
    s_state = PATROL_IDLE;
    s_currentWaypointIndex = 0;
}

void AutoMoveController::SetEnabled(bool enabled) {
    s_enabled = enabled;
    
    if (enabled) {
        AutoTargetController::GetStartPosition(
            s_centerPosition.x, 
            s_centerPosition.y, 
            s_centerPosition.z
        );
        
        if (g_pCIFAutoHuntSettings) {
            s_huntRange = g_pCIFAutoHuntSettings->GetRangeValue();
        } else {
            s_huntRange = 300.0f;
        }
        
        s_state = PATROL_IDLE;
        s_currentWaypointIndex = 0;
    } else {
        s_state = PATROL_IDLE;
    }
}

bool AutoMoveController::IsEnabled() {
    return s_enabled;
}

void AutoMoveController::SendWalkPacket(float x, float y, float z) {
    // Get player entity address
    DWORD playerAddr = GetPlayerAddressRaw();
    if (!playerAddr) {
        return;
    }
    
    // Get current player region
    WORD playerRegion = *(WORD*)(playerAddr + 0x70);
    
    // ============================================================
    // Build Walk Data Structure (passed to sub_41AB90)
    // ============================================================
    #pragma pack(push, 1)
    struct WalkData {
        BYTE type;     // +0x00 = 1 (ground click walk)
        BYTE pad;      // +0x01 = padding
        WORD region;   // +0x02 = region ID
        WORD posX;     // +0x04 = X coordinate
        WORD posY;     // +0x06 = Y coordinate (height)
        WORD posZ;     // +0x08 = Z coordinate
    };
    #pragma pack(pop)
    
    WalkData walkData;
    walkData.type = 1;
    walkData.pad = 0;
    walkData.region = playerRegion;
    walkData.posX = (WORD)(short)x;
    walkData.posY = (WORD)(short)y;
    walkData.posZ = (WORD)(short)z;
    
    // ============================================================
    // Packet System Functions (from IDA analysis)
    // ============================================================
    
    typedef int (__cdecl *CanSendPacket_t)(int opcode, int flags);
    CanSendPacket_t CanSendPacket = (CanSendPacket_t)0x5E4220;
    
    typedef DWORD (__thiscall *AllocBuffer_t)(void* bufferMgr);
    AllocBuffer_t AllocBuffer = (AllocBuffer_t)0x41D210;
    
    typedef int (__thiscall *WriteBuffer_t)(void* packetWriter, const void* data, int size);
    WriteBuffer_t WriteBuffer = (WriteBuffer_t)0x41AB90;
    
    typedef int (__cdecl *SendPacket_t)(void* packetWriter);
    SendPacket_t SendPacket = (SendPacket_t)0x5E4340;
    
    typedef void (__thiscall *CleanupWriter_t)(void* packetWriter);
    CleanupWriter_t CleanupWriter = (CleanupWriter_t)0x41AAD0;
    
    void* bufferManager = (void*)0xA00EB0;
    void* packetVTable = (void*)0x93B674;
    
    // Check if we can send
    if (!CanSendPacket(0x7021, 0)) {
        return;
    }
    
    // Allocate buffer
    DWORD buffer = AllocBuffer(bufferManager);
    if (!buffer) {
        return;
    }
    
    // PacketWriter structure
    struct PacketWriter {
        void* vtable;
        int field_04;
        int field_08;
        char active;
        DWORD bufferPtr;
        DWORD bufferCopy;
        short opcode;
    };
    
    PacketWriter writer;
    writer.vtable = packetVTable;
    writer.field_04 = 0;
    writer.field_08 = 0;
    writer.active = 1;
    writer.bufferPtr = buffer;
    writer.bufferCopy = buffer;
    writer.opcode = 0x7021;
    
    // Write walk data
    WriteBuffer(&writer, &walkData.type, 1);
    WriteBuffer(&writer, &walkData.region, 2);
    WriteBuffer(&writer, &walkData.posX, 6);
    
    // Send the packet
    writer.active = 0;
    writer.bufferCopy = buffer;
    writer.field_04 = 0;
    SendPacket(&writer);
    
    // Update movement controller local state
    DWORD moveController = playerAddr + 0x4F4;
    *(WORD*)(moveController + 0x0C) = playerRegion;
    *(float*)(moveController + 0x10) = x;
    *(float*)(moveController + 0x14) = y;
    *(float*)(moveController + 0x18) = z;
    *(DWORD*)(moveController + 0x44) = 0;
    *(BYTE*)(moveController + 0x1C) = 1;
    *(DWORD*)(moveController + 0x20) = 0;
    
    // Cleanup packet writer
    writer.vtable = packetVTable;
    CleanupWriter(&writer);
}


void AutoMoveController::MoveTo(float x, float z) {
    // SAFETY CHECK: Never move to a point outside hunt range
    float dx = x - s_centerPosition.x;
    float dz = z - s_centerPosition.z;
    float distFromCenter = sqrtf(dx*dx + dz*dz);
    
    if (distFromCenter > s_huntRange) {
        // Target is outside range - clamp to safe distance (80% of range)
        float safeRadius = s_huntRange * 0.8f;
        float scale = safeRadius / distFromCenter;
        x = s_centerPosition.x + dx * scale;
        z = s_centerPosition.z + dz * scale;
        AUTOMOVE_LOG("[AutoMove] Clamped target from %.0f to %.0f (range=%.0f)\n", distFromCenter, safeRadius, s_huntRange);
        // fflush(stdout);
    }
    
    // Get player's current height (Y) to use as target height
    // This is more accurate than using center position height
    DWORD playerAddr = GetPlayerAddressRaw();
    float y = 0.0f;
    if (playerAddr) {
        D3DVECTOR playerLoc = GetLocationRaw(playerAddr);
        y = playerLoc.y;
    }
    
    s_targetPosition.x = x;
    s_targetPosition.y = y;
    s_targetPosition.z = z;
    
    SendWalkPacket(x, y, z);
    
    s_state = PATROL_MOVING;
    s_lastMoveTime = GetTickCount();
}

void AutoMoveController::ReturnToCenter() {
    MoveTo(s_centerPosition.x, s_centerPosition.z);
    s_state = PATROL_RETURNING;
}

D3DVECTOR AutoMoveController::GetNextPatrolPoint() {
    D3DVECTOR waypoint = s_centerPosition;
    
    // Use 70% of range for patrol to ensure we stay inside the circle
    float safeRadius = s_huntRange * 0.7f;
    
    // Pattern: Edge -> Center -> Opposite Edge -> Center -> Next Edge...
    // N -> C -> S -> C -> E -> C -> W -> C -> (repeat)
    // Index: 0    1    2    3    4    5    6    7
    
    switch (s_currentWaypointIndex % 8) {
        case 0: // North
            waypoint.z += safeRadius;
            break;
        case 1: // Center
            break;
        case 2: // South (opposite of North)
            waypoint.z -= safeRadius;
            break;
        case 3: // Center
            break;
        case 4: // East
            waypoint.x += safeRadius;
            break;
        case 5: // Center
            break;
        case 6: // West (opposite of East)
            waypoint.x -= safeRadius;
            break;
        case 7: // Center
            break;
    }
    
    // Ensure coordinates don't go negative (world bounds)
    if (waypoint.x < 10.0f) waypoint.x = 10.0f;
    if (waypoint.z < 10.0f) waypoint.z = 10.0f;
    
    // Debug: Log patrol point and verify range
    float dx = waypoint.x - s_centerPosition.x;
    float dz = waypoint.z - s_centerPosition.z;
    float distFromCenter = sqrtf(dx*dx + dz*dz);
    AUTOMOVE_LOG("[AutoMove] Patrol #%d: (%.0f,%.0f) dist=%.0f range=%.0f safe=%.0f\n", 
           s_currentWaypointIndex % 8, waypoint.x, waypoint.z, distFromCenter, s_huntRange, safeRadius);
    fflush(stdout);
    
    s_currentWaypointIndex++;
    return waypoint;
}

float AutoMoveController::GetDistanceFromCenter() {
    // Use ECSRO_Classes helper for correct player address
    DWORD playerAddr = GetPlayerAddressRaw();
    if (!playerAddr) {
        // Player not loaded yet
        return 0.0f;
    }
    
    // Get player position using ECSRO_Classes helper
    D3DVECTOR playerLoc = GetLocationRaw(playerAddr);
    
    float dx = playerLoc.x - s_centerPosition.x;
    float dz = playerLoc.z - s_centerPosition.z;
    
    return sqrtf(dx*dx + dz*dz);
}

bool AutoMoveController::IsWithinRange() {
    float dist = GetDistanceFromCenter();
    return dist <= s_huntRange;
}

void AutoMoveController::StartPatrol() {
    if (s_state != PATROL_IDLE) return;
    
    D3DVECTOR nextPoint = GetNextPatrolPoint();
    MoveTo(nextPoint.x, nextPoint.z);
}

void AutoMoveController::StopMovement() {
    s_state = PATROL_IDLE;
}

bool AutoMoveController::IsMoving() {
    return s_state == PATROL_MOVING || s_state == PATROL_RETURNING || s_state == PATROL_AVOIDING;
}

bool AutoMoveController::HasClearPath(float x, float z) {
    DWORD playerAddr = GetPlayerAddressRaw();
    if (!playerAddr) return true;
    
    D3DVECTOR from = GetLocationRaw(playerAddr);
    D3DVECTOR to = {x, from.y, z};
    
    return !NavMeshCollision::IsPathBlocked(from, to);
}

void AutoMoveController::MoveToWithAvoidance(float x, float z) {
    // SAFETY CHECK: Don't even try to navigate to a point outside hunt range
    float dx = x - s_centerPosition.x;
    float dz = z - s_centerPosition.z;
    float distFromCenter = sqrtf(dx*dx + dz*dz);
    
    if (distFromCenter > s_huntRange) {
        AUTOMOVE_LOG("[AutoMove] MoveToWithAvoidance: Target (%.0f,%.0f) is outside range (%.0f > %.0f), skipping\n", 
               x, z, distFromCenter, s_huntRange);
        fflush(stdout);
        return;  // Don't attempt to navigate to targets outside range
    }
    
    DWORD playerAddr = GetPlayerAddressRaw();
    if (!playerAddr) return;
    
    D3DVECTOR from = GetLocationRaw(playerAddr);
    D3DVECTOR to = {x, from.y, z};
    
    Log("MoveToWithAvoidance: from(%.1f,%.1f) to(%.1f,%.1f)", from.x, from.z, x, z);
    
    // Check if direct path is blocked
    D3DVECTOR objCenter;
    float objRadius;
    
    if (NavMeshCollision::GetBlockingObject(from, to, objCenter, objRadius)) {
        Log("MoveToWithAvoidance: Path blocked by object at (%.1f,%.1f) r=%.1f",
            objCenter.x, objCenter.z, objRadius);
        
        // Generate waypoints around obstacle
        s_avoidanceWaypoints = NavMeshCollision::GeneratePathAroundObstacle(
            from, to, objCenter, objRadius);
        s_currentAvoidanceIndex = 0;
        s_finalTarget = to;
        
        if (!s_avoidanceWaypoints.empty()) {
            Log("MoveToWithAvoidance: Generated %d waypoints, moving to first",
                (int)s_avoidanceWaypoints.size());
            
            PathWaypoint& wp = s_avoidanceWaypoints[0];
            MoveTo(wp.position.x, wp.position.z);
            s_state = PATROL_AVOIDING;
            return;
        }
    }
    
    // Path is clear - direct movement
    Log("MoveToWithAvoidance: Path is clear, moving directly");
    s_avoidanceWaypoints.clear();
    MoveTo(x, z);
}

void AutoMoveController::ProcessWaypointQueue() {
    if (s_avoidanceWaypoints.empty()) {
        s_state = PATROL_IDLE;
        s_stuckCounter = 0;
        return;
    }
    if (s_state != PATROL_AVOIDING) return;
    
    DWORD playerAddr = GetPlayerAddressRaw();
    if (!playerAddr) return;
    
    D3DVECTOR currentPos = GetLocationRaw(playerAddr);
    DWORD now = GetTickCount();
    
    if (s_currentAvoidanceIndex >= (int)s_avoidanceWaypoints.size()) {
        s_avoidanceWaypoints.clear();
        s_state = PATROL_IDLE;
        s_stuckCounter = 0;
        return;
    }
    
    // Stuck detection - check if character has moved
    float moveDist = sqrtf(
        (currentPos.x - s_lastAvoidancePos.x) * (currentPos.x - s_lastAvoidancePos.x) +
        (currentPos.z - s_lastAvoidancePos.z) * (currentPos.z - s_lastAvoidancePos.z)
    );
    
    if (moveDist > 20.0f) {
        // Character is moving - reset stuck counter
        s_lastAvoidancePos = currentPos;
        s_lastAvoidanceMoveTime = now;
        s_stuckCounter = 0;
    } else if (now - s_lastAvoidanceMoveTime > 1500) {
        // Stuck for 1.5+ seconds
        s_stuckCounter++;
        Log("STUCK #%d at (%.0f,%.0f)", s_stuckCounter, currentPos.x, currentPos.z);
        
        if (s_stuckCounter >= 3) {
            // Been stuck 3 times - give up completely
            Log("Giving up on avoidance");
            s_avoidanceWaypoints.clear();
            s_state = PATROL_IDLE;
            s_stuckCounter = 0;
            return;
        }
        
        // Get final target (last waypoint)
        PathWaypoint& finalTarget = s_avoidanceWaypoints.back();
        float finalX = finalTarget.position.x;
        float finalZ = finalTarget.position.z;
        
        // Clear current path
        s_avoidanceWaypoints.clear();
        s_currentAvoidanceIndex = 0;
        s_lastAvoidanceMoveTime = now;
        
        // Recalculate path from current position to final target
        D3DVECTOR targetPos;
        targetPos.x = finalX;
        targetPos.y = currentPos.y;
        targetPos.z = finalZ;
        
        D3DVECTOR objCenter;
        float objRadius;
        
        if (NavMeshCollision::GetBlockingObject(currentPos, targetPos, objCenter, objRadius)) {
            // Still blocked - generate new path
            Log("Recalculating path to (%.0f,%.0f)", finalX, finalZ);
            s_avoidanceWaypoints = NavMeshCollision::GeneratePathAroundObstacle(
                currentPos, targetPos, objCenter, objRadius);
            
            if (!s_avoidanceWaypoints.empty()) {
                PathWaypoint& firstWp = s_avoidanceWaypoints[0];
                MoveTo(firstWp.position.x, firstWp.position.z);
                s_state = PATROL_AVOIDING;
            } else {
                // No path generated - give up
                s_state = PATROL_IDLE;
            }
        } else {
            // Path is now clear - go directly to target
            Log("Path now clear, going to target");
            s_avoidanceWaypoints.push_back(PathWaypoint(finalX, currentPos.y, finalZ));
            MoveTo(finalX, finalZ);
            s_state = PATROL_AVOIDING;
        }
        return;
    }
    
    PathWaypoint& wp = s_avoidanceWaypoints[s_currentAvoidanceIndex];
    
    // Check if reached current waypoint
    float dx = currentPos.x - wp.position.x;
    float dz = currentPos.z - wp.position.z;
    float dist = sqrtf(dx*dx + dz*dz);
    
    // Removed verbose per-frame log
    
    if (dist < 50.0f) {  // Reached waypoint (smaller threshold for precision)
        wp.reached = true;
        s_currentAvoidanceIndex++;
        s_stuckCounter = 0;
        
        Log("WP %d reached", s_currentAvoidanceIndex);
        
        // Move to next waypoint or final target
        if (s_currentAvoidanceIndex < (int)s_avoidanceWaypoints.size()) {
            PathWaypoint& nextWp = s_avoidanceWaypoints[s_currentAvoidanceIndex];
            MoveTo(nextWp.position.x, nextWp.position.z);
            s_state = PATROL_AVOIDING;
        } else {
            // All waypoints done - clear queue and go to final target
            Log("ProcessWaypointQueue: All waypoints done, reached destination");
            s_avoidanceWaypoints.clear();
            s_state = PATROL_IDLE;
        }
    }
}

PatrolState AutoMoveController::GetState() {
    return s_state;
}

void AutoMoveController::Update() {
    if (!s_initialized) {
        Initialize();
    }
    
    if (!s_enabled) return;
    
    // Check WalkAround setting - if disabled, check Go Back Center logic
    if (g_pCIFAutoHuntSettings && !g_pCIFAutoHuntSettings->IsWalkAroundChecked()) {
        // WalkAround is OFF - check if Go Back Center is enabled and no target
        bool goBackCenterEnabled = g_pCIFAutoHuntSettings->IsGoBackCenterChecked();
        bool hasTarget = AutoTargetController::HasValidTarget();
        
        if (goBackCenterEnabled && !hasTarget) {
            // No target and Go Back Center is ON - return to center
            DWORD playerAddr = GetPlayerAddressRaw();
            if (playerAddr) {
                D3DVECTOR playerPos = GetLocationRaw(playerAddr);
                float dx = playerPos.x - s_centerPosition.x;
                float dz = playerPos.z - s_centerPosition.z;
                float distFromCenter = sqrtf(dx*dx + dz*dz);
                
                // Only move if not already at center (>50 units away)
                if (distFromCenter > 50.0f && s_state != PATROL_RETURNING) {
                    AUTOMOVE_LOG("[AutoMove] No target, returning to center (dist=%.0f)\n", distFromCenter);
                    ReturnToCenter();
                    return;
                }
            }
        }
        
        // If already returning, continue until done
        if (s_state == PATROL_RETURNING) {
            DWORD playerAddr = GetPlayerAddressRaw();
            if (playerAddr) {
                D3DVECTOR playerPos = GetLocationRaw(playerAddr);
                float dx = playerPos.x - s_centerPosition.x;
                float dz = playerPos.z - s_centerPosition.z;
                float distFromCenter = sqrtf(dx*dx + dz*dz);
                
                if (distFromCenter < 50.0f) {
                    s_state = PATROL_IDLE;
                    AUTOMOVE_LOG("[AutoMove] Reached center\n");
                }
            }
            return;
        }
        
        s_state = PATROL_IDLE;
        return;
    }
    
    // Don't move while character is casting skill
    if (AutoAttackSkillController::IsCasting()) {
        static DWORD lastCastingLog = 0;
        if (GetTickCount() - lastCastingLog > 1000) {
            AUTOMOVE_LOG("[AutoMove] BLOCKED: Character is casting\n");
            lastCastingLog = GetTickCount();
        }
        return;  // Wait for skill animation to finish
    }
    
    DWORD now = GetTickCount();
    
    // Rate limit updates (100ms minimum)
    static DWORD s_lastUpdateTime = 0;
    if (now - s_lastUpdateTime < 100) {
        return;
    }
    s_lastUpdateTime = now;

    
    // Sync range from settings (in case user changed it)
    if (g_pCIFAutoHuntSettings) {
        float newRange = g_pCIFAutoHuntSettings->GetRangeValue();
        if (newRange != s_huntRange) {
            s_huntRange = newRange;
        }
    }
    
    // Don't patrol until all buffs are applied
    // This prevents walking before character is ready
    std::vector<DWORD> missingBuffs = AutoBuffController::GetMissingBuffs();
    if (!missingBuffs.empty()) {
        static DWORD lastBuffLog = 0;
        if (GetTickCount() - lastBuffLog > 2000) {
            AUTOMOVE_LOG("[AutoMove] BLOCKED: %d buffs missing\n", (int)missingBuffs.size());
            lastBuffLog = GetTickCount();
        }
        return;
    }
    
    // If there's a target (mob to attack), stop patrolling
    // BUT don't stop if we're navigating around obstacle to reach target
    if (AutoTargetController::HasValidTarget()) {
        if (s_state == PATROL_AVOIDING) {
            // Continue avoidance navigation - don't interrupt!
            // Process the waypoint queue
        } else if (s_state != PATROL_IDLE) {
            static DWORD lastTargetLog = 0;
            if (GetTickCount() - lastTargetLog > 2000) {
                AUTOMOVE_LOG("[AutoMove] BLOCKED: Has target, going IDLE (state was %d)\n", s_state);
                lastTargetLog = GetTickCount();
            }
            s_state = PATROL_IDLE;
            return;
        } else {
            return;  // Already idle, let AutoAttackSkillController handle
        }
    }
    
    // Get current player position
    DWORD playerAddr = GetPlayerAddressRaw();
    if (!playerAddr) {
        AUTOMOVE_LOG("[AutoMove] BLOCKED: No player address\n");
        return;
    }
    D3DVECTOR currentPos = GetLocationRaw(playerAddr);
    
    // ========== RANGE CHECK WARNING LOG ==========
    // Check distance from center and warn if outside range
    float dx = currentPos.x - s_centerPosition.x;
    float dz = currentPos.z - s_centerPosition.z;
    float distFromCenter = sqrtf(dx*dx + dz*dz);
    
    static DWORD lastRangeWarnLog = 0;
    if (distFromCenter > s_huntRange && now - lastRangeWarnLog > 2000) {
        AUTOMOVE_LOG("[AutoMove] WARNING: OUTSIDE RANGE! dist=%.0f range=%.0f (%.0f%% over) state=%d\n", 
               distFromCenter, s_huntRange, ((distFromCenter - s_huntRange) / s_huntRange) * 100.0f, s_state);
        AUTOMOVE_LOG("[AutoMove] PlayerPos: (%.0f,%.0f) CenterPos: (%.0f,%.0f)\n", 
               currentPos.x, currentPos.z, s_centerPosition.x, s_centerPosition.z);
        fflush(stdout);
        lastRangeWarnLog = now;
    }
    
    // State machine (no target = keep patrolling continuously)
    switch (s_state) {
        case PATROL_IDLE:
            // Check if we're outside hunt range (only when IDLE)
            if (!IsWithinRange()) {
                AUTOMOVE_LOG("[AutoMove] Outside range (dist=%.0f), returning to center\n", distFromCenter);
                fflush(stdout);
                ReturnToCenter();
                return;
            }
            // Start patrol - continuously walk when no targets
            StartPatrol();
            break;
            
        case PATROL_WAITING:
        case PATROL_MOVING:
        case PATROL_RETURNING:
            {
                // Check if character has moved close to target position
                float distToTarget = sqrtf(
                    (currentPos.x - s_targetPosition.x) * (currentPos.x - s_targetPosition.x) +
                    (currentPos.z - s_targetPosition.z) * (currentPos.z - s_targetPosition.z)
                );
                
                // Only send next walk if:
                // 1. Close to target (reached destination) OR
                // 2. 5 seconds passed (stuck or can't reach - timeout)
                if (distToTarget < 100.0f || now - s_lastMoveTime >= 5000) {
                    // Get next patrol point
                    D3DVECTOR nextPoint = GetNextPatrolPoint();
                    
                    // Check if path is blocked
                    D3DVECTOR objCenter;
                    float objRadius;
                    if (NavMeshCollision::GetBlockingObject(currentPos, nextPoint, objCenter, objRadius)) {
                        // Path blocked - use avoidance
                        Log("Patrol: Path to (%.0f,%.0f) blocked, using avoidance", nextPoint.x, nextPoint.z);
                        MoveToWithAvoidance(nextPoint.x, nextPoint.z);
                    } else {
                        // Path clear
                        MoveTo(nextPoint.x, nextPoint.z);
                    }
                }
            }
            break;
            
        case PATROL_AVOIDING:
            {
                // Process waypoint queue for obstacle avoidance
                ProcessWaypointQueue();
                
                // Timeout check - if stuck for too long, skip to next waypoint
                if (now - s_lastMoveTime >= 5000) {
                    Log("Update: PATROL_AVOIDING timeout, skipping to next waypoint");
                    s_currentAvoidanceIndex++;
                    if (s_currentAvoidanceIndex < (int)s_avoidanceWaypoints.size()) {
                        PathWaypoint& nextWp = s_avoidanceWaypoints[s_currentAvoidanceIndex];
                        MoveTo(nextWp.position.x, nextWp.position.z);
                    } else {
                        // Done with avoidance, go back to idle
                        s_avoidanceWaypoints.clear();
                        s_state = PATROL_IDLE;
                    }
                }
            }
            break;
    }
}
