#include "IFRankingWindow.h"
#include "GInterface.h"
#include "Game.h"
#include "IFNormalTile.h"
#include "IFDecoratedStatic.h"
#include "IFCloseButton.h"
#include "IFStatic.h"
#include "../../../DevKit_DLL/src/WindowedMode.h"  // For browser position callback during drag
#include <stdio.h>
#include <windows.h>

// Global frame pointer
CIFMainFrame* RankingWindowMainFrame = NULL;

// Static pointer for callback access
static CIFRankingWindow* g_pRankingWindow = NULL;

// Main window handle
extern HWND g_hMainWnd;

// D3D9 Viewport Resolution
extern int g_D3DViewportWidth;
extern int g_D3DViewportHeight;

GFX_IMPLEMENT_DYNCREATE(CIFRankingWindow, CIFMainFrame)
GFX_BEGIN_MESSAGE_MAP(CIFRankingWindow, CIFMainFrame)
    ONG_COMMAND(2004, &CIFRankingWindow::OnClick_CustomClose)
GFX_END_MESSAGE_MAP()

// Debug log - DISABLED
static void BrowserLog(const char* format, ...)
{
    // Logging disabled for release
    return;
}

// Static callback for WindowedMode WndProc - called during drag events
static void BrowserPositionUpdateCallback() {
    if (g_pRankingWindow && g_pRankingWindow->IsBrowserOpen()) {
        g_pRankingWindow->UpdateBrowserPosition();
    }
}

CIFRankingWindow::CIFRankingWindow(void) {
    g_pRankingWindow = this;  // Store for callback access
    m_browserUrl = "http://18.220.228.24/ranking.php";
    m_browserOpen = false;
    m_browserX = 10;       // Left padding
    m_browserY = 35;       // Below title bar
    m_browserWidth = 830;  // Reduced to leave room for corner DDJ
    m_browserHeight = 591; // Reduced to leave room
    m_lastFrameX = 0;
    m_lastFrameY = 0;
    m_pBackgroundTile = NULL;
    m_pCornerDecor = NULL;
    m_pNewCloseBtn = NULL;
    
    BrowserLog("Constructor called");
}

CIFRankingWindow::~CIFRankingWindow(void) {
    BrowserLog("Destructor called - closing browser");
    m_browser.Close();
    RankingWindowMainFrame = NULL;
}

bool CIFRankingWindow::OnCreate(long ln) {
    BrowserLog("OnCreate starting");
    
    if (!g_pCGInterface) {
        BrowserLog("ERROR: g_pCGInterface is NULL");
        return false;
    }
    
    // Browser window size (850x650)
    RECT BrowserRect = { 50, 50, 850, 650 };
    
    RankingWindowMainFrame = (CIFMainFrame*)CGWnd::CreateInstance(g_pCGInterface, GFX_RUNTIME_CLASS(CIFMainFrame), BrowserRect, 2001, 0);
    
    if (!RankingWindowMainFrame) {
        BrowserLog("ERROR: Failed to create MainFrame");
        return false;
    }
    
    // NOTE: CreateInstance already calls OnCreate internally!
    RankingWindowMainFrame->SetText("Web Rank");
    RankingWindowMainFrame->TB_Func_12("interface\\frame\\newgui_sub_wnd_", 1, 0);
    RankingWindowMainFrame->SetGWndSize(850, 650);
    
    wnd_rect sz;
    
    // Right-top corner decoration - CREATED FIRST so it's at the BACK
    // Child of RankingWindowMainFrame, positioned with negative Y
    sz.pos.x = 850 - 251;  // Right-aligned (window width - DDJ width)
    sz.pos.y = -26;        // Negative Y = above the window's top edge
    sz.size.width = 251;
    sz.size.height = 47;
    
    m_pCornerDecor = static_cast<CIFDecoratedStatic*>(CGWnd::CreateInstance(RankingWindowMainFrame, GFX_RUNTIME_CLASS(CIFDecoratedStatic), sz, 2003, 0));
    if (m_pCornerDecor) {
        *reinterpret_cast<DWORD*>(reinterpret_cast<char*>(m_pCornerDecor) + 0x6C) = 0x0093DA88;
        m_pCornerDecor->TB_Func_12("interface\\frame\\newgui_sub3_wnd_right_up.ddj", 0, 0);
        m_pCornerDecor->SetGWndSize(251, 47);
        BrowserLog("Corner decoration created");
    }
    
    // Hide the original close button (it's clipped to frame bounds)
    CIFCloseButton* pOrigCloseBtn = *reinterpret_cast<CIFCloseButton**>(reinterpret_cast<char*>(RankingWindowMainFrame) + 0x680);
    if (pOrigCloseBtn) {
        pOrigCloseBtn->ShowGWnd(false);
        BrowserLog("Original close button hidden");
    }
    
    // Hide original title text and create new one in decoration area
    CIFStatic* pOrigTitleText = *reinterpret_cast<CIFStatic**>(reinterpret_cast<char*>(RankingWindowMainFrame) + 0x678);
    if (pOrigTitleText) {
        pOrigTitleText->ShowGWnd(false);
        BrowserLog("Original title text hidden");
    }
    
    // Create new title text in decoration area (left of close button)
    {
        wnd_rect txtSz;
        txtSz.pos.x = 599 + 80;    // Decoration X + offset
        txtSz.pos.y = -26 + 5;    // Decoration Y + offset
        txtSz.size.width = 150;    // Width for text
        txtSz.size.height = 20;
        
        m_pNewTitleText = static_cast<CIFStatic*>(CGWnd::CreateInstance(RankingWindowMainFrame, GFX_RUNTIME_CLASS(CIFStatic), txtSz, 2005, 0));
        if (m_pNewTitleText) {
            m_pNewTitleText->SetText("Web Rank");
            m_pNewTitleText->SetTextColor(D3DCOLOR_XRGB(233, 208, 148));  // Gold color
            m_pNewTitleText->BringToFront();
            BrowserLog("New title text created in decoration area");
        }
    }
    
    // Create new close button as child of FRAME (so message handler works)
    // Position it in decoration area (decoration is at X=599, Y=-26)
    {
        wnd_rect btnSz;
        btnSz.pos.x = 599 + 225;   // Decoration X + button offset within decoration
        btnSz.pos.y = -26 + 5;    // Decoration Y + button offset = -15
        btnSz.size.width = 20;
        btnSz.size.height = 20;
        
        m_pNewCloseBtn = static_cast<CIFCloseButton*>(CGWnd::CreateInstance(RankingWindowMainFrame, GFX_RUNTIME_CLASS(CIFCloseButton), btnSz, 2, 0));  // ID 2 = same as original close button
        if (m_pNewCloseBtn) {
            m_pNewCloseBtn->BringToFront();
            BrowserLog("New close button created at frame-relative position %d, %d", btnSz.pos.x, btnSz.pos.y);
        }
    }
    
    // Background tile for browser area - created AFTER corner so it's on top
    sz.pos.x = m_browserX;
    sz.pos.y = m_browserY;
    sz.size.width = m_browserWidth;
    sz.size.height = m_browserHeight;
    
    m_pBackgroundTile = static_cast<CIFNormalTile*>(CGWnd::CreateInstance(RankingWindowMainFrame, GFX_RUNTIME_CLASS(CIFNormalTile), sz, 2002, 0));
    if (m_pBackgroundTile) {
        m_pBackgroundTile->TB_Func_12("interface\\frame\\rankbg.ddj", 1, 0);
        m_pBackgroundTile->SetGWndSize(m_browserWidth, m_browserHeight);
        BrowserLog("Background tile created");
    }
    
    // Hide window by default - browser NOT pre-initialized
    RankingWindowMainFrame->ShowGWnd(false);
    
    BrowserLog("OnCreate completed");
    return true;
}

int CIFRankingWindow::OnMouseMove(int a1, int x, int y) {
    return 0;
}

void CIFRankingWindow::OnUpdate() {
    if (!RankingWindowMainFrame) return;
    
    // Resolution-based resize is DISABLED to prevent input blocking issues
    // The window will maintain its initial 850x650 size at all resolutions
    
    bool isVisible = RankingWindowMainFrame->IsVisible();
    static bool wasVisible = false;
    
    // Detect when window becomes hidden (X button clicked)
    if (wasVisible && !isVisible) {
        BrowserLog("Window closed via X button - destroying browser");
        m_browser.Close();  // Full destroy
        m_browserOpen = false;
        // Corner decoration is child of frame, hides automatically
    }
    
    // If window is visible and browser is open, update position if needed
    if (isVisible && m_browserOpen) {
        UpdateBrowserPosition();
    }
    
    wasVisible = isVisible;
}

void CIFRankingWindow::ToggleBrowser() {
    if (!RankingWindowMainFrame) {
        BrowserLog("ERROR: RankingWindowMainFrame is NULL");
        return;
    }
    
    BrowserLog("Toggle called, visible=%d, browserOpen=%d", 
               RankingWindowMainFrame->IsVisible(), m_browserOpen);
    
    if (RankingWindowMainFrame->IsVisible()) {
        // Close window and destroy browser
        BrowserLog("Closing window - destroying browser");
        m_browser.Close();  // Full destroy
        m_browserOpen = false;
        
        // Hide corner decoration
        if (m_pCornerDecor) {
            m_pCornerDecor->ShowGWnd(false);
        }
        
        RankingWindowMainFrame->OnClick_Exit();
    } else {
        // Open window
        BrowserLog("Opening window");
        
        // Center window on screen
        int screenWidth = g_D3DViewportWidth > 0 ? g_D3DViewportWidth : 1024;
        int screenHeight = g_D3DViewportHeight > 0 ? g_D3DViewportHeight : 768;
        int posX = (screenWidth - 850) / 2;
        int posY = (screenHeight - 650) / 2;
        RankingWindowMainFrame->MoveGWnd(posX, posY);
        
        // Store position
        m_lastFrameX = posX;
        m_lastFrameY = posY;
        
        // Show window
        RankingWindowMainFrame->ShowGWnd(true);
        
        // Explicitly show corner decoration (OnClick_Exit may have hidden it)
        if (m_pCornerDecor) {
            m_pCornerDecor->ShowGWnd(true);
        }
        
        // Start browser (will load on top of background)
        StartBrowser();
    }
}

void CIFRankingWindow::StartBrowser() {
    if (m_browserOpen) return;
    
    if (!RankingWindowMainFrame) return;
    
    // Get frame position
    int frameX = *reinterpret_cast<int*>(reinterpret_cast<char*>(RankingWindowMainFrame) + 0x3C);
    int frameY = *reinterpret_cast<int*>(reinterpret_cast<char*>(RankingWindowMainFrame) + 0x40);
    
    // Browser is child of game window - use client-relative coordinates
    int browserClientX = frameX + m_browserX;
    int browserClientY = frameY + m_browserY;
    
    BrowserLog("Starting browser at (%d, %d)", browserClientX, browserClientY);
    m_browser.Start(browserClientX, browserClientY, m_browserWidth, m_browserHeight, m_browserUrl);
    m_browserOpen = true;
    
    // Register callback for real-time position updates during drag
    WindowedMode::RegisterBrowserPositionCallback(BrowserPositionUpdateCallback);
    BrowserLog("Registered drag callback for real-time position updates");
    
    m_lastFrameX = frameX;
    m_lastFrameY = frameY;
}

void CIFRankingWindow::CloseBrowser() {
    if (!m_browserOpen) return;
    
    BrowserLog("Closing browser");
    
    // Unregister callback
    WindowedMode::UnregisterBrowserPositionCallback();
    BrowserLog("Unregistered drag callback");
    
    m_browser.Close();
    m_browserOpen = false;
}

void CIFRankingWindow::UpdateBrowserPosition() {
    if (!RankingWindowMainFrame || !m_browserOpen) return;
    
    // Get current frame position
    int frameX = *reinterpret_cast<int*>(reinterpret_cast<char*>(RankingWindowMainFrame) + 0x3C);
    int frameY = *reinterpret_cast<int*>(reinterpret_cast<char*>(RankingWindowMainFrame) + 0x40);
    
    // Update immediately on ANY position change (no threshold - instant response)
    if (frameX != m_lastFrameX || frameY != m_lastFrameY) {
        int browserClientX = frameX + m_browserX;
        int browserClientY = frameY + m_browserY;
        
        m_browser.Update(browserClientX, browserClientY);
        
        m_lastFrameX = frameX;
        m_lastFrameY = frameY;
    }
}

void CIFRankingWindow::OnClick_CustomClose() {
    BrowserLog("Custom close button clicked!");
    ToggleBrowser();  // This will close the window
}
