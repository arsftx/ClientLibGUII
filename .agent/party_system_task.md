# Party System Reverse Engineering

## Overview
Party member detection and display on custom minimap.

## Completed Tasks

- [x] **Party Manager Structure Analysis**
  - Global: `unk_A01510` (0xA01510)
  - PartyData: PartyManager+24 (sub_629510)
  - Member List: PartyManager+52 (linked list)
  - Self World ID: PartyData+24 (PartyManager+48)

- [x] **Party Member Node Structure**
  - node+0: Next pointer
  - node+4: Prev pointer  
  - node+8: Name (std::wstring)
  - node+20: Guild (std::wstring)
  - node+36: World ID (DWORD) - **KEY FOR MATCHING**
  - node+40: Level

- [x] **Native Functions Identified**
  - `sub_629510` (0x629510): Returns PartyData pointer
  - `sub_62A1E0` (0x62A1E0): Find member by World ID
  - `sub_62A6C0` (0x62A6C0): IsInParty check
  - `sub_62A6D0` (0x62A6D0): IsLeader check
  - `sub_5BF370` (0x5BF370): OnPartyInfo packet handler

- [x] **Entity World ID**
  - CICUser+224: World ID (used for packets and party matching)

- [x] **CustomMinimap Integration**
  - Created `IsEntityInParty()` function
  - Using native `sub_62A1E0` for reliable lookup

## Files Modified

1. **PartyManager.h** - New header with all offsets
2. **CustomMinimap.cpp** - Party detection implementation

## Pending

- [ ] Test and verify party member cyan markers
- [ ] Verify selfWorldID from PartyData+24
