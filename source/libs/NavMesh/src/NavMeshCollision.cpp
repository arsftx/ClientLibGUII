#include <NavMesh/NavMeshCollision.h>
#include <NavMesh/RTNavMesh.h>
#include <NavMesh/RTNavMeshTerrain.h>
#include <NavMesh/SNavMeshInst.h>
#include <NavMesh/RTNavMeshObj.h>
#include <cstdio>
#include <cstdarg>
#include <cmath>

// Static member initialization
bool NavMeshCollision::s_debugLogEnabled = true;

// ============================================================
// Debug Logging
// ============================================================
void NavMeshCollision::Log(const char* format, ...) {
    if (!s_debugLogEnabled) return;
    
    // Commented out for performance - uncomment when debugging needed
    /*
    char buffer[512];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    // Console output
    printf("[NavMesh] %s\n", buffer);
    */
}

// ============================================================
// Geometry Helpers
// ============================================================

float NavMeshCollision::Distance2D(const D3DVECTOR& a, const D3DVECTOR& b) {
    float dx = a.x - b.x;
    float dz = a.z - b.z;
    return sqrtf(dx * dx + dz * dz);
}

bool NavMeshCollision::LineSegmentIntersect2D(
    float x1, float z1, float x2, float z2,
    float x3, float z3, float x4, float z4) 
{
    // Calculate direction vectors
    float d1x = x2 - x1;
    float d1z = z2 - z1;
    float d2x = x4 - x3;
    float d2z = z4 - z3;
    
    // Calculate cross product denominator
    float denom = d1x * d2z - d1z * d2x;
    
    // Parallel lines
    if (fabsf(denom) < 0.0001f) return false;
    
    // Calculate parameters
    float t = ((x3 - x1) * d2z - (z3 - z1) * d2x) / denom;
    float u = ((x3 - x1) * d1z - (z3 - z1) * d1x) / denom;
    
    // Check if intersection is within both segments
    return (t >= 0.0f && t <= 1.0f && u >= 0.0f && u <= 1.0f);
}

void NavMeshCollision::GetObjectBounds(const SNavMeshInst* pInst, 
                                        D3DVECTOR& outMin, D3DVECTOR& outMax,
                                        D3DVECTOR& outCenter, float& outRadius) 
{
    // Get object mesh
    CRTNavMeshObj* pObj = *(CRTNavMeshObj**)((char*)pInst + 0xB0);
    if (!pObj) {
        outMin.x = outMin.y = outMin.z = 0;
        outMax.x = outMax.y = outMax.z = 0;
        outCenter.x = outCenter.y = outCenter.z = 0;
        outRadius = 0;
        return;
    }
    
    // Get object world position and rotation
    D3DVECTOR objOffset;
    objOffset.x = *(float*)((char*)pInst + 0x06);
    objOffset.y = *(float*)((char*)pInst + 0x0A);
    objOffset.z = *(float*)((char*)pInst + 0x0E);
    float yaw = *(float*)((char*)pInst + 0x14);
    
    // Get global edges from object (these define the boundary)
    DWORD objAddr = (DWORD)pObj;
    BYTE* edgesBegin = *(BYTE**)(objAddr + 0x20);
    BYTE* edgesEnd = *(BYTE**)(objAddr + 0x20 + 4);
    
    if (!edgesBegin || !edgesEnd || edgesBegin >= edgesEnd) {
        // No edges - use object position with small radius
        outMin = outMax = outCenter = objOffset;
        outRadius = 20.0f;  // Default small radius
        return;
    }
    
    // Initialize bounds
    outMin.x = outMin.z = 999999.0f;
    outMax.x = outMax.z = -999999.0f;
    outMin.y = outMax.y = objOffset.y;
    
    // Edge size is 60 bytes (0x3C)
    int edgeSize = 60;
    int numEdges = (int)(edgesEnd - edgesBegin) / edgeSize;
    
    // Iterate edges to find bounds
    for (int i = 0; i < numEdges && i < 100; i++) {
        DWORD edgeAddr = (DWORD)edgesBegin + (i * edgeSize);
        
        if (IsBadReadPtr((void*)edgeAddr, edgeSize)) continue;
        
        DWORD* pVertex0 = *(DWORD**)(edgeAddr + 0x04);
        DWORD* pVertex1 = *(DWORD**)(edgeAddr + 0x08);
        
        if (!pVertex0 || !pVertex1) continue;
        if (IsBadReadPtr(pVertex0, 12) || IsBadReadPtr(pVertex1, 12)) continue;
        
        // Get vertex positions (local space)
        float v0x = *(float*)((DWORD)pVertex0 + 0x00);
        float v0z = *(float*)((DWORD)pVertex0 + 0x08);
        float v1x = *(float*)((DWORD)pVertex1 + 0x00);
        float v1z = *(float*)((DWORD)pVertex1 + 0x08);
        
        // Rotate by yaw
        float cosY = cosf(-yaw);
        float sinY = sinf(-yaw);
        
        float r0x = v0x * cosY - v0z * sinY;
        float r0z = v0x * sinY + v0z * cosY;
        float r1x = v1x * cosY - v1z * sinY;
        float r1z = v1x * sinY + v1z * cosY;
        
        // Transform to world space
        float w0x = r0x + objOffset.x;
        float w0z = r0z + objOffset.z;
        float w1x = r1x + objOffset.x;
        float w1z = r1z + objOffset.z;
        
        // Update bounds
        if (w0x < outMin.x) outMin.x = w0x;
        if (w0z < outMin.z) outMin.z = w0z;
        if (w0x > outMax.x) outMax.x = w0x;
        if (w0z > outMax.z) outMax.z = w0z;
        if (w1x < outMin.x) outMin.x = w1x;
        if (w1z < outMin.z) outMin.z = w1z;
        if (w1x > outMax.x) outMax.x = w1x;
        if (w1z > outMax.z) outMax.z = w1z;
    }
    
    // Calculate center and radius
    outCenter.x = (outMin.x + outMax.x) / 2.0f;
    outCenter.y = objOffset.y;
    outCenter.z = (outMin.z + outMax.z) / 2.0f;
    
    float halfWidth = (outMax.x - outMin.x) / 2.0f;
    float halfDepth = (outMax.z - outMin.z) / 2.0f;
    outRadius = sqrtf(halfWidth * halfWidth + halfDepth * halfDepth);
    
    // Add some margin
    outRadius += 30.0f;
}

// ============================================================
// Path Collision Detection
// ============================================================

bool NavMeshCollision::IsPathBlocked(const D3DVECTOR& from, const D3DVECTOR& to) {
    D3DVECTOR objCenter;
    float objRadius;
    return GetBlockingObject(from, to, objCenter, objRadius);
}

bool NavMeshCollision::GetBlockingObject(const D3DVECTOR& from, const D3DVECTOR& to,
                                          D3DVECTOR& outObjCenter, float& outObjRadius) 
{
    // Get current NavMesh
    CRTNavMeshTerrain* pNavMesh = GetCurrentNavMesh();
    if (!pNavMesh) {
        Log("GetBlockingObject: No NavMesh available");
        return false;
    }
    
    // Removed verbose log
    
    // Get object list using raw memory access
    DWORD baseAddr = (DWORD)pNavMesh;
    SNavMeshInst** objListBegin = *(SNavMeshInst***)(baseAddr + 0x14);
    SNavMeshInst** objListEnd = *(SNavMeshInst***)(baseAddr + 0x14 + 4);
    
    if (!objListBegin || !objListEnd) {
        Log("GetBlockingObject: No objects in region");
        return false;
    }
    
    int numObjects = (int)(objListEnd - objListBegin);
    // Removed verbose log
    
    float closestDist = 999999.0f;
    bool foundBlocking = false;
    
    for (int i = 0; i < numObjects && i < 100; i++) {
        SNavMeshInst* pInst = objListBegin[i];
        if (!pInst) continue;
        
        // Get object bounds
        D3DVECTOR objMin, objMax, objCenter;
        float objRadius;
        GetObjectBounds(pInst, objMin, objMax, objCenter, objRadius);
        
        if (objRadius < 1.0f) continue;  // Skip invalid objects
        if (objRadius > 500.0f) continue;  // Skip only map boundaries (very large)
        
        // Get object's global edges for precise intersection test
        CRTNavMeshObj* pObj = *(CRTNavMeshObj**)((char*)pInst + 0xB0);
        if (!pObj) continue;
        
        // Get object position and rotation
        D3DVECTOR objOffset;
        objOffset.x = *(float*)((char*)pInst + 0x06);
        objOffset.z = *(float*)((char*)pInst + 0x0E);
        float yaw = *(float*)((char*)pInst + 0x14);
        
        // Get global edges
        DWORD objAddr = (DWORD)pObj;
        BYTE* edgesBegin = *(BYTE**)(objAddr + 0x20);
        BYTE* edgesEnd = *(BYTE**)(objAddr + 0x20 + 4);
        
        if (!edgesBegin || !edgesEnd || edgesBegin >= edgesEnd) continue;
        
        int edgeSize = 60;
        int numEdges = (int)(edgesEnd - edgesBegin) / edgeSize;
        
        // Check if path line intersects any edge
        for (int e = 0; e < numEdges && e < 100; e++) {
            DWORD edgeAddr = (DWORD)edgesBegin + (e * edgeSize);
            
            if (IsBadReadPtr((void*)edgeAddr, edgeSize)) continue;
            
            // Check ALL global edges - buildings may have different flags
            // Global edges are collision boundaries regardless of flag
            
            DWORD* pVertex0 = *(DWORD**)(edgeAddr + 0x04);
            DWORD* pVertex1 = *(DWORD**)(edgeAddr + 0x08);
            
            if (!pVertex0 || !pVertex1) continue;
            if (IsBadReadPtr(pVertex0, 12) || IsBadReadPtr(pVertex1, 12)) continue;
            
            // Get vertices in local space
            float v0x = *(float*)((DWORD)pVertex0 + 0x00);
            float v0z = *(float*)((DWORD)pVertex0 + 0x08);
            float v1x = *(float*)((DWORD)pVertex1 + 0x00);
            float v1z = *(float*)((DWORD)pVertex1 + 0x08);
            
            // Rotate and translate to world space
            float cosY = cosf(-yaw);
            float sinY = sinf(-yaw);
            
            float w0x = (v0x * cosY - v0z * sinY) + objOffset.x;
            float w0z = (v0x * sinY + v0z * cosY) + objOffset.z;
            float w1x = (v1x * cosY - v1z * sinY) + objOffset.x;
            float w1z = (v1x * sinY + v1z * cosY) + objOffset.z;
            
            // Check intersection
            if (LineSegmentIntersect2D(from.x, from.z, to.x, to.z,
                                        w0x, w0z, w1x, w1z)) {
                float dist = Distance2D(from, objCenter);
                if (dist < closestDist) {
                    closestDist = dist;
                    outObjCenter = objCenter;
                    outObjRadius = objRadius;
                    foundBlocking = true;
                    
                    // Log only once when found
                }
                break;  // Found intersection with this object
            }
        }
    }
    
    // ========== ALSO CHECK TERRAIN GLOBAL EDGES ==========
    // These are edges that are part of the terrain itself (buildings, walls, etc)
    if (!foundBlocking) {
        BYTE* terrainEdgesBegin = *(BYTE**)(baseAddr + 0x2C);
        BYTE* terrainEdgesEnd = *(BYTE**)(baseAddr + 0x2C + 4);
        
        if (terrainEdgesBegin && terrainEdgesEnd && terrainEdgesBegin < terrainEdgesEnd) {
            int edgeSize = 60;
            int numTerrainEdges = (int)(terrainEdgesEnd - terrainEdgesBegin) / edgeSize;
            
            for (int e = 0; e < numTerrainEdges && e < 500; e++) {
                DWORD edgeAddr = (DWORD)terrainEdgesBegin + (e * edgeSize);
                
                if (IsBadReadPtr((void*)edgeAddr, edgeSize)) continue;
                
                DWORD* pVertex0 = *(DWORD**)(edgeAddr + 0x04);
                DWORD* pVertex1 = *(DWORD**)(edgeAddr + 0x08);
                
                if (!pVertex0 || !pVertex1) continue;
                if (IsBadReadPtr(pVertex0, 12) || IsBadReadPtr(pVertex1, 12)) continue;
                
                float v0x = *(float*)((DWORD)pVertex0 + 0x00);
                float v0z = *(float*)((DWORD)pVertex0 + 0x08);
                float v1x = *(float*)((DWORD)pVertex1 + 0x00);
                float v1z = *(float*)((DWORD)pVertex1 + 0x08);
                
                if (LineSegmentIntersect2D(from.x, from.z, to.x, to.z, v0x, v0z, v1x, v1z)) {
                    outObjCenter.x = (v0x + v1x) / 2.0f;
                    outObjCenter.y = from.y;
                    outObjCenter.z = (v0z + v1z) / 2.0f;
                    
                    float edgeLen = sqrtf((v1x - v0x) * (v1x - v0x) + (v1z - v0z) * (v1z - v0z));
                    outObjRadius = edgeLen / 2.0f + 30.0f;
                    
                    foundBlocking = true;
                    Log("Blocked by terrain edge at (%.0f,%.0f)", outObjCenter.x, outObjCenter.z);
                    break;
                }
            }
        }
    }
    
    return foundBlocking;
}

// ============================================================
// GetNearbyObstacleCluster - Find ALL obstacles near a point
// ============================================================
bool NavMeshCollision::GetNearbyObstacleCluster(const D3DVECTOR& point, float searchRadius,
                                                  D3DVECTOR& outClusterCenter, float& outClusterRadius) 
{
    CRTNavMeshTerrain* pNavMesh = GetCurrentNavMesh();
    if (!pNavMesh) return false;
    
    DWORD baseAddr = (DWORD)pNavMesh;
    SNavMeshInst** objListBegin = *(SNavMeshInst***)(baseAddr + 0x14);
    SNavMeshInst** objListEnd = *(SNavMeshInst***)(baseAddr + 0x14 + 4);
    
    if (!objListBegin || !objListEnd) return false;
    
    int numObjects = (int)(objListEnd - objListBegin);
    
    // Find bounding box of all nearby obstacles
    float minX = 999999.0f, minZ = 999999.0f;
    float maxX = -999999.0f, maxZ = -999999.0f;
    int foundCount = 0;
    
    for (int i = 0; i < numObjects && i < 100; i++) {
        SNavMeshInst* pInst = objListBegin[i];
        if (!pInst) continue;
        
        D3DVECTOR objMin, objMax, objCenter;
        float objRadius;
        GetObjectBounds(pInst, objMin, objMax, objCenter, objRadius);
        
        if (objRadius < 1.0f) continue;
        
        // Check if object is within search radius
        float dist = Distance2D(point, objCenter);
        if (dist < searchRadius + objRadius) {
            // Object is nearby - include in bounding box
            float objMinX = objCenter.x - objRadius;
            float objMaxX = objCenter.x + objRadius;
            float objMinZ = objCenter.z - objRadius;
            float objMaxZ = objCenter.z + objRadius;
            
            if (objMinX < minX) minX = objMinX;
            if (objMaxX > maxX) maxX = objMaxX;
            if (objMinZ < minZ) minZ = objMinZ;
            if (objMaxZ > maxZ) maxZ = objMaxZ;
            
            foundCount++;
        }
    }
    
    if (foundCount == 0) return false;
    
    // Calculate cluster center and radius
    outClusterCenter.x = (minX + maxX) / 2.0f;
    outClusterCenter.y = point.y;
    outClusterCenter.z = (minZ + maxZ) / 2.0f;
    
    float halfWidth = (maxX - minX) / 2.0f;
    float halfHeight = (maxZ - minZ) / 2.0f;
    outClusterRadius = sqrtf(halfWidth * halfWidth + halfHeight * halfHeight);
    
    Log("GetNearbyObstacleCluster: Found %d objects, cluster center=(%.0f,%.0f) r=%.0f",
        foundCount, outClusterCenter.x, outClusterCenter.z, outClusterRadius);
    
    return true;
}

// ============================================================
// Path Generation
// ============================================================

// ============================================================
// Grid-Based NavMesh Pathfinding System
// ============================================================

// Grid configuration
static const float GRID_CELL_SIZE = 5.0f;   // 0.5m per cell (finer resolution)
static const int MAX_GRID_SIZE = 400;       // 400x400 max grid = 2000 units

// Grid cell
struct GridCell {
    bool blocked;
    GridCell() : blocked(false) {}
};

// Cached navigation grid (static for persistence)
static GridCell s_navGrid[MAX_GRID_SIZE][MAX_GRID_SIZE];
static float s_gridOriginX = 0;
static float s_gridOriginZ = 0;
static int s_gridWidth = 0;
static int s_gridHeight = 0;
static bool s_gridBuilt = false;
static DWORD s_lastNavMeshAddr = 0;

// A* Node for grid
struct GridAStarNode {
    int x, z;
    float gCost, hCost, fCost;
    int parentX, parentZ;
    bool inOpenSet, inClosedSet;
    
    GridAStarNode() : x(0), z(0), gCost(999999.0f), hCost(0), fCost(999999.0f),
                      parentX(-1), parentZ(-1), inOpenSet(false), inClosedSet(false) {}
};

// Convert world position to grid cell
static void WorldToGrid(float wx, float wz, int& gx, int& gz) {
    gx = (int)((wx - s_gridOriginX) / GRID_CELL_SIZE);
    gz = (int)((wz - s_gridOriginZ) / GRID_CELL_SIZE);
    
    if (gx < 0) gx = 0;
    if (gz < 0) gz = 0;
    if (gx >= s_gridWidth) gx = s_gridWidth - 1;
    if (gz >= s_gridHeight) gz = s_gridHeight - 1;
}

// Convert grid cell to world position
static void GridToWorld(int gx, int gz, float& wx, float& wz) {
    wx = s_gridOriginX + (gx + 0.5f) * GRID_CELL_SIZE;
    wz = s_gridOriginZ + (gz + 0.5f) * GRID_CELL_SIZE;
}

// Rasterize edge onto grid
static void RasterizeEdge(float x0, float z0, float x1, float z1, float margin) {
    int gx0, gz0, gx1, gz1;
    WorldToGrid(x0, z0, gx0, gz0);
    WorldToGrid(x1, z1, gx1, gz1);
    
    int dx = abs(gx1 - gx0);
    int dz = abs(gz1 - gz0);
    int sx = (gx0 < gx1) ? 1 : -1;
    int sz = (gz0 < gz1) ? 1 : -1;
    int err = dx - dz;
    
    int cx = gx0, cz = gz0;
    int marginCells = (int)(margin / GRID_CELL_SIZE) + 1;
    
    while (true) {
        for (int mx = -marginCells; mx <= marginCells; mx++) {
            for (int mz = -marginCells; mz <= marginCells; mz++) {
                int fx = cx + mx;
                int fz = cz + mz;
                if (fx >= 0 && fx < s_gridWidth && fz >= 0 && fz < s_gridHeight) {
                    s_navGrid[fx][fz].blocked = true;
                }
            }
        }
        
        if (cx == gx1 && cz == gz1) break;
        
        int e2 = 2 * err;
        if (e2 > -dz) { err -= dz; cx += sx; }
        if (e2 < dx) { err += dx; cz += sz; }
    }
}

// Build navigation grid from NavMesh
void NavMeshCollision::BuildNavigationGrid() {
    CRTNavMeshTerrain* pNavMesh = GetCurrentNavMesh();
    if (!pNavMesh) return;
    
    DWORD navAddr = (DWORD)pNavMesh;
    if (s_gridBuilt && s_lastNavMeshAddr == navAddr) return;
    
    Log("BuildNavigationGrid: Building new grid...");
    
    // Reset grid
    for (int x = 0; x < MAX_GRID_SIZE; x++) {
        for (int z = 0; z < MAX_GRID_SIZE; z++) {
            s_navGrid[x][z].blocked = false;
        }
    }
    
    s_gridOriginX = 0.0f;
    s_gridOriginZ = 0.0f;
    s_gridWidth = 400;   // 400 cells * 5 = 2000 units
    s_gridHeight = 400;
    
    DWORD baseAddr = navAddr;
    float margin = 15.0f;  // Smaller margin for finer pathing
    
    // Object Global Edges
    SNavMeshInst** objListBegin = *(SNavMeshInst***)(baseAddr + 0x14);
    SNavMeshInst** objListEnd = *(SNavMeshInst***)(baseAddr + 0x14 + 4);
    
    if (objListBegin && objListEnd) {
        int numObjects = (int)(objListEnd - objListBegin);
        
        for (int i = 0; i < numObjects && i < 100; i++) {
            SNavMeshInst* pInst = objListBegin[i];
            if (!pInst) continue;
            
            CRTNavMeshObj* pObj = *(CRTNavMeshObj**)((char*)pInst + 0xB0);
            if (!pObj) continue;
            
            float objOffsetX = *(float*)((char*)pInst + 0x06);
            float objOffsetZ = *(float*)((char*)pInst + 0x0E);
            float yaw = *(float*)((char*)pInst + 0x14);
            
            DWORD objAddr = (DWORD)pObj;
            BYTE* edgesBegin = *(BYTE**)(objAddr + 0x20);
            BYTE* edgesEnd = *(BYTE**)(objAddr + 0x20 + 4);
            
            if (!edgesBegin || !edgesEnd || edgesBegin >= edgesEnd) continue;
            
            int edgeSize = 60;
            int numEdges = (int)(edgesEnd - edgesBegin) / edgeSize;
            float cosY = cosf(-yaw);
            float sinY = sinf(-yaw);
            
            for (int e = 0; e < numEdges && e < 50; e++) {
                DWORD edgeAddr = (DWORD)edgesBegin + (e * edgeSize);
                if (IsBadReadPtr((void*)edgeAddr, edgeSize)) continue;
                
                DWORD* pVertex0 = *(DWORD**)(edgeAddr + 0x04);
                DWORD* pVertex1 = *(DWORD**)(edgeAddr + 0x08);
                
                if (!pVertex0 || !pVertex1) continue;
                if (IsBadReadPtr(pVertex0, 12) || IsBadReadPtr(pVertex1, 12)) continue;
                
                float v0x = *(float*)((DWORD)pVertex0 + 0x00);
                float v0z = *(float*)((DWORD)pVertex0 + 0x08);
                float v1x = *(float*)((DWORD)pVertex1 + 0x00);
                float v1z = *(float*)((DWORD)pVertex1 + 0x08);
                
                float w0x = (v0x * cosY - v0z * sinY) + objOffsetX;
                float w0z = (v0x * sinY + v0z * cosY) + objOffsetZ;
                float w1x = (v1x * cosY - v1z * sinY) + objOffsetX;
                float w1z = (v1x * sinY + v1z * cosY) + objOffsetZ;
                
                RasterizeEdge(w0x, w0z, w1x, w1z, margin);
            }
        }
    }
    
    // NOTE: Terrain Global Edges are walkable boundaries, NOT blockers
    // Only object edges are obstacles - terrain edges define where you CAN walk
    
    s_gridBuilt = true;
    s_lastNavMeshAddr = navAddr;
    
    int blockedCount = 0;
    for (int x = 0; x < s_gridWidth; x++) {
        for (int z = 0; z < s_gridHeight; z++) {
            if (s_navGrid[x][z].blocked) blockedCount++;
        }
    }
    Log("BuildNavigationGrid: %d/%d cells blocked", blockedCount, s_gridWidth * s_gridHeight);
}

// Grid-based A* pathfinding
std::vector<PathWaypoint> NavMeshCollision::FindPathAStar(
    const D3DVECTOR& start, 
    const D3DVECTOR& end,
    float searchRadius) 
{
    std::vector<PathWaypoint> path;
    
    BuildNavigationGrid();
    
    if (!s_gridBuilt) {
        path.push_back(PathWaypoint(end.x, start.y, end.z));
        return path;
    }
    
    int startX, startZ, endX, endZ;
    WorldToGrid(start.x, start.z, startX, startZ);
    WorldToGrid(end.x, end.z, endX, endZ);
    
    Log("FindPathAStar: World (%.0f,%.0f)->(%.0f,%.0f) Grid (%d,%d)->(%d,%d)", 
        start.x, start.z, end.x, end.z, startX, startZ, endX, endZ);
    Log("FindPathAStar: StartBlocked=%d EndBlocked=%d", 
        s_navGrid[startX][startZ].blocked ? 1 : 0,
        s_navGrid[endX][endZ].blocked ? 1 : 0);
    
    // If start is blocked, find nearest unblocked cell
    if (s_navGrid[startX][startZ].blocked) {
        bool found = false;
        for (int radius = 1; radius <= 10 && !found; radius++) {
            for (int dx = -radius; dx <= radius && !found; dx++) {
                for (int dz = -radius; dz <= radius && !found; dz++) {
                    int nx = startX + dx;
                    int nz = startZ + dz;
                    if (nx >= 0 && nx < s_gridWidth && nz >= 0 && nz < s_gridHeight) {
                        if (!s_navGrid[nx][nz].blocked) {
                            Log("FindPathAStar: Start blocked, using nearby cell (%d,%d)", nx, nz);
                            startX = nx;
                            startZ = nz;
                            found = true;
                        }
                    }
                }
            }
        }
        if (!found) {
            Log("FindPathAStar: No unblocked cell near start!");
            path.push_back(PathWaypoint(end.x, start.y, end.z));
            return path;
        }
    }
    
    // If end is blocked, find nearest unblocked cell
    if (s_navGrid[endX][endZ].blocked) {
        bool found = false;
        for (int radius = 1; radius <= 10 && !found; radius++) {
            for (int dx = -radius; dx <= radius && !found; dx++) {
                for (int dz = -radius; dz <= radius && !found; dz++) {
                    int nx = endX + dx;
                    int nz = endZ + dz;
                    if (nx >= 0 && nx < s_gridWidth && nz >= 0 && nz < s_gridHeight) {
                        if (!s_navGrid[nx][nz].blocked) {
                            Log("FindPathAStar: End blocked, using nearby cell (%d,%d)", nx, nz);
                            endX = nx;
                            endZ = nz;
                            found = true;
                        }
                    }
                }
            }
        }
    }
    
    // Direct path if neither blocked now
    if (!s_navGrid[endX][endZ].blocked && !s_navGrid[startX][startZ].blocked) {
        // Check if start == end after adjustment
        if (startX == endX && startZ == endZ) {
            Log("FindPathAStar: Same cell after adjustment");
            path.push_back(PathWaypoint(end.x, start.y, end.z));
            return path;
        }
    }
    
    // A* on grid
    static GridAStarNode nodes[MAX_GRID_SIZE][MAX_GRID_SIZE];
    
    for (int x = 0; x < s_gridWidth; x++) {
        for (int z = 0; z < s_gridHeight; z++) {
            nodes[x][z].gCost = 999999.0f;
            nodes[x][z].parentX = -1;
            nodes[x][z].parentZ = -1;
            nodes[x][z].inOpenSet = false;
            nodes[x][z].inClosedSet = false;
        }
    }
    
    nodes[startX][startZ].gCost = 0;
    nodes[startX][startZ].hCost = sqrtf((float)((endX-startX)*(endX-startX) + (endZ-startZ)*(endZ-startZ)));
    nodes[startX][startZ].fCost = nodes[startX][startZ].hCost;
    nodes[startX][startZ].inOpenSet = true;
    
    int dx[] = {-1, 0, 1, -1, 1, -1, 0, 1};
    int dz[] = {-1, -1, -1, 0, 0, 1, 1, 1};
    float costs[] = {1.414f, 1.0f, 1.414f, 1.0f, 1.0f, 1.414f, 1.0f, 1.414f};
    
    int iterations = 0;
    
    while (iterations++ < 5000) {
        int currentX = -1, currentZ = -1;
        float lowestF = 999999.0f;
        
        for (int x = 0; x < s_gridWidth; x++) {
            for (int z = 0; z < s_gridHeight; z++) {
                if (nodes[x][z].inOpenSet && nodes[x][z].fCost < lowestF) {
                    lowestF = nodes[x][z].fCost;
                    currentX = x;
                    currentZ = z;
                }
            }
        }
        
        if (currentX == -1) break;
        
        if (currentX == endX && currentZ == endZ) {
            int px = endX, pz = endZ;
            while (px != -1 && pz != -1) {
                float wx, wz;
                GridToWorld(px, pz, wx, wz);
                path.push_back(PathWaypoint(wx, start.y, wz));
                int newPx = nodes[px][pz].parentX;
                int newPz = nodes[px][pz].parentZ;
                px = newPx;
                pz = newPz;
            }
            
            std::vector<PathWaypoint> reversed;
            for (int i = (int)path.size() - 1; i >= 0; i--) {
                reversed.push_back(path[i]);
            }
            
            Log("FindPathAStar: Found path with %d waypoints", (int)reversed.size());
            return reversed;
        }
        
        nodes[currentX][currentZ].inOpenSet = false;
        nodes[currentX][currentZ].inClosedSet = true;
        
        for (int d = 0; d < 8; d++) {
            int nx = currentX + dx[d];
            int nz = currentZ + dz[d];
            
            if (nx < 0 || nx >= s_gridWidth || nz < 0 || nz >= s_gridHeight) continue;
            if (nodes[nx][nz].inClosedSet) continue;
            if (s_navGrid[nx][nz].blocked) continue;
            
            float tentativeG = nodes[currentX][currentZ].gCost + costs[d] * GRID_CELL_SIZE;
            
            if (tentativeG < nodes[nx][nz].gCost) {
                nodes[nx][nz].parentX = currentX;
                nodes[nx][nz].parentZ = currentZ;
                nodes[nx][nz].gCost = tentativeG;
                nodes[nx][nz].hCost = sqrtf((float)((endX-nx)*(endX-nx) + (endZ-nz)*(endZ-nz))) * GRID_CELL_SIZE;
                nodes[nx][nz].fCost = nodes[nx][nz].gCost + nodes[nx][nz].hCost;
                nodes[nx][nz].inOpenSet = true;
            }
        }
    }
    
    Log("FindPathAStar: No path found after %d iterations", iterations);
    path.push_back(PathWaypoint(end.x, start.y, end.z));
    return path;
}

// Check if grid line of sight is clear (Bresenham raycast on grid)
static bool IsGridLineOfSightClear(int x0, int z0, int x1, int z1) {
    int dx = abs(x1 - x0);
    int dz = abs(z1 - z0);
    int sx = (x0 < x1) ? 1 : -1;
    int sz = (z0 < z1) ? 1 : -1;
    int err = dx - dz;
    
    int cx = x0, cz = z0;
    
    while (true) {
        if (cx == x1 && cz == z1) return true;  // Reached destination
        
        // Check if current cell is blocked
        if (cx >= 0 && cx < s_gridWidth && cz >= 0 && cz < s_gridHeight) {
            if (s_navGrid[cx][cz].blocked) return false;  // Blocked!
        }
        
        int e2 = 2 * err;
        if (e2 > -dz) { err -= dz; cx += sx; }
        if (e2 < dx) { err += dx; cz += sz; }
    }
    return true;
}

// Smooth path by removing unnecessary intermediate waypoints
static std::vector<PathWaypoint> SmoothPath(const std::vector<PathWaypoint>& path, float y) {
    if (path.size() <= 2) return path;
    
    std::vector<PathWaypoint> smoothed;
    smoothed.push_back(path[0]);  // Always keep first point
    
    size_t current = 0;
    
    while (current < path.size() - 1) {
        size_t farthest = current + 1;
        
        // Find farthest visible point
        for (size_t test = current + 2; test < path.size(); test++) {
            int cx, cz, tx, tz;
            WorldToGrid(path[current].position.x, path[current].position.z, cx, cz);
            WorldToGrid(path[test].position.x, path[test].position.z, tx, tz);
            
            if (IsGridLineOfSightClear(cx, cz, tx, tz)) {
                farthest = test;  // Can skip to this point
            }
        }
        
        smoothed.push_back(path[farthest]);
        current = farthest;
    }
    
    NavMeshCollision::Log("SmoothPath: %d -> %d waypoints", (int)path.size(), (int)smoothed.size());
    return smoothed;
}

std::vector<PathWaypoint> NavMeshCollision::GeneratePathAroundObstacle(
    const D3DVECTOR& from, 
    const D3DVECTOR& to,
    const D3DVECTOR& objCenter,
    float objRadius) 
{
    Log("GeneratePath: Grid A* from (%.0f,%.0f) to (%.0f,%.0f)", from.x, from.z, to.x, to.z);
    std::vector<PathWaypoint> rawPath = FindPathAStar(from, to, 500.0f);
    
    // If A* only returned 1 waypoint (direct to end = failed), use simple bypass
    if (rawPath.size() <= 1) {
        Log("GeneratePath: A* failed, using simple bypass");
        std::vector<PathWaypoint> fallback;
        
        // Calculate perpendicular direction to path
        float dx = to.x - from.x;
        float dz = to.z - from.z;
        float len = sqrtf(dx * dx + dz * dz);
        if (len > 0.1f) {
            dx /= len;
            dz /= len;
        }
        
        // Perpendicular offset (try going around the object)
        float perpX = -dz;
        float perpZ = dx;
        
        // Distance to go around
        float bypassDist = objRadius + 50.0f;
        
        // First waypoint: go perpendicular
        float wp1x = objCenter.x + perpX * bypassDist;
        float wp1z = objCenter.z + perpZ * bypassDist;
        fallback.push_back(PathWaypoint(wp1x, from.y, wp1z));
        
        // Second waypoint: target
        fallback.push_back(PathWaypoint(to.x, from.y, to.z));
        
        Log("GeneratePath: Fallback bypass via (%.0f,%.0f)", wp1x, wp1z);
        return fallback;
    }
    
    // Smooth the path to reduce zigzag
    return SmoothPath(rawPath, from.y);
}
