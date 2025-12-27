#pragma once
/**
 * @file PartyManager.h
 * @brief Party Manager class reverse engineering
 * 
 * REVERSE ENGINEERING SOURCE:
 * ==========================
 * - Decompile output analysis from SRO_Decompile_Output
 * - Zero-assumption methodology applied
 * 
 * GLOBAL INSTANCE:
 * ================
 * Address: unk_A01510 (0xA01510)
 * Used by: sub_62A6C0, sub_62A6D0, sub_62A6F0, sub_62A780, sub_629510
 * 
 * KEY FUNCTION ANALYSIS:
 * ======================
 * sub_629510: return this + 24;  --> PartyData pointer
 * sub_62A6C0: return sub_629510(this)[13];  --> IsInParty (PartyData+52)
 * sub_62A6D0: return sub_629510(this)[12];  --> IsLeader (PartyData+48)
 * sub_62A6F0: checks v4[9] == worldID  --> Member node+36 = World ID
 * sub_62A780: v3[5] = worldID  --> PartyData+20 = selfWorldID storage
 * sub_65A5D0: return this[1279];  --> Player World ID at +5116
 */

#ifndef PARTYMANAGER_H
#define PARTYMANAGER_H

#include <windows.h>

// =============================================================================
// Party Manager Global Address
// =============================================================================

/**
 * Global Party Manager instance address
 * Source: unk_A01510 references throughout decompile
 */
#define PARTYMANAGER_GLOBAL_ADDR        0xA01510

// =============================================================================
// Party Manager Struct Offsets (from unk_A01510 base)
// =============================================================================

/**
 * PartyData struct pointer offset
 * Source: sub_629510 returns this+24
 * Usage: PartyManager+24 = PartyData pointer
 */
#define PARTYMGR_OFF_PARTYDATA          24      // +0x18

/**
 * Party member linked list head
 * Source: sub_62A6F0 line 2170: v3 = *(this + 52)
 * Usage: Double-linked list of party member nodes
 */
#define PARTYMGR_OFF_MEMBER_LIST        52      // +0x34

// =============================================================================
// PartyData Struct Offsets (relative to PartyData = PartyManager+24)
// =============================================================================

/**
 * Self World ID stored when becoming party leader
 * Source: sub_62A780 line 2250: v3[5] = a2 (a2 = worldID param)
 * Note: This is set when you become party leader, may be 0 otherwise
 * PREFER: Use Player+5116 for reliable self World ID
 */
#define PARTYDATA_OFF_SELF_WORLDID      20      // +0x14 (PartyData[5])

/**
 * Party type or mode flag
 * Source: Various sub_629510 result accesses
 */
#define PARTYDATA_OFF_TYPE_0            0       // +0x00 (PartyData[0])
#define PARTYDATA_OFF_TYPE_4            4       // +0x04 (PartyData[1])
#define PARTYDATA_OFF_TYPE_8            8       // +0x08 (PartyData[2])

/**
 * IsLeader flag (byte)
 * Source: sub_62A6D0: return sub_629510(this)[12]
 * Value: 1 = party leader, 0 = not leader
 */
#define PARTYDATA_OFF_IS_LEADER         48      // +0x30 (PartyData[12])

/**
 * IsInParty flag (byte)
 * Source: sub_62A6C0: return sub_629510(this)[13]
 * Value: 1 = in party, 0 = not in party
 */
#define PARTYDATA_OFF_IS_IN_PARTY       52      // +0x34 (PartyData[13])

// =============================================================================
// Party Member Node Offsets (Linked List Node Structure)
// =============================================================================

/**
 * Party Member Node Structure (76 bytes = 0x4C):
 * =============================================
 * Offset   Size   Description
 * ------   ----   -----------
 * +0       4      Next node pointer (linked list)
 * +4       4      Previous node pointer (doubly-linked)
 * +8       varies Name string (std::string, 12 bytes)
 * +20      varies Guild name string (std::string, 12 bytes)
 * +32      4      Unknown
 * +36      4      WORLD ID (node[9]) <-- CRITICAL FOR PARTY DETECTION
 * +40      4      Character level
 * +44      4      Current HP percentage (0-10 scale)
 * +48      4      Max HP indicator
 * +52      4      Character class/job
 * +56      4      Unknown flags
 * +60      4      Unknown
 * +64      4      X position (int16)
 * +68      4      Unknown
 * +72      4      Z position (int16)
 */

#define PMEMBER_OFF_NEXT                0       // +0x00 - Next node
#define PMEMBER_OFF_PREV                4       // +0x04 - Previous node
#define PMEMBER_OFF_NAME                8       // +0x08 - Name (std::string)
#define PMEMBER_OFF_GUILD               20      // +0x14 - Guild name (std::string)
#define PMEMBER_OFF_WORLD_ID            36      // +0x24 - World ID (DWORD) <-- CRITICAL
#define PMEMBER_OFF_LEVEL               40      // +0x28 - Level
#define PMEMBER_OFF_HP_CURRENT          44      // +0x2C - HP (0-10 scale)
#define PMEMBER_OFF_HP_MAX              48      // +0x30 - HP max indicator
#define PMEMBER_OFF_CLASS               52      // +0x34 - Character class
#define PMEMBER_OFF_POS_X               64      // +0x40 - X position
#define PMEMBER_OFF_POS_Z               72      // +0x48 - Z position
#define PMEMBER_NODE_SIZE               76      // 0x4C bytes total

// =============================================================================
// CICPlayer (Local Player) World ID Offset
// =============================================================================

/**
 * Local player World ID offset
 * Source: sub_65A5D0 returns this[1279] = offset 5116
 * Usage: *(DWORD*)(playerPtr + 5116) = self World ID
 */
#define PLAYER_OFF_WORLD_ID             5116    // +0x13FC (Player[1279])

// =============================================================================
// Entity (CICUser - Other Players) ID Offsets
// =============================================================================

/**
 * Other player entity World ID offset (for network packets)
 * Source: CICUser constructor, packet writes at +224
 * Usage: *(DWORD*)(entityPtr + 224) = entity World ID (for packets)
 * NOTE: This is NOT the ID used for party matching!
 */
#define CICUSER_OFF_WORLD_ID            224     // +0xE0 - World ID for packets

/**
 * Other player entity UNIQUE ID offset (for party matching)
 * Source: source_part_9.cpp line 22276: sub_62A6F0(*(_DWORD *)(v10 + 412))
 * Source: source_part_20.cpp line 6924: sub_401810(a2, this + 412, 4)
 * Usage: *(DWORD*)(entityPtr + 412) = entity unique ID (for party check)
 * CRITICAL: This is the ID that matches party member node+36!
 */
#define CICUSER_OFF_UNIQUE_ID           412     // +0x19C - Unique ID for party matching

// =============================================================================
// Helper Macros
// =============================================================================

/**
 * Get Party Manager global instance
 */
#define PARTYMANAGER_GET_INSTANCE() \
    ((DWORD)PARTYMANAGER_GLOBAL_ADDR)

/**
 * Get PartyData pointer from PartyManager
 */
#define PARTYMANAGER_GET_PARTYDATA() \
    (PARTYMANAGER_GLOBAL_ADDR + PARTYMGR_OFF_PARTYDATA)

/**
 * Check if player is in a party
 * Returns: BYTE (1 = in party, 0 = not)
 */
#define PARTYMANAGER_IS_IN_PARTY() \
    (*(BYTE*)(PARTYMANAGER_GLOBAL_ADDR + PARTYMGR_OFF_PARTYDATA + PARTYDATA_OFF_IS_IN_PARTY))

/**
 * Check if player is party leader
 * Returns: BYTE (1 = leader, 0 = not)
 */
#define PARTYMANAGER_IS_LEADER() \
    (*(BYTE*)(PARTYMANAGER_GLOBAL_ADDR + PARTYMGR_OFF_PARTYDATA + PARTYDATA_OFF_IS_LEADER))

/**
 * Get party member list head pointer
 * Returns: DWORD pointer to first node (or sentinel)
 */
#define PARTYMANAGER_GET_MEMBER_LIST() \
    (*(DWORD*)(PARTYMANAGER_GLOBAL_ADDR + PARTYMGR_OFF_MEMBER_LIST))

/**
 * Get local player World ID from player pointer
 */
#define PLAYER_GET_WORLD_ID(playerPtr) \
    (*(DWORD*)((DWORD)(playerPtr) + PLAYER_OFF_WORLD_ID))

/**
 * Get entity World ID from CICUser pointer
 */
#define ENTITY_GET_WORLD_ID(entityPtr) \
    (*(DWORD*)((DWORD)(entityPtr) + CICUSER_OFF_WORLD_ID))

/**
 * Get World ID from party member node
 */
#define PMEMBER_GET_WORLD_ID(nodePtr) \
    (*(DWORD*)((DWORD)(nodePtr) + PMEMBER_OFF_WORLD_ID))

// =============================================================================
// Native Function Addresses
// =============================================================================

#define PARTYMGR_FUNC_GET_PARTYDATA     0x629510    // sub_629510: returns this+24
#define PARTYMGR_FUNC_IS_IN_PARTY       0x62A6C0    // sub_62A6C0: checks PartyData[13]
#define PARTYMGR_FUNC_IS_LEADER         0x62A6D0    // sub_62A6D0: checks PartyData[12]
#define PARTYMGR_FUNC_CHECK_MEMBER      0x62A6F0    // sub_62A6F0: checks if ID in party
#define PARTYMGR_FUNC_SET_LEADER        0x62A780    // sub_62A780: sets leader World ID

#define PLAYER_FUNC_GET_WORLD_ID        0x65A5D0    // sub_65A5D0: returns this[1279]

#endif // PARTYMANAGER_H
