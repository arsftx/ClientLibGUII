#pragma once

#include <windows.h>
#include "CIFWnd_ECSRO.h"

// =============================================================================
// CIFMinimap - Minimap window class
// =============================================================================
// 
// REVERSE ENGINEERING EVIDENCE:
//   Registration: sub_539620 -> sub_898D80("CIFMinimap", ..., 880, 0)
//   Constructor:  sub_5397E0 (allocates via sub_899010(880))
//   Destructor:   sub_539940
//   OnCreate:     sub_539AA0 (texture loading, UI init)
//   UpdateMap:    sub_53A5A0 (coordinates, tiles)
//   Render:       sub_53AD20 (entity markers)
//
// VERIFIED VALUES:
//   Size:         0x0370 (880 bytes) - from sub_539620 registration
//   VTable:       0x0094AD48 - from sub_5397E0 line 11178
//   Inner VTable: 0x0094AD00 - from sub_5397E0 line 11179 (offset +108)
//   IRM ID:       GDR_MINIMAP = 10 (from ginterface.txt)
//
// UNVERIFIED (ASM REQUIRED):
//   +744, +764 - No direct write evidence in decompile or constructor ASM
// =============================================================================

class CIFMinimap {
public:
    // === Base CIFWnd (692 bytes) ===
    char pad_CIFWnd[0x02B4];               // 0x0000 - CIFWnd base class (+0 to +691)
    
    // === Texture Pointers (from OnCreate sub_539AA0) ===
    void* m_pTexPartyArrow;                // 0x02B4 (+692) - mm_sign_partyarrow.ddj
    void* m_pMapBackgroundTex;             // 0x02B8 (+696) - From Button ID 1, map background texture ref
    void* m_pTexCharacter;                 // 0x02BC (+700) - mm_sign_character.ddj
    
    // === Map Tile Texture Array (3x3 grid) ===
    void* m_pMapTiles[9];                  // 0x02C0 (+704) to 0x02E0 (+736) - 3x3 map tile textures
    
    // === Entity Marker Textures (verified from sub_53AD20 render function) ===
    //
    // RENDER ARCHITECTURE (sub_53AD20):
    // ----------------------------------
    // 1. ENTITY LOOP (12602-12717): Uses dword_9C99A4 linked list
    //    - sub_898FD0(&unk_A04320) = Monster → +748 or +752 (if type==3)
    //    - sub_898FD0(&unk_A01DD8) = NPC → +776
    //    - sub_898FD0(&unk_A0436C) = Item → +756
    //    - sub_898FD0(&unk_A04490) = Player → +760 (OTHER_PLAYER!)
    //    NOTE: Entity loop does NOT check party membership!
    //
    // 2. PARTY SECTION (12773-12984): Separate pass via PartyManager
    //    - Uses sub_629510(&unk_A01510) → PartyData
    //    - Iterates PartyData+28 linked list
    //    - FAR (distance >= flt_94AE20): +692 party arrow
    //    - CLOSE (distance < threshold): +740 NPC square
    //
    // 3. OTHER PLAYERS SECTION (12986-13199): Via sub_4751F0
    //    - FAR: +768 party marker
    //    - CLOSE: +772 animal marker
    //
    void* m_pTexNPC;                       // 0x02E4 (+740) - mm_sign_npc.ddj (BLUE) [sub_539AA0:11411]
    char pad_02E8[4];                      // 0x02E8 (+744) - ⚠️ UNVERIFIED (no decompile evidence)
    void* m_pTexMonster;                   // 0x02EC (+748) - mm_sign_monster.ddj - normal monsters
    void* m_pTexUnique;                    // 0x02F0 (+752) - mm_sign_unique.ddj - unique/elite monsters (type==3)
    void* m_pTexItem;                      // 0x02F4 (+756) - mm_sign_item.ddj - CICPickedItem
    void* m_pTexOtherPlayer;               // 0x02F8 (+760) - mm_sign_otherplayer.ddj - CICPlayer
    char pad_02FC[4];                      // 0x02FC (+764) - ⚠️ UNVERIFIED (no decompile evidence)
    void* m_pTexParty;                     // 0x0300 (+768) - mm_sign_party.ddj - party members / far pets
    void* m_pTexAnimal;                    // 0x0304 (+772) - mm_sign_animal.ddj - near pets (CICCos)
    void* m_pTexQuestNPC;                  // 0x0308 (+776) - mm_sign_questnpc.ddj (GOLD) - ALL NPCs in entity loop!
    void* m_pTexQuestArrow;                // 0x030C (+780) - mm_sign_questarrow.ddj - quest target arrow
    
    // === Map Tile Linked List ===
    void* m_pMinimapTileList;              // 0x0310 (+784) - Linked list head for map tiles
    
    // === Player Position Cache (from sub_53A5A0) ===
    float m_fPlayerPosX;                   // 0x0314 (+788) - Player X position within region
    float m_fPlayerPosY;                   // 0x0318 (+792) - Player Y (height)
    float m_fPlayerPosZ;                   // 0x031C (+796) - Player Z position within region
    float m_fPlayerRotation;               // 0x0320 (+800) - Player rotation/heading (radians) [ASM: ebx=0]
    
    // === Unknown Fields (ASM VERIFIED: initialized to -1) ===
    int m_nUnknown_804;                    // 0x0324 (+804) - [ASM: mov [esi+324h], -1]
    int m_nUnknown_808;                    // 0x0328 (+808) - [ASM: mov [esi+328h], -1]
    char pad_032C[4];                      // 0x032C (+812) - padding (4 bytes to +816)
    
    // === Zoom (ASM VERIFIED: default = 0x43200000 = 160.0f) ===
    float m_fZoomFactor;                   // 0x0330 (+816) - Minimap zoom scale [ASM: mov [esi+330h], 43200000h]
    float m_fZoomTarget;                   // 0x0334 (+820) - Target zoom [ASM: mov [esi+334h], 43200000h]
    
    // === Arrow Screen Position (from sub_53A5A0) ===
    // Formula: arrowX = posX * zoom * 0.01f
    //          arrowY = (192.0f - posZ) * zoom * 0.01f
    float m_fArrowScreenX;                 // 0x0338 (+824) - Calculated arrow X screen position
    float m_fArrowScreenY;                 // 0x033C (+828) - Calculated arrow Y screen position
    
    // === UI Element Pointers (from OnCreate) ===
    void* m_pBtnZoomIn;                    // 0x0340 (+832) - Button ID 5
    void* m_pBtnZoomOut;                   // 0x0344 (+836) - Button ID 6
    void* m_pBtnOptions;                   // 0x0348 (+840) - Button ID 8
    void* m_pTextRegion;                   // 0x034C (+844) - Button ID 2 (region name display)
    void* m_pTextX;                        // 0x0350 (+848) - Button ID 3 (X coord display)
    void* m_pTextY;                        // 0x0354 (+852) - Button ID 4 (Y coord display)
    
    // === Region Coordinates ===
    int m_nCurrentRegionX;                 // 0x0358 (+856) - Current region X (0-255)
    int m_nCurrentRegionY;                 // 0x035C (+860) - Current region Y (0-255)
    int m_nPrevRegionX;                    // 0x0360 (+864) - Previous region X
    int m_nPrevRegionY;                    // 0x0364 (+868) - Previous region Y
    
    // === Flags & State (ASM VERIFIED) ===
    BYTE m_bDungeonFlag;                   // 0x0368 (+872) - [ASM: mov [esi+368h], bl (=0)]
    char pad_0369;                         // 0x0369 (+873) - ✅ VERIFIED: alignment (not touched in ctor)
    WORD m_wPrevRegionID;                  // 0x036A (+874) - [ASM: mov [esi+36Ah], bx (=0)]
    void* m_pPrevZoneInfo;                 // 0x036C (+876) - [ASM: mov [esi+36Ch], ebx (=0)]
    
    // Total: 0x0370 (880 bytes)

public:
    // === Static functions ===
    static CIFMinimap* GetInstance();
    
    // === Native function wrappers ===
    
    // RuntimeClass registration thunk
    static void* Create() {
        return reinterpret_cast<void*(*)()>(0x00539620)();
    }
    
    // OnCreate - loads textures and initializes UI
    bool OnCreate(int a2) {
        return reinterpret_cast<bool(__thiscall*)(CIFMinimap*, int)>(0x00539AA0)(this, a2);
    }
    
    // Update map display (coordinates, tiles, entities)
    void UpdateMap(int forceRefresh) {
        reinterpret_cast<void(__thiscall*)(CIFMinimap*, int)>(0x0053A5A0)(this, forceRefresh);
    }
    
    // Render minimap (draws entities, player marker, etc)
    void Render() {
        reinterpret_cast<int(__thiscall*)(CIFMinimap*)>(0x0053AD20)(this);
    }
    
    // === Getters for region coordinates ===
    int GetCurrentRegionX() const { return m_nCurrentRegionX; }
    int GetCurrentRegionY() const { return m_nCurrentRegionY; }
    int GetPrevRegionX() const { return m_nPrevRegionX; }
    int GetPrevRegionY() const { return m_nPrevRegionY; }
    
    // === Getters for player position ===
    float GetPlayerPosX() const { return m_fPlayerPosX; }
    float GetPlayerPosZ() const { return m_fPlayerPosZ; }
    float GetPlayerRotation() const { return m_fPlayerRotation; }
    
    // === Getters/Setters for zoom ===
    float GetZoomFactor() const { return m_fZoomFactor; }
    void SetZoomFactor(float zoom) { m_fZoomFactor = zoom; }
    float GetZoomTarget() const { return m_fZoomTarget; }
    void SetZoomTarget(float target) { m_fZoomTarget = target; }
    
    // === Getters for arrow position ===
    float GetArrowScreenX() const { return m_fArrowScreenX; }
    float GetArrowScreenY() const { return m_fArrowScreenY; }
    
    // === Is dungeon map check ===
    bool IsDungeonMap() const { return m_bDungeonFlag != 0; }
    
    // === Calculate display coordinates (native formula from sub_53A5A0) ===
    // Formula: ((3 * region - offset) << 6) - (int)(pos * 10.0f)
    int CalculateDisplayX() const {
        return ((3 * m_nCurrentRegionX - 405) << 6) - (int)(m_fPlayerPosX * 10.0f);
    }
    int CalculateDisplayY() const {
        return ((3 * m_nCurrentRegionY - 276) << 6) - (int)(m_fPlayerPosZ * 10.0f);
    }
    
    // === Get Region Name (from TextStringManager using region ID) ===
    const char* GetRegionName() const;
    
    // === Get full RegionID (combined XY) ===
    int GetRegionID() const {
        return (m_nCurrentRegionY << 8) | m_nCurrentRegionX;
    }
    
    // === Show/Hide ===
    void ShowGWnd(bool bShow) {
        void** vtable = *(void***)this;
        reinterpret_cast<void(__thiscall*)(void*, bool)>(vtable[22])(this, bShow);
    }
    
    void MoveGWnd(int x, int y) {
        reinterpret_cast<void(__thiscall*)(void*, int, int)>(0x0089F230)(this, x, y);
    }
    
    // === Texture getters ===
    void* GetTexNPC() const { return m_pTexNPC; }
    void* GetTexMonster() const { return m_pTexMonster; }
    void* GetTexPlayer() const { return m_pTexOtherPlayer; }
    void* GetTexPartyMember() const { return m_pTexParty; }
    void* GetTexQuestNPC() const { return m_pTexQuestNPC; }
    void* GetTexCharacter() const { return m_pTexCharacter; }

}; // Size: 0x0370 (880 bytes)
