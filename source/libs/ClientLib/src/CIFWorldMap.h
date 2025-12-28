#pragma once

#include <windows.h>
#include "CIFWnd_ECSRO.h"

// =============================================================================
// CIFWorldMap - World Map window class (full map, not minimap)
// =============================================================================
// 
// REVERSE ENGINEERING EVIDENCE:
//   Registration: sub_552D50 -> sub_898D80("CIFWorldMap", ..., 15620, 0)
//   Constructor:  sub_552EC0 (allocates via sub_899010(15620))
//   Destructor:   sub_553090
//   OnCreate:     sub_553490 (resinfo\ifworldmap.txt, "Create")
//   UpdateMap:    sub_553500 (vertex/position calculations)
//   Render:       sub_553B90 (main render pipeline)
//   MouseHandler: sub_553980 (pan/click detection)
//
// VERIFIED VALUES:
//   Size:         0x3D04 (15620 bytes) - from sub_552D50 registration
//   VTable:       0x0094B270 - from sub_552EC0 line: *(this) = off_94B270
//   Inner VTable: 0x0094B228 - from sub_552EC0 line: *(this+27) = &off_94B228
//   IRM ID:       TBD (needs verification from ginterface.txt)
//
// =============================================================================
// KEY DIFFERENCES FROM CIFMinimap:
// =============================================================================
//   - CIFMinimap:  880 bytes (0x370)  - always visible corner map
//   - CIFWorldMap: 15620 bytes (0x3D04) - full screen world map overlay
//   - WorldMap has pan/drag functionality
//   - WorldMap has zoom toggle (world/local)
//   - WorldMap has clickable location markers
//   - WorldMap renders 19x7 tile grid vs 3x3
//
// =============================================================================
// RENDER PIPELINE (sub_553B90):
// =============================================================================
//   1. sub_564640()       - Begin render setup  
//   2. sub_446600(+3888)  - Set viewport/clip rect
//   3. sub_554B90(this)   - Render map tiles
//   4. sub_554DC0(this)   - Render location markers
//   5. sub_553D10(this)   - Render party members + player arrow
//   6. sub_554770(this)   - Render other players
//   7. sub_554EF0(this)   - Render additional elements
//   8. sub_446680()       - End render
//
// =============================================================================
// COORDINATE SYSTEM:
// =============================================================================
//   World -> Map transformation:
//     RegionCalcX = MapInfo[72] + 192 * (RegionX - 135)
//     RegionCalcY = MapInfo[76] + 192 * (RegionY - 92)
//     
//   Player screen position:
//     ScreenX = PlayerWorldX / (ScaleX * flt_946584) + ViewX
//     ScreenY = MapHeight - PlayerWorldZ / (ScaleY * flt_946584) + ViewY
//
// =============================================================================
// MAP MODES:
// =============================================================================
//   Mode 0: World Map (652x424 pixels)
//   Mode 1: Local Map (268x296 pixels)
//     Index 1 = Jangan
//     Index 2 = Donhwan  
//     Index 3 = Khotan
//
// =============================================================================

// Forward declarations
struct WorldMapMarker;

class CIFWorldMap {
public:
    // =========================================================================
    // BASE CLASS (CIFWnd) - ~692 bytes at start
    // =========================================================================
    char pad_CIFWnd[0x02B4];               // 0x0000-0x02B3 (+0 to +691)
    
    // =========================================================================
    // UNKNOWN REGION 0x02B4 to 0x0684
    // =========================================================================
    char pad_02B4[0x03D0];                 // 0x02B4-0x0683 (padding, content TBD)
    
    // =========================================================================
    // MOUSE STATE (from sub_553980)
    // =========================================================================
    int m_nPrevMouseX;                     // 0x0684 (+1668) - Previous mouse X
    int m_nPrevMouseY;                     // 0x0688 (+1672) - Previous mouse Y
    int m_nMouseX;                         // 0x068C (+1676) - Current mouse X
    int m_nMouseY;                         // 0x0690 (+1680) - Current mouse Y
    
    // =========================================================================
    // PATH STRING BUFFER (from sub_552EC0)
    // =========================================================================
    char pad_0694[8];                      // 0x0694 padding
    void* m_pPathBuffer;                   // 0x069C (+1692) - Path string data
    void* m_pPathEnd;                      // 0x06A0 (+1696) - String end ptr
    void* m_pPathCapacity;                 // 0x06A4 (+1700) - Allocated capacity
    
    // =========================================================================
    // UNKNOWN REGION 0x06A8 to 0x06BC
    // =========================================================================
    char pad_06A8[0x18];                   // 0x06A8-0x06BF
    
    // =========================================================================
    // TEXTURE POINTERS (from sub_553D10)
    // =========================================================================
    void* m_pTexture1;                     // 0x06BC (+1724) - Texture used in render
    void* m_pTexture2;                     // 0x06C0 (+1728) - Texture used in render
    
    // =========================================================================
    // UNKNOWN REGION 0x06C4 to 0x08E0
    // =========================================================================
    char pad_06C4[0x021C];                 // Padding to tile vertices
    
    // =========================================================================
    // TILE VERTEX DATA (from sub_553500)
    // 19 columns x 7 rows = 133 tiles
    // Each tile = 4 vertices x 6 floats = 24 floats = 96 bytes
    // Total: 133 * 96 = 12768 bytes
    // =========================================================================
    float m_fTileVertices[133 * 24];       // 0x08E0 (+2272) - Tile vertex buffer
    
    // =========================================================================
    // PADDING TO BACKGROUND VERTICES
    // =========================================================================
    // Note: Exact padding depends on tile vertex size confirmation
    
    // =========================================================================
    // BACKGROUND VERTICES (from sub_553500 - lines 29273-29288)
    // =========================================================================
    // These are at offsets 15036, 15040, 15060, 15064, 15084, 15088, 15108, 15112
    // Hex offsets: 0x3AAC, 0x3AB0, 0x3AC4, 0x3AC8, 0x3ADC, 0x3AE0, 0x3AF4, 0x3AF8
    // These form the 4 corners of the map background quad
    
    // =========================================================================
    // PLAYER POSITION (from sub_553BE0)
    // =========================================================================
    // float m_fPlayerScreenX;             // +15516 (0x3CAC) - Player screen X
    // float m_fPlayerScreenY;             // +15520 (0x3CB0) - Player screen Y
    // float m_fPlayerWorldX;              // +15524 (0x3CB4) - Player world X
    // float m_fPlayerWorldY;              // +15528 (0x3CB8) - Player world Y  
    // float m_fPlayerWorldZ;              // +15532 (0x3CBC) - Player world Z
    // float m_fPlayerRotation;            // +15536 (0x3CC0) - Player rotation (radians)
    
    // =========================================================================
    // MAP STATE FLAGS
    // =========================================================================
    // DWORD m_nFlag1;                     // +15544 (0x3CC8) - State flag
    // int m_nViewX;                       // +15552 (0x3CD0) - View origin X
    // int m_nViewY;                       // +15556 (0x3CD4) - View origin Y
    // int m_nViewWidth;                   // +15560 (0x3CD8) - View width
    // int m_nViewHeight;                  // +15564 (0x3CDC) - View height
    // int m_nRegionOffsetX;               // +15568 (0x3CE0)
    // int m_nRegionOffsetY;               // +15580 (0x3CEC)
    // float m_fScaleX;                    // +15584 (0x3CF0)
    // float m_fScaleY;                    // +15588 (0x3CF4)
    // DWORD m_nClickedAreaID;             // +15592 (0x3CF8)
    
    // =========================================================================
    // MAP INFO & MARKERS
    // =========================================================================
    // void* m_pMapInfo;                   // +15612 (0x3D0C) - Current map info
    // void* m_pMarkerListHead;            // +15616 (0x3D10) - Linked list of markers
    
    // Note: Full layout with exact byte positions requires more ASM verification
    // Current analysis maps key fields, but gaps exist between sections

public:
    // =========================================================================
    // STATIC INSTANCE ACCESS
    // =========================================================================
    static CIFWorldMap* GetInstance() {
        // TODO: Find IRM ID for world map and retrieve via IRM
        // For now, placeholder - needs ginterface.txt verification
        return NULL;
    }
    
    // =========================================================================
    // NATIVE FUNCTION WRAPPERS
    // =========================================================================
    
    // Registration thunk
    static void* Create() {
        return reinterpret_cast<void*(*)()>(0x00552D50)();
    }
    
    // Constructor (called internally by Create)
    // sub_552EC0
    
    // OnCreate - loads UI from ifworldmap.txt
    bool OnCreate(int a2) {
        return reinterpret_cast<bool(__thiscall*)(CIFWorldMap*, int)>(0x00553490)(this, a2);
    }
    
    // UpdateMap - recalculates vertices and positions
    void UpdateMap() {
        reinterpret_cast<void(__thiscall*)(CIFWorldMap*)>(0x00553500)(this);
    }
    
    // Main render function
    void Render() {
        reinterpret_cast<int(__thiscall*)(CIFWorldMap*)>(0x00553B90)(this);
    }
    
    // Update player position on map
    void UpdatePlayerPosition(int forceUpdate = 0) {
        reinterpret_cast<int(__thiscall*)(CIFWorldMap*, int)>(0x00553BE0)(this, forceUpdate);
    }
    
    // Set map index (0=world, 1=jangan, 2=donhwan, 3=khotan)
    void SetMapIndex(int mode, int localIndex) {
        reinterpret_cast<int(__thiscall*)(CIFWorldMap*, int, int)>(0x00555530)(this, mode, localIndex);
    }
    
    // Toggle between world and local map
    void ToggleZoom() {
        reinterpret_cast<void*(__thiscall*)(CIFWorldMap*)>(0x005559E0)(this);
    }
    
    // Pan/move the map view
    void PanMap(int deltaX, int deltaY) {
        reinterpret_cast<void*(__thiscall*)(CIFWorldMap*, int, int)>(0x00555240)(this, deltaX, deltaY);
    }
    
    // Check if point is inside map view area
    bool IsPointInView(int x, int y) {
        return reinterpret_cast<bool(__thiscall*)(CIFWorldMap*, int, int)>(0x00555510)(this, x, y);
    }
    
    // Clear all markers
    void ClearMarkers() {
        reinterpret_cast<void(__thiscall*)(CIFWorldMap*)>(0x00556520)(this);
    }
    
    // Load location markers for current map
    void LoadLocations(int mapIndex) {
        reinterpret_cast<int(__thiscall*)(CIFWorldMap*, int)>(0x00555AD0)(this, mapIndex);
    }
    
    // =========================================================================
    // HELPER ACCESSORS
    // =========================================================================
    
    int GetMouseX() const { return m_nMouseX; }
    int GetMouseY() const { return m_nMouseY; }
    
    // Show/Hide using vtable
    void ShowGWnd(bool bShow) {
        void** vtable = *(void***)this;
        reinterpret_cast<void(__thiscall*)(void*, bool)>(vtable[22])(this, bShow);
    }
    
    // Move window position
    void MoveGWnd(int x, int y) {
        reinterpret_cast<void(__thiscall*)(void*, int, int)>(0x0089F230)(this, x, y);
    }

}; // Size: 0x3D04 (15620 bytes)

// =============================================================================
// WorldMapMarker Structure (from sub_555AD0 analysis)
// =============================================================================
// Used in the linked list at offset +15616
// Each marker represents an NPC, teleport point, or area on the map

struct WorldMapMarker {
    DWORD type;                            // +0: 1=Icon/NPC, 2=Area
    DWORD field_4;                         // +4: Unknown
    DWORD mapIndex;                        // +8: Which local map (0=world)
    DWORD field_C;                         // +12: offset 3 in analysis
    DWORD field_10;                        // +16
    DWORD field_14;                        // +20
    
    // String buffer at +24 (offset 6)
    void* pNameBuffer;                     // +24
    void* pNameEnd;                        // +28
    void* pNameCapacity;                   // +32
    
    // Fields continue...
    // +36 onwards has icon data, texture references
    // +124 (31*4) has text reference
    // +128 has width
    // +132 has height
    // +140 has screen X
    // +144 has screen Y
    // +148 has local X offset
    // +152 has local Y offset
    // +156 has region X
    // +160 has region Y
    // +164-240 has vertex data for type 2 (areas)
};

// =============================================================================
// GLOBAL POINTERS
// =============================================================================

// Player instance pointer (used in sub_553BE0)
// dword_A0465C - Current player/character data

// Party manager (used in sub_553D10)
// unk_A01510 - Party manager base

