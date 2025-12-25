#include "NavMeshTool.h"
#include "../MathUtil.h"

#include <imgui/imgui.h>
#include <GFX3DFunction/GFXVideo3d.h>
#include <GFX3DFunction/DrawingHelpers.h>
#include <unsorted.h>

void NavMeshTool::Render() {

    if (!bShow) return;

    CGFXVideo3d *gfx = CGFXVideo3d::get();

    if (!bFreeze || m_pNavmesh == 0) {
        m_pNavmesh = g_CurrentNavMesh;
    }

    ImGui::Begin("NavMesh Info", &bShow);

    // Null check - GetCurrentNavMesh() may return nullptr if player/region not loaded
    if (!m_pNavmesh) {
        ImGui::Text("NavMesh: NULL");
        ImGui::Text("(Player not loaded or region has no navmesh)");
        ImGui::End();
        return;
    }

    ImGui::Text("0x%p", m_pNavmesh);
    ImGui::Text("Region: 0x%04x", m_pNavmesh->m_Region);

    // Raw memory access using correct ECSRO offsets for verification
    DWORD baseAddr = (DWORD)m_pNavmesh;
    int rawCellsSize = (*(int*)(baseAddr + 0x38 + 4) - *(int*)(baseAddr + 0x38)) / 100;
    ImGui::Text("Total Cells: %d", rawCellsSize);
    ImGui::Text("Open Cells: %d", *(int*)(baseAddr + 0x12044));

    ImGui::Checkbox("Freeze Current Mesh", &bFreeze);
    ImGui::Checkbox("Render Cells", &bCells);
    ImGui::Checkbox("Render Edge (internal)", &bEdgeInternal);
    ImGui::Checkbox("Render Edge (global)", &bEdgeGlobal);
    ImGui::Checkbox("Render Object Origin", &bObjectOrigin);
    ImGui::Checkbox("Render Object Cells", &bObjectCells);
    ImGui::Checkbox("Render Object Internal Edges", &bObjectInternalEdges);
    ImGui::Checkbox("Render Object Global Edges", &bObjectGlobalEdges);
    ImGui::Checkbox("Render Object Grid", &bObjectGrid);

    ImGui::InputInt("Step", &step);

    // Render NavMeshTerrain Cells
    if (bCells) {
        RenderNavCells(m_pNavmesh);
    }

    // Render NavMeshTerrain Edges (internal)
    if (bEdgeInternal) {
        RenderNavEdgeInternal(m_pNavmesh);
    }

    if (bEdgeGlobal) {
        RenderNavEdgeGlobal(m_pNavmesh);
    }

    if (bObjectOrigin || bObjectCells || bObjectGlobalEdges || bObjectInternalEdges || bObjectGrid) {
        // Raw memory access for m_sObjList at offset 0x14
        DWORD baseAddr = (DWORD)m_pNavmesh;
        SNavMeshInst** objListBegin = *(SNavMeshInst***)(baseAddr + 0x14);
        SNavMeshInst** objListEnd = *(SNavMeshInst***)(baseAddr + 0x14 + 4);
        
        if (objListBegin && objListEnd) {
            int numObjects = (int)(objListEnd - objListBegin);
            
            for (int i = 0; i < numObjects && i < 100; i++) {
                SNavMeshInst* pInst = objListBegin[i];
                if (!pInst) continue;
                
                // m_pObject at offset +0xB0 contains CRTNavMeshObj*
                CRTNavMeshObj* pObjMesh = *(CRTNavMeshObj**)((char*)pInst + 0xB0);
                if (!pObjMesh) continue;
                
                // Get object position from m_sObj.Offset at +0x06
                D3DVECTOR objOffset = {0, 0, 0};
                objOffset.x = *(float*)((char*)pInst + 0x06);
                objOffset.y = *(float*)((char*)pInst + 0x0A);
                objOffset.z = *(float*)((char*)pInst + 0x0E);
                
                if (bObjectOrigin) {
                    RenderObjectOrigin(pInst, i == 0);
                }
                
                if (bObjectCells) {
                    RenderObjectCells(pInst);
                }
                
                if (bObjectInternalEdges) {
                    RenderObjectInternalEdges(pInst);
                }
                
                if (bObjectGlobalEdges) {
                    RenderObjectGlobalEdges(pInst);
                }
                
                if (bObjectGrid) {
                    RenderObjectGrid(pInst);
                }
            }
        }
    }

    ImGui::End();
}

NavMeshTool::NavMeshTool() : m_pNavmesh(0), bShow(false), bFreeze(false), bCells(false), bEdgeInternal(false),
                             bEdgeGlobal(false),
                             bObjectOrigin(false), bObjectCells(false), bObjectInternalEdges(false),
                             bObjectGlobalEdges(false), bObjectGrid(false), step(20) {

}

void NavMeshTool::RenderNavEdgeGlobal(const CRTNavMeshTerrain *pNavmesh) const {
    // Use raw memory access for global edges at offset 0x2C
    DWORD baseAddr = (DWORD)pNavmesh;
    BYTE* edgesBegin = *(BYTE**)(baseAddr + 0x2C);
    BYTE* edgesEnd = *(BYTE**)(baseAddr + 0x2C + 4);
    
    if (!edgesBegin || !edgesEnd) return;
    
    // Edge size = 60 bytes (0x3C) for global edges (4 extra bytes for wAssocRgn[2])
    int rawSize = (int)(edgesEnd - edgesBegin);
    int edgeSize = 60;  // Global edges slightly larger
    int numEdges = rawSize / edgeSize;
    
    for (int i = 0; i < numEdges && i < 5000; i++) {
        DWORD edgeAddr = (DWORD)edgesBegin + (i * edgeSize);
        
        // Same coordinate offsets as internal edges
        float minX = *(float*)(edgeAddr + 0x14);
        float minY = *(float*)(edgeAddr + 0x18);
        float maxX = *(float*)(edgeAddr + 0x1C);
        float maxY = *(float*)(edgeAddr + 0x20);
        BYTE btFlag = *(BYTE*)(edgeAddr + 0x30);
        
        D3DVECTOR edgeMin = {minX, 0, minY};
        D3DVECTOR edgeMax = {maxX, 0, maxY};
        
        PrettyLine3D(edgeMin, edgeMax, step, GetColorFromEdgeFlag((EDGE_FLAG)btFlag), pNavmesh);
    }
}

void NavMeshTool::RenderNavCells(const CRTNavMeshTerrain *pNavmesh) const {
    // Use raw memory access for cells at offset 0x38
    DWORD baseAddr = (DWORD)pNavmesh;
    CRTNavCellQuad* cellsBegin = *(CRTNavCellQuad**)(baseAddr + 0x38);
    CRTNavCellQuad* cellsEnd = *(CRTNavCellQuad**)(baseAddr + 0x38 + 4);
    
    if (!cellsBegin || !cellsEnd) return;
    
    // ECSRO uses 100 bytes per cell
    int rawSize = (int)((DWORD)cellsEnd - (DWORD)cellsBegin);
    int numCells = rawSize / 100;
    
    for (int i = 0; i < numCells && i < 1000; i++) {
        DWORD cellAddr = (DWORD)cellsBegin + (i * 100);
        
        // Cell coordinates from verified ECSRO offsets:
        // +0x24, +0x28: m_vMin
        // +0x2C, +0x30: m_vMax
        float minX = *(float*)(cellAddr + 0x24);  
        float minY = *(float*)(cellAddr + 0x28);
        float maxX = *(float*)(cellAddr + 0x2C);
        float maxY = *(float*)(cellAddr + 0x30);
        
        D3DVECTOR v2 = {minX, 30.0f, minY};
        D3DVECTOR v3 = {maxX, 30.0f, maxY};
        D3DVECTOR pmiss1 = {maxX, 30.0f, minY};
        D3DVECTOR pmiss2 = {minX, 30.0f, maxY};

        PrettyLine3D(v2, pmiss1, step, pNavmesh);
        PrettyLine3D(pmiss1, v3, step, pNavmesh);
        PrettyLine3D(v3, pmiss2, step, pNavmesh);
        PrettyLine3D(pmiss2, v2, step, pNavmesh);
    }
}

void NavMeshTool::RenderNavEdgeInternal(const CRTNavMeshTerrain *pNavmesh) const {
    // Use raw memory access for edges at offset 0x20
    DWORD baseAddr = (DWORD)pNavmesh;
    BYTE* edgesBegin = *(BYTE**)(baseAddr + 0x20);
    BYTE* edgesEnd = *(BYTE**)(baseAddr + 0x20 + 4);
    
    if (!edgesBegin || !edgesEnd) return;
    
    // Edge size = 56 bytes (0x38) verified from dump
    int rawSize = (int)(edgesEnd - edgesBegin);
    int numEdges = rawSize / 56;
    
    for (int i = 0; i < numEdges && i < 5000; i++) {
        DWORD edgeAddr = (DWORD)edgesBegin + (i * 56);
        
        // Edge coordinates from verified offsets:
        // +0x14: sLine.Min.x
        // +0x18: sLine.Min.y
        // +0x1C: sLine.Max.x
        // +0x20: sLine.Max.y
        // +0x30: btFlag (lower byte)
        float minX = *(float*)(edgeAddr + 0x14);
        float minY = *(float*)(edgeAddr + 0x18);
        float maxX = *(float*)(edgeAddr + 0x1C);
        float maxY = *(float*)(edgeAddr + 0x20);
        BYTE btFlag = *(BYTE*)(edgeAddr + 0x30);
        
        D3DVECTOR edgeMin = {minX, 0, minY};
        D3DVECTOR edgeMax = {maxX, 0, maxY};
        
        PrettyLine3D(edgeMin, edgeMax, step, GetColorFromEdgeFlag((EDGE_FLAG)btFlag), pNavmesh);
    }
}

void NavMeshTool::RenderObjectOrigin(const SNavMeshInst *pInst, bool bIsFirst) const {
    CGFXVideo3d *gfx = CGFXVideo3d::get();
    
    // Use raw memory access
    // m_sObj.Offset at +0x06 (x,y,z floats)
    // m_sObj.Yaw at +0x14
    D3DVECTOR vec;
    vec.x = *(float*)((char*)pInst + 0x06);
    vec.y = *(float*)((char*)pInst + 0x0A);
    vec.z = *(float*)((char*)pInst + 0x0E);
    float yaw = -*(float*)((char*)pInst + 0x14);
    
    D3DVECTOR vec2d;
    if (gfx->Project(vec, vec2d) > 0) {
        {
            // Red X
            D3DXVECTOR3 pTarget(20, 0, 0);
            rotatey(pTarget, yaw);

            pTarget.x += vec.x;
            pTarget.y += vec.y;
            pTarget.z += vec.z;

            D3DVECTOR pTarget2D;
            if (gfx->Project(*(D3DVECTOR*)&pTarget, pTarget2D) > 0)
                DXDrawLine((int) vec2d.x, (int) vec2d.y, (int) pTarget2D.x, (int) pTarget2D.y,
                           D3DCOLOR_ARGB(0, 255, 0, 0), 1.0);
        }

        {
            // Blue Y
            D3DXVECTOR3 pTarget(0, 20, 0);
            rotatey(pTarget, yaw);

            pTarget.x += vec.x;
            pTarget.y += vec.y;
            pTarget.z += vec.z;

            D3DVECTOR pTarget2D;
            if (gfx->Project(*(D3DVECTOR*)&pTarget, pTarget2D) > 0)
                DXDrawLine((int) vec2d.x, (int) vec2d.y, (int) pTarget2D.x, (int) pTarget2D.y,
                           D3DCOLOR_ARGB(0, 0, 0, 255), 1.0);
        }

        {
            // Green Z
            D3DXVECTOR3 pTarget(0, 0, 20);
            rotatey(pTarget, yaw);

            pTarget.x += vec.x;
            pTarget.y += vec.y;
            pTarget.z += vec.z;

            D3DVECTOR pTarget2D;
            if (gfx->Project(*(D3DVECTOR*)&pTarget, pTarget2D) > 0)
                DXDrawLine((int) vec2d.x, (int) vec2d.y, (int) pTarget2D.x, (int) pTarget2D.y,
                           D3DCOLOR_ARGB(0, 0, 255, 0), 1.0);
        }

        if (bIsFirst) {
            DrawRect((int) vec2d.x - 5, (int) vec2d.y - 5, 10, 10, D3DCOLOR_ARGB(0, 255, 0, 0));
        } else {
            DrawRect((int) vec2d.x - 5, (int) vec2d.y - 5, 10, 10, D3DCOLOR_ARGB(0, 255, 255, 0));
        }
    }
}

void NavMeshTool::RenderObjectCells(const SNavMeshInst *pInst) const {
    // Use raw memory access for SNavMeshInst
    // m_pObject at +0xB0
    // m_sObj.Offset at +0x06 (x,y,z floats)
    // m_sObj.Yaw at +0x14
    
    CRTNavMeshObj* pObj = *(CRTNavMeshObj**)((char*)pInst + 0xB0);
    if (!pObj) return;
    
    D3DVECTOR offset;
    offset.x = *(float*)((char*)pInst + 0x06);
    offset.y = *(float*)((char*)pInst + 0x0A);
    offset.z = *(float*)((char*)pInst + 0x0E);
    float yaw = -*(float*)((char*)pInst + 0x14);
    
    // CRTNavMeshObj m_Cells at offset 0x20 (std::vector)
    // For now, skip object cells - they use CRTNavCellTri which has complex structure
    // TODO: Implement raw access for CRTNavMeshObj cells
}

void NavMeshTool::RenderObjectGrid(const SNavMeshInst *pInst) const {
    // TODO: Implement with raw memory access
    // CRTNavMeshObj::m_Grid offset needs investigation
}

void NavMeshTool::RenderObjectGlobalEdges(const SNavMeshInst *pInst) const {
    CGFXVideo3d *gfx = CGFXVideo3d::get();
    
    // Raw memory access for SNavMeshInst
    CRTNavMeshObj* pObj = *(CRTNavMeshObj**)((char*)pInst + 0xB0);
    if (!pObj) return;
    
    D3DVECTOR offset;
    offset.x = *(float*)((char*)pInst + 0x06);
    offset.y = *(float*)((char*)pInst + 0x0A);
    offset.z = *(float*)((char*)pInst + 0x0E);
    float yaw = -*(float*)((char*)pInst + 0x14);
    
    // CRTNavMeshObj::m_GlobalEdges at offset 0x20 (std::vector)
    DWORD objAddr = (DWORD)pObj;
    BYTE* edgesBegin = *(BYTE**)(objAddr + 0x20);
    BYTE* edgesEnd = *(BYTE**)(objAddr + 0x20 + 4);
    
    if (!edgesBegin || !edgesEnd || edgesBegin >= edgesEnd) return;
    
    // Validate vector memory is readable
    if (IsBadReadPtr(edgesBegin, 64)) return;
    
    int edgeSize = 60; // 0x3C based on raw bytes analysis
    int numEdges = (int)(edgesEnd - edgesBegin) / edgeSize;
    
    for (int i = 0; i < numEdges && i < 1000; i++) {
        DWORD edgeAddr = (DWORD)edgesBegin + (i * edgeSize);
        
        // Validate edge memory is readable
        if (IsBadReadPtr((void*)edgeAddr, edgeSize)) continue;
        
        EDGE_FLAG btFlag = *(EDGE_FLAG*)(edgeAddr + 0x00);
        DWORD* pVertex0 = *(DWORD**)(edgeAddr + 0x04);
        DWORD* pVertex1 = *(DWORD**)(edgeAddr + 0x08);
        
        if (!pVertex0 || !pVertex1) continue;
        
        // Validate vertex pointers are readable (12 bytes for D3DXVECTOR3)
        if (IsBadReadPtr(pVertex0, 12) || IsBadReadPtr(pVertex1, 12)) continue;
        
        // PrimNavMeshVertex: D3DXVECTOR3 Position at offset 0
        D3DXVECTOR3 p1, p2, p1_2d, p2_2d;
        p1.x = *(float*)((DWORD)pVertex0 + 0x00);
        p1.y = *(float*)((DWORD)pVertex0 + 0x04);
        p1.z = *(float*)((DWORD)pVertex0 + 0x08);
        
        p2.x = *(float*)((DWORD)pVertex1 + 0x00);
        p2.y = *(float*)((DWORD)pVertex1 + 0x04);
        p2.z = *(float*)((DWORD)pVertex1 + 0x08);
        
        rotatey(p1, yaw);
        p1.x += offset.x;
        p1.y += offset.y;
        p1.z += offset.z;
        
        rotatey(p2, yaw);
        p2.x += offset.x;
        p2.y += offset.y;
        p2.z += offset.z;
        
        bool vis1 = gfx->Project(p1, p1_2d) > 0;
        bool vis2 = gfx->Project(p2, p2_2d) > 0;
        
        if (vis1 && vis2) {
            DXDrawLine((int) p1_2d.x, (int) p1_2d.y, (int) p2_2d.x, (int) p2_2d.y,
                       GetColorFromEdgeFlag(btFlag), 1.0);
        }
    }
}

void NavMeshTool::RenderObjectInternalEdges(const SNavMeshInst *pInst) const {
    CGFXVideo3d *gfx = CGFXVideo3d::get();
    
    // Raw memory access for SNavMeshInst
    CRTNavMeshObj* pObj = *(CRTNavMeshObj**)((char*)pInst + 0xB0);
    if (!pObj) return;
    
    D3DVECTOR offset;
    offset.x = *(float*)((char*)pInst + 0x06);
    offset.y = *(float*)((char*)pInst + 0x0A);
    offset.z = *(float*)((char*)pInst + 0x0E);
    float yaw = -*(float*)((char*)pInst + 0x14);
    
    // CRTNavMeshObj::m_InternalEdges at offset 0x2C (std::vector)
    DWORD objAddr = (DWORD)pObj;
    BYTE* edgesBegin = *(BYTE**)(objAddr + 0x2C);
    BYTE* edgesEnd = *(BYTE**)(objAddr + 0x2C + 4);
    
    if (!edgesBegin || !edgesEnd || edgesBegin >= edgesEnd) return;
    
    // Validate vector memory is readable
    if (IsBadReadPtr(edgesBegin, 64)) return;
    
    int edgeSize = 60; // 0x3C based on raw bytes analysis
    int numEdges = (int)(edgesEnd - edgesBegin) / edgeSize;
    
    for (int i = 0; i < numEdges && i < 1000; i++) {
        DWORD edgeAddr = (DWORD)edgesBegin + (i * edgeSize);
        
        // Validate edge memory is readable
        if (IsBadReadPtr((void*)edgeAddr, edgeSize)) continue;
        
        EDGE_FLAG btFlag = *(EDGE_FLAG*)(edgeAddr + 0x00);
        DWORD* pVertex0 = *(DWORD**)(edgeAddr + 0x04);
        DWORD* pVertex1 = *(DWORD**)(edgeAddr + 0x08);
        
        if (!pVertex0 || !pVertex1) continue;
        
        // Validate vertex pointers are readable (12 bytes for D3DXVECTOR3)
        if (IsBadReadPtr(pVertex0, 12) || IsBadReadPtr(pVertex1, 12)) continue;
        
        D3DXVECTOR3 p1, p2, p1_2d, p2_2d;
        p1.x = *(float*)((DWORD)pVertex0 + 0x00);
        p1.y = *(float*)((DWORD)pVertex0 + 0x04);
        p1.z = *(float*)((DWORD)pVertex0 + 0x08);
        
        p2.x = *(float*)((DWORD)pVertex1 + 0x00);
        p2.y = *(float*)((DWORD)pVertex1 + 0x04);
        p2.z = *(float*)((DWORD)pVertex1 + 0x08);
        
        rotatey(p1, yaw);
        p1.x += offset.x;
        p1.y += offset.y;
        p1.z += offset.z;
        
        rotatey(p2, yaw);
        p2.x += offset.x;
        p2.y += offset.y;
        p2.z += offset.z;
        
        bool vis1 = gfx->Project(p1, p1_2d) > 0;
        bool vis2 = gfx->Project(p2, p2_2d) > 0;
        
        if (vis1 && vis2) {
            DXDrawLine((int) p1_2d.x, (int) p1_2d.y, (int) p2_2d.x, (int) p2_2d.y,
                       GetColorFromEdgeFlag(btFlag), 1.0);
        }
    }
}

void NavMeshTool::MenuItem() {
    ImGui::MenuItem("NavMesh Explorer", 0, &bShow);
}
