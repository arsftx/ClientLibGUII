# CIFMinimap Custom Minimap Task

## Status: ✅ Entity Markers Complete

---

## Completed ✅

### CIFMinimap Header (880 bytes)
- All offsets documented (+692-780 textures, +788-800 position, +816-820 zoom, +824-828 arrow, +856-868 region)

### Entity Markers
- [x] Monsters - red circles
- [x] Unique Monsters - orange with glow  
- [x] NPCs - blue squares
- [x] Players - **bright green triangles**
- [x] Items - yellow diamonds
- [x] Pets/COS - **orange circles**

### Features
- [x] Player arrow always centered
- [x] Arrow rotation (PI - rotation formula)
- [x] Zoom buttons (+/-)
- [x] Loading screen check
- [x] Coordinates display

---

## Entity Detection (from sub_53AD20)
```
unk_A04320 = CICMonster  → tex +748/+752
unk_A01DD8 = CICNPC      → tex +776
unk_A0436C = CICPickedItem → tex +756
unk_A04490 = CICPlayer   → tex +760
CICCOS                   → tex +768 (pets)
```
