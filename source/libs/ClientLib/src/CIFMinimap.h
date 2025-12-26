#pragma once

#include "CIFWnd_ECSRO.h"

// CIFMinimap - Minimap window class
// Based on IDA analysis of constructor at 0x005397E0
// Size: 0x0370 (880 bytes)
// VTable: 0x0094AD48
// Inner VTable: 0x0094AD00 at offset +108

class CIFMinimap {
public:
    // === Base CIFWnd (692 bytes) ===
    char pad_CIFWnd[0x02B4];           // 0x0000 - CIFWnd base class
    
    // === CIFMinimap specific members ===
    char pad_02B4[0x28];               // 0x02B4 - Internal data (+692 to +732)
    
    // === Texture Pointers (DDJ loaded) ===
    void* m_pTexSignPartyArrow;        // 0x02B4 (+692) - mm_sign_partyarrow.ddj
    char pad_02B8[4];                  // 0x02B8
    void* m_pTexSignCharacter;         // 0x02BC (+700) - mm_sign_character.ddj
    char pad_02C0[0x28];               // 0x02C0
    void* m_pTexNPCSign;               // 0x02E4 (+740) - mm_sign_npc.ddj
    char pad_02E8[4];                  // 0x02E8
    void* m_pTexMonsterSign;           // 0x02EC (+748)
    void* m_pTexPlayerSign;            // 0x02F0 (+752)
    void* m_pTexPickedItemSign;        // 0x02F4 (+756)
    void* m_pTexTargetSign;            // 0x02F8 (+760)
    char pad_02FC[4];                  // 0x02FC
    void* m_pTexPartyMemberSign;       // 0x0300 (+768)
    void* m_pTexGuildSign;             // 0x0304 (+772)
    void* m_pTexQuestNPCSign;          // 0x0308 (+776)
    void* m_pTexWorldMapSign;          // 0x030C (+780)
    
    
    // === Linked List ===
    void* m_pMinimapList;              // 0x0310 (+784) - Linked list head for markers
    
    // === Player Position Cache (from sub_53A5A0) ===
    float m_fPlayerPosX;               // 0x0314 (+788) - Player X position within region
    float m_fPlayerPosY;               // 0x0318 (+792) - Player Y (height)
    float m_fPlayerPosZ;               // 0x031C (+796) - Player Z position within region
    float m_fPlayerRotation;           // 0x0320 (+800) - Player rotation/heading
    
    char pad_0324[0x1C];               // 0x0324 - padding to UI elements
    
    // === UI Element Pointers ===
    void* m_pBtnZoomIn;                // 0x0340 (+832) - Button ID 5
    void* m_pBtnZoomOut;               // 0x0344 (+836) - Button ID 6
    void* m_pBtnOptions;               // 0x0348 (+840) - Button ID 8
    void* m_pBtnMoveUp;                // 0x034C (+844) - Button ID 2 - X coord text
    void* m_pBtnMoveDown;              // 0x0350 (+848) - Button ID 3 - Y coord text
    void* m_pBtnMoveLeft;              // 0x0354 (+852) - Button ID 4 - Region text
    
    // === Map Coordinates ===
    int m_nCurrentRegionX;             // 0x0358 (+856) - Current region X (1-255)
    int m_nCurrentRegionY;             // 0x035C (+860) - Current region Y (1-255)
    int m_nPrevRegionX;                // 0x0360 (+864) - Previous region X (for change detection)
    int m_nPrevRegionY;                // 0x0364 (+868) - Previous region Y
    
    char pad_0368[8];                  // 0x0368 - remaining bytes to 880

public:
    // === Static functions ===
    static CIFMinimap* GetInstance();
    
    // === Native function wrappers ===
    
    // Constructor thunk
    static void* Create() {
        return reinterpret_cast<void*(*)()>(0x00539620)();
    }
    
    // OnCreate - loads textures and initializes UI
    bool OnCreate(int a2) {
        return reinterpret_cast<bool(__thiscall*)(CIFMinimap*, int)>(0x00539AA0)(this, a2);
    }
    
    // Update map display
    void UpdateMap(int forceRefresh) {
        reinterpret_cast<void(__thiscall*)(CIFMinimap*, int)>(0x0053A5A0)(this, forceRefresh);
    }
    
    // === Getters for region coordinates ===
    int GetCurrentRegionX() const { return m_nCurrentRegionX; }
    int GetCurrentRegionY() const { return m_nCurrentRegionY; }
    int GetPrevRegionX() const { return m_nPrevRegionX; }
    int GetPrevRegionY() const { return m_nPrevRegionY; }
    
    // === Getters for player position ===
    float GetPlayerPosX() const { return m_fPlayerPosX; }
    float GetPlayerPosZ() const { return m_fPlayerPosZ; }
    
    // === Calculate display coordinates (native formula from sub_53A5A0) ===
    // Formula: ((3 * region - offset) << 6) - (int)(pos * 10.0f)
    int CalculateDisplayX() const {
        return ((3 * m_nCurrentRegionX - 405) << 6) - (int)(m_fPlayerPosX * 10.0f);
    }
    int CalculateDisplayY() const {
        return ((3 * m_nCurrentRegionY - 276) << 6) - (int)(m_fPlayerPosZ * 10.0f);
    }
    
    // === Get Region Name (from TextStringManager using region ID) ===
    // Native minimap uses: sprintf(buffer, "%d", regionID); TSM->GetString(buffer);
    const char* GetRegionName() const;
    
    // === Get full RegionID (combined XY) ===
    int GetRegionID() const {
        return (m_nCurrentRegionY << 8) | m_nCurrentRegionX;
    }
    
    // === Show/Hide ===
    void ShowGWnd(bool bShow) {
        // Call parent CIFWnd ShowGWnd at vtable+88
        void** vtable = *(void***)this;
        reinterpret_cast<void(__thiscall*)(void*, bool)>(vtable[22])(this, bShow);
    }
    
    void MoveGWnd(int x, int y) {
        // CGWnd::MoveGWnd at offset 0 in vtable
        reinterpret_cast<void(__thiscall*)(void*, int, int)>(0x0089F230)(this, x, y);
    }

}; // Size: 0x0370 (880 bytes)
