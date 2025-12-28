#pragma once
/**
 * @file CustomWorldMap.h
 * @brief Custom World Map using ImGui + DirectX ID3DXSprite renderer
 * 
 * Uses the same pattern as CustomPlayerMiniInfo:
 * - ImGui for window management, controls, and user interaction
 * - ID3DXSprite for native DirectX texture rendering
 * - Game's native DDJ texture loader (sub_409E10) for texture loading
 */

#include <imgui/imgui.h>
#include <d3d9.h>
#include <d3dx9.h>

// D3DX Sprite flags (VS 2005 SDK compatibility)
#ifndef D3DXSPRITE_ALPHABLEND
#define D3DXSPRITE_ALPHABLEND 0x10
#endif

// Forward declarations
class CIFWorldMap;

// Maximum map tiles for world map (19x7 = 133 tiles)
#define MAX_WORLDMAP_TILES 140

// Location marker structure
struct WorldMapLocation {
    bool valid;
    int type;           // 1 = NPC/Icon, 2 = Area
    int mapIndex;       // Which map (0=world, 1=jangan, 2=donhwan, 3=khotan)
    float screenX;      // Current screen X position
    float screenY;      // Current screen Y position
    float width;        // Marker width
    float height;       // Marker height
    char name[64];      // Location name
    DWORD refObjID;     // Reference object ID (for click handling)
};

// Texture info structure
struct WorldMapTexture {
    IDirect3DTexture9* pTexture;
    int width;
    int height;
};

class CustomWorldMap {
public:
    static CustomWorldMap& Instance();
    
    // Initialize - call once at startup
    bool Initialize();
    
    // Shutdown - cleanup resources
    void Shutdown();
    
    // Main render function (registered with CustomGUISession)
    void Render();
    
    // Show/Hide the world map
    void Show() { m_bVisible = true; }
    void Hide() { m_bVisible = false; }
    void Toggle() { m_bVisible = !m_bVisible; }
    bool IsVisible() const { return m_bVisible; }
    
    // Debug menu item (for ImGui_Windows integration)
    void MenuItem();
    
    // Set map mode (0=world, 1=local)
    void SetMapMode(int mode) { m_nMapMode = mode; }
    int GetMapMode() const { return m_nMapMode; }
    
    // Set local map index (1=jangan, 2=donhwan, 3=khotan)
    void SetLocalMapIndex(int index) { m_nLocalMapIndex = index; }
    int GetLocalMapIndex() const { return m_nLocalMapIndex; }
    
    // Device lost/reset handlers
    void OnDeviceLost();
    void OnDeviceReset();
    
private:
    CustomWorldMap();
    ~CustomWorldMap();
    
    // Prevent copying
    CustomWorldMap(const CustomWorldMap&);
    CustomWorldMap& operator=(const CustomWorldMap&);
    
    // =========================================================================
    // TEXTURE LOADING
    // =========================================================================
    bool LoadTextures();
    void ReleaseTextures();
    bool LoadMapTile(int x, int y, int index);  // Load specific map tile
    void LoadAllMapTiles();     // Load all world map tiles
    void LoadLocalMapTiles();   // Load local map tiles
    
    // =========================================================================
    // DIRECTX SPRITE RENDERING
    // =========================================================================
    void InitNativeSprite(IDirect3DDevice9* pDevice);
    void ReleaseNativeSprite();
    
    // Render full texture at position
    void RenderSprite(IDirect3DTexture9* pTexture, 
                      float x, float y, float w, float h, 
                      D3DCOLOR color = 0xFFFFFFFF);
    
    // Render texture with source rect (for partial/clipped rendering)
    void RenderSpriteRect(IDirect3DTexture9* pTexture,
                          const RECT* pSrcRect,
                          float x, float y, float w, float h,
                          D3DCOLOR color = 0xFFFFFFFF);
    
    // Begin/End sprite batch
    void BeginSpriteBatch();
    void EndSpriteBatch();
    
    // =========================================================================
    // RENDERING FUNCTIONS
    // =========================================================================
    void RenderMapBackground();     // Render window frame and background
    void RenderMapTiles();          // Render map tile grid
    void RenderLocationMarkers();   // Render NPC/teleport markers
    void RenderPlayerMarker();      // Render player position
    void RenderPartyMembers();      // Render party member positions
    void RenderZoomControls();      // Render zoom toggle buttons
    void RenderCoordinates();       // Render current coordinates
    void RenderDebugWindow();       // Render debug control panel
    
    // =========================================================================
    // INPUT HANDLING
    // =========================================================================
    void HandleMouseInput();        // Pan/drag handling
    void HandleClick(int x, int y); // Location click handling
    
    // =========================================================================
    // COORDINATE CALCULATIONS
    // =========================================================================
    void UpdatePlayerPosition();    // Read player position from game
    void CalculateMapTransform();   // Calculate view transform
    
    // Convert world coords to screen coords
    void WorldToScreen(float worldX, float worldZ, float& screenX, float& screenY);
    
    // Convert screen coords to world coords
    void ScreenToWorld(float screenX, float screenY, float& worldX, float& worldZ);
    
private:
    // =========================================================================
    // STATE
    // =========================================================================
    bool m_bInitialized;
    bool m_bVisible;
    bool m_bTexturesLoaded;
    
    // Map mode: 0 = world map, 1 = local map
    int m_nMapMode;
    
    // Local map index: 1=Jangan, 2=Donhwan, 3=Khotan
    int m_nLocalMapIndex;
    
    // =========================================================================
    // WINDOW
    // =========================================================================
    ImVec2 m_vWindowPos;
    ImVec2 m_vWindowSize;
    
    // World map mode: 652x424
    // Local map mode: 268x296
    float m_fWorldMapWidth;
    float m_fWorldMapHeight;
    float m_fLocalMapWidth;
    float m_fLocalMapHeight;
    
    // =========================================================================
    // VIEW TRANSFORM
    // =========================================================================
    float m_fViewX;             // View origin X (pan offset)
    float m_fViewY;             // View origin Y (pan offset)
    float m_fScaleX;            // World to screen scale X
    float m_fScaleY;            // World to screen scale Y
    
    // =========================================================================
    // PLAYER DATA
    // =========================================================================
    float m_fPlayerWorldX;
    float m_fPlayerWorldY;
    float m_fPlayerWorldZ;
    float m_fPlayerRotation;
    int m_nPlayerRegionX;
    int m_nPlayerRegionY;
    
    // =========================================================================
    // MOUSE STATE
    // =========================================================================
    bool m_bDragging;
    float m_fDragStartX;
    float m_fDragStartY;
    float m_fDragViewStartX;
    float m_fDragViewStartY;
    
    // =========================================================================
    // DIRECTX RESOURCES
    // =========================================================================
    IDirect3DDevice9* m_pDevice;
    ID3DXSprite* m_pSprite;
    
    // =========================================================================
    // TEXTURES
    // =========================================================================
    // Window frame textures
    WorldMapTexture m_texWindowEdge[4];   // 4 edge textures
    WorldMapTexture m_texBackground;       // wmap_bg.ddj
    WorldMapTexture m_texZoomBtn;          // wmap_zoom.ddj
    WorldMapTexture m_texCharacter;        // mm_sign_character.ddj
    
    // Map tile textures (world map: 19x7 = 133 tiles)
    WorldMapTexture m_mapTiles[MAX_WORLDMAP_TILES];
    int m_nLoadedTileCount;
    
    // Tile coordinate tracking
    int m_nTileStartX;  // First tile X coord (102)
    int m_nTileStartY;  // First tile Y coord (81)
    int m_nTileCountX;  // Number of tiles X (19)
    int m_nTileCountY;  // Number of tiles Y (7)
    
    // =========================================================================
    // LOCATION MARKERS
    // =========================================================================
    WorldMapLocation m_locations[64];
    int m_nLocationCount;
};

// Initialize CustomWorldMap - call from DllMain or initialization
void InitializeCustomWorldMap();
