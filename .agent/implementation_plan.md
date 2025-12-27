# CustomMinimap Implementation Plan

## Current Status: DDJ Tile System Implemented ✅

### Architecture Overview

```
┌─────────────────────────────────────────────┐
│           CustomMinimap::Render()           │
├─────────────────────────────────────────────┤
│ 1. UpdateMap() - Native tile loading        │
│ 2. DrawMapTiles() - 3x3 DDJ tile grid       │
│ 3. DrawMinimapBackground() - Border only    │
│ 4. DrawEntityMarkers() - Monsters/NPCs/etc  │
│ 5. DrawPartyMembers() - Cyan party markers  │
│ 6. DrawPlayerMarker() - Yellow arrow        │
│ 7. DrawZoomControls() - +/- buttons         │
│ 8. DrawCoordinates() - X:Y region text      │
└─────────────────────────────────────────────┘
```

### Native CIFMinimap Integration

**Tiles (using native directly):**
- `CIFMinimap::m_pMapTiles[9]` at offset +704
- 3x3 grid loaded by native `UpdateMap()` (sub_53A5A0)
- Path format: `minimap\%dx%d.ddj`

**Entity Detection:**
- Uses `dword_9C99A4` native linked list
- `GetEntityTypeByRuntimeClass()` checks RuntimeClass pointers

**Party Members:**
- Separate `DrawPartyMembers()` function
- Uses `PartyManager` at `unk_A01510`
- Cyan color, separate from entity loop

### Files Modified

| File | Changes |
|------|---------|
| `CustomMinimap.h` | Added tile members, function declarations |
| `CustomMinimap.cpp` | DDJ tile system, party system, optimizations |
| `CIFMinimap.h` | Added `UpdateMap()` wrapper |

### Performance Notes

- Native linked list only (no EntityManager traversal)
- Quick address checks instead of IsBadReadPtr
- 500 entity loop limit for safety
- Native tiles - no duplicate loading
