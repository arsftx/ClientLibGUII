# Party System Deep Reverse Engineering

## Problem
Custom minimap'te party member iconları ve cross-region entity visibility düzgün çalışmıyordu.

## Completed Tasks

### Phase 1: Party State Storage ✅
- [x] Party Manager: `unk_A01510` (0xA01510)
- [x] Party member list: PartyManager+52 (linked list)
- [x] Party node Unique ID: node+36
- [x] Entity Unique ID: **entity+412** (NOT +224!)

### Phase 2: Party Functions ✅
- [x] `sub_62A1E0`: FindPartyMemberByID
- [x] `sub_62A6C0`: IsInParty check
- [x] `sub_62A6F0`: CheckMemberByID

### Phase 3: CIFMinimap Render Architecture ✅
- [x] Native uses **separate party section** (lines 12773-12984)
- [x] Entity loop NEVER checks party membership
- [x] Entity position: **entity+0x84/0x8C** (NOT 0x74/0x7C!)
- [x] Player position: player+0x74/0x7C

### Phase 4: Cross-Region Fix ✅
- [x] Entity offsets 0x84/0x8C are WORLD coordinates
- [x] No region adjustment needed
- [x] Fixed `DrawEntityMarkers()` to use correct offsets

## Status: ✅ COMPLETE
- Party members display as CYAN
- Cross-region entities now visible
- All header files updated
