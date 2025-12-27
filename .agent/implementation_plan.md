# CustomMinimap Fix - Implementation Plan

## Problems Solved

### 1. Party Member Icons ✅
Party members were showing as GREEN instead of CYAN.

**Solution**: Native CIFMinimap uses a separate render section for party members (lines 12773-12984), not within the entity loop. Implemented `DrawPartyMembers()` to match native behavior.

### 2. Cross-Region Entity Visibility ✅
Entities in adjacent regions (at region borders) were not visible.

**Solution**: Entity positions were being read from wrong offset:
- ❌ GetLocationRaw() uses 0x74/0x7C (region-local)
- ✅ Native uses 0x84/0x8C (world coordinates)

---

## Changes Made

### CustomMinimap.cpp

```cpp
// BEFORE (wrong):
D3DVECTOR entityLoc = GetLocationRaw(entityPtr);

// AFTER (correct):
float entityX = *(float*)(entityPtr + ENTITY_OFFSET_POSEX);  // 0x84
float entityZ = *(float*)(entityPtr + ENTITY_OFFSET_POSEZ);  // 0x8C
```

---

## Verification

1. Rebuild: `msbuild ClientLibGUI.sln /p:Configuration=Release /p:Platform=Win32 /t:DevKit_DLL`
2. Test in-game at region borders
3. Verify:
   - Cross-region NPCs visible on minimap
   - Party members show as CYAN
   - Other players show as GREEN

## Status: ✅ COMPLETE
