#pragma once

struct SNavMeshInst;

#include <d3dx9.h>
#include "RTNavMeshObj.h"

struct LinkEdge
{
	WORD m_wLinkedObjID;
	WORD m_wLinkedObjEdgeID;
	WORD m_wEdgeID;
};

// ============================================================
// ECSRO SNavMeshInst Layout (from IDA analysis)
// ============================================================
// Size: ~0xB8 bytes
//
// +0x00: gap[2]
// +0x02: SMapObjInfo m_sObj (packed)
// +0xA0: m_LinkEdges (std::vector<LinkEdge>)
// +0xAC: m_pParent (CRTNavMesh*)
// +0xB0: m_pObject (CRTNavMeshObj*)
// +0xB4: m_ObjExtInfo (void*)
// ============================================================

struct SNavMeshInst
{
	char gap0[2]; // +0x00

#pragma pack(push,1)
	struct SMapObjInfo
	{
		int Index;           // +0x02
		D3DXVECTOR3 Offset;  // +0x06 (12 bytes)
		short Type;          // +0x12
		float Yaw;           // +0x14
		short ID;            // +0x18
		short Short0;        // +0x1A (padding?)
		bool IsBig;          // +0x1C
		bool IsStruct;       // +0x1D
		short RegionID;      // +0x1E
	} m_sObj;                // Total: ~0x20 bytes
#pragma pack(pop)

	_D3DMATRIX m_WorldToLocal;  // +0x20? (64 bytes)
	_D3DMATRIX m_LocalToWorld;  // +0x60? (64 bytes)
	
	// +0xA0: LinkEdges vector
	std::vector<LinkEdge> m_LinkEdges;
	
	// +0xAC: Parent NavMesh pointer
	CRTNavMesh* m_pParent;
	
	// +0xB0: Object NavMesh pointer
	CRTNavMeshObj* m_pObject;
	
	// +0xB4: Object extension info (dungeon data from objext.ifo)
	void* m_ObjExtInfo;
};