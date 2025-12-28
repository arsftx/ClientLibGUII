#include "CustomWorldMap.h"
#include "CustomGUISession.h"
#include <GFX3DFunction/GFXVideo3d.h>
#include <CIFWorldMap.h>
#include <stdio.h>
#include <cmath>

#pragma comment(lib, "d3dx9.lib")

// =============================================================================
// Address Constants
// =============================================================================
static const DWORD ADDR_PLAYER_PTR = 0x00A0465C;
static const DWORD ADDR_LOADING_MANAGER = 0xA00524;
static const DWORD OFFSET_LOADING_FLAG = 0xBC;

// =============================================================================
// Game's Native String Structure (for texture loading)
// =============================================================================
struct GameString {
    char* data;
    char* end;
    char* capacity;
};

// sub_406190: String constructor (thiscall)
typedef void (__thiscall *tStringConstruct)(GameString* pThis, const char* start, const char* end);
static tStringConstruct StringConstruct = (tStringConstruct)0x00406190;

// sub_409E10: Texture loader (cdecl)
typedef IDirect3DBaseTexture9* (__cdecl *tLoadTexture)(GameString* pPath);
static tLoadTexture LoadGameTexture = (tLoadTexture)0x00409E10;

// Helper: Create GameString from const char*
static void CreateGameString(GameString* pStr, const char* text) {
    size_t len = strlen(text);
    StringConstruct(pStr, text, text + len);
}

// Helper: Load DDJ texture using game's native loader
static IDirect3DTexture9* LoadDDJTexture(const char* path) {
    GameString str = {0, 0, 0};
    CreateGameString(&str, path);
    return (IDirect3DTexture9*)LoadGameTexture(&str);
}

// =============================================================================
// Global Instance
// =============================================================================
static CustomWorldMap* g_pCustomWorldMap = NULL;

// Render callback for CustomGUISession
static void WorldMap_RenderCallback() {
    if (g_pCustomWorldMap) {
        g_pCustomWorldMap->Render();  // Render() handles visibility check after RenderDebugWindow()
    }
}

// Initialize CustomWorldMap - call from DllMain or initialization
void InitializeCustomWorldMap() {
    if (!g_pCustomWorldMap) {
        g_pCustomWorldMap = &CustomWorldMap::Instance();
        g_pCustomWorldMap->Initialize();
    }
}

// =============================================================================
// Singleton
// =============================================================================
CustomWorldMap& CustomWorldMap::Instance() {
    static CustomWorldMap instance;
    return instance;
}

// =============================================================================
// Debug Menu Item
// =============================================================================
void CustomWorldMap::MenuItem() {
    if (ImGui::BeginMenu("World Map")) {
        bool visible = m_bVisible;
        if (ImGui::MenuItem("Show Map", "M", &visible)) {
            m_bVisible = visible;
        }
        
        ImGui::Separator();
        
        if (ImGui::MenuItem("World View", NULL, m_nMapMode == 0)) {
            m_nMapMode = 0;
            m_fViewX = 0;
            m_fViewY = 0;
        }
        
        if (ImGui::MenuItem("Local View", NULL, m_nMapMode == 1)) {
            m_nMapMode = 1;
            m_fViewX = 0;
            m_fViewY = 0;
        }
        
        ImGui::EndMenu();
    }
}

// =============================================================================
// Constructor / Destructor
// =============================================================================
CustomWorldMap::CustomWorldMap() 
    : m_bInitialized(false)
    , m_bVisible(false)
    , m_bTexturesLoaded(false)
    , m_nMapMode(0)
    , m_nLocalMapIndex(1)
    , m_vWindowPos(100, 100)
    , m_fWorldMapWidth(652.0f)
    , m_fWorldMapHeight(424.0f)
    , m_fLocalMapWidth(268.0f)
    , m_fLocalMapHeight(296.0f)
    , m_fViewX(0.0f)
    , m_fViewY(0.0f)
    , m_fScaleX(1.0f)
    , m_fScaleY(1.0f)
    , m_fPlayerWorldX(0.0f)
    , m_fPlayerWorldY(0.0f)
    , m_fPlayerWorldZ(0.0f)
    , m_fPlayerRotation(0.0f)
    , m_nPlayerRegionX(0)
    , m_nPlayerRegionY(0)
    , m_bDragging(false)
    , m_fDragStartX(0.0f)
    , m_fDragStartY(0.0f)
    , m_fDragViewStartX(0.0f)
    , m_fDragViewStartY(0.0f)
    , m_pDevice(NULL)
    , m_pSprite(NULL)
    , m_nLoadedTileCount(0)
    , m_nTileStartX(102)
    , m_nTileStartY(81)
    , m_nTileCountX(19)
    , m_nTileCountY(7)
    , m_nLocationCount(0)
{
    // Initialize texture structures
    memset(&m_texWindowEdge, 0, sizeof(m_texWindowEdge));
    memset(&m_texBackground, 0, sizeof(m_texBackground));
    memset(&m_texZoomBtn, 0, sizeof(m_texZoomBtn));
    memset(&m_texCharacter, 0, sizeof(m_texCharacter));
    memset(&m_mapTiles, 0, sizeof(m_mapTiles));
    memset(&m_locations, 0, sizeof(m_locations));
    
    // Set window size based on mode
    m_vWindowSize = ImVec2(m_fWorldMapWidth + 20, m_fWorldMapHeight + 60);
}

CustomWorldMap::~CustomWorldMap() {
    Shutdown();
}

// =============================================================================
// Initialize / Shutdown
// =============================================================================
bool CustomWorldMap::Initialize() {
    if (m_bInitialized) return true;
    
    // Register render callback with CustomGUISession
    int callbackId = g_CustomGUI.RegisterRenderCallback(WorldMap_RenderCallback);
    if (callbackId < 0) {
        return false;
    }
    
    m_bInitialized = true;
    return true;
}

void CustomWorldMap::Shutdown() {
    ReleaseNativeSprite();
    ReleaseTextures();
    m_bInitialized = false;
}

// =============================================================================
// Device Lost/Reset
// =============================================================================
void CustomWorldMap::OnDeviceLost() {
    if (m_pSprite) {
        m_pSprite->OnLostDevice();
    }
}

void CustomWorldMap::OnDeviceReset() {
    if (m_pSprite) {
        m_pSprite->OnResetDevice();
    }
}

// =============================================================================
// Texture Loading
// =============================================================================
bool CustomWorldMap::LoadTextures() {
    if (m_bTexturesLoaded) return true;
    
    // Load window edge textures
    const char* edgePaths[] = {
        "interface\\worldmap\\wmap_window_edge_1.ddj",
        "interface\\worldmap\\wmap_window_edge_2.ddj",
        "interface\\worldmap\\wmap_window_edge_3.ddj",
        "interface\\worldmap\\wmap_window_edge_4.ddj"
    };
    
    for (int i = 0; i < 4; i++) {
        IDirect3DTexture9* pTex = LoadDDJTexture(edgePaths[i]);
        if (pTex) {
            m_texWindowEdge[i].pTexture = pTex;
            D3DSURFACE_DESC desc;
            if (SUCCEEDED(pTex->GetLevelDesc(0, &desc))) {
                m_texWindowEdge[i].width = desc.Width;
                m_texWindowEdge[i].height = desc.Height;
            }
        }
    }
    
    // Load background texture
    IDirect3DTexture9* pBgTex = LoadDDJTexture("interface\\worldmap\\wmap_bg.ddj");
    if (pBgTex) {
        m_texBackground.pTexture = pBgTex;
        D3DSURFACE_DESC desc;
        if (SUCCEEDED(pBgTex->GetLevelDesc(0, &desc))) {
            m_texBackground.width = desc.Width;
            m_texBackground.height = desc.Height;
        }
    }
    
    // Load zoom button texture
    IDirect3DTexture9* pZoomTex = LoadDDJTexture("interface\\worldmap\\wmap_zoom.ddj");
    if (pZoomTex) {
        m_texZoomBtn.pTexture = pZoomTex;
        D3DSURFACE_DESC desc;
        if (SUCCEEDED(pZoomTex->GetLevelDesc(0, &desc))) {
            m_texZoomBtn.width = desc.Width;
            m_texZoomBtn.height = desc.Height;
        }
    }
    
    // Load character marker texture
    IDirect3DTexture9* pCharTex = LoadDDJTexture("interface\\minimap\\mm_sign_character.ddj");
    if (pCharTex) {
        m_texCharacter.pTexture = pCharTex;
        D3DSURFACE_DESC desc;
        if (SUCCEEDED(pCharTex->GetLevelDesc(0, &desc))) {
            m_texCharacter.width = desc.Width;
            m_texCharacter.height = desc.Height;
        }
    }
    
    // Load map tiles
    LoadAllMapTiles();
    
    m_bTexturesLoaded = true;
    return true;
}

void CustomWorldMap::ReleaseTextures() {
    // Note: Game manages texture memory, we don't release manually
    memset(&m_texWindowEdge, 0, sizeof(m_texWindowEdge));
    memset(&m_texBackground, 0, sizeof(m_texBackground));
    memset(&m_texZoomBtn, 0, sizeof(m_texZoomBtn));
    memset(&m_texCharacter, 0, sizeof(m_texCharacter));
    memset(&m_mapTiles, 0, sizeof(m_mapTiles));
    m_nLoadedTileCount = 0;
    m_bTexturesLoaded = false;
}

bool CustomWorldMap::LoadMapTile(int x, int y, int index) {
    if (index < 0 || index >= MAX_WORLDMAP_TILES) return false;
    
    char path[128];
    sprintf(path, "interface\\worldmap\\map\\map_world_%dx%d.ddj", x, y);
    
    IDirect3DTexture9* pTex = LoadDDJTexture(path);
    if (pTex) {
        m_mapTiles[index].pTexture = pTex;
        D3DSURFACE_DESC desc;
        if (SUCCEEDED(pTex->GetLevelDesc(0, &desc))) {
            m_mapTiles[index].width = desc.Width;
            m_mapTiles[index].height = desc.Height;
        }
        return true;
    }
    return false;
}

void CustomWorldMap::LoadAllMapTiles() {
    // World map tiles: X from 102 to 178 (step 4), Y from 109 to 81 (step -4)
    // 19 columns x 7 rows = 133 tiles
    int index = 0;
    for (int x = 102; x < 178; x += 4) {
        for (int y = 109; y > 81; y -= 4) {
            if (index < MAX_WORLDMAP_TILES) {
                LoadMapTile(x, y, index);
                index++;
            }
        }
    }
    m_nLoadedTileCount = index;
}

void CustomWorldMap::LoadLocalMapTiles() {
    // TODO: Load local map tiles based on m_nLocalMapIndex
}

// =============================================================================
// DirectX Sprite Rendering
// =============================================================================
void CustomWorldMap::InitNativeSprite(IDirect3DDevice9* pDevice) {
    if (m_pSprite) return;
    if (!pDevice) return;
    
    m_pDevice = pDevice;
    
    HRESULT hr = D3DXCreateSprite(pDevice, &m_pSprite);
    if (FAILED(hr)) {
        m_pSprite = NULL;
    }
}

void CustomWorldMap::ReleaseNativeSprite() {
    if (m_pSprite) {
        m_pSprite->Release();
        m_pSprite = NULL;
    }
}

void CustomWorldMap::BeginSpriteBatch() {
    if (m_pSprite && m_pDevice) {
        // Setup alpha blending manually (VS 2005 SDK Begin() has no parameters)
        m_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
        m_pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
        m_pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
        m_pSprite->Begin();
    }
}

void CustomWorldMap::EndSpriteBatch() {
    if (m_pSprite) {
        m_pSprite->End();
    }
}

void CustomWorldMap::RenderSprite(IDirect3DTexture9* pTexture, 
    float x, float y, float w, float h, D3DCOLOR color) {
    if (!m_pSprite || !pTexture) return;
    
    // Get original texture size
    D3DSURFACE_DESC desc;
    if (FAILED(pTexture->GetLevelDesc(0, &desc))) return;
    
    // Calculate scale to fit desired size
    float scaleX = w / (float)desc.Width;
    float scaleY = h / (float)desc.Height;
    D3DXVECTOR2 scaling(scaleX, scaleY);
    D3DXVECTOR2 position(x, y);
    
    m_pSprite->Draw(
        pTexture,
        NULL,           // Full texture
        &scaling,       // Scaling
        NULL,           // Rotation center
        0.0f,           // Rotation angle
        &position,      // Position
        color           // Tint color
    );
}

void CustomWorldMap::RenderSpriteRect(IDirect3DTexture9* pTexture,
    const RECT* pSrcRect, float x, float y, float w, float h, D3DCOLOR color) {
    if (!m_pSprite || !pTexture || !pSrcRect) return;
    
    // Calculate scale based on source rect
    float srcWidth = (float)(pSrcRect->right - pSrcRect->left);
    float srcHeight = (float)(pSrcRect->bottom - pSrcRect->top);
    
    if (srcWidth <= 0 || srcHeight <= 0) return;
    
    float scaleX = w / srcWidth;
    float scaleY = h / srcHeight;
    D3DXVECTOR2 scaling(scaleX, scaleY);
    D3DXVECTOR2 position(x, y);
    
    m_pSprite->Draw(
        pTexture,
        pSrcRect,       // Source rect
        &scaling,       // Scaling
        NULL,           // Rotation center
        0.0f,           // Rotation angle
        &position,      // Position
        color           // Tint color
    );
}

// =============================================================================
// Main Render Function
// =============================================================================
void CustomWorldMap::Render() {
    // === WORLDMAP DEBUG WINDOW ===
    // Always render debug window (like MiniInfo Debug), regardless of map visibility
    RenderDebugWindow();
    
    if (!m_bVisible) return;
    
    // Check D3D device state
    if (!g_CD3DApplication || !g_CD3DApplication->m_pd3dDevice) return;
    
    IDirect3DDevice9* pDevice = g_CD3DApplication->m_pd3dDevice;
    
    HRESULT hr = pDevice->TestCooperativeLevel();
    if (hr == D3DERR_DEVICELOST || hr == D3DERR_DEVICENOTRESET) return;
    if (g_CD3DApplication->IsLost()) return;
    
    // Check loading state
    DWORD loadingManager = *(DWORD*)ADDR_LOADING_MANAGER;
    if (loadingManager != 0) {
        BYTE isLoading = *(BYTE*)(loadingManager + OFFSET_LOADING_FLAG);
        if (isLoading != 0) return;
    }
    
    // Initialize sprite renderer
    InitNativeSprite(pDevice);
    
    // Load textures on first render
    if (!m_bTexturesLoaded) {
        LoadTextures();
    }
    
    // Update player position
    UpdatePlayerPosition();
    
    // Calculate view transform
    CalculateMapTransform();
    
    // Setup ImGui window
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoScrollbar |
                              ImGuiWindowFlags_NoScrollWithMouse |
                              ImGuiWindowFlags_NoCollapse;
    
    // Set window size based on map mode
    float mapWidth = (m_nMapMode == 0) ? m_fWorldMapWidth : m_fLocalMapWidth;
    float mapHeight = (m_nMapMode == 0) ? m_fWorldMapHeight : m_fLocalMapHeight;
    m_vWindowSize = ImVec2(mapWidth + 20, mapHeight + 80);
    
    ImGui::SetNextWindowPos(m_vWindowPos, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(m_vWindowSize, ImGuiCond_Always);
    
    if (ImGui::Begin("World Map", &m_bVisible, flags)) {
        // Get window position for sprite rendering
        ImVec2 windowPos = ImGui::GetWindowPos();
        ImVec2 contentPos = ImGui::GetCursorScreenPos();
        
        // Handle mouse input for panning
        HandleMouseInput();
        
        // Begin sprite batch
        BeginSpriteBatch();
        
        // Render map tiles
        RenderMapTiles();
        
        // Render location markers
        RenderLocationMarkers();
        
        // Render player marker
        RenderPlayerMarker();
        
        // Render party members
        RenderPartyMembers();
        
        // End sprite batch
        EndSpriteBatch();
        
        // Render ImGui controls
        RenderZoomControls();
        RenderCoordinates();
    }
    ImGui::End();
}

// =============================================================================
// Rendering Functions
// =============================================================================
void CustomWorldMap::RenderMapBackground() {
    // Render window frame textures using ID3DXSprite
    // TODO: Implement window frame rendering
}

void CustomWorldMap::RenderMapTiles() {
    if (m_nLoadedTileCount == 0) return;
    
    ImVec2 contentPos = ImGui::GetCursorScreenPos();
    float mapWidth = (m_nMapMode == 0) ? m_fWorldMapWidth : m_fLocalMapWidth;
    float mapHeight = (m_nMapMode == 0) ? m_fWorldMapHeight : m_fLocalMapHeight;
    
    // Tile size on screen
    float tileWidth = mapWidth / (float)m_nTileCountX;
    float tileHeight = mapHeight / (float)m_nTileCountY;
    
    int index = 0;
    for (int col = 0; col < m_nTileCountX && index < m_nLoadedTileCount; col++) {
        for (int row = 0; row < m_nTileCountY && index < m_nLoadedTileCount; row++) {
            if (m_mapTiles[index].pTexture) {
                float x = contentPos.x + col * tileWidth + m_fViewX;
                float y = contentPos.y + row * tileHeight + m_fViewY;
                
                RenderSprite(m_mapTiles[index].pTexture, x, y, tileWidth, tileHeight);
            }
            index++;
        }
    }
}

void CustomWorldMap::RenderLocationMarkers() {
    // TODO: Render NPC/teleport markers
    for (int i = 0; i < m_nLocationCount; i++) {
        if (!m_locations[i].valid) continue;
        
        // Convert world position to screen position
        float screenX, screenY;
        WorldToScreen(m_locations[i].screenX, m_locations[i].screenY, screenX, screenY);
        
        // Draw marker at position (using ImGui for now)
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        if (m_locations[i].type == 1) {
            // NPC marker - blue square
            drawList->AddRectFilled(
                ImVec2(screenX - 4, screenY - 4),
                ImVec2(screenX + 4, screenY + 4),
                IM_COL32(100, 180, 255, 255));
        } else if (m_locations[i].type == 2) {
            // Area marker - draw name
            drawList->AddText(ImVec2(screenX, screenY), IM_COL32(255, 255, 255, 255), m_locations[i].name);
        }
    }
}

void CustomWorldMap::RenderPlayerMarker() {
    if (!m_texCharacter.pTexture) return;
    
    // Calculate player screen position
    float screenX, screenY;
    WorldToScreen(m_fPlayerWorldX, m_fPlayerWorldZ, screenX, screenY);
    
    // Draw player marker
    float markerSize = 16.0f;
    RenderSprite(m_texCharacter.pTexture, 
                 screenX - markerSize/2, screenY - markerSize/2,
                 markerSize, markerSize);
}

void CustomWorldMap::RenderPartyMembers() {
    // TODO: Read party member data and render markers
}

void CustomWorldMap::RenderZoomControls() {
    ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 30);
    
    if (ImGui::Button("World")) {
        m_nMapMode = 0;
        m_fViewX = 0;
        m_fViewY = 0;
    }
    ImGui::SameLine();
    
    if (ImGui::Button("Local")) {
        m_nMapMode = 1;
        m_fViewX = 0;
        m_fViewY = 0;
    }
}

void CustomWorldMap::RenderCoordinates() {
    // Display player coordinates
    ImGui::SameLine(100);
    ImGui::Text("X:%d Y:%d  Region: %d,%d", 
                (int)m_fPlayerWorldX, (int)m_fPlayerWorldZ,
                m_nPlayerRegionX, m_nPlayerRegionY);
}

// =============================================================================
// Debug Window (Separate panel like MiniInfo Debug)
// =============================================================================
void CustomWorldMap::RenderDebugWindow() {
    // Always render the debug window, regardless of map visibility
    if (ImGui::Begin("WorldMap Debug")) {
        // Main visibility toggle
        ImGui::Text("=== World Map Controls ===");
        if (ImGui::Checkbox("Show World Map", &m_bVisible)) {
            // Toggle handled by checkbox
        }
        
        ImGui::Separator();
        
        // Map Mode Selection
        ImGui::Text("Map Mode");
        if (ImGui::RadioButton("World Map", m_nMapMode == 0)) {
            m_nMapMode = 0;
            m_fViewX = 0;
            m_fViewY = 0;
        }
        ImGui::SameLine();
        if (ImGui::RadioButton("Local Map", m_nMapMode == 1)) {
            m_nMapMode = 1;
            m_fViewX = 0;
            m_fViewY = 0;
        }
        
        ImGui::Separator();
        
        // Player Position Info
        ImGui::Text("=== Player Position ===");
        ImGui::Text("World: X=%.1f Y=%.1f Z=%.1f", m_fPlayerWorldX, m_fPlayerWorldY, m_fPlayerWorldZ);
        ImGui::Text("Region: %d, %d", m_nPlayerRegionX, m_nPlayerRegionY);
        ImGui::Text("Rotation: %.1f", m_fPlayerRotation);
        
        ImGui::Separator();
        
        // View Transform Info
        ImGui::Text("=== View Transform ===");
        ImGui::Text("View Offset: X=%.1f Y=%.1f", m_fViewX, m_fViewY);
        ImGui::Text("Scale: X=%.4f Y=%.4f", m_fScaleX, m_fScaleY);
        ImGui::SliderFloat("Pan X", &m_fViewX, -500.0f, 500.0f);
        ImGui::SliderFloat("Pan Y", &m_fViewY, -500.0f, 500.0f);
        
        if (ImGui::Button("Reset View")) {
            m_fViewX = 0;
            m_fViewY = 0;
        }
        
        ImGui::Separator();
        
        // Texture Loading Info
        ImGui::Text("=== Textures ===");
        ImGui::Text("Textures Loaded: %s", m_bTexturesLoaded ? "Yes" : "No");
        ImGui::Text("Map Tiles Loaded: %d / %d", m_nLoadedTileCount, MAX_WORLDMAP_TILES);
        ImGui::Text("Tile Grid: %d x %d", m_nTileCountX, m_nTileCountY);
        
        if (ImGui::Button("Reload Textures")) {
            m_bTexturesLoaded = false;
        }
        
        ImGui::Separator();
        
        // Window Size Info
        ImGui::Text("=== Window Size ===");
        float mapWidth = (m_nMapMode == 0) ? m_fWorldMapWidth : m_fLocalMapWidth;
        float mapHeight = (m_nMapMode == 0) ? m_fWorldMapHeight : m_fLocalMapHeight;
        ImGui::Text("Current: %.0f x %.0f", mapWidth, mapHeight);
        ImGui::Text("Window Pos: %.0f, %.0f", m_vWindowPos.x, m_vWindowPos.y);
    }
    ImGui::End();
}

// =============================================================================
// Input Handling
// =============================================================================
void CustomWorldMap::HandleMouseInput() {
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 mousePos = io.MousePos;
    ImVec2 windowPos = ImGui::GetWindowPos();
    ImVec2 windowSize = ImGui::GetWindowSize();
    
    // Check if mouse is over window
    bool isOverWindow = (mousePos.x >= windowPos.x && mousePos.x < windowPos.x + windowSize.x &&
                         mousePos.y >= windowPos.y && mousePos.y < windowPos.y + windowSize.y);
    
    if (!isOverWindow) {
        m_bDragging = false;
        return;
    }
    
    // Left mouse button drag for panning
    if (ImGui::IsMouseClicked(0)) {
        m_bDragging = true;
        m_fDragStartX = mousePos.x;
        m_fDragStartY = mousePos.y;
        m_fDragViewStartX = m_fViewX;
        m_fDragViewStartY = m_fViewY;
    }
    
    if (ImGui::IsMouseReleased(0)) {
        if (m_bDragging) {
            // Check if it was a click (not a drag)
            float dragDist = sqrtf((mousePos.x - m_fDragStartX) * (mousePos.x - m_fDragStartX) +
                                   (mousePos.y - m_fDragStartY) * (mousePos.y - m_fDragStartY));
            if (dragDist < 5.0f) {
                HandleClick((int)mousePos.x, (int)mousePos.y);
            }
        }
        m_bDragging = false;
    }
    
    if (m_bDragging && ImGui::IsMouseDragging(0)) {
        float dx = mousePos.x - m_fDragStartX;
        float dy = mousePos.y - m_fDragStartY;
        m_fViewX = m_fDragViewStartX + dx;
        m_fViewY = m_fDragViewStartY + dy;
    }
}

void CustomWorldMap::HandleClick(int x, int y) {
    // Check if clicked on a location marker
    for (int i = 0; i < m_nLocationCount; i++) {
        if (!m_locations[i].valid) continue;
        
        float locX = m_locations[i].screenX;
        float locY = m_locations[i].screenY;
        
        if (x >= locX - 8 && x <= locX + 8 &&
            y >= locY - 8 && y <= locY + 8) {
            // Clicked on location - TODO: teleport or show info
            break;
        }
    }
}

// =============================================================================
// Coordinate Calculations
// =============================================================================
void CustomWorldMap::UpdatePlayerPosition() {
    __try {
        DWORD playerPtr = *(DWORD*)ADDR_PLAYER_PTR;
        if (!playerPtr) return;
        
        // Read region ID (player + 0x70)
        WORD regionID = *(WORD*)(playerPtr + 0x70);
        m_nPlayerRegionX = regionID & 0xFF;
        m_nPlayerRegionY = (regionID >> 8) & 0xFF;
        
        // Read world position
        m_fPlayerWorldX = *(float*)(playerPtr + 0x74);
        m_fPlayerWorldY = *(float*)(playerPtr + 0x78);
        m_fPlayerWorldZ = *(float*)(playerPtr + 0x7C);
        
        // Read rotation
        m_fPlayerRotation = *(float*)(playerPtr + 0x80);
    }
    __except(1) {
        // Exception - use default values
    }
}

void CustomWorldMap::CalculateMapTransform() {
    // Calculate scale factors for world->screen transformation
    // Based on native CIFWorldMap formulas
    
    // World map covers regions roughly 102-178 X and 81-109 Y
    // Each region is 192 world units
    
    float mapWidth = (m_nMapMode == 0) ? m_fWorldMapWidth : m_fLocalMapWidth;
    float mapHeight = (m_nMapMode == 0) ? m_fWorldMapHeight : m_fLocalMapHeight;
    
    // Total world coverage for world map
    float worldWidth = 19 * 4 * 192.0f;   // 19 tiles * 4 step * 192 units
    float worldHeight = 7 * 4 * 192.0f;   // 7 tiles * 4 step * 192 units
    
    m_fScaleX = mapWidth / worldWidth;
    m_fScaleY = mapHeight / worldHeight;
}

void CustomWorldMap::WorldToScreen(float worldX, float worldZ, float& screenX, float& screenY) {
    ImVec2 contentPos = ImGui::GetCursorScreenPos();
    
    // Calculate global world position
    float globalX = (m_nPlayerRegionX - 135) * 192.0f + worldX;
    float globalZ = (m_nPlayerRegionY - 92) * 192.0f + worldZ;
    
    // Convert to screen coords
    screenX = contentPos.x + globalX * m_fScaleX + m_fViewX;
    screenY = contentPos.y + globalZ * m_fScaleY + m_fViewY;
}

void CustomWorldMap::ScreenToWorld(float screenX, float screenY, float& worldX, float& worldZ) {
    ImVec2 contentPos = ImGui::GetCursorScreenPos();
    
    // Inverse of WorldToScreen
    float globalX = (screenX - contentPos.x - m_fViewX) / m_fScaleX;
    float globalZ = (screenY - contentPos.y - m_fViewY) / m_fScaleY;
    
    worldX = globalX;
    worldZ = globalZ;
}
