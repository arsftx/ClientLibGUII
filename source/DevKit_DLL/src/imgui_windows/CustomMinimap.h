#pragma once

#include <imgui/imgui.h>
#include <d3d9.h>

// CustomMinimap - ImGui based minimap for testing
// Reads data from native CIFMinimap and renders with ImGui

class CustomMinimap {
public:
    CustomMinimap();
    ~CustomMinimap();
    
    // Initialize with D3D device
    bool Initialize(IDirect3DDevice9* pDevice);
    
    // Main render function
    void Render();
    
    // Cleanup
    void Shutdown();
    
    // Toggle visibility
    void SetVisible(bool bVisible) { m_bVisible = bVisible; }
    bool IsVisible() const { return m_bVisible; }
    
private:
    // Read player position from game
    void UpdatePlayerPosition();
    
    // Draw minimap background
    void DrawMinimapBackground(ImDrawList* drawList, const ImVec2& pos, float size);
    
    // Draw player marker at calculated position with rotation
    void DrawPlayerMarker(ImDrawList* drawList, const ImVec2& mapPos, float mapSize);
    
    // Draw entity markers (monsters, NPCs, players)
    void DrawEntityMarkers(ImDrawList* drawList, const ImVec2& mapPos, float mapSize);
    
    // Draw coordinate text
    void DrawCoordinates(ImDrawList* drawList, const ImVec2& pos);
    
    // Draw zoom controls
    void DrawZoomControls(const ImVec2& mapPos, float mapSize);
    
private:
    bool m_bVisible;
    bool m_bInitialized;
    
    // Coordinates from native CIFMinimap
    int m_nRegionX;           // Region X coordinate (from CIFMinimap)
    int m_nRegionY;           // Region Y coordinate (from CIFMinimap)
    int m_nDisplayX;          // Display X coordinate (calculated)
    int m_nDisplayY;          // Display Y coordinate (calculated)
    const char* m_pRegionName; // Region name (from TextStringManager)
    
    // Player position and rotation (from native CIFMinimap offsets)
    float m_fPlayerPosX;       // Player X position within region
    float m_fPlayerPosZ;       // Player Z position within region
    float m_fPlayerRotation;   // Player rotation/heading (radians)
    
    // Minimap display settings
    float m_fMinimapSize;      // Size in pixels (default 192)
    ImVec2 m_vMinimapPos;      // Screen position
    float m_fZoomFactor;       // Zoom scale (from native CIFMinimap 0x330)
    
    // Arrow position on minimap (calculated, not fixed at center)
    float m_fArrowOffsetX;     // Arrow X offset from center
    float m_fArrowOffsetY;     // Arrow Y offset from center
    
    // D3D Device reference
    IDirect3DDevice9* m_pDevice;
    
    // Loaded textures (from DDJ files)
    void* m_pTexCharacter;     // mm_sign_character.ddj - player marker
    void* m_pTexPartyArrow;    // mm_sign_partyarrow.ddj
    void* m_pTexNPC;           // mm_sign_npc.ddj
    void* m_pTexMonster;       // mm_sign_monster.ddj (if exists)
    void* m_pTexPartyMember;   // mm_sign_partymember.ddj
    
    // Load DDJ textures
    void LoadTextures();
};

// Initialize CustomMinimap - call from DllMain or initialization
void InitializeCustomMinimap();
