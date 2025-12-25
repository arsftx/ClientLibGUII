#pragma once

#include <d3dx9.h>
#include <vector>
#include <windows.h>

// Forward declarations
class CRTNavMeshTerrain;
struct SNavMeshInst;

// ============================================================
// PathWaypoint - Single waypoint in obstacle avoidance path
// ============================================================
struct PathWaypoint {
    D3DVECTOR position;
    bool reached;
    
    PathWaypoint() : reached(false) {
        position.x = 0; position.y = 0; position.z = 0;
    }
    
    PathWaypoint(float x, float y, float z) : reached(false) {
        position.x = x; position.y = y; position.z = z;
    }
};

// ============================================================
// AStarNode - Node for A* pathfinding on visibility graph
// ============================================================
struct AStarNode {
    float x, z;           // Position
    float gCost;          // Cost from start
    float hCost;          // Heuristic (distance to end)
    float fCost;          // gCost + hCost
    int parentIndex;      // Index of parent node in path
    
    AStarNode() : x(0), z(0), gCost(0), hCost(0), fCost(0), parentIndex(-1) {}
    AStarNode(float _x, float _z) : x(_x), z(_z), gCost(999999.0f), hCost(0), fCost(999999.0f), parentIndex(-1) {}
};

// ============================================================
// NavMeshCollision - Collision detection and path generation
// ============================================================
class NavMeshCollision {
public:
    // --------------------------------------------------------
    // Path Collision Detection
    // --------------------------------------------------------
    
    // Check if direct path from A to B is blocked by any object
    // Returns true if path is blocked
    static bool IsPathBlocked(const D3DVECTOR& from, const D3DVECTOR& to);
    
    // Get the blocking object's center and approximate radius
    // Returns true if a blocking object was found
    static bool GetBlockingObject(const D3DVECTOR& from, const D3DVECTOR& to,
                                   D3DVECTOR& outObjCenter, float& outObjRadius);
    
    // Get bounding box of ALL obstacles near a point (within searchRadius)
    // Returns true if any obstacles were found
    static bool GetNearbyObstacleCluster(const D3DVECTOR& point, float searchRadius,
                                          D3DVECTOR& outClusterCenter, float& outClusterRadius);
    
    // --------------------------------------------------------
    // Grid-Based Pathfinding
    // --------------------------------------------------------
    
    // Build 2D navigation grid from NavMesh edges
    // Called automatically on first pathfind, or when region changes
    static void BuildNavigationGrid();
    
    // Find path from start to end using A* on navigation grid
    static std::vector<PathWaypoint> FindPathAStar(
        const D3DVECTOR& start, 
        const D3DVECTOR& end,
        float searchRadius = 500.0f);
    
    // --------------------------------------------------------
    // Path Generation (Legacy - uses A* internally now)
    // --------------------------------------------------------
    
    // Generate waypoints around the blocking obstacle
    // Returns vector of waypoints that form a path around the object
    static std::vector<PathWaypoint> GeneratePathAroundObstacle(
        const D3DVECTOR& from, 
        const D3DVECTOR& to,
        const D3DVECTOR& objCenter,
        float objRadius);
    
    // --------------------------------------------------------
    // Geometry Helpers
    // --------------------------------------------------------
    
    // 2D line-segment intersection test (XZ plane)
    static bool LineSegmentIntersect2D(
        float x1, float z1, float x2, float z2,  // Line 1
        float x3, float z3, float x4, float z4); // Line 2
    
    // Get object bounding box from its edges (world space)
    static void GetObjectBounds(const SNavMeshInst* pInst, 
                                 D3DVECTOR& outMin, D3DVECTOR& outMax,
                                 D3DVECTOR& outCenter, float& outRadius);
    
    // Calculate 2D distance (XZ plane)
    static float Distance2D(const D3DVECTOR& a, const D3DVECTOR& b);
    
    // --------------------------------------------------------
    // Debug Logging
    // --------------------------------------------------------
    static void SetDebugLog(bool enabled) { s_debugLogEnabled = enabled; }
    static bool IsDebugLogEnabled() { return s_debugLogEnabled; }
    static void Log(const char* format, ...);
    
private:
    static bool s_debugLogEnabled;
};
