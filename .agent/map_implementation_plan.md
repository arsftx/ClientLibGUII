# CIFWorldMap (CIFMap) Reverse Engineering Implementation Plan

## Project Overview

**Goal:** 100% reverse engineer the native CIFMap (WorldMap) system and create a completely custom, extensible map system.

**Reference:** CIFMinimap reverse engineering (already completed) - Many systems are similar or shared.

---

## Discovery Summary

### Class Registration (VERIFIED)
```
Address: 0x552D50
Function: sub_552D50
Call: sub_898D80("CIFWorldMap", sub_552D80, sub_552DE0, &unk_9FFED4, 15620, 0)

Parameters:
- Class Name: "CIFWorldMap"
- Create Function: sub_552D80
- Delete Function: sub_552DE0
- RuntimeClass Ptr: 0x9FFED4
- Size: 15620 bytes (0x3D04)
- Flags: 0
```

### Constructor (sub_552EC0)
```cpp
// Key initializations:
*(this + 1692) = string buffer (path storage)
*this = off_94B270;           // VTable
*(this + 108) = &off_94B228;  // Inner VTable (offset 27*4=108)
*(this + 3904) = linked list head  // Marker/point list

// String path initialized with Dir variable
```

### VTable Address
```
Primary VTable: 0x94B270
Inner VTable: 0x94B228
```

---

## Key Functions Identified

| Address | Function | Purpose | Strings |
|---------|----------|---------|--------|
| 0x552D50 | sub_552D50 | Registration | "CIFWorldMap" |
| 0x552D80 | sub_552D80 | Create | - |
| 0x552DE0 | sub_552DE0 | Delete | - |
| 0x552EC0 | sub_552EC0 | Constructor | "P0U" |
| 0x553090 | sub_553090 | Destructor | wmap textures |
| 0x553490 | sub_553490 | OnCreate | "resinfo\\ifworldmap.txt", "Create" |
| 0x553500 | sub_553500 | UpdateMap | - |
| **0x553920** | **sub_553920** | **Button Handler 1** | Calls ToggleZoom |
| **0x553960** | **sub_553960** | **Button Handler 2** | Calls SetMapIndex(0,0) |
| **0x553980** | **sub_553980** | **Mouse/Input Handler** | Pan/click detection |
| **0x553B90** | **sub_553B90** | **Main Render** | Calls sub-renderers |
| **0x553BE0** | **sub_553BE0** | **Player Position Update** | Uses dword_A0465C (Player) |
| **0x553D10** | **sub_553D10** | **Render Party Members** | Party + player arrow |
| **0x554770** | **sub_554770** | **Render Other Players** | Via sub_4751F0 chain |
| **0x554B90** | **sub_554B90** | **Render Sub1** | Called from main render |
| **0x554DC0** | **sub_554DC0** | **Render Sub2** | Called from main render |
| **0x554EF0** | **sub_554EF0** | **Render Sub3** | Called from main render |
| 0x555240 | sub_555240 | Pan/Move Map | Delta X/Y movement |
| 0x555490 | sub_555490 | Initialize View Pos | Sets 3888-3891 offsets |
| 0x555510 | sub_555510 | Point Inside Check | Hit test |
| 0x555530 | sub_555530 | SetMapIndex | Towns, error handling |
| 0x5559E0 | sub_5559E0 | ToggleZoom | World/Local toggle |
| 0x555AD0 | sub_555AD0 | LoadLocations | NPC markers etc |
| **0x556520** | **sub_556520** | **Clear Markers** | Called before reload |
| **0x556750** | **sub_556750** | **Unknown** | Window related |

---

## Render Pipeline (sub_553B90)

```cpp
int __thiscall CIFWorldMap::Render(CIFWorldMap* this) {
    sub_564640();                    // Begin render setup
    sub_446600(this + 3888);         // Set viewport/clip rect
    sub_554B90(this);                // Render map tiles
    sub_554DC0(this);                // Render location markers
    sub_553D10(this);                // Render party members + player arrow
    sub_554770(this);                // Render other players
    sub_554EF0(this);                // Render additional elements
    return sub_446680();             // End render
}
```

---

## Memory Layout (Detailed Analysis)

### Offset Structure (15620 bytes = 0x3D04)

```
Offset      Size    Type        Member Name              Evidence / Formula
────────────────────────────────────────────────────────────────────────────
0x0000      4       DWORD*      VTable                   *(this) = off_94B270
0x006C      4       DWORD*      InnerVTable              *(this+27*4) = &off_94B228

// === Mouse State (from sub_553980) ===
0x0684      4       int         m_nPrevMouseX            this+1668 (a2[5] stored)
0x0688      4       int         m_nPrevMouseY            this+1672 (a2[6] stored)
0x068C      4       int         m_nMouseX                this+1676
0x0690      4       int         m_nMouseY                this+1680

// === Path String Buffer ===
0x069C      ???     String      m_strPath                this+1692 (char buffer)
0x06B0      4       void*       m_pTexture1              this+1724
0x06C0      4       void*       m_pTexture2              this+1728

// === Tile Vertex Data (from sub_553500) ===
0x08E0      ???     float[]     m_fTileVertices          this+2272 (tile grid vertices)
                                // 19x7 tiles × 4 verts × 6 floats each
                                // Total: 133 × 4 × 24 = 12768 bytes

// === Map Background Vertices (from sub_553500) ===
0x3AEC      4       float       m_fBgVertex1X            this+15036
0x3AF0      4       float       m_fBgVertex1Y            this+15040
0x3AFC      4       float       m_fBgVertex2X            this+15060
0x3B00      4       float       m_fBgVertex2Y            this+15064
0x3B14      4       float       m_fBgVertex3X            this+15084
0x3B18      4       float       m_fBgVertex3Y            this+15088
0x3B24      4       float       m_fBgVertex4X            this+15108
0x3B28      4       float       m_fBgVertex4Y            this+15112

// === Player Position (from sub_553BE0) ===
0x3CA4      4       float       m_fPlayerScreenX         this+15516
0x3CA8      4       float       m_fPlayerScreenY         this+15520
0x3CAC      4       float       m_fPlayerWorldX          this+15524
0x3CB0      4       float       m_fPlayerWorldY          this+15528
0x3CB4      4       float       m_fPlayerWorldZ          this+15532
0x3CB8      4       float       m_fPlayerRotation        this+15536

// === Map State Flags ===
0x3CC8      4       DWORD       m_nFlag1                 this+15544
0x3CE0      4       int         m_nViewX                 this+15552
0x3CE4      4       int         m_nViewY                 this+15556
0x3CE8      4       int         m_nViewWidth             this+15560
0x3CEC      4       int         m_nViewHeight            this+15564
0x3CF0      4       int         m_nRegionOffsetX         this+15568 (192 * regionX)
0x3CF4      4       int         m_nRegionOffsetY         this+15580
0x3CF8      4       float       m_fScaleX                this+15584
0x3CFC      4       float       m_fScaleY                this+15588
0x3D00      4       DWORD       m_nClickedAreaID         this+15592
0x3D04      4       void*       m_pMapInfo               this+15603 (sub_606210 target)
0x3D0C      4       void*       m_pMarkerListHead        this+15616

// === View Zone Rectangle ===
0x3CC0+     4       int         m_nViewRectX             this+3888
0x3CC4      4       int         m_nViewRectY             this+3889
0x3CC8      4       int         m_nViewRectW             this+3890
0x3CCC      4       int         m_nViewRectH             this+3891

// === Region Calculations ===
0x3CD0      4       int         m_nRegionCalcX           this+3892
0x3CD4      4       int         m_nRegionCalcY           this+3893
0x3CD8      4       int         m_nRegionCalcX2          this+3894
0x3CDC      4       int         m_nRegionCalcY2          this+3895
0x3CE0      4       float       m_fMapScaleX             this+3896
0x3CE4      4       float       m_fMapScaleY             this+3897

// === Zoom Button Pointers ===
0x3CF4      4       void*       m_pBtnZoomIn             this+3901
0x3CF8      4       void*       m_pBtnZoomOut            this+3902

// === Map Info ===
0x3CFC      4       void*       m_pCurrentMapInfo        this+3903
0x3D00      4       void*       m_pMarkerList            this+3904
```

### Sub-structure: View Zone (this+3888 to 3891)
```
Offset 3888: X position (screen)
Offset 3889: Y position (screen)
Offset 3890: Width
Offset 3891: Height
```

---

## Texture Resources (from sub_553090 destructor)

```
interface\worldmap\wmap_window_edge_1.ddj    (window edge 1)
interface\worldmap\wmap_window_edge_2.ddj    (window edge 2)
interface\worldmap\wmap_window_edge_3.ddj    (window edge 3)
interface\worldmap\wmap_window_edge_4.ddj    (window edge 4)
interface\minimap\mm_sign_character.ddj      (shared with minimap)
interface\worldmap\wmap_bg.ddj               (background)
interface\worldmap\wmap_zoom.ddj             (zoom controls)

Map Tiles (136 tiles total):
interface\worldmap\map\map_world_{X}x{Y}.ddj
  X range: 102 to 178 (step 4) = 19 tiles
  Y range: 81 to 109 (step 4) = 7 tiles (reverse order)
  Total: 19 * 7 = 133 tiles (loop generates these)
```

---

## Coordinate System Analysis

### World Map Coordinates
```
Region based calculation (from sub_555530):
- RegionCalcX = *(MapInfo+72) + 192 * (RegionX - 135)
- RegionCalcY = *(MapInfo+76) + 192 * (RegionY - 92)
- RegionCalcX2 = *(MapInfo+80) + 192 * (RegionX2 - 135)
- RegionCalcY2 = *(MapInfo+84) + 192 * (RegionY2 - 92)

Scale calculation:
- ScaleX = abs((RegionCalcX2 - RegionCalcX) / MapWidth)
- ScaleY = abs((RegionCalcY - RegionCalcY2) / MapHeight)
```

### Local Map Modes
```
Mode 0: World Map (652x424 pixels)
Mode 1: Local Map (268x296 pixels)

Local map indices:
1 = Jangan (UIIT_STT_JANGAN)
2 = Donhwan (UIIT_STT_DONHWAN)  
3 = Khotan (UIIT_STT_KHOTAN)
```

---

## Marker/Point System

### MarkerData Structure (from sub_555AD0 analysis)
```cpp
struct WorldMapMarker {
    DWORD  type;           // +0: 1=NPC, 2=Area
    DWORD  field_4;        // +4
    DWORD  field_8;        // +8
    DWORD  mapIndex;       // +12 (offset 3)
    char*  name;           // +24 (offset 6) - String buffer
    ???    field_36;       // +36 (offset 9)
    ???    iconData;       // +41 onwards
    DWORD  regionX;        // +156 (offset 39)
    DWORD  regionY;        // +160 (offset 40)
    // ... more fields
};
```

---

## Comparison with CIFMinimap

| Feature | CIFMinimap | CIFWorldMap |
|---------|------------|-------------|
| Size | 880 bytes | 15620 bytes |
| VTable | 0x94AD48 | 0x94B270 |
| IRM ID | 10 | TBD |
| Tile count | 9 (3x3) | 133 (19x7) |
| Zoom levels | 80-320 | World/Local toggle |
| Markers | Entity-based | Point-based |

---

## Next Steps

1. **Complete offset mapping** - Systematically analyze constructor and all functions
2. **VTable analysis** - Map all virtual function entries
3. **Render function** - Find and analyze the render/draw function
4. **Create header file** - CIFWorldMap.h with all structures
5. **IDA script** - apply_CIFWorldMap.py for automation
6. **Integration test** - Hook and verify access

---

## Files to Create

```
ClientLibGUI/source/libs/ClientLib/src/
├── CIFWorldMap.h         [NEW] - Class definition
├── CIFWorldMap.cpp       [NEW] - Implementation

ClientLibGUI/source/DevKit_DLL/src/imgui_windows/
├── CustomWorldMap.h      [NEW] - Custom map header
├── CustomWorldMap.cpp    [NEW] - Custom map implementation

ClientLibGUI/ida_scripts/
├── apply_CIFWorldMap.py  [NEW] - IDA automation
```

---

## Change Log

| Date | Change | Reason |
|------|--------|--------|
| 2024-12-28 | Initial plan created | Project start |

