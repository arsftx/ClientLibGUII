#pragma once

#include <vector>
#include <d3d9.h>

class CRTNavMeshTerrain;

#include "RTNavMesh.h"
#include "ObjectList.h"

#include "RTNavCellQuad.h"
#include "RTNavEdgeGlobal.h"
#include "RTNavEdgeInternal.h"
#include "SNavMeshInst.h"

// ============================================================
// ECSRO CRTNavMeshTerrain Layout (VERIFIED from debug)
// ============================================================
// +0x00: vtable (4 bytes)
// +0x04: m_ParentMesh (4 bytes)
// +0x08: m_Region (2 bytes) - VERIFIED
// +0x0A: padding (2 bytes)
// +0x0C: m_Type (4 bytes)
// +0x10: m_File (4 bytes)
// +0x14: m_sObjList (12 bytes - std::vector)
// +0x20: m_InternalEdges (12 bytes - std::vector)
// +0x2C: m_GlobalEdges (12 bytes - std::vector)
// +0x38: m_Cells (12 bytes - std::vector) - VERIFIED (150 cells)
// +0x44: m_TileMap[96*96] - 73728 bytes (0x12000)
// +0x12044: m_OpenCellCount - VERIFIED (85)
// +0x12048: m_HeightMap[97*97]
// ============================================================

// TileMap entry structure (8 bytes)
struct SNavMeshTile {
    int m_CellID;       // 4 bytes
    short m_Flag;       // 2 bytes
    short m_TextureID;  // 2 bytes
};

// ============================================================
// CRTNavMeshTerrain - ECSRO Compatible Layout
// Uses raw byte array and accessor methods to ensure correct offsets
// ============================================================
class CRTNavMeshTerrain
{
private:
    // Raw memory layout - we access everything via raw offsets
    // This ensures we match ECSRO exactly regardless of compiler settings
    char _rawData[0x1B400];  // Large enough for entire struct

public:
    // Accessor methods using verified ECSRO offsets
    
    // +0x08: m_Region
    short& GetRegion() { return *(short*)((char*)this + 0x08); }
    short GetRegion() const { return *(short*)((char*)this + 0x08); }
    __declspec(property(get=GetRegion)) short m_Region;
    
    // +0x0C: m_Type  
    int& GetType() { return *(int*)((char*)this + 0x0C); }
    int GetType() const { return *(int*)((char*)this + 0x0C); }
    __declspec(property(get=GetType)) int m_Type;
    
    // +0x14: m_sObjList (std::vector<SNavMeshInst*>)
    std::vector<SNavMeshInst*>& GetSObjList() { 
        return *(std::vector<SNavMeshInst*>*)((char*)this + 0x14); 
    }
    const std::vector<SNavMeshInst*>& GetSObjList() const { 
        return *(const std::vector<SNavMeshInst*>*)((char*)this + 0x14); 
    }
    __declspec(property(get=GetSObjList)) std::vector<SNavMeshInst*>& m_sObjList;
    
    // +0x20: m_InternalEdges
    std::vector<CRTNavEdgeInternal>& GetInternalEdges() { 
        return *(std::vector<CRTNavEdgeInternal>*)((char*)this + 0x20); 
    }
    const std::vector<CRTNavEdgeInternal>& GetInternalEdges() const { 
        return *(const std::vector<CRTNavEdgeInternal>*)((char*)this + 0x20); 
    }
    __declspec(property(get=GetInternalEdges)) std::vector<CRTNavEdgeInternal>& m_InternalEdges;
    
    // +0x2C: m_GlobalEdges
    std::vector<CRTNavEdgeGlobal>& GetGlobalEdges() { 
        return *(std::vector<CRTNavEdgeGlobal>*)((char*)this + 0x2C); 
    }
    const std::vector<CRTNavEdgeGlobal>& GetGlobalEdges() const { 
        return *(const std::vector<CRTNavEdgeGlobal>*)((char*)this + 0x2C); 
    }
    __declspec(property(get=GetGlobalEdges)) std::vector<CRTNavEdgeGlobal>& m_GlobalEdges;
    
    // +0x38: m_Cells (std::vector<CRTNavCellQuad>)
    std::vector<CRTNavCellQuad>& GetCells() { 
        return *(std::vector<CRTNavCellQuad>*)((char*)this + 0x38); 
    }
    const std::vector<CRTNavCellQuad>& GetCells() const { 
        return *(const std::vector<CRTNavCellQuad>*)((char*)this + 0x38); 
    }
    __declspec(property(get=GetCells)) std::vector<CRTNavCellQuad>& m_Cells;
    
    // +0x44: m_TileMap
    SNavMeshTile* GetTileMap() { return (SNavMeshTile*)((char*)this + 0x44); }
    const SNavMeshTile* GetTileMap() const { return (const SNavMeshTile*)((char*)this + 0x44); }
    __declspec(property(get=GetTileMap)) SNavMeshTile* m_TileMap;
    
    // +0x12044: m_OpenCellCount
    int& GetOpenCellCount() { return *(int*)((char*)this + 0x12044); }
    int GetOpenCellCount() const { return *(int*)((char*)this + 0x12044); }
    __declspec(property(get=GetOpenCellCount)) int m_OpenCellCount;
    
    // +0x12048: m_HeightMap
    float* GetHeightMap() { return (float*)((char*)this + 0x12048); }
    const float* GetHeightMap() const { return (const float*)((char*)this + 0x12048); }
    __declspec(property(get=GetHeightMap)) float* m_HeightMap;
    
    // Methods - implemented in RTNavMeshTerrain.cpp
    int FindHeight(const D3DVECTOR& vPos) const;
    short GetTileFlag(D3DVECTOR& vPos);
};

