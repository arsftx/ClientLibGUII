#pragma once
/**
 * @file CustomPlayerMiniInfo.h
 * @brief Custom ImGui overlay for player HP/MP/Level display (VS2005 Compatible)
 * 
 * Replaces the original CIFPlayerMiniInfo by hiding it and rendering
 * a custom ImGui-based player info panel with PNG textures.
 * Uses textures from Media.pk2/newui/playerminiinfo/
 */

#include <ICPlayer.h>
#include <IFPlayerMiniInfo.h>
#include <d3d9.h>
#include <d3dx9.h>  // For ID3DXSprite

// Color structure for VS2005 compatibility
struct CustomPlayerMiniInfoColors {
    float hpBar[4];
    float mpBar[4];
    float hpCritical[4];
    float background[4];
    float text[4];
    float levelText[4];
};

// Texture info structure
struct TextureInfo {
    IDirect3DTexture9* pTexture;
    int width;
    int height;
};

class CustomPlayerMiniInfo {
public:
    // Singleton access
    static CustomPlayerMiniInfo& Instance();
    
    // Initialization (registers OnEndScene hook for independent rendering)
    bool Initialize();
    
    // Texture management
    bool LoadTextures();
    void ReleaseTextures();
    void OnDeviceLost();
    void OnDeviceReset();
    
    // ImGui window interface
    void Render();
    void MenuItem();
    
    // Control methods
    void Enable();
    void Disable();
    bool IsEnabled() const;
    
    // Position/Size
    void SetPosition(float x, float y);
    void GetPosition(float& x, float& y) const;
    
private:
    CustomPlayerMiniInfo();
    
    // Render sub-components (using CICPlayerEcsro for ECSRO HP/MP compatibility)
    void RenderHPBar(CICPlayerEcsro* pPlayer);
    void RenderMPBar(CICPlayerEcsro* pPlayer);
    void RenderHwanBar(CICPlayerEcsro* pPlayer);
    void RenderLevelInfo(CICPlayerEcsro* pPlayer);
    void RenderZerkPoints(CIFPlayerMiniInfo* pMiniInfo);
    void RenderStatsPanel(CICPlayerEcsro* pPlayer);
    void RenderStatsPopup(CICPlayerEcsro* pPlayer);  // Full stats popup window
    
    // Original GUI control
    void HideOriginalGUI();
    void ShowOriginalGUI();
    CIFPlayerMiniInfo* GetOriginalPlayerMiniInfo();
    
    // State
    bool m_bShow;
    bool m_bEnabled;
    bool m_bShowStats;
    bool m_bShowStatsPopup;  // Full stats popup visible
    bool m_bTexturesLoaded;
    
    // Position
    float m_fPosX;
    float m_fPosY;
    
    // Animation state
    float m_fAnimatedHP;
    float m_fAnimatedMP;
    float m_fAnimationSpeed;
    
    // Colors
    CustomPlayerMiniInfoColors m_colors;
    
    // Textures (from Media.pk2/newui/playerminiinfo/)
    TextureInfo m_texBackground;   // mainbackground.ddj - base layer
    TextureInfo m_texPortraitBg;   // portraitbg.ddj - on top of bars
    TextureInfo m_texPortraitFrame; // portraitframe.ddj - top layer
    TextureInfo m_texLevelFrame;   // levelframe.ddj - behind level text
    TextureInfo m_texHpFill;       // hpbar_health.ddj
    TextureInfo m_texMpFill;       // hpbar_mana.ddj
    TextureInfo m_texHwanFill;     // hpbar_hwan.ddj
    TextureInfo m_texCharIcon;     // icon_character.ddj - character stats button
    TextureInfo m_texStatIcon;      // icon_stat.ddj - stat point reset button
    TextureInfo m_texHwanIcon;      // icon_hwan.ddj - shows when hwan = 5
    TextureInfo m_texStatsPopupBg;  // window_decorated.ddj - stats popup background
    TextureInfo m_texTitleBar;      // title_standart_1.ddj - title bar
    // Exit button states
    TextureInfo m_texExitBtnNormal;  // buttonexit_standart_normal.ddj
    TextureInfo m_texExitBtnHover;   // buttonexit_standart_hovered.ddj
    TextureInfo m_texExitBtnPressed; // buttonexit_standart_pressed.ddj
    
    // Cached pointers
    CIFPlayerMiniInfo* m_pCachedMiniInfo;
    
    // DirectX Native Sprite Renderer (for original SRO quality)
    ID3DXSprite* m_pSprite;
    
    // === Animation State Variables ===
    // Smooth bar animation (lerp)
    float m_displayHP;           // Current displayed HP (lerps to actual HP)
    float m_displayMP;           // Current displayed MP (lerps to actual MP)
    
    // Damage flash effect
    int m_lastHP;                // Previous HP value (to detect damage)
    float m_damageFlashTimer;    // Timer for red flash effect (0 = no flash)
    
    // Native sprite rendering methods
    void InitNativeSprite(IDirect3DDevice9* pDevice);
    void ReleaseNativeSprite();
    void RenderNativeSprite(IDirect3DTexture9* pTexture, float x, float y, float w, float h, D3DCOLOR color = 0xFFFFFFFF);
    void RenderNativeSpriteUV(IDirect3DTexture9* pTexture, float x, float y, float w, float h, float uvMaxX, D3DCOLOR color = 0xFFFFFFFF);
};

