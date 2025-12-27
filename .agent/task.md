# CustomMinimap - Tile-Entity Position Sync Debug

## Current Problem
Entity markers (NPCs, monsters, pets) don't sync with DDJ tiles on minimap.

## Debug Status [/]
- [x] Verified native ASM formula (sub_53AD20)
- [x] Confirmed entity 0x84, player 0x74 offsets
- [x] Added debug display showing P74, P84, E, D values
- [ ] Determine why scale is wrong
- [ ] Fix entity-tile alignment

## ASM Verified Formulas
```cpp
// Entity position reading (line 12620-12621):
relX = entity+0x84 - player+0x74
relZ = entity+0x8C - player+0x7C

// Screen position (line 12678-12680):
screenX = relX * zoom * 0.01 + halfWidth + windowX
screenY = halfHeight + windowY - relZ * zoom * 0.01

// Tile rendering (line 12371-12378):
tileLeft = dx * zoom - arrowOffsetX + halfWidth
tileSize = zoom pixels
1 tile = 192 world units
```

## Key Insight
- Native entity formula uses `zoom * 0.01` = 1.6 for zoom=160
- Tile formula uses `zoom` directly = 160 pixels per tile
- 1 tile = 192 world units
- So entity should use `zoom / 192` = 0.83 for proper sync

## Next Steps
1. Test debug build to see P74 vs P84 vs E84 values
2. Confirm which coordinate space is correct
3. Apply correct scale (zoom/192 instead of zoom*0.01)
