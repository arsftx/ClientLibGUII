#pragma once
/**
 * @file CICUser.h
 * @brief CICUser class reverse engineering - Other player entities
 * 
 * REVERSE ENGINEERING SOURCE:
 * - Decompile output from SRO_Decompile_Output
 * - Zero-assumption methodology applied
 * 
 * CLASS INFORMATION:
 * ==================
 * RuntimeClass String: "CICUser"
 * RuntimeClass Function: sub_65BF30 at 0x65BF30
 * 
 * Class Size: 1716 bytes (from sub_65BF30: sub_898D80("CICUser", ..., 1716, 0))
 * VTable Address: off_94E9D8
 * Sub-VTable (offset 76): off_94E9D0
 * 
 * Constructor: sub_65BFD0 at 0x65BFD0
 * Parent Constructor: sub_64FD90 at 0x64FD90 (inherits from unknown class)
 * 
 * INHERITANCE CHAIN:
 * CICUser (1716 bytes) -> sub_64FD90 (VT: off_94E370) -> sub_665310 -> ...
 */

#ifndef CICUSER_H
#define CICUSER_H

#include <windows.h>

// =============================================================================
// CICUser Class Constants
// =============================================================================

/**
 * VTable address for CICUser class
 * Source: Constructor sub_65BFD0, line 2079: *this = off_94E9D8
 */
#define CICUSER_VTABLE                  0x94E9D8

/**
 * Sub-VTable address at offset +76
 * Source: Constructor sub_65BFD0, line 2080: *(this + 76) = &off_94E9D0
 */
#define CICUSER_SUBVTABLE               0x94E9D0

/**
 * Total class size in bytes
 * Source: sub_65BF30 RuntimeClass registration: 1716
 */
#define CICUSER_SIZE                    1716

// =============================================================================
// CICUser Member Offsets - CONFIRMED from Decompile
// =============================================================================

/**
 * @section CORE_OFFSETS
 * =====================
 */

/**
 * VTable pointer (inherited)
 * Type: void**
 * Source: Constructor line 2079
 */
#define CICUSER_OFF_VTABLE              0       // +0x00

/**
 * Sub-VTable pointer
 * Type: void**
 * Source: Constructor line 2080
 */
#define CICUSER_OFF_SUBVTABLE           76      // +0x4C

/**
 * Pointer to 0x104 byte sub-object
 * Type: void*
 * Source: Constructor line 2086: *(this + 144) = sub_6C11B0(operator new(0x104))
 */
#define CICUSER_OFF_SUBOBJECT_144       144     // +0x90

/**
 * @section WORLD_ID - CRITICAL FOR PARTY DETECTION
 * =================================================
 * 
 * CONFIRMED: Entity World ID is at offset +224 (0xE0)
 * 
 * Evidence from decompile:
 * 1. source_part_9.cpp line 22300: sub_41AB90(&v43, (char *)(v10 + 224), 4)
 *    - Written to network packet as 4-byte DWORD
 * 2. source_part_9.cpp line 20905: sub_41AB90(&v10, (char *)(v4 + 224), 4)
 *    - Same pattern - network packet usage
 * 3. sub_62A780 (PartyManager): compares party member node[9] with world ID param
 * 4. sub_65A5D0: Returns player[1279] (offset 5116) as world ID for local player
 * 
 * Usage: Compare entity+224 with party member node+36 to detect party members
 */
#define CICUSER_OFF_WORLD_ID            224     // +0xE0 - DWORD

/**
 * @section POSITION_DATA
 * ======================
 * These offsets are inherited from parent class (position data)
 */

/**
 * Unknown DWORD fields
 * Source: Constructor line 2107
 */
#define CICUSER_OFF_UNK_348             348     // +0x15C - float (1101004800 = 50.0f)

/**
 * Float fields (scale/size related?)
 * Source: Constructor lines 2099-2106, values 1065353216 = 1.0f
 */
#define CICUSER_OFF_FLOAT_368           368     // +0x170 - float (50.0f)
#define CICUSER_OFF_FLOAT_372           372     // +0x174 - float (50.0f)
#define CICUSER_OFF_FLOAT_376           376     // +0x178 - float (50.0f)
#define CICUSER_OFF_FLOAT_380           380     // +0x17C - DWORD (0)
#define CICUSER_OFF_FLOAT_384           384     // +0x180 - DWORD (1)
#define CICUSER_OFF_FLOAT_388           388     // +0x184 - float (1.0f)
#define CICUSER_OFF_FLOAT_392           392     // +0x188 - float (1.0f)
#define CICUSER_OFF_FLOAT_396           396     // +0x18C - float (1.0f)

/**
 * Unknown DWORD
 * Source: Constructor line 2089
 */
#define CICUSER_OFF_UNK_468             468     // +0x1D4 - DWORD (0)

/**
 * Unknown byte flag
 * Source: Line 2097
 */
#define CICUSER_OFF_FLAG_1010           1010    // +0x3F2 - BYTE (0)

/**
 * @section VECTORS (std::vector structures)
 * =========================================
 * Each vector is 12 bytes: start, end, capacity pointers
 */

/**
 * Vector 1: Unknown purpose
 * Source: Constructor lines 2055-2062
 * Structure: DWORD start, DWORD end, DWORD capacity
 */
#define CICUSER_OFF_VECTOR1_START       1656    // +0x678
#define CICUSER_OFF_VECTOR1_END         1660    // +0x67C
#define CICUSER_OFF_VECTOR1_CAP         1664    // +0x680

/**
 * Vector 2: Unknown purpose
 * Source: Constructor lines 2063-2070
 */
#define CICUSER_OFF_VECTOR2_START       1692    // +0x69C
#define CICUSER_OFF_VECTOR2_END         1696    // +0x6A0
#define CICUSER_OFF_VECTOR2_CAP         1700    // +0x6A4

/**
 * Vector 3: Unknown purpose
 * Source: Constructor lines 2071-2078
 */
#define CICUSER_OFF_VECTOR3_START       1704    // +0x6A8
#define CICUSER_OFF_VECTOR3_END         1708    // +0x6AC
#define CICUSER_OFF_VECTOR3_CAP         1712    // +0x6B0

/**
 * @section FLAGS AND STATE
 * ========================
 */

/**
 * Unknown DWORDs
 * Source: Constructor lines 2087-2088
 */
#define CICUSER_OFF_UNK_1644            1644    // +0x66C - DWORD (0)
#define CICUSER_OFF_UNK_1648            1648    // +0x670 - DWORD (0)

/**
 * Flag byte - initialized to 1
 * Source: Constructor line 2090
 */
#define CICUSER_OFF_FLAG_1668           1668    // +0x684 - BYTE (1)

/**
 * State flags byte - bits 0 and 1 checked
 * Source: sub_65C360 checks (this[1669] & 2), sub_65C370 checks (this[1669] & 1)
 */
#define CICUSER_OFF_STATE_FLAGS         1669    // +0x685 - BYTE (flags)

/**
 * Additional flag bytes
 * Source: Constructor lines 2091-2097, 2108-2109
 */
#define CICUSER_OFF_FLAG_1676           1676    // +0x68C - BYTE (0)
#define CICUSER_OFF_FLAG_1677           1677    // +0x68D - BYTE (0)
#define CICUSER_OFF_UNK_1680            1680    // +0x690 - DWORD (0)
#define CICUSER_OFF_FLAG_1684           1684    // +0x694 - BYTE (1)
#define CICUSER_OFF_FLAG_1685           1685    // +0x695 - BYTE (1)
#define CICUSER_OFF_FLAG_1686           1686    // +0x696 - BYTE (1)
#define CICUSER_OFF_FLAG_1687           1687    // +0x697 - BYTE (0)
#define CICUSER_OFF_UNK_1688            1688    // +0x698 - DWORD (0)

// =============================================================================
// CICUser Function Addresses
// =============================================================================

/**
 * RuntimeClass registration function
 * Calls: sub_898D80("CICUser", sub_65BF60, sub_65BFC0, &unk_A04130, 1716, 0)
 */
#define CICUSER_FUNC_RUNTIMECLASS       0x65BF30

/**
 * Factory function - allocates and constructs CICUser
 * Calls: sub_899010(1716) then sub_65BFD0
 */
#define CICUSER_FUNC_FACTORY            0x65BF60

/**
 * Constructor
 * Initializes all member variables, sets VTable
 */
#define CICUSER_FUNC_CONSTRUCTOR        0x65BFD0

/**
 * Destructor wrapper
 */
#define CICUSER_FUNC_DESTRUCTOR_WRAP    0x65C1B0

/**
 * Destructor implementation
 */
#define CICUSER_FUNC_DESTRUCTOR         0x65C1D0

// =============================================================================
// Helper Macros for Runtime Access
// =============================================================================

/**
 * Get World ID from CICUser entity pointer
 * @param entityPtr DWORD pointer to CICUser object
 * @return DWORD world ID value
 */
#define CICUSER_GET_WORLD_ID(entityPtr) \
    (*(DWORD*)((DWORD)(entityPtr) + CICUSER_OFF_WORLD_ID))

/**
 * Check if entity is a CICUser by VTable
 * @param entityPtr DWORD pointer to entity
 * @return true if VTable matches CICUser
 */
#define CICUSER_IS_CICUSER(entityPtr) \
    (*(DWORD*)(entityPtr) == CICUSER_VTABLE)

/**
 * Get state flags byte
 * @param entityPtr DWORD pointer to CICUser object
 * @return BYTE state flags
 */
#define CICUSER_GET_STATE_FLAGS(entityPtr) \
    (*(BYTE*)((DWORD)(entityPtr) + CICUSER_OFF_STATE_FLAGS))

// =============================================================================
// Native Function Type Definitions
// =============================================================================

/**
 * GetWorldID function signature (for local player)
 * Address: 0x65A5D0
 * Returns: player[1279] = offset 5116
 */
typedef int (__thiscall* fnCICUser_GetWorldID)(void* pThis);
#define CICUSER_NATIVE_GET_WORLD_ID     ((fnCICUser_GetWorldID)0x65A5D0)

#endif // CICUSER_H
