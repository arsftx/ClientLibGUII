# Replace CIFLattice with CIFSlotWithHelp

## Problem
CIFLattice has vertex recalc timing issue causing 3-corner glitch during window move.
Glitch only appears during drag, fixes when window stops - timing issue we cannot fix with OnUpdate vertex recalc.

## Solution
Use CIFSlotWithHelp (individual slot windows) like native CIFInventory does. No vertex glitch since each slot is independent window.

---

## CIFSlotWithHelp Class (IDA Analysis)

| Property | Value |
|----------|-------|
| Name | CIFSlotWithHelp |
| RuntimeClass | `0x9FFD04` |
| Size | `0x4B8` bytes |
| Parent | CIFWnd (`0x9FE5C0`) |
| Constructor | `sub_53DA80` |

### Key Functions
- `sub_53DA80` - Constructor (needs reverse engineering)
- `sub_5425E0` - Slot index setter (called in inventory OnCreate)
- `sub_5425A0` - Some param (0x46 passed)
- `sub_542B40` - Enable/disable (1 = enabled)
- `sub_542B00` - Another setter
- `sub_542750` - Another setter

---

## Implementation Steps

### Phase 1: Reverse Engineer
1. Analyze `sub_53DA80` constructor member initialization
2. Identify texture loading mechanism
3. Create CIFSlotWithHelp.h / CIFSlotWithHelp.cpp

### Phase 2: Rewrite AutoHuntSettings
1. Remove CIFLattice and CIFStretchWnd outline code
2. Create 6x8 grid (48 slots) per skill panel using CIFSlotWithHelp
3. Position each slot at: x = col * 34, y = row * 34 (approximate)
4. Load appropriate slot textures

### Phase 3: Test
1. Verify no corner glitch during window move
2. Verify skill display works correctly
