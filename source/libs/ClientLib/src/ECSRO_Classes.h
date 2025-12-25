#pragma once

#include <windows.h>
#include <vector>
#include <d3d9.h>
#include "GlobalHelpersThatHaveNoHomeYet.h"

// ============================================================
// ECSRO-Specific Class Wrappers
// These use raw memory access to avoid std::map layout issues
// ============================================================

// ECSRO memory offsets (from IDA analysis)
#define ECSRO_OBJECT_LOCATION_X  0x74
#define ECSRO_OBJECT_LOCATION_Y  0x78
#define ECSRO_OBJECT_LOCATION_Z  0x7C
#define ECSRO_OBJECT_REGION      0x70

// CEntityManager offsets (from sub_89FE90)
#define ECSRO_ENTITYMGR_MAP_HEAD 0x1C  // std::map head node

// std::map node offsets (from sub_650680 IDA analysis)
// Node layout: [?, parent, left, right, key, value]
#define ECSRO_NODE_PARENT 0x04     // v3[1] - parent/root node
#define ECSRO_NODE_LEFT   0x08     // v5[2] - left child
#define ECSRO_NODE_RIGHT  0x0C     // v5[3] - right child
#define ECSRO_NODE_KEY    0x10     // v5[4] - int key
#define ECSRO_NODE_VALUE  0x14     // node + 20 - CIObject* value

// Global addresses
#define ECSRO_ADDR_ENTITY_MANAGER 0x00C5DCF0
#define ECSRO_ADDR_PLAYER         0xA0465C

// ============================================================
// Helper functions for raw memory access
// ============================================================

// Get location from any CIObject-derived class using raw offset
inline D3DVECTOR GetLocationRaw(DWORD objectAddr) {
    D3DVECTOR loc;
    loc.x = *(float*)(objectAddr + ECSRO_OBJECT_LOCATION_X);
    loc.y = *(float*)(objectAddr + ECSRO_OBJECT_LOCATION_Y);
    loc.z = *(float*)(objectAddr + ECSRO_OBJECT_LOCATION_Z);
    return loc;
}

// Get region from any CIObject-derived class
inline uregion GetRegionRaw(DWORD objectAddr) {
    return *(uregion*)(objectAddr + ECSRO_OBJECT_REGION);
}

// ============================================================
// Entity iteration using raw memory access
// Based on IDA analysis of sub_650680
// ============================================================

struct EntityInfo {
    int id;
    DWORD address;
};

// Traverse std::map tree in-order (based on sub_650680 analysis)
inline void TraverseMapNode(DWORD nodeAddr, DWORD headAddr, std::vector<EntityInfo>& entities, int maxEntities) {
    if (!nodeAddr || nodeAddr == headAddr || entities.size() >= (size_t)maxEntities) {
        return;
    }
    
    // Check if valid pointer
    if (IsBadReadPtr((void*)nodeAddr, 0x18)) {
        return;
    }
    
    // Traverse left subtree first (in-order)
    DWORD leftNode = *(DWORD*)(nodeAddr + ECSRO_NODE_LEFT);
    if (leftNode && leftNode != headAddr) {
        TraverseMapNode(leftNode, headAddr, entities, maxEntities);
    }
    
    // Process this node - get key and value
    int key = *(int*)(nodeAddr + ECSRO_NODE_KEY);
    DWORD value = *(DWORD*)(nodeAddr + ECSRO_NODE_VALUE);
    
    if (value && !IsBadReadPtr((void*)value, 4)) {
        EntityInfo info;
        info.id = key;
        info.address = value;
        entities.push_back(info);
    }
    
    // Traverse right subtree
    DWORD rightNode = *(DWORD*)(nodeAddr + ECSRO_NODE_RIGHT);
    if (rightNode && rightNode != headAddr) {
        TraverseMapNode(rightNode, headAddr, entities, maxEntities);
    }
}

// Get all entities from EntityManager using raw memory access
inline std::vector<EntityInfo> GetAllEntitiesRaw() {
    std::vector<EntityInfo> entities;
    
    DWORD entityMgrPtr = *(DWORD*)ECSRO_ADDR_ENTITY_MANAGER;
    if (!entityMgrPtr || IsBadReadPtr((void*)entityMgrPtr, 0x40)) {
        return entities;
    }
    
    DWORD mapHead = *(DWORD*)(entityMgrPtr + ECSRO_ENTITYMGR_MAP_HEAD);
    
    if (!mapHead || IsBadReadPtr((void*)mapHead, 0x18)) {
        return entities;
    }
    
    // Root node is at head->parent (offset 0x04)
    DWORD rootNode = *(DWORD*)(mapHead + ECSRO_NODE_PARENT);
    
    if (!rootNode || rootNode == mapHead || IsBadReadPtr((void*)rootNode, 0x18)) {
        return entities;
    }
    
    entities.reserve(100);
    TraverseMapNode(rootNode, mapHead, entities, 1000);
    
    return entities;
}

// Get player location
inline D3DVECTOR GetPlayerLocationRaw() {
    DWORD playerPtr = *(DWORD*)ECSRO_ADDR_PLAYER;
    if (!playerPtr || IsBadReadPtr((void*)playerPtr, 0x80)) {
        D3DVECTOR zero = {0, 0, 0};
        return zero;
    }
    return GetLocationRaw(playerPtr);
}

// Get player address
inline DWORD GetPlayerAddressRaw() {
    return *(DWORD*)ECSRO_ADDR_PLAYER;
}
