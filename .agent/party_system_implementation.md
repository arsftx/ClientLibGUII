# Party System & Minimap Implementation

## Problem
1. Party members show as GREEN instead of CYAN on custom minimap
2. Cross-region entities (NPCs at region borders) not visible

---

## Root Cause Analysis

### Issue 1: Party Icon Detection
**Native CIFMinimap (sub_53AD20) approach:**
- Entity loop (12602-12717): Draws **ALL** players as OTHER_PLAYER using texture +760
- Party section (12773-12984): **SEPARATE loop** using PartyManager linked list
- **Bug**: Custom minimap tried to check party in entity loop (wrong approach)
- **Fix**: Added `DrawPartyMembers()` called after entity loop

### Issue 2: Cross-Region Visibility
**Entity position offsets:**
- `GetLocationRaw()` used: 0x74/0x7C (CIObject base) → **WRONG**
- Native CIFMinimap uses: **0x84/0x8C** (entity derived class)
- Positions at 0x84/0x8C are WORLD coordinates, no region adjustment needed
- **Fix**: Changed to use `ENTITY_OFFSET_POSEX (0x84)` and `ENTITY_OFFSET_POSEZ (0x8C)`

---

## Key Offsets Reference

| Item | Offset | Notes |
|------|--------|-------|
| Entity World X | +0x84 (132) | Native CIFMinimap uses this |
| Entity World Z | +0x8C (140) | Native CIFMinimap uses this |
| Entity Local X | +0x74 (116) | CIObject base - DON'T USE |
| Entity Local Z | +0x7C (124) | CIObject base - DON'T USE |
| Entity Unique ID | +412 (0x19C) | For party matching |
| Party Member ID | node+36 | PartyMemberNode |
| PartyManager | 0xA01510 | Global instance |

---

## Files Modified

1. **CustomMinimap.cpp**
   - `DrawEntityMarkers()`: Uses correct position offsets (0x84/0x8C)
   - `DrawPartyMembers()`: Draws party members separately as CYAN
   - Removed party check from entity loop

2. **PartyManager.h** - CIFMinimap render section documentation added

3. **CIFMinimap.h** - Full render architecture documented

---

## Native CIFMinimap Render Architecture

```
sub_53AD20 (CIFMinimap::Render)
├── 1. Entity Loop (12602-12717)
│   ├── Monster → +748/+752
│   ├── NPC → +776
│   ├── Item → +756
│   └── Player → +760 (GREEN - always!)
│
├── 2. Party Section (12773-12984)
│   ├── source: sub_629510(&unk_A01510)
│   ├── list: PartyData+28
│   └── icon: +692 (far) / +740 (close)
│
└── 3. Other Players (12986-13199)
    └── source: sub_4751F0
```

## Status: ✅ COMPLETE
