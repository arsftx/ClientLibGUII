# CIFMinimap Complete Reverse Engineering Analysis

## Class Information
- **Constructor**: `sub_539620` (factory), `sub_5397E0` (real constructor)
- **Class Size**: 880 bytes
- **VTable**: `0x94AD48`

## Key Member Offsets

| Offset (dec) | Offset (hex) | Type | Description | Source |
|--------------|--------------|------|-------------|--------|
| 788 | 0x314 | float | Player LOCAL X (adjusted for tile) | UpdateMap line 11735, 11770 |
| 792 | 0x318 | float | Player LOCAL Y (height) | UpdateMap line 11736 |
| 796 | 0x31C | float | Player LOCAL Z (adjusted for tile) | UpdateMap line 11737, 11771 |
| 800 | 0x320 | float | Player Rotation | UpdateMap line 11738 |
| 816 | 0x330 | float | Zoom Factor (default 160.0) | Constructor line 11176 |
| 820 | 0x334 | float | ??? | Need ASM |
| 824 | 0x338 | float | arrowOffsetX | UpdateMap line 11801 |
| 828 | 0x33C | float | arrowOffsetY | UpdateMap line 11800, 11802 |
| 856 | 0x358 | int | Current Tile X | UpdateMap line 11731, 11767 |
| 860 | 0x35C | int | Current Tile Y | UpdateMap line 11733, 11769 |

## Key Global Constants

| Address | Name | Value | Description |
|---------|------|-------|-------------|
| `flt_94AE08` | SCALE_001 | **???** | Used for coordinate scaling - NEED ASM |
| `flt_94AE04` | SCALE_01 | **???** | Used for display coordinates |
| `flt_93B838` | TILE_SIZE | **???** | Probably 192.0 (tile size) - NEED ASM |
| `flt_93B884` | HALF_CONST | **???** | 0.5 - NEED ASM |
| `flt_93B84C` | MARKER_SIZE | **???** | Marker offset - NEED ASM |
| `dword_A0465C` | g_pPlayer | ptr | Player pointer global |

## UpdateMap (sub_53A5A0) Formula Analysis

### Player Position Loading (line 11735-11737)
```cpp
*(float*)(this + 788) = *(float*)(player + 116);  // player+0x74 -> this+0x314
*(float*)(this + 792) = *(float*)(player + 120);  // player+0x78 -> this+0x318
*(float*)(this + 796) = *(float*)(player + 124);  // player+0x7C -> this+0x31C
```

### Tile Index Calculation (Dungeon mode, line 11760-11771)
```cpp
tileX = floor(playerLocalX * flt_94AE08) + 128;  // this+856
tileY = floor(playerLocalZ * flt_94AE08) + 128;  // this+860

// Adjust player position to be LOCAL within tile
playerLocalX -= floor(playerLocalX * flt_94AE08) * flt_93B838;
playerLocalZ -= floor(playerLocalZ * flt_94AE08) * flt_93B838;
```

### Arrow Offset Calculation (line 11800-11802)
```cpp
arrowOffsetX = playerLocalX * zoom * flt_94AE08;  // this+824
arrowOffsetY = (flt_93B838 - playerLocalZ) * zoom * flt_94AE08;  // this+828
```

## Render (sub_53AD20) Entity Position Analysis

### Variables Initialization (line 12355-12360)
```cpp
v188 = viewport.left;    // viewport X origin
v199 = viewport.top;     // viewport Y origin  
v196 = viewport.width;
v198 = viewport.height;
v189 = viewport.width * flt_93B884;   // halfWidth
v200 = viewport.height * flt_93B884;  // halfHeight
```

### Entity Relative Position (line 12620-12624)
```cpp
// v31 = entity pointer (from linked list at dword_9C99A4)
relZ = *(float*)(entity + 140) - *(float*)(player + 124);  // entity+0x8C - player+0x7C
relX = *(float*)(entity + 132) - *(float*)(player + 116);  // entity+0x84 - player+0x74
```

### Entity Screen Position (line 12677-12682)
```cpp
v37 = relX * zoom * flt_94AE08 + v189;  // X screen (relative to centerX)
screenY = v200 + v199 - relZ * zoom * flt_94AE08;  // Y screen

// Final marker corners:
v216 = v37 + v188 + flt_93B84C;  // right-top X
v211 = screenY - flt_93B84C;     // right-top Y
```

## Critical Questions Requiring ASM Verification

> [!CAUTION]
> The following values are assumed from pseudo-code but NOT verified:

1. **`flt_94AE08` at address 0x0094AE08**
   - Assumed: 0.01f
   - Need ASM verification

2. **`flt_93B838` at address 0x0093B838**
   - Assumed: 192.0f (tile size)
   - Need ASM verification

3. **`flt_93B884` at address 0x0093B884**
   - Assumed: 0.5f
   - Need ASM verification

4. **Entity position formula at 0x53B100-0x53B180**
   - The pseudo-code may have decompilation errors
   - Need exact ASM for lines 12677-12688

## Tile vs Entity Coordinate Flow

### Tiles (DrawMapTiles equivalent, lines 12363-12518):
```
1. Load viewport rect
2. For each tile in 3x3 grid:
   - tileLeft = (dx-1) * zoom - arrowOffsetX + halfWidth
   - tileTop = (dy-1) * zoom - arrowOffsetY + halfHeight
   - Draw tile at (tileLeft + viewportX, tileTop + viewportY)
```

### Entities (lines 12602-12717):
```
1. For each entity in linked list:
   - relX = entityWorldX - playerWorldX  
   - relZ = entityWorldZ - playerWorldZ
   - screenX = relX * zoom * 0.01 + halfWidth + viewportX
   - screenY = halfHeight + viewportY - relZ * zoom * 0.01
   - Draw marker at (screenX, screenY)
```

## Identified Potential Issues

1. **Player position source mismatch**:
   - UpdateMap stores LOCAL position (adjusted) at this+788/796
   - Entity rendering uses player+116/124 (WORLD position)
   - These should be in the same coordinate system

2. **Missing coordinate transformation**:
   - IF player+116/124 are WORLD coordinates
   - AND entity+132/140 are also WORLD coordinates
   - THEN relX/relZ calculation should work...
   - BUT if one is local and one is world, the math breaks

## Request for ASM

To resolve the coordinate mismatch, I need ASM dumps of:

1. **`flt_94AE08` value** - read from address `0x0094AE08`
2. **`flt_93B838` value** - read from address `0x0093B838`
3. **Entity position calculation** - function `sub_53AD20` around address `0x53B100-0x53B180`
