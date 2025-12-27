# Party System Implementation Plan

## Problem Statement
Party members appear as "OTHER_PLAYER" (red triangles) instead of party member icons (cyan triangles) on the custom minimap.

---

## Reverse Engineering Findings

### Global Addresses (VSRO)

| Address | Name | Description |
|---------|------|-------------|
| `0xA01510` | `unk_A01510` | Party Manager global instance |
| `0xA0465C` | `dword_A0465C` | Local player pointer |

### Party Manager Structure

```
PartyManager (at 0xA01510):
├── +0     VTable
├── +24    PartyData (sub_629510 returns this+24)
│   ├── +0   Type/flags
│   ├── +20  Leader World ID (only set for leaders)
│   ├── +24  Self World ID ← USE THIS!
│   ├── +48  IsLeader flag
│   └── +52  IsInParty flag
└── +52    Member List Head (linked list sentinel)
```

### Party Member Node Structure (76 bytes)

```
PartyMemberNode:
├── +0     Next pointer
├── +4     Prev pointer
├── +8     Name (std::wstring, 12 bytes)
├── +20    Guild (std::wstring, 12 bytes)
├── +32    Unknown
├── +36    World ID (DWORD) ← KEY FOR MATCHING
├── +40    Level
├── +44    HP Current
├── +48    HP Max
├── +52    Class
├── +56    Unknown
├── +60    Unknown
├── +64    Position X
├── +68    Unknown
└── +72    Position Z
```

### Key Discovery: Entity World ID

Entity offset `+224` contains the World ID that matches party member node `+36`.

**Evidence from decompile:**
- `sub_5BF370` line 25732: `sub_62A1E0(&unk_A01510, v36)` uses World ID
- `sub_62A1E0` line 1813: `if (v4[9] == a2)` → node+36 == worldID param

---

## Native Functions

### sub_62A1E0 - Find Party Member by World ID
```cpp
// Address: 0x62A1E0
// Prototype: _DWORD* __thiscall sub_62A1E0(_DWORD* partyManager, int worldID)
// Returns: Pointer to member data (node+8) if found, NULL otherwise

// Implementation:
// 1. Gets member list head from this[13] (PartyManager+52)
// 2. Iterates linked list, comparing node[9] (node+36) with worldID
// 3. Returns node+2 (node+8 = name) if match found
```

### sub_62A6C0 - Is In Party Check
```cpp
// Address: 0x62A6C0
// Returns: BYTE (1 = in party, 0 = not)
// Checks: sub_629510(this)[13] = PartyData+52
```

### sub_5BF370 - OnPartyInfo Packet Handler
```cpp
// Address: 0x5BF370
// Key line 25318: if (v10 == *(sub_629510(&unk_A01510) + 24))
// This confirms PartyData+24 = Self World ID
```

---

## Implementation

### CustomMinimap.cpp Changes

```cpp
// Native function typedef
typedef void* (__thiscall *fnFindPartyMemberByWorldID)(void* partyManager, int worldID);
static fnFindPartyMemberByWorldID g_pfnFindPartyMemberByWorldID = 
    (fnFindPartyMemberByWorldID)0x0062A1E0;

// IsEntityInParty function:
// 1. Get entity World ID from entity+224
// 2. Get self World ID from PartyManager+48 (PartyData+24)
// 3. Skip if entity is ourselves
// 4. Call native sub_62A1E0 to check if in party
// 5. Return true if memberData != NULL
```

### PartyManager.h (New File)

Created header with all confirmed offsets:
- `PARTYMANAGER_GLOBAL_ADDR = 0xA01510`
- `PARTYMGR_OFF_PARTYDATA = 24`
- `PARTYMGR_OFF_MEMBER_LIST = 52`
- `PARTYDATA_OFF_SELF_WORLDID = 24` (from PartyData base)
- `PMEMBER_OFF_WORLD_ID = 36`
- `CICUSER_OFF_WORLD_ID = 224`

---

## Verification Plan

### Expected Debug Output
```
IsEntityInParty: entity=0x..., entityWorldID=53470, selfWorldID=12345, memberData=0x...
PARTY MATCH! entity=0x..., worldID=53470, memberData=0x...
```

### Visual Check
- Party members should appear as cyan triangles
- Other players should appear as red triangles
- Self should not trigger party check

---

## Source References

| Function | File | Line | Description |
|----------|------|------|-------------|
| sub_5BF370 | source_part_15.cpp | 25574 | OnPartyInfo handler |
| sub_62A1E0 | source_part_18.cpp | 1795 | Find member by World ID |
| sub_629510 | source_part_18.cpp | 621 | Get PartyData pointer |
| sub_62A6C0 | source_part_18.cpp | 2103 | IsInParty check |
