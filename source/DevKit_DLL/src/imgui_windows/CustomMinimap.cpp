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

// Get minimap data directly from player pointer (like native minimap does)
// Native code reads from dword_A0465C (player ptr) + offsets
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
        
        // Calculate display coordinates (EXACT native formula from sub_53A5A0 ASM)
        // Native ASM:
        //   fmul ds:flt_94AE04          ; posX * 10.0f
        //   call __ftol                 ; simple truncation to int
        //   lea ecx, [ebx+ebx*2-195h]   ; regionX*3 - 405
        //   shl ecx, 6                  ; << 6
        //   sub ecx, eax                ; - scaled_pos
        int posXScaled = (int)(posX * COORD_SCALE);  // __ftol = simple truncation
        int posZScaled = (int)(posZ * COORD_SCALE);
        
        // Formula: ((region*3 - offset) << 6) - scaled_position
        // X: offset = 0x195 = 405
        // Y: offset = 0x114 = 276
        displayX = ((regionX * 3 - 405) << 6) - posXScaled;
        displayY = ((regionY * 3 - 276) << 6) - posZScaled;
        
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

void CustomMinimap::LoadTextures() {
    // Load minimap marker textures using native DDJ loader
    m_pTexCharacter = LoadDDJTexture("interface\\minimap\\mm_sign_character.ddj");
    m_pTexPartyArrow = LoadDDJTexture("interface\\minimap\\mm_sign_partyarrow.ddj");
    m_pTexNPC = LoadDDJTexture("interface\\minimap\\mm_sign_npc.ddj");
    m_pTexPartyMember = LoadDDJTexture("interface\\minimap\\mm_sign_partymember.ddj");
    
    // Note: mm_sign_monster.ddj may not exist, skip for now
    // m_pTexMonster = LoadDDJTexture("interface\\minimap\\mm_sign_monster.ddj");
}
