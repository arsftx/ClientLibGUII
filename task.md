# Custom Minimap Implementation

## Current Status: 🔄 Quest NPC Detection In Progress

---

## Completed ✅

### Entity Markers
- [x] Monsters (red) - `CICMonster` (+748 texture)
- [x] Unique Monsters (orange glow) - offset 0x668 == 3 (+752 texture)
- [x] NPCs (blue) - `CICNPC`
- [x] Players (green) - `CICPlayer` (+760 texture)
- [x] Items (yellow) - `CICPickedItem` (+756 texture)
- [x] Pets (orange) - `CICCos` (+772 texture)

### Features
- [x] Player arrow centered with rotation (PI - rotation)
- [x] Zoom +/- buttons
- [x] Loading screen check
- [x] Coordinates display

---

## In Progress 🔄

### Quest NPC Detection (Gold Marker) ✅ COMPLETE
- [x] **ASM Analysis Complete** (sub_53AD20 fully analyzed)
- [x] Found sub_605040 - NPC quest check function
- [x] Implemented `IsNPCQuestTarget()` using native sub_605040
- [x] Gold color for quest NPCs, blue for normal NPCs

### ASM Analysis Findings (sub_53AD20)

| Offset | Hex | Texture File | Usage |
|--------|-----|--------------|-------|
| 740 | 0x2E4 | mm_sign_npc.ddj | Party far (BLUE) |
| 748 | 0x2EC | mm_sign_monster.ddj | Normal monster |
| 752 | 0x2F0 | mm_sign_unique.ddj | Unique monster |
| 756 | 0x2F4 | mm_sign_item.ddj | Items |
| 760 | 0x2F8 | mm_sign_otherplayer.ddj | Players |
| 768 | 0x300 | mm_sign_party.ddj | Party/far pets |
| 772 | 0x304 | mm_sign_animal.ddj | Near pets |
| **776** | **0x308** | **mm_sign_questnpc.ddj** | **ALL NPCs - unconditional!** |
| 780 | 0x30C | mm_sign_questarrow.ddj | Quest arrow |

**KEY FINDING:** Native entity loop uses +776 (gold) for ALL NPCs unconditionally.
Quest NPC detection requires CIFQuest integration.

---

## Next: CIFQuest Integration
1. Find active quest target NPC IDs from CIFQuest
2. Compare NPC IDs in entity loop
3. Quest match → Gold, Otherwise → Blue
