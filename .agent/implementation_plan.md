# Party Member Icon Detection Fix

## Problem
Custom minimap shows OTHER_PLAYER (green triangle) instead of PARTY_MEMBER (cyan triangle) for party members.

## Root Cause Analysis (Decompile Evidence)

### Evidence 1: Entity Party Check Offset
**source_part_9.cpp:22276**
```cpp
|| (unsigned __int8)sub_62A6C0(&unk_A01510) && (unsigned __int8)sub_62A6F0(*(_DWORD *)(v10 + 412))
```
- `entity+412` is used for party member matching, **NOT** `entity+224`

### Evidence 2: Party Member Node Structure
**source_part_18.cpp:2174 (sub_62A6F0)**
```cpp
while ( v4[9] != a2 )  // node[9] = node+36 compared with parameter
```
- Party node offset +36 = Unique ID

### Evidence 3: Native CIFMinimap Approach
**source_part_12.cpp:12986-13024**
- Native minimap uses **dedicated party member list** from `sub_4751F0`
- Iterates party members directly, NOT entity list
- Uses `sub_619AC0` to get party member data by ID

### Evidence 4: Current Custom Minimap Bug
**CustomMinimap.cpp**
```cpp
DWORD entityUniqueID = *(DWORD*)(entityPtr + 412);  // Fixed to +412
```
- Previously used +224 which is **WRONG**
- Debug log showed: `entityWorldID=53470, selfWorldID=1` - incorrect values

---

## Proposed Changes

### [MODIFY] [CustomMinimap.cpp](file:///C:/Users/FuatAras/Desktop/Server/ClientLibGUI/source/DevKit_DLL/src/imgui_windows/CustomMinimap.cpp)

1. **Verify entity+412 fix is working** - Code already changed, needs rebuild
2. **Update OTHER_PLAYER debug log** at line 1006 to use +412 instead of +224

---

## Verification Plan

### Automated Tests
- None available for this module

### Manual Verification
1. **Rebuild the project**:
   ```
   msbuild ClientLibGUI.sln /p:Configuration=Release /p:Platform=Win32 /t:DevKit_DLL /v:m /m
   ```

2. **In-game test**:
   - Start client, join a party with another player
   - Open custom minimap
   - Check debug log for:
     ```
     IsEntityInParty: entityUniqueID=XXXX, selfUniqueID=YYYY, inParty=1, memberData=0x...
     PartyMember[0]: uniqueID=XXXX  ← should match entityUniqueID
     ```
   - Verify party member shows as CYAN triangle, not GREEN

3. **Expected debug output after fix**:
   ```
   IsEntityInParty: entityUniqueID=53470, selfUniqueID=12345, inParty=1, memberData=0x1234ABCD
   PARTY MATCH! entity=0x224ABD94, uniqueID=53470, memberData=0x1234ABCD
   ```

---

## Key Offsets Reference

| Item | Offset | Source |
|------|--------|--------|
| Entity Unique ID (for party) | +412 | source_part_9.cpp:22276 |
| Entity World ID (for packets) | +224 | CICUser constructor |
| Party node Unique ID | +36 | source_part_18.cpp:2174 |
| Party Manager global | 0xA01510 | decompile |
| Party member list head | PartyManager+52 | sub_62A6F0 |
