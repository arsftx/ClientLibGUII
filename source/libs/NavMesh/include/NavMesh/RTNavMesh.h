#pragma once

class CRTNavMesh;
class CRTNavMeshTerrain;

#include <d3d9.h>
#include <string>

enum RTNavMeshType : int
{
	None = 0,
	Terrain = 1,
	Object = 2,
	Dungeon = 3,
};

// ECSRO CRTNavMesh Layout:
// +0x00: vtable = 0x9656D8
// +0x04: m_ParentMesh
// +0x08: m_Region (short)
// +0x0C: m_Type

class CRTNavMesh
{
public:
	CRTNavMesh();
	
	virtual ~CRTNavMesh();
	virtual int FindHeight(const D3DVECTOR &vPos) const;
	virtual void Move(); // int a2, int a3, SNavMeshPosition* pSrc, SNavMeshPosition* pDst, pRaycastContext* pContext
	virtual void ResolveCellAndHeight(void* position); //SNavMeshPosition* but the linking is killing me...
	virtual int F05();
	virtual int F06(int, int);

public:
	CRTNavMesh* m_ParentMesh;    // +0x04
	short m_Region;              // +0x08
	RTNavMeshType m_Type;        // +0x0C
	std::string *m_File;         // +0x10

};

// ============================================================
// ECSRO NavMesh Global Addresses
// ============================================================
// NavMesh Manager: 0xC55DEC
// NavMesh Pool:    0xC55DE0
// NavMesh Alloc:   0xC55D90
// Control Flag:    0xC55DC4
// GetNavMeshForRegion: 0x8128E0

#define g_NavMeshManager (*reinterpret_cast<void**>(0x00C55DEC))

// ECSRO Player address (from ECSRO_Classes.h)
#define ECSRO_ADDR_PLAYER_NAVMESH 0xA0465C
#define ECSRO_OBJECT_REGION_OFFSET 0x70

// ECSRO sub_81A450 - LoadOrGetTerrainNavMesh(RegionMgr, regionId, flag) -> returns NavMesh pointer
// This is __thiscall, ecx = RegionMgr (dword_C55DEC)
#define ECSRO_ADDR_SUB_81A450 0x0081A450

// Helper to log to file (flushed immediately for crash debugging)
inline void NavMeshLog(const char* msg) {
    FILE* f = fopen("navmeshlog.txt", "a");
    if (f) {
        fprintf(f, "%s", msg);
        fflush(f);
        fclose(f);
    }
}

// Get current NavMesh based on player's region
// Note: Uses static caching to avoid loading navmesh every frame
inline CRTNavMeshTerrain* GetCurrentNavMesh() {
    static CRTNavMeshTerrain* s_cachedNavMesh = NULL;
    static short s_cachedRegion = 0;
    
    // Get player address
    DWORD playerPtr = *(DWORD*)ECSRO_ADDR_PLAYER_NAVMESH;
    if (!playerPtr) {
        return NULL;
    }
    
    // Get player's current region
    short region = *(short*)(playerPtr + ECSRO_OBJECT_REGION_OFFSET);
    if (region == 0 || region == -1) {
        return NULL;
    }
    
    // Check if this is a dungeon region (0x8000 flag)
    if (region & 0x8000) {
        return NULL;  // Dungeon not supported yet
    }
    
    // Return cached navmesh if region hasn't changed
    if (s_cachedNavMesh && region == s_cachedRegion) {
        return s_cachedNavMesh;
    }
    
    // Get NavMesh Manager
    DWORD navMeshMgr = *(DWORD*)0x00C55DEC;
    if (!navMeshMgr) {
        return NULL;
    }
    
    // Call sub_81A450 - LoadOrGetTerrainNavMesh
    CRTNavMeshTerrain* navmesh = NULL;
    int regionParam = region;
    DWORD funcAddr = ECSRO_ADDR_SUB_81A450;
    
    __asm {
        push 1                  // flag = 1
        push regionParam        // regionId
        mov ecx, navMeshMgr     // this = NavMeshManager
        call funcAddr           // call sub_81A450
        mov navmesh, eax        // save result
    }
    
    // Update cache
    s_cachedNavMesh = navmesh;
    s_cachedRegion = region;
    
    return navmesh;
}

// Legacy macro for compatibility - now calls function
#define g_CurrentNavMesh (GetCurrentNavMesh())









