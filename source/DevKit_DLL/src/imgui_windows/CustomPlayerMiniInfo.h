#pragma once
/**
 * @file CustomPlayerMiniInfo.h
 * @brief Custom ImGui overlay for player HP/MP/Level display (VS2005 Compatible)
 * 
 * Replaces the original CIFPlayerMiniInfo by hiding it and rendering
 * a custom ImGui-based player info panel.
 * Uses CICPlayerEcsro for HP/MP access (same pattern as working AutoPotion)
 */

#include <ICPlayer.h>
#include <IFPlayerMiniInfo.h>

// Color structure for VS2005 compatibility
struct CustomPlayerMiniInfoColors {
    float hpBar[4];
    float mpBar[4];
    float hpCritical[4];
    float background[4];
    float text[4];
    float levelText[4];
};

class CustomPlayerMiniInfo {
public:
    // Singleton access
    static CustomPlayerMiniInfo& Instance();
    
    // Initialization (registers OnEndScene hook for independent rendering)
    bool Initialize();
    
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
    
    // Position
    float m_fPosX;
    float m_fPosY;
    
    // Animation state
    float m_fAnimatedHP;
    float m_fAnimatedMP;
    float m_fAnimationSpeed;
    
    // Colors
    CustomPlayerMiniInfoColors m_colors;
    
    // Cached pointers
    CIFPlayerMiniInfo* m_pCachedMiniInfo;
};
