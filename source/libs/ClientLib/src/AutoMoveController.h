#pragma once

#include <windows.h>
#include <d3dx9.h>
#include <vector>
#include <NavMesh/NavMeshCollision.h>

// ============================================================
// AutoMoveController
// Manages automatic movement for Auto Hunt patrol system.
// 
// Features:
//   1. Walk to target position (0x7021 opcode)
//   2. Return to center when no mobs
//   3. Patrol within range
//   4. Stay within hunt range
//   5. Obstacle avoidance using NavMesh
// ============================================================

enum PatrolState {
    PATROL_IDLE,           // Not moving
    PATROL_MOVING,         // Walking to target
    PATROL_WAITING,        // Reached target, waiting
    PATROL_RETURNING,      // Returning to center
    PATROL_AVOIDING        // Following waypoints around obstacle
};

class AutoMoveController {
public:
    // Initialize the controller
    static void Initialize();
    
    // Main update loop - call periodically
    static void Update();
    
    // Enable/Disable movement
    static void SetEnabled(bool enabled);
    static bool IsEnabled();
    
    // Move to specific position (direct, no avoidance)
    static void MoveTo(float x, float z);
    
    // Move to position with obstacle avoidance
    static void MoveToWithAvoidance(float x, float z);
    
    // Process waypoint queue for obstacle avoidance
    static void ProcessWaypointQueue();
    
    // Check if path to target is clear
    static bool HasClearPath(float x, float z);
    
    // Return to center (start position)
    static void ReturnToCenter();
    
    // Start patrol mode
    static void StartPatrol();
    
    // Stop all movement
    static void StopMovement();
    
    // Check if currently moving
    static bool IsMoving();
    
    // Check if character is within hunt range
    static bool IsWithinRange();
    
    // Get current patrol state
    static PatrolState GetState();

private:
    // Send walk packet (0x7021)
    static void SendWalkPacket(float x, float y, float z);
    
    // Get next patrol waypoint
    static D3DVECTOR GetNextPatrolPoint();
    
    // Calculate distance from center
    static float GetDistanceFromCenter();
    
    // Log helper
    static void Log(const char* format, ...);

private:
    static bool s_initialized;
    static bool s_enabled;
    static PatrolState s_state;
    
    // Movement state
    static D3DVECTOR s_targetPosition;
    static D3DVECTOR s_centerPosition;  // Start/hunt position
    static float s_huntRange;
    
    // Patrol waypoints (N, S, E, W)
    static int s_currentWaypointIndex;
    
    // Obstacle avoidance waypoint queue
    static std::vector<PathWaypoint> s_avoidanceWaypoints;
    static int s_currentAvoidanceIndex;
    static D3DVECTOR s_finalTarget;  // Ultimate destination after avoidance
    
    // Timing
    static DWORD s_lastMoveTime;
    static DWORD s_moveIntervalMs;
    static DWORD s_waitStartTime;
    static DWORD s_waitDurationMs;
};
