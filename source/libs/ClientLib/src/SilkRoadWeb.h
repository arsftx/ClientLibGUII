#pragma once
#include <windows.h>
#include <string>

// Forward declarations for WebView2 COM interfaces
class ICoreWebView2;
class ICoreWebView2Controller;
class ICoreWebView2Environment;

// ============================================================================
// Browser bounds export for WndProc hook filtering
// When browser is open, mouse messages in this area should be ignored by game
// ============================================================================
extern RECT g_BrowserBounds;      // Current browser bounds (client coords)
extern bool g_BrowserIsActive;    // Is browser currently active?

// ============================================================================
// SilkRoadWeb - Modern Chromium-based browser using WebView2
// ============================================================================
class SilkRoadWeb
{
public:
    SilkRoadWeb();
    ~SilkRoadWeb();
    
    // Open browser window
    void Start(int X, int Y, int Width, int Height, const std::string& url);
    
    // Update window position
    void Update(int X, int Y);
    
    // Close browser
    void Close();
    
    // Is browser visible?
    bool IsOpen() const { return m_bVisible && m_hBrowserWnd != NULL; }
    
    // Navigate to URL
    void Navigate(const std::string& url);
    
    // Check if WebView2 runtime is available
    static bool IsWebView2Available();
    
    // Callbacks for async WebView2 initialization
    void OnEnvironmentCreated(HRESULT hr, ICoreWebView2Environment* env);
    void OnControllerCreated(HRESULT hr, ICoreWebView2Controller* controller);
    
private:
    // Child window for browser
    HWND m_hBrowserWnd;
    
    // Dimensions and position
    int m_Width;
    int m_Height;
    int m_X;
    int m_Y;
    
    // Visibility state
    bool m_bVisible;
    
    // URL to navigate after initialization
    std::string m_pendingUrl;
    
    // WebView2 components
    ICoreWebView2Environment* m_pEnvironment;
    ICoreWebView2Controller* m_pController;
    ICoreWebView2* m_pWebView;
    
    // WebView2Loader.dll handle
    HMODULE m_hWebView2Loader;
    
    // Initialization state
    bool m_bInitializing;
    
    // Initialize WebView2
    bool InitializeWebView2();
    
    // Create browser child window
    bool CreateBrowserWindow(int X, int Y, int Width, int Height);
    
    // Update global browser bounds for WndProc filtering
    void UpdateBrowserBounds();
};
