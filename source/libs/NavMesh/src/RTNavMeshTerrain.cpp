#include "RTNavMeshTerrain.h"

// ============================================================
// ECSRO Function Addresses
// ============================================================
// FindHeight is called via vtable - [this+0]+4
// GetTileFlag needs direct address (TODO)
// ============================================================

int CRTNavMeshTerrain::FindHeight(const D3DVECTOR& vPos) const
{
    // Get vtable pointer
    DWORD vtable = *(DWORD*)this;
    if (!vtable) return 0;
    
    // Call vtable[1] (offset +4) = virtual FindHeight
    // Signature: int __thiscall FindHeight(const D3DVECTOR* vPos)
    typedef int (__thiscall* FindHeightFunc)(const CRTNavMeshTerrain*, const D3DVECTOR*);
    FindHeightFunc func = (FindHeightFunc)(*(DWORD*)(vtable + 4));
    
    if (!func) return 0;
    
    return func(this, &vPos);
}

short CRTNavMeshTerrain::GetTileFlag(D3DVECTOR& vPos)
{
    // TODO: Find correct ECSRO GetTileFlag address and implement
    return 0;
}