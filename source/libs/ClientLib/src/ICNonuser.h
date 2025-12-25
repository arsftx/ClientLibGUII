/**
 * @file ICNonuser.h
 * @brief CICNonuser class - Base class for non-user characters (NPCs, Monsters, Pets)
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
#pragma once

#include "ICharactor.h"


class CICNonuser : public CICharactor {
    GFX_DECLARE_DYNAMIC_EXISTING(CICNonuser, 0x00A0434C)  // ECSRO RuntimeClass address


private:
    CClassLink<CICNonuser> m_classlink; //0x0648 (ECSRO) - was 0x078C (VSRO)

private:
   /* BEGIN_FIXTURE()
        ENSURE_SIZE(0x658)  // ECSRO size - was 0x79C (VSRO)
    END_FIXTURE()
    RUN_FIXTURE(CICNonuser)*/
}; //Size: 0x0658 (ECSRO) - was 0x079C (VSRO)
