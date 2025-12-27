# Native CIFMinimap Tile Rendering Deep Analysis

## sub_53AD20 Tile Render Loop (Lines 12352-12518)

### Key Variables and Offsets
```cpp
this + 704   = m_pMapTiles[9] start (v5 = this + 704)
this + 816   = m_fZoomFactor (zoom value, default 160.0f)
this + 824   = m_fArrowScreenX (calculated from player position)
this + 828   = m_fArrowScreenY (calculated from player position)

flt_93B884   = 0.5f (half factor constant)
flt_93B834   = 0.0f (zero constant)
flt_93CFE8   = 1.0f (one constant)

v295[0..3]   = Window rect (x, y, width, height) from sub_89F0C0
```

### Loop Structure
```
v4 (outer loop) = 0 to 2   (X tile index: -1, 0, +1)
v6 (inner loop) = 0 to 2   (Y tile index: -1, 0, +1)
v209 = v4 - 1              (dx offset: -1, 0, +1)
v187/v6-1 = v6 - 1         (dy offset: -1, 0, +1)
```

### Position Calculation (Lines 12371-12380)
```cpp
// Where zoom = this + 816, arrowX = this + 824, arrowY = this + 828
// halfWidth = v196 * 0.5, halfHeight = v198 * 0.5

v7 = (v4 - 1) * zoom;                        // dx * zoom
v8 = v7 - arrowX;                            // dx * zoom - arrowX
v181 = v8 + halfWidth;                       // Left edge: dx*zoom - arrowX + halfWidth

v191 = (v6 - 1) * zoom - arrowY + halfHeight; // Top edge: dy*zoom - arrowY + halfHeight

v190 = v181 + zoom;                          // Right edge: left + zoom
v187 = v191 + zoom;                          // Bottom edge: top + zoom
```

### Vertex Positions (Lines 12415-12431)
```cpp
// Window position offsets
v263 = bottomLeftX = v188 + v11 - halfWidth;
v264 = topLeftY = v191 + v199 - halfHeight;
v271 = topRightX = v190 + v188 - halfWidth;
v272 = topRightY = v264;
...etc for 4 corners, then subtracting halfWidth/halfHeight
```

## Key Insights

### 1. Arrow Position (this+824, this+828)
These are PRE-CALCULATED in `sub_53A5A0` (UpdateMap):
```cpp
// From sub_53A5A0 lines 11800-11802:
// arrowX = zoom * localPosX * 0.01f
// arrowY = (192.0f - localPosZ) * zoom * 0.01f
*(float*)(this + 824) = *(float*)(this + 816) * localPosX * flt_94AE08;
*(float*)(this + 828) = (flt_93B838 - localPosZ) * *(float*)(this + 816) * flt_94AE08;
```

Where:
- `flt_93B838` = 192.0f (region size)
- `flt_94AE08` = 0.01f (scale factor)

### 2. Tile Position Formula
For tile at (dx, dy) offset:
```
tileLeftX = halfWidth + dx * zoom - arrowX + windowX
tileTopY = halfHeight + dy * zoom - arrowY + windowY
```

### 3. What We're Missing
We need to use the PRE-CALCULATED arrow offsets from native CIFMinimap:
- `pNative->m_fArrowScreenX` (offset +824)
- `pNative->m_fArrowScreenY` (offset +828)
- `pNative->m_fZoomFactor` (offset +816)

## Proposed Fix

Instead of calculating our own position offsets, USE the native calculated values:
```cpp
float nativeZoom = pNative->GetZoomFactor();      // +816
float arrowOffsetX = pNative->GetArrowScreenX();  // +824  
float arrowOffsetY = pNative->GetArrowScreenY();  // +828

// For tile at (dx, dy):
float tileLeftX = (mapSize * 0.5f) + dx * nativeZoom - arrowOffsetX;
float tileTopY = (mapSize * 0.5f) + dy * nativeZoom - arrowOffsetY;
```

## Native Constants Needed
```
flt_93B884 = 0.5f
flt_93B838 = 192.0f (REGION_SIZE)
flt_94AE08 = 0.01f
```
