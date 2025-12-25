#pragma once

#include "IFMainFrame.h"
#include "SilkRoadWeb.h"
#include "IFNormalTile.h"
#include "IFDecoratedStatic.h"
#include "IFCloseButton.h"
#include "IFStatic.h"
#include <string>

// External global frame pointer
extern CIFMainFrame* RankingWindowMainFrame;

// Browser Window - Clean implementation based on IFMainFrame
class CIFRankingWindow : public CIFMainFrame {
    GFX_DECLARE_DYNCREATE(CIFRankingWindow)
    GFX_DECLARE_MESSAGE_MAP(CIFRankingWindow)
    
public:
    CIFRankingWindow(void);
    ~CIFRankingWindow(void);

    bool OnCreate(long ln) override;
    void OnUpdate() override;
    int OnMouseMove(int a1, int x, int y) override;
    
    // Browser control
    void ToggleBrowser();
    void StartBrowser();
    void CloseBrowser();
    void UpdateBrowserPosition();
    
    // Close button handler
    void OnClick_CustomClose();
    
    // Check if browser is currently open
    bool IsBrowserOpen() const { return m_browserOpen; }

private:
    // Browser
    SilkRoadWeb m_browser;
    std::string m_browserUrl;
    bool m_browserOpen;
    
    // Browser dimensions (relative to frame)
    int m_browserX;
    int m_browserY;
    int m_browserWidth;
    int m_browserHeight;
    
    // Track last window position for move detection
    int m_lastFrameX;
    int m_lastFrameY;
    
    // UI elements
    CIFNormalTile* m_pBackgroundTile;
    CIFDecoratedStatic* m_pCornerDecor;
    CIFCloseButton* m_pNewCloseBtn;
    CIFStatic* m_pNewTitleText;
};
