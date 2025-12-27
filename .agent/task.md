# CustomMinimap DDJ Tile System - Implementation Status

## Current Status: Zoom Sync Fix Applied ✅

### Latest Fix (This Session)
**Problem**: Tiles used native zoom (160.0f) but entities used m_fZoomFactor (0.5-16.0)
**Solution**: All rendering now uses native CIFMinimap values consistently

### Unified Scale System
```cpp
// All rendering uses same scale:
float nativeZoom = pNative->GetZoomFactor();  // ~160.0f
float scale = nativeZoom / 192.0f;            // pixels per world unit

// Entity screen position:
screenX = center.x + relX * scale;
screenY = center.y - relZ * scale;

// Tile screen position:
tileX = halfWidth + dx * nativeZoom - arrowOffsetX;
tileY = halfHeight + dy * nativeZoom - arrowOffsetY;
```

### Files Modified Today
| File | Function | Change |
|------|----------|--------|
| CustomMinimap.cpp | DrawMapTiles | Uses native zoom/arrow values |
| CustomMinimap.cpp | DrawEntityMarkers | Uses native zoom for scale |
| CustomMinimap.cpp | Render | DrawPartyMembers uses native scale |

### Native CIFMinimap Values Used
| Offset | Getter | Purpose |
|--------|--------|---------|
| +704 | m_pMapTiles[9] | DDJ texture array |
| +816 | GetZoomFactor() | Tile size in pixels (~160) |
| +824 | GetArrowScreenX() | X scroll offset |
| +828 | GetArrowScreenY() | Y scroll offset |

### Formula Derivation
```
Native: 1 region = 192 world units
Native: 1 region = nativeZoom screen pixels
Therefore: scale = nativeZoom / 192.0f (pixels per world unit)

Entity at world pos (x,z) relative to player:
  screenX = center + relX * scale
  screenY = center - relZ * scale  (Y inverted)
```

## Testing Checklist
- [ ] Tiles render correctly at all zoom levels
- [ ] Entities stay in correct positions on tiles
- [ ] Zoom in/out works without breaking layout
- [ ] Party members position correctly
- [ ] Player arrow stays at center
