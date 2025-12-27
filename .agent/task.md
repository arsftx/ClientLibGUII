# Party System Deep Reverse Engineering

## Problem
Custom minimap'te OTHER_PLAYER iconu party'ye girince PARTY_MEMBER iconuna dönüşmüyor.

## Completed Tasks

### Phase 1: Party State Storage ✅
- [x] Party Manager: `unk_A01510` (0xA01510)
- [x] Party member list: PartyManager+52 (linked list)
- [x] Party node Unique ID: node+36
- [x] Entity Unique ID: **entity+412** (NOT +224!)

### Phase 2: Party Functions ✅
- [x] `sub_62A1E0`: FindPartyMemberByID (uses this[13]=+52, compares node[9]=+36)
- [x] `sub_62A6C0`: IsInParty check
- [x] `sub_62A6F0`: CheckMemberByID (source_part_9.cpp:22276 confirms +412)

### Phase 3: CIFMinimap ✅
- [x] Native uses **separate party list** via `sub_4751F0`, NOT entity iteration
- [x] Party member icon at CIFMinimap+768

### Phase 4: Root Cause ✅
- [x] **Bug**: Custom minimap used entity+224 instead of entity+412
- [x] **Fix**: Changed `IsEntityInParty` to use entity+412

## Pending
- [ ] Rebuild project and test in-game
- [ ] Verify party member cyan marker appears
