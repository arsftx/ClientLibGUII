#include "SilkRoadWeb.h"
#include <stdio.h>
#include <stdarg.h>
#include <GFX3DFunction/GFXVideo3d.h>  // For g_CD3DApplication->m_hWnd

// Include WebView2 header
#include <WebView2.h>

// Manual ICoreWebView2Settings interface definition
#ifndef __ICoreWebView2Settings_INTERFACE_DEFINED__
#define __ICoreWebView2Settings_INTERFACE_DEFINED__
MIDL_INTERFACE("e562e4f0-d7fa-43ac-8d71-c05150499f00")
ICoreWebView2Settings : public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE get_IsScriptEnabled(BOOL* isScriptEnabled) = 0;
    virtual HRESULT STDMETHODCALLTYPE put_IsScriptEnabled(BOOL isScriptEnabled) = 0;
    virtual HRESULT STDMETHODCALLTYPE get_IsWebMessageEnabled(BOOL* isWebMessageEnabled) = 0;
    virtual HRESULT STDMETHODCALLTYPE put_IsWebMessageEnabled(BOOL isWebMessageEnabled) = 0;
    virtual HRESULT STDMETHODCALLTYPE get_AreDefaultScriptDialogsEnabled(BOOL* areDefaultScriptDialogsEnabled) = 0;
    virtual HRESULT STDMETHODCALLTYPE put_AreDefaultScriptDialogsEnabled(BOOL areDefaultScriptDialogsEnabled) = 0;
    virtual HRESULT STDMETHODCALLTYPE get_IsStatusBarEnabled(BOOL* isStatusBarEnabled) = 0;
    virtual HRESULT STDMETHODCALLTYPE put_IsStatusBarEnabled(BOOL isStatusBarEnabled) = 0;
    virtual HRESULT STDMETHODCALLTYPE get_AreDevToolsEnabled(BOOL* areDevToolsEnabled) = 0;
    virtual HRESULT STDMETHODCALLTYPE put_AreDevToolsEnabled(BOOL areDevToolsEnabled) = 0;
    virtual HRESULT STDMETHODCALLTYPE get_AreDefaultContextMenusEnabled(BOOL* enabled) = 0;
    virtual HRESULT STDMETHODCALLTYPE put_AreDefaultContextMenusEnabled(BOOL enabled) = 0;
    virtual HRESULT STDMETHODCALLTYPE get_AreHostObjectsAllowed(BOOL* allowed) = 0;
    virtual HRESULT STDMETHODCALLTYPE put_AreHostObjectsAllowed(BOOL allowed) = 0;
    virtual HRESULT STDMETHODCALLTYPE get_IsZoomControlEnabled(BOOL* enabled) = 0;
    virtual HRESULT STDMETHODCALLTYPE put_IsZoomControlEnabled(BOOL enabled) = 0;
    virtual HRESULT STDMETHODCALLTYPE get_IsBuiltInErrorPageEnabled(BOOL* enabled) = 0;
    virtual HRESULT STDMETHODCALLTYPE put_IsBuiltInErrorPageEnabled(BOOL enabled) = 0;
};
#endif

// External game window handle from WindowedMode.cpp
extern HWND g_hMainWnd;

// Global browser bounds for WndProc filtering
RECT g_BrowserBounds;
bool g_BrowserIsActive = false;

// SRO UI system - global UI tracking pointers (from IDA sub_89C560 analysis)
// All these need to be cleared when browser closes to prevent input blocking
static void** g_pUIGlobal_C5DE40 = (void**)0x00C5DE40;  // Unknown UI pointer 1
static void** g_pUIGlobal_C5DE44 = (void**)0x00C5DE44;  // Unknown UI pointer 2
static void** g_pUIGlobal_C5DE48 = (void**)0x00C5DE48;  // Unknown UI pointer 3
static void** g_pCurrentIfUnderCursor = (void**)0x00C5DE4C;  // Cursor under UI element

// Game's main app/window object (used by WndProc sub_8A45F0)
static DWORD* g_pGameAppObject = (DWORD*)0x00C5DCE8;

// sub_89CA10 - Sets the current UI element under cursor
// This function properly sends mouse leave (0x400A) event to old element
// and mouse enter (0x4009) event to new element before updating the pointer
// Calling with NULL will clear the focus properly
typedef int(__cdecl* SetCurrentUIUnderCursorFunc)(void* pUIElement);
static SetCurrentUIUnderCursorFunc g_pfnSetCurrentUIUnderCursor = (SetCurrentUIUnderCursorFunc)0x0089CA10;

// Game's input processing function at this+0x24 uses sub_8A4B30/sub_8A4B40
// We'll use a simpler approach: send fake mouse messages to force state reset

// Function pointer types for calling game's input processing
typedef void(__thiscall* GameInputQueueFunc)(void* thisPtr, void* msgData);
static GameInputQueueFunc g_pfnInputQueuePush = (GameInputQueueFunc)0x008A4B30;
static GameInputQueueFunc g_pfnInputQueueProcess = (GameInputQueueFunc)0x008A4B40;

// Debug log helper - DISABLED
static void WebLog(const char* format, ...)
{
    // Logging disabled for release
    return;
}

// Helper to convert std::string to std::wstring
static std::wstring StringToWide(const std::string& str)
{
    if (str.empty()) return L"";
    int size = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), NULL, 0);
    std::wstring wstr(size, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), &wstr[0], size);
    return wstr;
}

// Helper function to get game window
static HWND GetGameWindow()
{
    // First check cached handle
    if (g_hMainWnd && IsWindow(g_hMainWnd))
        return g_hMainWnd;
    
    // Try g_CD3DApplication->m_hWnd (most reliable in fullscreen mode)
    if (g_CD3DApplication && g_CD3DApplication->m_hWnd && IsWindow(g_CD3DApplication->m_hWnd)) {
        g_hMainWnd = g_CD3DApplication->m_hWnd;
        return g_hMainWnd;
    }
    
    // Fallback to FindWindow
    HWND hWnd = FindWindowA("SRO_Client", NULL);
    if (hWnd) {
        g_hMainWnd = hWnd;
        return g_hMainWnd;
    }
    
    hWnd = FindWindowA("SR_CLIENT", NULL);
    if (hWnd) {
        g_hMainWnd = hWnd;
        return g_hMainWnd;
    }
    
    hWnd = FindWindowA(NULL, "Silkroad Online");
    if (hWnd) {
        g_hMainWnd = hWnd;
        return g_hMainWnd;
    }
    
    return NULL;
}

// WebView2Loader.dll function types
typedef HRESULT(STDAPICALLTYPE* CreateCoreWebView2EnvironmentWithOptionsFunc)(
    PCWSTR browserExecutableFolder,
    PCWSTR userDataFolder,
    ICoreWebView2EnvironmentOptions* environmentOptions,
    ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler* environmentCreatedHandler);

typedef HRESULT(STDAPICALLTYPE* GetAvailableCoreWebView2BrowserVersionStringFunc)(
    PCWSTR browserExecutableFolder,
    LPWSTR* versionInfo);

// Load WebView2Loader.dll
static HMODULE LoadWebView2Loader()
{
    HMODULE hModule = LoadLibraryW(L"WebView2Loader.dll");
    if (!hModule)
    {
        wchar_t path[MAX_PATH];
        GetModuleFileNameW(NULL, path, MAX_PATH);
        wchar_t* lastSlash = wcsrchr(path, L'\\');
        if (lastSlash)
        {
            wcscpy(lastSlash + 1, L"WebView2Loader.dll");
            hModule = LoadLibraryW(path);
        }
    }
    return hModule;
}

// ============================================================================
// Environment Created Handler - COM callback
// ============================================================================
class EnvironmentCreatedHandler : public ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler
{
public:
    EnvironmentCreatedHandler(SilkRoadWeb* pOwner) : m_pOwner(pOwner), m_refCount(1) {}
    
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) override
    {
        if (riid == IID_IUnknown || riid == IID_ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler)
        {
            *ppvObject = this;
            AddRef();
            return S_OK;
        }
        *ppvObject = NULL;
        return E_NOINTERFACE;
    }
    
    ULONG STDMETHODCALLTYPE AddRef() override { return InterlockedIncrement(&m_refCount); }
    ULONG STDMETHODCALLTYPE Release() override
    {
        ULONG count = InterlockedDecrement(&m_refCount);
        if (count == 0) delete this;
        return count;
    }
    
    HRESULT STDMETHODCALLTYPE Invoke(HRESULT errorCode, ICoreWebView2Environment* createdEnvironment) override
    {
        if (m_pOwner)
            m_pOwner->OnEnvironmentCreated(errorCode, createdEnvironment);
        return S_OK;
    }
    
private:
    SilkRoadWeb* m_pOwner;
    LONG m_refCount;
};

// ============================================================================
// Controller Created Handler - COM callback
// ============================================================================
class ControllerCreatedHandler : public ICoreWebView2CreateCoreWebView2ControllerCompletedHandler
{
public:
    ControllerCreatedHandler(SilkRoadWeb* pOwner) : m_pOwner(pOwner), m_refCount(1) {}
    
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) override
    {
        if (riid == IID_IUnknown || riid == IID_ICoreWebView2CreateCoreWebView2ControllerCompletedHandler)
        {
            *ppvObject = this;
            AddRef();
            return S_OK;
        }
        *ppvObject = NULL;
        return E_NOINTERFACE;
    }
    
    ULONG STDMETHODCALLTYPE AddRef() override { return InterlockedIncrement(&m_refCount); }
    ULONG STDMETHODCALLTYPE Release() override
    {
        ULONG count = InterlockedDecrement(&m_refCount);
        if (count == 0) delete this;
        return count;
    }
    
    HRESULT STDMETHODCALLTYPE Invoke(HRESULT errorCode, ICoreWebView2Controller* createdController) override
    {
        if (m_pOwner)
            m_pOwner->OnControllerCreated(errorCode, createdController);
        return S_OK;
    }
    
private:
    SilkRoadWeb* m_pOwner;
    LONG m_refCount;
};

// ============================================================================
// SilkRoadWeb Implementation
// ============================================================================

SilkRoadWeb::SilkRoadWeb()
    : m_hBrowserWnd(NULL)
    , m_Width(0)
    , m_Height(0)
    , m_X(0)
    , m_Y(0)
    , m_bVisible(false)
    , m_pEnvironment(NULL)
    , m_pController(NULL)
    , m_pWebView(NULL)
    , m_hWebView2Loader(NULL)
    , m_bInitializing(false)
{
}

SilkRoadWeb::~SilkRoadWeb()
{
    // Ensure browser is marked as closed
    g_BrowserIsActive = false;
    g_BrowserBounds.left = 0;
    g_BrowserBounds.top = 0;
    g_BrowserBounds.right = 0;
    g_BrowserBounds.bottom = 0;
    
    if (m_pController)
    {
        m_pController->Close();
        m_pController->Release();
        m_pController = NULL;
    }
    if (m_pWebView)
    {
        m_pWebView->Release();
        m_pWebView = NULL;
    }
    if (m_pEnvironment)
    {
        m_pEnvironment->Release();
        m_pEnvironment = NULL;
    }
    if (m_hBrowserWnd && IsWindow(m_hBrowserWnd))
    {
        DestroyWindow(m_hBrowserWnd);
        m_hBrowserWnd = NULL;
    }
    if (m_hWebView2Loader)
    {
        FreeLibrary(m_hWebView2Loader);
        m_hWebView2Loader = NULL;
    }
}

void SilkRoadWeb::UpdateBrowserBounds()
{
    if (m_bVisible)
    {
        g_BrowserBounds.left = m_X;
        g_BrowserBounds.top = m_Y;
        g_BrowserBounds.right = m_X + m_Width;
        g_BrowserBounds.bottom = m_Y + m_Height;
        g_BrowserIsActive = true;
        WebLog("Browser bounds updated: (%d,%d)-(%d,%d)", 
               g_BrowserBounds.left, g_BrowserBounds.top,
               g_BrowserBounds.right, g_BrowserBounds.bottom);
    }
    else
    {
        g_BrowserBounds.left = 0;
        g_BrowserBounds.top = 0;
        g_BrowserBounds.right = 0;
        g_BrowserBounds.bottom = 0;
        g_BrowserIsActive = false;
        WebLog("Browser bounds cleared");
    }
}

bool SilkRoadWeb::IsWebView2Available()
{
    HMODULE hLoader = LoadWebView2Loader();
    if (!hLoader)
    {
        WebLog("WebView2Loader.dll not found");
        return false;
    }
    
    GetAvailableCoreWebView2BrowserVersionStringFunc getVersion = 
        (GetAvailableCoreWebView2BrowserVersionStringFunc)GetProcAddress(hLoader, "GetAvailableCoreWebView2BrowserVersionString");
    
    if (!getVersion)
    {
        WebLog("GetAvailableCoreWebView2BrowserVersionString not found");
        FreeLibrary(hLoader);
        return false;
    }
    
    LPWSTR versionInfo = NULL;
    HRESULT hr = getVersion(NULL, &versionInfo);
    
    if (SUCCEEDED(hr) && versionInfo)
    {
        WebLog("WebView2 Runtime available: %S", versionInfo);
        CoTaskMemFree(versionInfo);
        FreeLibrary(hLoader);
        return true;
    }
    
    WebLog("WebView2 Runtime NOT available");
    FreeLibrary(hLoader);
    return false;
}

bool SilkRoadWeb::CreateBrowserWindow(int X, int Y, int Width, int Height)
{
    HWND hGameWnd = GetGameWindow();
    if (!hGameWnd)
    {
        WebLog("ERROR: Failed to get game window handle");
        return false;
    }
    
    m_hBrowserWnd = CreateWindowExA(
        0,
        "STATIC",
        "WebView2Host",
        WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN,
        X, Y, Width, Height,
        hGameWnd,
        NULL, NULL, NULL
    );
    
    if (!m_hBrowserWnd)
    {
        WebLog("ERROR: Failed to create browser window");
        return false;
    }
    
    WebLog("Browser window created at (%d, %d) size (%d x %d)", X, Y, Width, Height);
    return true;
}

bool SilkRoadWeb::InitializeWebView2()
{
    if (m_bInitializing)
        return true;
    
    m_hWebView2Loader = LoadWebView2Loader();
    if (!m_hWebView2Loader)
    {
        WebLog("ERROR: Failed to load WebView2Loader.dll");
        return false;
    }
    
    CreateCoreWebView2EnvironmentWithOptionsFunc createEnv = 
        (CreateCoreWebView2EnvironmentWithOptionsFunc)GetProcAddress(m_hWebView2Loader, "CreateCoreWebView2EnvironmentWithOptions");
    
    if (!createEnv)
    {
        WebLog("ERROR: CreateCoreWebView2EnvironmentWithOptions not found");
        FreeLibrary(m_hWebView2Loader);
        m_hWebView2Loader = NULL;
        return false;
    }
    
    wchar_t userDataPath[MAX_PATH];
    GetTempPathW(MAX_PATH, userDataPath);
    wcscat(userDataPath, L"SilkroadWebView2");
    
    EnvironmentCreatedHandler* handler = new EnvironmentCreatedHandler(this);
    HRESULT hr = createEnv(NULL, userDataPath, NULL, handler);
    
    if (FAILED(hr))
    {
        WebLog("ERROR: CreateCoreWebView2EnvironmentWithOptions failed");
        handler->Release();
        FreeLibrary(m_hWebView2Loader);
        m_hWebView2Loader = NULL;
        return false;
    }
    
    m_bInitializing = true;
    WebLog("WebView2 environment creation initiated");
    return true;
}

void SilkRoadWeb::OnEnvironmentCreated(HRESULT hr, ICoreWebView2Environment* env)
{
    if (FAILED(hr) || !env)
    {
        WebLog("ERROR: Environment creation failed");
        m_bInitializing = false;
        return;
    }
    
    WebLog("Environment created successfully");
    m_pEnvironment = env;
    m_pEnvironment->AddRef();
    
    ControllerCreatedHandler* handler = new ControllerCreatedHandler(this);
    m_pEnvironment->CreateCoreWebView2Controller(m_hBrowserWnd, handler);
}

void SilkRoadWeb::OnControllerCreated(HRESULT hr, ICoreWebView2Controller* controller)
{
    m_bInitializing = false;
    
    if (FAILED(hr) || !controller)
    {
        WebLog("ERROR: Controller creation failed");
        return;
    }
    
    WebLog("Controller created successfully");
    m_pController = controller;
    m_pController->AddRef();
    
    m_pController->get_CoreWebView2(&m_pWebView);
    
    // Configure WebView2 settings - HIDE STATUS BAR
    if (m_pWebView)
    {
        ICoreWebView2Settings* settings = NULL;
        if (SUCCEEDED(m_pWebView->get_Settings(&settings)) && settings)
        {
            settings->put_IsStatusBarEnabled(FALSE);  // Hide URL tooltip
            settings->put_AreDefaultContextMenusEnabled(FALSE);  // Disable right-click
            settings->put_AreDevToolsEnabled(FALSE);  // Disable F12
            WebLog("WebView2 settings: status bar HIDDEN");
            settings->Release();
        }
    }
    
    RECT bounds = {0, 0, m_Width, m_Height};
    m_pController->put_Bounds(bounds);
    m_pController->put_IsVisible(TRUE);
    
    if (!m_pendingUrl.empty())
    {
        Navigate(m_pendingUrl);
        m_pendingUrl.clear();
    }
    
    WebLog("WebView2 fully initialized");
}

void SilkRoadWeb::Start(int X, int Y, int Width, int Height, const std::string& url)
{
    WebLog("Start called: pos=(%d, %d), size=(%d x %d)", X, Y, Width, Height);
    
    m_X = X;
    m_Y = Y;
    m_Width = Width;
    m_Height = Height;
    
    // If already created, just show
    if (m_hBrowserWnd && IsWindow(m_hBrowserWnd))
    {
        SetWindowPos(m_hBrowserWnd, NULL, X, Y, Width, Height, SWP_SHOWWINDOW | SWP_NOZORDER);
        if (m_pController)
        {
            RECT bounds = {0, 0, Width, Height};
            m_pController->put_Bounds(bounds);
            m_pController->put_IsVisible(TRUE);
        }
        m_bVisible = true;
        UpdateBrowserBounds();  // Update global bounds
        if (!url.empty())
            Navigate(url);
        return;
    }
    
    m_pendingUrl = url;
    
    if (!CreateBrowserWindow(X, Y, Width, Height))
    {
        WebLog("ERROR: Failed to create browser window");
        return;
    }
    
    if (!InitializeWebView2())
    {
        WebLog("ERROR: Failed to initialize WebView2");
        DestroyWindow(m_hBrowserWnd);
        m_hBrowserWnd = NULL;
        return;
    }
    
    m_bVisible = true;
    UpdateBrowserBounds();  // Update global bounds
    ShowWindow(m_hBrowserWnd, SW_SHOW);
    UpdateWindow(m_hBrowserWnd);
}

void SilkRoadWeb::Update(int X, int Y)
{
    if (m_hBrowserWnd && IsWindow(m_hBrowserWnd) && m_bVisible)
    {
        m_X = X;
        m_Y = Y;
        SetWindowPos(m_hBrowserWnd, NULL, X, Y, m_Width, m_Height, SWP_NOZORDER);
        UpdateBrowserBounds();  // Update global bounds
        
        if (m_pController)
            m_pController->NotifyParentWindowPositionChanged();
    }
}

void SilkRoadWeb::Navigate(const std::string& url)
{
    if (!m_pWebView)
    {
        m_pendingUrl = url;
        return;
    }
    
    std::wstring wurl = StringToWide(url);
    m_pWebView->Navigate(wurl.c_str());
    WebLog("Navigating to: %s", url.c_str());
}

void SilkRoadWeb::Close()
{
    if (!m_bVisible)
        return;
    
    WebLog("Close() called - AGGRESSIVE CLEANUP starting");
    m_bVisible = false;
    
    // ============================================================
    // STEP 1: Clear global bounds FIRST - tells WndProc to stop filtering
    // ============================================================
    g_BrowserIsActive = false;
    g_BrowserBounds.left = 0;
    g_BrowserBounds.top = 0;
    g_BrowserBounds.right = 0;
    g_BrowserBounds.bottom = 0;
    WebLog("Browser bounds cleared - WndProc filter disabled");
    
    // ============================================================
    // STEP 2: DESTROY WebView2 controller completely
    // This is the key - controller's window is what blocks input
    // ============================================================
    if (m_pController)
    {
        WebLog("Closing WebView2 controller...");
        m_pController->Close();  // This destroys the controller's internal window
        m_pController->Release();
        m_pController = NULL;
        WebLog("WebView2 controller closed and released");
    }
    
    // Release WebView reference
    if (m_pWebView)
    {
        m_pWebView->Release();
        m_pWebView = NULL;
        WebLog("WebView2 webview released");
    }
    
    // ============================================================
    // STEP 3: DESTROY browser host window completely
    // Do not just hide - DESTROY it to remove from window hierarchy
    // ============================================================
    if (m_hBrowserWnd && IsWindow(m_hBrowserWnd))
    {
        WebLog("Destroying browser host window...");
        DestroyWindow(m_hBrowserWnd);
        m_hBrowserWnd = NULL;
        WebLog("Browser host window destroyed");
    }
    
    // ============================================================
    // STEP 4: Reset initialization state
    // ============================================================
    m_bInitializing = false;
    
    // ============================================================
    // STEP 5: Reset ALL SRO UI tracking pointers (from IDA sub_89C560)
    // These are the 4 global pointers that track UI elements
    // ============================================================
    WebLog("Clearing SRO UI state using proper function call...");
    
    // Log current values before clearing
    if (g_pUIGlobal_C5DE40) WebLog("C5DE40 was: 0x%08X", *g_pUIGlobal_C5DE40);
    if (g_pUIGlobal_C5DE44) WebLog("C5DE44 was: 0x%08X", *g_pUIGlobal_C5DE44);
    if (g_pUIGlobal_C5DE48) WebLog("C5DE48 was: 0x%08X", *g_pUIGlobal_C5DE48);
    if (g_pCurrentIfUnderCursor) WebLog("C5DE4C was: 0x%08X", *g_pCurrentIfUnderCursor);
    
    // CRITICAL: Call sub_89CA10(NULL) - this properly sends mouse leave event
    // to the current UI element before clearing the pointer
    if (g_pfnSetCurrentUIUnderCursor) {
        WebLog("Calling sub_89CA10(NULL) to send proper mouse leave event...");
        g_pfnSetCurrentUIUnderCursor(NULL);
        WebLog("sub_89CA10(NULL) called successfully");
    }
    
    // Also clear the other 3 pointers manually (they may not have proper clear functions)
    if (g_pUIGlobal_C5DE40) *g_pUIGlobal_C5DE40 = NULL;
    if (g_pUIGlobal_C5DE44) *g_pUIGlobal_C5DE44 = NULL;
    if (g_pUIGlobal_C5DE48) *g_pUIGlobal_C5DE48 = NULL;
    
    WebLog("All SRO UI pointers cleared");
    
    // ============================================================
    // STEP 6: Release mouse capture and cursor clipping
    // ============================================================
    ReleaseCapture();
    ClipCursor(NULL);
    WebLog("ReleaseCapture and ClipCursor called");
    
    // ============================================================
    // STEP 7: Force game window to recalculate input state
    // ============================================================
    HWND hGameWnd = GetGameWindow();
    if (hGameWnd && IsWindow(hGameWnd))
    {
        // Force invalidate and repaint - this can help clear cached UI regions
        InvalidateRect(hGameWnd, NULL, FALSE);
        
        // Get current mouse position
        POINT cursorPos;
        GetCursorPos(&cursorPos);
        ScreenToClient(hGameWnd, &cursorPos);
        
        // Send fake mouse move to force recalculation
        LPARAM lParam = MAKELPARAM(cursorPos.x, cursorPos.y);
        PostMessage(hGameWnd, WM_MOUSEMOVE, 0, lParam);
        WebLog("Posted WM_MOUSEMOVE to (%d, %d)", cursorPos.x, cursorPos.y);
        
        // Return focus to game
        SetFocus(hGameWnd);
        SetActiveWindow(hGameWnd);
        SetForegroundWindow(hGameWnd);
        WebLog("Focus returned to game window");
    }
    
    WebLog("Close() completed - browser DESTROYED (not hidden)");
}

