# Custom Minimap - Implementation Plan

## Goal
Implement custom ImGui minimap with quest NPC detection for gold markers.

---

## Phase 1: Core Minimap ✅
- Player arrow with rotation
- Entity detection (Monster, NPC, Player, Item, Pet)
- Zoom controls, coordinates display

## Phase 2: ASM Analysis ✅

### sub_53AD20 Full Analysis Complete

**Entity Loop (0x53B742 - 0x53BA5B):**
```
CICMonster → +748 (normal) / +752 (unique if 0x668==3)
CICNPC     → +776 (quest texture) - UNCONDITIONAL!
CICItem    → +756
CICPlayer  → +760
```

**Party Section (0x53BB5B+):**
- Uses +740 for far party members

**Pet Section (0x53C320+):**
- Uses +772 (near), +768 (far)

---

## Phase 3: Quest NPC Detection 🔄

### Problem
Native uses +776 for ALL NPCs - no quest condition in entity loop.
User screenshots show blue/gold NPC based on active quest.

### Solution
1. **Reverse engineer CIFQuest** (size: 732 bytes, VTable: 0x9401F0)
2. **Get active quest target NPC IDs** from quest slots
3. **Compare in entity loop:** If NPC ID matches quest target → Gold, else → Blue

### CIFQuest Key Info (from analysis)
- Constructor: `sub_473CB0` → `sub_473E10`
- Size: 732 bytes
- Global vars: `dword_9AE940` - `dword_9AE97C`
- Quest lists at offsets: 692, 704, 716

---

## Verification
- [ ] Compile and test
- [ ] Verify quest NPC shows gold when quest active
- [ ] Verify normal NPC shows blue when no quest
