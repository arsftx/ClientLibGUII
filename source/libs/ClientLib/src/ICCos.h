/**
 * @file ICCos.h
 * @brief CICCos class - Pet/Costume character class
 * 
 * ============================================
 * ECSRO FIX - Memory addresses updated for ECSRO client
 * Date: 2024-12-11
 * Changes:
 *   - RuntimeClass: 0x00EEF6F0 -> 0x00A01DD8
 *   - Class Size: 0x838 -> 0x6D4
 *   - m_classlink: 0x079C -> 0x0658
 *   - m_ownername: 0x07B4 -> 0x0670
 *   - fonttexture_ownername: 0x07D0 -> 0x067C
 * ============================================
 */
#pragma once

#include "ICNonuser.h"


class CICCos : public CICNonuser {
    GFX_DECLARE_DYNAMIC_EXISTING(CICCos, 0x00A01DD8)  // ECSRO RuntimeClass address

public:
    // Test function to verify ECSRO offsets
    void LogDebugInfo();
    
    // Auto-trigger hooks for spawn/despawn logging
    static void OnPetSpawn(CICCos* pPet);
    static void OnPetDespawn(CICCos* pPet);
    
    // Scan all CICCos instances and log their info (call periodically from EndScene)
    static void LogAllPets();
    
    // Manual test function - call with a CICCos pointer address
    static void TestPetAtAddress(DWORD address);
    
    // Install hook to automatically detect pet spawns (call once at startup)
    static void InstallSpawnHook();
    
    // Queue pet for delayed logging (3 seconds after spawn)
    static void QueuePetForDelayedLog(DWORD address);
    
private:
    CClassLink<CICCos> m_classlink; //0x0658 (ECSRO) - was 0x079C (VSRO)
    char pad_0668[8]; //0x0668 (ECSRO) - was 0x07AC (VSRO)
    std::n_wstring m_ownername; //0x0670 (ECSRO) - was 0x07B4 (VSRO)
    CGFontTexture fonttexture_ownername; //0x067C (ECSRO) - was 0x07D0 (VSRO)
private:
    /*BEGIN_FIXTURE()
        ENSURE_SIZE(0x6D4)  // ECSRO size - was 0x838 (VSRO)
        ENSURE_OFFSET(m_ownername, 0x670)
        ENSURE_OFFSET(fonttexture_ownername, 0x67C)
        END_FIXTURE()
        RUN_FIXTURE(CICCos)*/
};