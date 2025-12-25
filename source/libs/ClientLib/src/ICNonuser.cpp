/**
 * @file ICNonuser.cpp
 * @brief CICNonuser class implementation - Base class for non-user characters
 * 
 * ============================================
 * ECSRO FIX - Memory addresses updated for ECSRO client
 * Date: 2024-12-11
 * Changes:
 *   - RuntimeClass: 0x00EF1AA4 -> 0x00A0434C
 *   - Class Size: 0x79C -> 0x658
 *   - m_classlink: 0x078C -> 0x0648
 * ============================================
 */
#include "ICNonuser.h"

GFX_IMPLEMENT_DYNAMIC_EXISTING(CICNonuser, 0x00A0434C)  // ECSRO RuntimeClass address

CLASSLINK_STATIC_IMPL(CICNonuser)
