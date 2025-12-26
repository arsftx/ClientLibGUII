#include "CustomMinimap.h"
#include "CustomGUISession.h"
#include <TextStringManager.h>  // For region name lookup
#include <GFX3DFunction/GFXVideo3d.h>
#include <stdio.h>

// Debug log function - uses OutputDebugStringA for safe DLL initialization
// (fopen/fprintf can crash during DLL_PROCESS_ATTACH)
static void DebugLog(const char* msg) {
    OutputDebugStringA("[CustomMinimap] ");
    OutputDebugStringA(msg);
    OutputDebugStringA("\n");
}

// Global instance
static CustomMinimap* g_pCustomMinimap = NULL;

// Render callback for CustomGUISession
static void Minimap_RenderCallback() {
    if (g_pCustomMinimap) {
        g_pCustomMinimap->Render();
    }
}

// Initialize function - call from DllMain or elsewhere
void InitializeCustomMinimap() {
    DebugLog("InitializeCustomMinimap: START");
    if (!g_pCustomMinimap) {
        DebugLog("InitializeCustomMinimap: Creating CustomMinimap");
        g_pCustomMinimap = new CustomMinimap();
        DebugLog("InitializeCustomMinimap: CustomMinimap created");
        
        // Don't access g_CD3DApplication here - it may not be ready yet
        // Device will be obtained lazily in Render() when ImGui is ready
        DebugLog("InitializeCustomMinimap: Skipping device init (lazy)");
        
        DebugLog("InitializeCustomMinimap: Registering callback");
        g_CustomGUI.RegisterRenderCallback(Minimap_RenderCallback);
        DebugLog("InitializeCustomMinimap: Callback registered");
    }
    DebugLog("InitializeCustomMinimap: END");
}

// Player pointer address
static const DWORD ADDR_PLAYER_PTR = 0x00A0465C;

// Scale factor for coordinate calculation (from IDA: flt_94AE04)
static const float COORD_SCALE = 10.0f;

// Entity Linked List (from IDA: dword_9C99A4)
static const DWORD ADDR_ENTITY_LIST_HEAD = 0x009C99A4;

// RuntimeClass pointers for entity type checking (from IDA analysis)
static const DWORD RUNTIMECLASS_MONSTER = 0x00A04320;  // CICMonster
static const DWORD RUNTIMECLASS_NPC     = 0x00A01DD8;  // CICNPC
static const DWORD RUNTIMECLASS_ITEM    = 0x00A0436C;  // CICPickedItem
static const DWORD RUNTIMECLASS_PLAYER  = 0x00A04490;  // CICPlayer

// Entity offsets (from IDA sub_53AD20)
static const DWORD ENTITY_OFFSET_STATE      = 0x1E6;   // Dead state = 4
static const DWORD ENTITY_OFFSET_POSEX      = 0x84;    // X position (float)
static const DWORD ENTITY_OFFSET_POSEZ      = 0x8C;    // Z position (float)
static const DWORD ENTITY_OFFSET_MONSTERTYPE = 0x668;  // Monster type (3 = Unique)
static const DWORD ENTITY_LINKED_OFFSET     = 436;     // v31 = v30 - 436

// Get minimap data directly from player pointer (like native minimap does)
// Native code reads from dword_A0465C (player ptr) + offsets
// Fixed: Use GLOBAL coordinate formula matching native minimap display
static bool GetMinimapData(int& regionX, int& regionY, int& displayX, int& displayY, const char*& regionName) {
    __try {
        // Read player pointer
        DWORD playerPtr = *(DWORD*)ADDR_PLAYER_PTR;
        if (playerPtr == 0) return false;
        
        // Read RegionID from player + 0x70 (112 decimal)
        // WORD format: low byte = X, high byte = Y
        WORD regionID = *(WORD*)(playerPtr + 0x70);
        regionX = (unsigned char)(regionID & 0xFF);        // Low byte
        regionY = (unsigned char)((regionID >> 8) & 0xFF); // High byte
        
        // Read position from player + 0x74, 0x78, 0x7C
        float posX = *(float*)(playerPtr + 0x74);
        // float posY = *(float*)(playerPtr + 0x78);  // Height, not used
        float posZ = *(float*)(playerPtr + 0x7C);
        
        // Calculate GLOBAL display coordinates (matches native minimap X:6290 Y:499)
        // SRO global coordinate formula:
        // GlobalX = (RegionX - 135) * 192 + (localPosX / 10)
        // GlobalY = (RegionY - 92) * 192 + (localPosZ / 10)
        // Region 135,92 is the center reference point
        displayX = (regionX - 135) * 192 + (int)(posX / COORD_SCALE);
        displayY = (regionY - 92) * 192 + (int)(posZ / COORD_SCALE);
        
        // Get region name from TextStringManager
        // Native: sprintf("%d", regionID); TSM->GetString(buffer);
        static char regionIdBuffer[16];
        sprintf(regionIdBuffer, "%d", (int)regionID);
        regionName = g_CTextStringManager->GetString3(regionIdBuffer);
        
        return true;
    }
    __except (1) { return false; }
}

CustomMinimap::CustomMinimap() {
    m_bVisible = true;
    m_bInitialized = false;
    m_nRegionX = 0;
    m_nRegionY = 0;
    m_nDisplayX = 0;
    m_nDisplayY = 0;
    m_pRegionName = NULL;
    m_fMinimapSize = 192.0f;
    m_vMinimapPos = ImVec2(10.0f, 240.0f);  // Below PlayerMiniInfo
    m_pDevice = NULL;
    m_pTexCharacter = NULL;
    m_pTexPartyArrow = NULL;
    m_pTexNPC = NULL;
    m_pTexMonster = NULL;
    m_pTexPartyMember = NULL;
}

CustomMinimap::~CustomMinimap() {
    Shutdown();
}

bool CustomMinimap::Initialize(IDirect3DDevice9* pDevice) {
    if (!pDevice) return false;
    
    m_pDevice = pDevice;
    
    // Don't load textures here - too early!
    // LoadTextures will be called lazily in Render() when game is ready
    
    m_bInitialized = true;
    return true;
}

void CustomMinimap::Shutdown() {
    m_bInitialized = false;
    m_pDevice = NULL;
}

void CustomMinimap::UpdatePlayerPosition() {
    // Get coordinates and region name from native CIFMinimap instance
    GetMinimapData(m_nRegionX, m_nRegionY, m_nDisplayX, m_nDisplayY, m_pRegionName);
}

void CustomMinimap::Render() {
    if (!m_bVisible) return;
    
    // Lazy initialization - set initialized on first render
    if (!m_bInitialized) {
        m_bInitialized = true;
    }
    
    // Lazy texture loading - only load once when game is ready
    if (m_pTexCharacter == NULL) {
        LoadTextures();
    }
    
    // Update position data
    UpdatePlayerPosition();
    
    // Setup ImGui window flags
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | 
                             ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoScrollbar |
                             ImGuiWindowFlags_NoBackground;
    
    ImGui::SetNextWindowPos(m_vMinimapPos, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(m_fMinimapSize + 20, m_fMinimapSize + 40));
    
    if (ImGui::Begin("CustomMinimap", NULL, flags)) {
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 windowPos = ImGui::GetWindowPos();
        
        // Minimap background position
        ImVec2 mapPos = ImVec2(windowPos.x + 10, windowPos.y + 10);
        
        // Draw minimap background
        DrawMinimapBackground(drawList, mapPos, m_fMinimapSize);
        
        // Draw player marker at center
        ImVec2 center = ImVec2(mapPos.x + m_fMinimapSize * 0.5f, 
                               mapPos.y + m_fMinimapSize * 0.5f);
        
        // Draw entity markers (monsters, NPCs, players, items)
        DrawEntityMarkers(drawList, mapPos, m_fMinimapSize);
        
        // Draw player marker last (on top)
        DrawPlayerMarker(drawList, center);
        
        // Draw coordinates below map
        ImVec2 coordPos = ImVec2(mapPos.x, mapPos.y + m_fMinimapSize + 5);
        DrawCoordinates(drawList, coordPos);
    }
    ImGui::End();
}

void CustomMinimap::DrawMinimapBackground(ImDrawList* drawList, const ImVec2& pos, float size) {
    ImVec2 min = pos;
    ImVec2 max = ImVec2(pos.x + size, pos.y + size);
    
    // Background - dark with some transparency
    drawList->AddRectFilled(min, max, IM_COL32(20, 20, 30, 200));
    
    // Border
    drawList->AddRect(min, max, IM_COL32(80, 80, 100, 255), 0.0f, 0, 2.0f);
    
    // Grid lines for visual reference
    ImU32 gridColor = IM_COL32(40, 40, 60, 150);
    float gridSize = size / 4.0f;
    
    for (int i = 1; i < 4; i++) {
        float offset = gridSize * i;
        // Vertical lines
        drawList->AddLine(
            ImVec2(min.x + offset, min.y),
            ImVec2(min.x + offset, max.y),
            gridColor);
        // Horizontal lines
        drawList->AddLine(
            ImVec2(min.x, min.y + offset),
            ImVec2(max.x, min.y + offset),
            gridColor);
    }
}

void CustomMinimap::DrawPlayerMarker(ImDrawList* drawList, const ImVec2& center) {
    // Player arrow/triangle pointing up
    float arrowSize = 8.0f;
    
    ImVec2 p1 = ImVec2(center.x, center.y - arrowSize);           // Top
    ImVec2 p2 = ImVec2(center.x - arrowSize * 0.6f, center.y + arrowSize * 0.5f); // Bottom left
    ImVec2 p3 = ImVec2(center.x + arrowSize * 0.6f, center.y + arrowSize * 0.5f); // Bottom right
    
    // Filled triangle - green color
    drawList->AddTriangleFilled(p1, p2, p3, IM_COL32(50, 255, 50, 255));
    
    // Border for visibility
    drawList->AddTriangle(p1, p2, p3, IM_COL32(255, 255, 255, 200), 1.5f);
}

void CustomMinimap::DrawCoordinates(ImDrawList* drawList, const ImVec2& pos) {
    // Display coordinates like native minimap: "X:123  Y:456"
    char coordText[64];
    sprintf(coordText, "X:%d  Y:%d", m_nDisplayX, m_nDisplayY);
    
    // Shadow
    drawList->AddText(ImVec2(pos.x + 1, pos.y + 1), IM_COL32(0, 0, 0, 200), coordText);
    // Text
    drawList->AddText(pos, IM_COL32(200, 200, 255, 255), coordText);
    
    // Region name on second line (from TextStringManager)
    const char* regionName = m_pRegionName ? m_pRegionName : "Unknown";
    drawList->AddText(ImVec2(pos.x + 1, pos.y + 15), IM_COL32(0, 0, 0, 200), regionName);
    drawList->AddText(ImVec2(pos.x, pos.y + 14), IM_COL32(255, 220, 100, 255), regionName);
}

// Native DDJ texture loader function pointer (sub_409E10)
// Takes std::string* as parameter and returns texture pointer
typedef void* (__cdecl* LoadDDJTextureFunc)(void* pStdString);
static LoadDDJTextureFunc g_pfnLoadDDJTexture = (LoadDDJTextureFunc)0x00409E10;

// std::string constructor at 0x406190 - ACTUALLY __thiscall!
// Signature: int __thiscall sub_406190(int *this, void *Src, int a3)
// this = pointer to string struct (ECX)
// Src = start of char buffer
// a3 = end of char buffer (Src + len)
static const DWORD ADDR_StdStringCtor = 0x00406190;

// Helper to call __thiscall std::string constructor
// Uses inline assembly because VS2005 doesn't support __thiscall function pointers easily
static void CallStdStringCtor(void* pThis, const char* begin, const char* end) {
    __asm {
        push end          // a3 - end pointer
        push begin        // Src - start pointer
        mov ecx, pThis    // this pointer in ECX (__thiscall)
        mov eax, ADDR_StdStringCtor
        call eax
    }
}

// Helper to create std::string and load texture
static void* LoadDDJTexture(const char* path) {
    // Simple std::string structure (12 bytes for VS2005)
    char strBuffer[16] = {0};
    
    // Create std::string with path using correct __thiscall convention
    size_t len = strlen(path);
    CallStdStringCtor(strBuffer, path, path + len);
    
    // Load texture using native function
    void* pTexture = g_pfnLoadDDJTexture(strBuffer);
    
    // Note: strBuffer will be cleaned up when function returns (leak, but small)
    return pTexture;
}

// Entity type enum for clarity
enum EntityType {
    ENTITY_UNKNOWN = 0,
    ENTITY_MONSTER,
    ENTITY_MONSTER_UNIQUE,
    ENTITY_NPC,
    ENTITY_PLAYER,
    ENTITY_ITEM
};

// Get RuntimeClass from entity (first DWORD points to VTable, VTable+0 has RuntimeClass getter)
static DWORD GetEntityRuntimeClass(DWORD entityPtr) {
    __try {
        // GetRuntimeClass is usually at VTable+0, returns pointer to CRuntimeClass
        DWORD* pVTable = *(DWORD**)entityPtr;
        if (!pVTable) return 0;
        
        // Alternative: Check specific known offsets for entity type identification
        // In SRO, entity type is often stored at a fixed offset
        return pVTable[0];  // First entry often used for type checks
    }
    __except (1) { return 0; }
}

// Determine entity type from entity pointer
static EntityType GetEntityType(DWORD entityPtr) {
    __try {
        // Check CRuntimeClass pointer at entityPtr - these are direct comparisons
        // The native code uses comparisons like: if (*(DWORD*)entityPtr == RUNTIMECLASS_MONSTER)
        
        // Read the VTable pointer (first DWORD of object)
        DWORD vTablePtr = *(DWORD*)entityPtr;
        
        // Monster VTable check - CICMonster
        // From IDA: if entity VTable matches CICMonster
        if (vTablePtr == 0x009E7F08) {  // CICMonster VTable
            // Check if unique monster (offset 0x668 == 3)
            BYTE monsterType = *(BYTE*)(entityPtr + ENTITY_OFFSET_MONSTERTYPE);
            if (monsterType == 3) {
                return ENTITY_MONSTER_UNIQUE;
            }
            return ENTITY_MONSTER;
        }
        
        // NPC VTable check - CICNPC
        if (vTablePtr == 0x009E6F18) {  // CICNPC VTable
            return ENTITY_NPC;
        }
        
        // Player VTable check - CICPlayer
        if (vTablePtr == 0x009E84FC) {  // CICPlayer VTable
            return ENTITY_PLAYER;
        }
        
        // Picked Item VTable check
        if (vTablePtr == 0x009E8224) {  // CICPickedItem VTable
            return ENTITY_ITEM;
        }
        
        return ENTITY_UNKNOWN;
    }
    __except (1) { return ENTITY_UNKNOWN; }
}

// Get entity position
static bool GetEntityPosition(DWORD entityPtr, float& outX, float& outZ) {
    __try {
        outX = *(float*)(entityPtr + ENTITY_OFFSET_POSEX);
        outZ = *(float*)(entityPtr + ENTITY_OFFSET_POSEZ);
        return true;
    }
    __except (1) { return false; }
}

// Check if entity is dead
static bool IsEntityDead(DWORD entityPtr) {
    __try {
        BYTE state = *(BYTE*)(entityPtr + ENTITY_OFFSET_STATE);
        return (state == 4);  // 4 = dead
    }
    __except (1) { return true; }  // Assume dead on error
}

void CustomMinimap::DrawEntityMarkers(ImDrawList* drawList, const ImVec2& mapPos, float mapSize) {
    __try {
        // Get player pointer for position reference
        DWORD playerPtr = *(DWORD*)ADDR_PLAYER_PTR;
        if (playerPtr == 0) return;
        
        // Player position (center of minimap)
        float playerX = *(float*)(playerPtr + 0x74);
        float playerZ = *(float*)(playerPtr + 0x7C);
        
        // Minimap center
        ImVec2 center = ImVec2(mapPos.x + mapSize * 0.5f, mapPos.y + mapSize * 0.5f);
        
        // Minimap range (approximate world units visible on minimap)
        // Native minimap shows about 100 units in each direction
        float minimapRange = 100.0f;
        float scale = (mapSize * 0.5f) / minimapRange;
        
        // Iterate entity linked list (from native CIFMinimap sub_53AD20)
        // dword_9C99A4 = head of entity list
        DWORD listNode = *(DWORD*)ADDR_ENTITY_LIST_HEAD;
        
        int maxIterations = 500;  // Safety limit
        while (listNode != 0 && maxIterations-- > 0) {
            // Get entity from list node (native: v31 = v30 - 436)
            DWORD entityPtr = listNode - ENTITY_LINKED_OFFSET;
            
            // Skip if dead
            if (!IsEntityDead(entityPtr)) {
                // Get position
                float entityX, entityZ;
                if (GetEntityPosition(entityPtr, entityX, entityZ)) {
                    // Calculate relative position from player
                    float relX = entityX - playerX;
                    float relZ = entityZ - playerZ;
                    
                    // Check if in minimap range
                    if (fabsf(relX) < minimapRange && fabsf(relZ) < minimapRange) {
                        // Convert to screen position
                        // Note: Z is typically north (up on minimap), X is east (right)
                        float screenX = center.x + (relX * scale);
                        float screenY = center.y - (relZ * scale);  // Invert Z for screen coords
                        
                        // Clamp to minimap bounds
                        float margin = 4.0f;
                        screenX = max(mapPos.x + margin, min(mapPos.x + mapSize - margin, screenX));
                        screenY = max(mapPos.y + margin, min(mapPos.y + mapSize - margin, screenY));
                        
                        // Get entity type and draw appropriate marker
                        EntityType type = GetEntityType(entityPtr);
                        ImVec2 markerPos = ImVec2(screenX, screenY);
                        float markerSize = 4.0f;
                        
                        switch (type) {
                            case ENTITY_MONSTER:
                                // Normal monster - red circle
                                drawList->AddCircleFilled(markerPos, markerSize, IM_COL32(255, 80, 80, 220));
                                break;
                                
                            case ENTITY_MONSTER_UNIQUE:
                                // Unique/Elite monster - orange star-like marker
                                drawList->AddCircleFilled(markerPos, markerSize + 2, IM_COL32(255, 165, 0, 255));
                                drawList->AddCircle(markerPos, markerSize + 4, IM_COL32(255, 255, 100, 180), 8, 2.0f);
                                break;
                                
                            case ENTITY_NPC:
                                // NPC - blue square
                                drawList->AddRectFilled(
                                    ImVec2(markerPos.x - markerSize, markerPos.y - markerSize),
                                    ImVec2(markerPos.x + markerSize, markerPos.y + markerSize),
                                    IM_COL32(100, 150, 255, 220));
                                break;
                                
                            case ENTITY_PLAYER:
                                // Other player - green triangle
                                if (entityPtr != playerPtr) {  // Skip self
                                    ImVec2 p1 = ImVec2(markerPos.x, markerPos.y - markerSize);
                                    ImVec2 p2 = ImVec2(markerPos.x - markerSize, markerPos.y + markerSize);
                                    ImVec2 p3 = ImVec2(markerPos.x + markerSize, markerPos.y + markerSize);
                                    drawList->AddTriangleFilled(p1, p2, p3, IM_COL32(100, 200, 100, 220));
                                }
                                break;
                                
                            case ENTITY_ITEM:
                                // Picked item - small yellow diamond
                                drawList->AddQuadFilled(
                                    ImVec2(markerPos.x, markerPos.y - 3),
                                    ImVec2(markerPos.x + 3, markerPos.y),
                                    ImVec2(markerPos.x, markerPos.y + 3),
                                    ImVec2(markerPos.x - 3, markerPos.y),
                                    IM_COL32(255, 255, 100, 200));
                                break;
                                
                            default:
                                // Unknown - skip
                                break;
                        }
                    }
                }
            }
            
            // Next node in linked list (offset +12)
            listNode = *(DWORD*)(listNode + 12);
        }
    }
    __except (1) {
        // Silent exception handling
    }
}

void CustomMinimap::LoadTextures() {
    // Load minimap marker textures using native DDJ loader
    m_pTexCharacter = LoadDDJTexture("interface\\minimap\\mm_sign_character.ddj");
    m_pTexPartyArrow = LoadDDJTexture("interface\\minimap\\mm_sign_partyarrow.ddj");
    m_pTexNPC = LoadDDJTexture("interface\\minimap\\mm_sign_npc.ddj");
    m_pTexPartyMember = LoadDDJTexture("interface\\minimap\\mm_sign_partymember.ddj");
    
    // Note: mm_sign_monster.ddj may not exist, skip for now
    // m_pTexMonster = LoadDDJTexture("interface\\minimap\\mm_sign_monster.ddj");
}
