#include "WindowedMode.h"
#include "hooks/Hooks.h"
#include <GFX3DFunction/GFXVideo3d.h>// For g_CD3DApplication and ToggleFullscreen
#include <stdarg.h>
#include <stdio.h>
#include <time.h>
#include <windows.h>
#include <windowsx.h>

// Log function for WindowedMode - DISABLED
static void WModeLog(const char *format, ...) {
    // Logging disabled for release
    return;
}

typedef LONG(WINAPI *ChangeDisplaySettingsA_t)(DEVMODEA *, DWORD);
typedef HWND(WINAPI *CreateWindowExA_t)(DWORD, LPCSTR, LPCSTR, DWORD, int, int, int, int, HWND, HMENU, HINSTANCE, LPVOID);
typedef HWND(WINAPI *CreateWindowExW_t)(DWORD, LPCWSTR, LPCWSTR, DWORD, int, int, int, int, HWND, HMENU, HINSTANCE, LPVOID);
typedef LONG(WINAPI *SetWindowLongA_t)(HWND, int, LONG);
typedef LONG(WINAPI *SetWindowLongW_t)(HWND, int, LONG);
typedef BOOL(WINAPI *MoveWindow_t)(HWND, int, int, int, int, BOOL);
typedef BOOL(WINAPI *GetCursorPos_t)(LPPOINT);
typedef BOOL(WINAPI *SetCursorPos_t)(int, int);
typedef BOOL(WINAPI *ScreenToClient_t)(HWND, LPPOINT);
typedef BOOL(WINAPI *ClientToScreen_t)(HWND, LPPOINT);
typedef BOOL(WINAPI *GetClientRect_t)(HWND, LPRECT);
typedef BOOL(WINAPI *GetWindowRect_t)(HWND, LPRECT);
typedef BOOL(WINAPI *AdjustWindowRect_t)(LPRECT, DWORD, BOOL);
typedef BOOL(WINAPI *AdjustWindowRectEx_t)(LPRECT, DWORD, BOOL, DWORD);

ChangeDisplaySettingsA_t Original_ChangeDisplaySettingsA = NULL;
CreateWindowExA_t Original_CreateWindowExA = NULL;
CreateWindowExW_t Original_CreateWindowExW = NULL;
SetWindowLongA_t Original_SetWindowLongA = NULL;
SetWindowLongW_t Original_SetWindowLongW = NULL;
MoveWindow_t Original_MoveWindow = NULL;
GetCursorPos_t Original_GetCursorPos = NULL;
SetCursorPos_t Original_SetCursorPos = NULL;
ScreenToClient_t Original_ScreenToClient = NULL;
ClientToScreen_t Original_ClientToScreen = NULL;
GetClientRect_t Original_GetClientRect = NULL;
GetWindowRect_t Original_GetWindowRect = NULL;
AdjustWindowRect_t Original_AdjustWindowRect = NULL;
AdjustWindowRectEx_t Original_AdjustWindowRectEx = NULL;

HWND g_hMainWnd = NULL;
WNDPROC g_OriginalWndProc = NULL;
int g_GameWidth = 1024, g_GameHeight = 768;

#define FIXED_STYLE (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_VISIBLE)
#define FIXED_EXSTYLE (WS_EX_APPWINDOW)

// Runtime toggle for windowed mode
// When true, hooks force windowed mode
// When false, hooks pass through to original functions (fullscreen)
bool g_WindowedModeEnabled = true;

// Browser position update callback - called during drag events
static WindowedMode::BrowserPositionCallback g_BrowserPositionCallback = NULL;

// Forward declaration
void SwitchToFullscreen();
void SwitchToWindowed();

// Called when checkbox state changes
void WindowedMode_SetEnabled(bool enabled) {
    WModeLog("SetEnabled called: enabled=%s, current g_WindowedModeEnabled=%s",
             enabled ? "true" : "false",
             g_WindowedModeEnabled ? "true" : "false");

    if (g_WindowedModeEnabled == enabled) {
        WModeLog("SetEnabled: No change needed, returning");
        return;
    }

    g_WindowedModeEnabled = enabled;
    WModeLog("SetEnabled: g_WindowedModeEnabled changed to %s", enabled ? "true" : "false");

    if (enabled) {
        WModeLog("SetEnabled: Calling SwitchToWindowed()");
        SwitchToWindowed();
    } else {
        WModeLog("SetEnabled: Calling SwitchToFullscreen()");
        SwitchToFullscreen();
    }
}

bool WindowedMode_IsEnabled() {
    return g_WindowedModeEnabled;
}


void *SimpleDetour(void *target, void *detour) {
    if (!target || !detour) return NULL;
    DWORD old, old2;
    if (!VirtualProtect(target, 5, PAGE_EXECUTE_READWRITE, &old)) return NULL;
    void *tramp = VirtualAlloc(NULL, 25, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!tramp) {
        VirtualProtect(target, 5, old, &old2);
        return NULL;
    }
    memcpy(tramp, target, 5);
    *((BYTE *) tramp + 5) = 0xE9;
    *(DWORD *) ((BYTE *) tramp + 6) = (DWORD) target + 5 - ((DWORD) tramp + 5 + 5);
    *(BYTE *) target = 0xE9;
    *(DWORD *) ((BYTE *) target + 1) = (DWORD) detour - ((DWORD) target + 5);
    VirtualProtect(target, 5, old, &old2);
    return tramp;
}

// Check if window belongs to WebView2 (should NOT be treated as game window)
// WebView2 uses Chrome-based window classes that need real coordinate transformation
bool IsWebView2Window(HWND hWnd) {
    if (!hWnd) return false;
    char className[256] = {0};
    if (GetClassNameA(hWnd, className, sizeof(className)) > 0) {
        // WebView2 uses Chrome-based window classes
        if (strstr(className, "Chrome") != NULL) return true;
        if (strstr(className, "WebView") != NULL) return true;
        if (strstr(className, "Intermediate D3D Window") != NULL) return true;
        // Also check for the browser host window we create
        if (strcmp(className, "STATIC") == 0) {
            // Check if parent is game window and window name is our host
            char windowName[256] = {0};
            GetWindowTextA(hWnd, windowName, sizeof(windowName));
            if (strstr(windowName, "WebView2Host") != NULL) return true;
        }
    }
    return false;
}

bool IsGameWindow(HWND hWnd) {
    if (!hWnd) return false;
    if (g_hMainWnd && !IsWindow(g_hMainWnd)) g_hMainWnd = NULL;

    // CRITICAL: Exclude WebView2 windows - they need real coordinate transformation
    if (IsWebView2Window(hWnd)) {
        // Debug log (rate limited)
        static DWORD lastLogTime = 0;
        DWORD now = GetTickCount();
        if (now - lastLogTime > 2000) {
            lastLogTime = now;
            char className[256] = {0};
            GetClassNameA(hWnd, className, sizeof(className));
            WModeLog("IsGameWindow: EXCLUDED WebView2 window (class=%s)", className);
        }
        return false;
    }

    // Also check parent chain for WebView2 windows
    HWND hParent = GetParent(hWnd);
    while (hParent && hParent != g_hMainWnd) {
        if (IsWebView2Window(hParent)) {
            return false;
        }
        hParent = GetParent(hParent);
    }

    return (hWnd == g_hMainWnd) || (g_hMainWnd && IsChild(g_hMainWnd, hWnd));
}

void FixWindowSize(HWND hWnd, int w, int h) {
    if (!hWnd || !IsWindow(hWnd)) return;
    if (Original_SetWindowLongA) {
        Original_SetWindowLongA(hWnd, GWL_STYLE, FIXED_STYLE);
        Original_SetWindowLongA(hWnd, GWL_EXSTYLE, FIXED_EXSTYLE);
    } else {
        SetWindowLongA(hWnd, GWL_STYLE, FIXED_STYLE);
        SetWindowLongA(hWnd, GWL_EXSTYLE, FIXED_EXSTYLE);
    }
    SetWindowPos(hWnd, NULL, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);

    RECT rcWnd, rcClient;
    (Original_GetWindowRect ? Original_GetWindowRect : GetWindowRect)(hWnd, &rcWnd);
    (Original_GetClientRect ? Original_GetClientRect : GetClientRect)(hWnd, &rcClient);

    int fw = w + (rcWnd.right - rcWnd.left) - rcClient.right;
    int fh = h + (rcWnd.bottom - rcWnd.top) - rcClient.bottom;
    int x = (GetSystemMetrics(SM_CXSCREEN) - fw) / 2, y = (GetSystemMetrics(SM_CYSCREEN) - fh) / 2;

    RECT rcCur;
    (Original_GetWindowRect ? Original_GetWindowRect : GetWindowRect)(hWnd, &rcCur);
    if ((rcCur.right - rcCur.left) != fw || (rcCur.bottom - rcCur.top) != fh)
        SetWindowPos(hWnd, HWND_TOP, x < 0 ? 0 : x, y < 0 ? 0 : y, fw, fh, SWP_NOCOPYBITS | SWP_NOACTIVATE);
}

LRESULT CALLBACK Hooked_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    // Debug: Log mouse click messages
    if (uMsg == WM_LBUTTONDOWN || uMsg == WM_LBUTTONUP) {
        int x = GET_X_LPARAM(lParam);
        int y = GET_Y_LPARAM(lParam);
        WModeLog("WndProc: %s at (%d, %d) hWnd=%p g_hMainWnd=%p",
                 uMsg == WM_LBUTTONDOWN ? "LBUTTONDOWN" : "LBUTTONUP", x, y, hWnd, g_hMainWnd);
    }

    if (uMsg >= WM_MOUSEFIRST && uMsg <= WM_MOUSELAST) {
        RECT rc;
        if ((Original_GetClientRect ? Original_GetClientRect : GetClientRect)(hWnd, &rc) && rc.right > 0 && rc.bottom > 0) {
            if (uMsg == WM_MOUSEWHEEL) {
                POINT pt = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
                Original_ScreenToClient(hWnd, &pt);
                lParam = MAKELPARAM((pt.x * g_GameWidth) / rc.right, (pt.y * g_GameHeight) / rc.bottom);
            } else {
                int origX = GET_X_LPARAM(lParam);
                int origY = GET_Y_LPARAM(lParam);
                int scaledX = (origX * g_GameWidth) / rc.right;
                int scaledY = (origY * g_GameHeight) / rc.bottom;
                lParam = MAKELPARAM(scaledX, scaledY);
            }
        }

        // CRITICAL: Call browser position callback during mouse drag
        // This ensures WebView2 follows the game frame in real-time during drag
        if (uMsg == WM_MOUSEMOVE && (wParam & MK_LBUTTON) && g_BrowserPositionCallback) {
            g_BrowserPositionCallback();
        }
    } else if (uMsg == WM_WINDOWPOSCHANGING) {
        WINDOWPOS *pWP = (WINDOWPOS *) lParam;
        RECT rc = {0, 0, g_GameWidth, g_GameHeight};
        (Original_AdjustWindowRectEx ? Original_AdjustWindowRectEx : AdjustWindowRectEx)(&rc, FIXED_STYLE, FALSE, FIXED_EXSTYLE);
        int tw = rc.right - rc.left, th = rc.bottom - rc.top;
        if (!(pWP->flags & SWP_NOSIZE)) {
            if (pWP->cx != tw || pWP->cy != th) {
                pWP->cx = tw;
                pWP->cy = th;
                pWP->flags &= ~SWP_NOSIZE;
                pWP->flags |= SWP_NOCOPYBITS;
            }
        } else {
            RECT rcCur;
            (Original_GetWindowRect ? Original_GetWindowRect : GetWindowRect)(hWnd, &rcCur);
            if ((rcCur.right - rcCur.left) != tw || (rcCur.bottom - rcCur.top) != th) {
                pWP->cx = tw;
                pWP->cy = th;
                pWP->flags &= ~SWP_NOSIZE;
                pWP->flags |= SWP_NOCOPYBITS;
            }
        }
    } else if (uMsg == WM_TIMER && wParam == 999) {
        FixWindowSize(hWnd, g_GameWidth, g_GameHeight);
    } else if (uMsg == WM_GETMINMAXINFO) {
        RECT rc = {0, 0, g_GameWidth, g_GameHeight};
        (Original_AdjustWindowRectEx ? Original_AdjustWindowRectEx : AdjustWindowRectEx)(&rc, FIXED_STYLE, FALSE, FIXED_EXSTYLE);
        ((MINMAXINFO *) lParam)->ptMinTrackSize.x = ((MINMAXINFO *) lParam)->ptMaxTrackSize.x = ((MINMAXINFO *) lParam)->ptMaxSize.x = rc.right - rc.left;
        ((MINMAXINFO *) lParam)->ptMinTrackSize.y = ((MINMAXINFO *) lParam)->ptMaxTrackSize.y = ((MINMAXINFO *) lParam)->ptMaxSize.y = rc.bottom - rc.top;
        return 0;
    }
    return g_OriginalWndProc ? CallWindowProc(g_OriginalWndProc, hWnd, uMsg, wParam, lParam) : DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void RedirectCoordinates(LPRECT lpRect) {
    lpRect->left = 0;
    lpRect->top = 0;
    lpRect->right = g_GameWidth;
    lpRect->bottom = g_GameHeight;
}

BOOL WINAPI Hook_AdjustWindowRectEx(LPRECT lpRect, DWORD dwStyle, BOOL bMenu, DWORD dwExStyle) {
    if ((lpRect->right - lpRect->left) < (g_GameWidth - 100)) return Original_AdjustWindowRectEx(lpRect, dwStyle, bMenu, dwExStyle);
    RedirectCoordinates(lpRect);
    return Original_AdjustWindowRectEx(lpRect, FIXED_STYLE, FALSE, FIXED_EXSTYLE);
}
BOOL WINAPI Hook_AdjustWindowRect(LPRECT lpRect, DWORD dwStyle, BOOL bMenu) { return Hook_AdjustWindowRectEx(lpRect, dwStyle, bMenu, 0); }

BOOL WINAPI Hook_GetClientRect(HWND hWnd, LPRECT lpRect) {
    // Debug: Log WebView2 window detection
    static DWORD lastLogTime = 0;
    DWORD now = GetTickCount();
    if (IsWebView2Window(hWnd) && (now - lastLogTime > 1000)) {
        lastLogTime = now;
        char className[256] = {0};
        GetClassNameA(hWnd, className, sizeof(className));
        WModeLog("GetClientRect: WebView2 window detected, passing through (class=%s)", className);
    }

    if (IsGameWindow(hWnd)) {
        RedirectCoordinates(lpRect);
        return TRUE;
    }
    return Original_GetClientRect(hWnd, lpRect);
}
BOOL WINAPI Hook_GetWindowRect(HWND hWnd, LPRECT lpRect) {
    if (IsGameWindow(hWnd)) {
        RedirectCoordinates(lpRect);
        Original_AdjustWindowRectEx(lpRect, FIXED_STYLE, FALSE, FIXED_EXSTYLE);
        return TRUE;
    }
    return Original_GetWindowRect(hWnd, lpRect);
}


BOOL WINAPI Hook_GetCursorPos(LPPOINT lpPoint) {
    if (!Original_GetCursorPos(lpPoint)) return FALSE;

    // Don't scale for WebView2 - it needs real screen coordinates
    // We check this by looking at window under cursor
    HWND hWndUnderCursor = WindowFromPoint(*lpPoint);
    if (hWndUnderCursor && IsWebView2Window(hWndUnderCursor)) {
        return TRUE;// Return unscaled coordinates for WebView2
    }

    // Scale coordinates for game
    if (g_hMainWnd && IsWindow(g_hMainWnd)) {
        Original_ScreenToClient(g_hMainWnd, lpPoint);
        RECT rc;
        if ((Original_GetClientRect ? Original_GetClientRect : GetClientRect)(g_hMainWnd, &rc) && rc.right > 0 && rc.bottom > 0) {
            lpPoint->x = (lpPoint->x * g_GameWidth) / rc.right;
            lpPoint->y = (lpPoint->y * g_GameHeight) / rc.bottom;
        }
    }
    return TRUE;
}
BOOL WINAPI Hook_SetCursorPos(int X, int Y) {
    // Scale coordinates for game window
    if (g_hMainWnd && IsWindow(g_hMainWnd)) {
        RECT rc;
        if ((Original_GetClientRect ? Original_GetClientRect : GetClientRect)(g_hMainWnd, &rc) && rc.right > 0 && rc.bottom > 0) {
            POINT pt = {(X * rc.right) / g_GameWidth, (Y * rc.bottom) / g_GameHeight};
            Original_ClientToScreen(g_hMainWnd, &pt);
            return Original_SetCursorPos(pt.x, pt.y);
        }
    }
    return Original_SetCursorPos(X, Y);
}

// RESTORED: Original working behavior - just return TRUE for game windows
// The scaling is done in Hooked_WndProc and Hook_GetCursorPos, not here
// Adding scaling here causes DOUBLE SCALING which breaks click coordinates!
BOOL WINAPI Hook_ScreenToClient(HWND hWnd, LPPOINT lpPoint) {
    // For WebView2 windows, use real conversion
    if (IsWebView2Window(hWnd)) {
        return Original_ScreenToClient(hWnd, lpPoint);
    }
    // For game windows, just return TRUE (game expects virtual coordinates)
    return IsGameWindow(hWnd) ? TRUE : Original_ScreenToClient(hWnd, lpPoint);
}

// RESTORED: Original working behavior - just return TRUE for game windows
// The scaling is done in Hook_SetCursorPos, not here
// Adding scaling here causes DOUBLE SCALING which breaks cursor positioning!
BOOL WINAPI Hook_ClientToScreen(HWND hWnd, LPPOINT lpPoint) {
    // For WebView2 windows, use real conversion
    if (IsWebView2Window(hWnd)) {
        return Original_ClientToScreen(hWnd, lpPoint);
    }
    // For game windows, just return TRUE (game expects virtual coordinates)
    return IsGameWindow(hWnd) ? TRUE : Original_ClientToScreen(hWnd, lpPoint);
}

void EnsureWndProcHook(HWND hWnd) {
    // CRITICAL: Never hijack g_hMainWnd with WebView2 windows!
    if (IsWebView2Window(hWnd)) {
        WModeLog("EnsureWndProcHook: BLOCKED WebView2 window from hijacking g_hMainWnd");
        return;
    }

    if (g_hMainWnd != hWnd) {
        WModeLog("EnsureWndProcHook: Setting g_hMainWnd from %p to %p", g_hMainWnd, hWnd);
        g_hMainWnd = hWnd;
        g_OriginalWndProc = (WNDPROC) (Original_SetWindowLongA ? Original_SetWindowLongA : SetWindowLongA)(hWnd, -4, (LONG) Hooked_WndProc);
        SetTimer(hWnd, 999, 500, NULL);
    }
}

BOOL WINAPI Hook_MoveWindow(HWND hWnd, int X, int Y, int nWidth, int nHeight, BOOL bRepaint) {
    // If windowed mode disabled, pass through
    if (!g_WindowedModeEnabled) {
        return Original_MoveWindow(hWnd, X, Y, nWidth, nHeight, bRepaint);
    }

    if (IsGameWindow(hWnd) || (nWidth >= g_GameWidth && nHeight > 500)) {
        if ((WNDPROC) GetWindowLongA(hWnd, -4) != Hooked_WndProc) EnsureWndProcHook(hWnd);
        FixWindowSize(hWnd, g_GameWidth, g_GameHeight);
        return TRUE;
    }
    return Original_MoveWindow(hWnd, X, Y, nWidth, nHeight, bRepaint);
}

template<typename Func>
LONG HandleSetWindowLong(HWND hWnd, int nIndex, LONG dwNewLong, Func OriginalFunc) {
    // If windowed mode disabled, pass through
    if (!g_WindowedModeEnabled) {
        return OriginalFunc(hWnd, nIndex, dwNewLong);
    }

    if (nIndex == GWL_STYLE || nIndex == GWL_EXSTYLE) {
        if (IsGameWindow(hWnd)) {
            LONG ret = OriginalFunc(hWnd, nIndex, (nIndex == GWL_STYLE) ? (LONG) FIXED_STYLE : (LONG) FIXED_EXSTYLE);
            SetWindowPos(hWnd, NULL, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
            return ret;
        }
        RECT rc;
        (Original_GetWindowRect ? Original_GetWindowRect : GetWindowRect)(hWnd, &rc);
        if ((rc.right - rc.left) >= g_GameWidth && (rc.bottom - rc.top) > 500) {
            EnsureWndProcHook(hWnd);
            LONG ret = OriginalFunc(hWnd, nIndex, (nIndex == GWL_STYLE) ? (LONG) FIXED_STYLE : (LONG) FIXED_EXSTYLE);
            SetWindowPos(hWnd, NULL, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
            return ret;
        }
    }
    return OriginalFunc(hWnd, nIndex, dwNewLong);
}
LONG WINAPI Hook_SetWindowLongA(HWND hWnd, int nIndex, LONG dwNewLong) { return HandleSetWindowLong(hWnd, nIndex, dwNewLong, Original_SetWindowLongA); }
LONG WINAPI Hook_SetWindowLongW(HWND hWnd, int nIndex, LONG dwNewLong) { return HandleSetWindowLong(hWnd, nIndex, dwNewLong, Original_SetWindowLongW); }

LONG WINAPI Hook_ChangeDisplaySettingsA(DEVMODEA *lpDevMode, DWORD dwFlags) {
    if (lpDevMode && lpDevMode->dmPelsWidth > 0 && lpDevMode->dmPelsHeight > 0) {
        g_GameWidth = lpDevMode->dmPelsWidth;
        g_GameHeight = lpDevMode->dmPelsHeight;
    }

    // If windowed mode disabled, pass through to original (allow fullscreen)
    if (!g_WindowedModeEnabled) {
        return Original_ChangeDisplaySettingsA(lpDevMode, dwFlags);
    }

    // Windowed mode enabled - force window size and block fullscreen
    if (g_hMainWnd && IsWindow(g_hMainWnd)) FixWindowSize(g_hMainWnd, g_GameWidth, g_GameHeight);
    return DISP_CHANGE_SUCCESSFUL;
}

HWND HandleCreateWindow(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int x, int y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam, bool unicode) {
    // Check if this is a game window BEFORE any mode checks
    bool isGame = false, isSplash = false;
    if (lpClassName && ((DWORD) lpClassName & 0xFFFF0000)) {
        bool match = unicode ? (wcsstr((LPCWSTR) lpClassName, L"SRO_Client") != NULL) : (strstr(lpClassName, "SRO_Client") != NULL);
        if (match) {
            isGame = true;
            isSplash = (nWidth == CW_USEDEFAULT || nWidth < g_GameWidth || nWidth < 600);
        }
    }

    // If windowed mode disabled, create window normally but STILL track the game window
    if (!g_WindowedModeEnabled) {
        HWND hWnd = unicode ? Original_CreateWindowExW(dwExStyle, (LPCWSTR) lpClassName, (LPCWSTR) lpWindowName, dwStyle, x, y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam)
                            : Original_CreateWindowExA(dwExStyle, lpClassName, lpWindowName, dwStyle, x, y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);

        // CRITICAL: Always track the main game window for runtime mode switching
        if (isGame && hWnd && !isSplash) {
            g_hMainWnd = hWnd;
        }
        return hWnd;
    }

    // Windowed mode enabled - apply windowed style
    if (isGame) {
        if (isSplash) {
            dwStyle = WS_POPUP | WS_VISIBLE;
            dwExStyle = WS_EX_TOOLWINDOW | WS_EX_TOPMOST;
        } else {
            dwStyle = FIXED_STYLE & ~WS_VISIBLE;
            dwExStyle = FIXED_EXSTYLE;
            if (nWidth != CW_USEDEFAULT) {
                RECT rc;
                RedirectCoordinates(&rc);
                (Original_AdjustWindowRectEx ? Original_AdjustWindowRectEx : AdjustWindowRectEx)(&rc, FIXED_STYLE, FALSE, FIXED_EXSTYLE);
                nWidth = rc.right - rc.left;
                nHeight = rc.bottom - rc.top;
            }
        }
    }
    HWND hWnd = unicode ? Original_CreateWindowExW(dwExStyle, (LPCWSTR) lpClassName, (LPCWSTR) lpWindowName, dwStyle, x, y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam)
                        : Original_CreateWindowExA(dwExStyle, lpClassName, lpWindowName, dwStyle, x, y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
    if (isGame && hWnd) {
        if (!isSplash) {
            EnsureWndProcHook(hWnd);
            FixWindowSize(hWnd, g_GameWidth, g_GameHeight);
        }
        ShowWindow(hWnd, SW_SHOW);
    }
    return hWnd;
}
HWND WINAPI Hook_CreateWindowExA(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int x, int y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam) {
    return HandleCreateWindow(dwExStyle, lpClassName, lpWindowName, dwStyle, x, y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam, false);
}
HWND WINAPI Hook_CreateWindowExW(DWORD dwExStyle, LPCWSTR lpClassName, LPCWSTR lpWindowName, DWORD dwStyle, int x, int y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam) {
    return HandleCreateWindow(dwExStyle, (LPCSTR) lpClassName, (LPCSTR) lpWindowName, dwStyle, x, y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam, true);
}

// Runtime switching functions

// Helper to find game window if not already tracked
static HWND FindGameWindow() {
    // First check if we already have a valid handle
    if (g_hMainWnd && IsWindow(g_hMainWnd)) {
        WModeLog("FindGameWindow - Using cached g_hMainWnd: %p", g_hMainWnd);
        return g_hMainWnd;
    }

    // Try to get from g_CD3DApplication first (most reliable)
    if (g_CD3DApplication && g_CD3DApplication->m_hWnd && IsWindow(g_CD3DApplication->m_hWnd)) {
        g_hMainWnd = g_CD3DApplication->m_hWnd;
        WModeLog("FindGameWindow - Got from g_CD3DApplication->m_hWnd: %p", g_hMainWnd);
        return g_hMainWnd;
    }

    // Fallback: Try to find SRO_Client window
    HWND hWnd = FindWindowA("SRO_Client", NULL);
    if (hWnd) {
        g_hMainWnd = hWnd;
        WModeLog("FindGameWindow - Found via FindWindowA: %p", g_hMainWnd);
        return g_hMainWnd;
    }

    // Last resort: try SR_CLIENT class
    hWnd = FindWindowA("SR_CLIENT", NULL);
    if (hWnd) {
        g_hMainWnd = hWnd;
        WModeLog("FindGameWindow - Found via FindWindowA(SR_CLIENT): %p", g_hMainWnd);
        return g_hMainWnd;
    }

    WModeLog("FindGameWindow - No window found!");
    return NULL;
}


void SwitchToFullscreen() {
    WModeLog("[WindowedMode] SwitchToFullscreen called");

    HWND hWnd = FindGameWindow();
    if (!hWnd) {
        WModeLog("[WindowedMode] SwitchToFullscreen - No game window found!");
        return;
    }
    WModeLog("[WindowedMode] SwitchToFullscreen - Window: %p, Size: %dx%d", hWnd, g_GameWidth, g_GameHeight);

    // IMPORTANT: Notify D3D resources BEFORE changing display mode
    // This allows ID3DXSprite and other D3DPOOL_DEFAULT resources to release
    WModeLog("[WindowedMode] SwitchToFullscreen - Triggering PreSetSize callbacks...");
    TriggerPreSetSize(g_GameWidth, g_GameHeight);

    // Remove windowed mode hooks from WndProc
    if (g_OriginalWndProc) {
        (Original_SetWindowLongA ? Original_SetWindowLongA : SetWindowLongA)(hWnd, GWL_WNDPROC, (LONG) g_OriginalWndProc);
    }
    KillTimer(hWnd, 999);

    // Change window style to fullscreen popup
    DWORD fullscreenStyle = WS_POPUP | WS_VISIBLE;
    (Original_SetWindowLongA ? Original_SetWindowLongA : SetWindowLongA)(hWnd, GWL_STYLE, fullscreenStyle);
    (Original_SetWindowLongA ? Original_SetWindowLongA : SetWindowLongA)(hWnd, GWL_EXSTYLE, 0);

    // Apply display settings change
    DEVMODEA dm;
    ZeroMemory(&dm, sizeof(dm));
    dm.dmSize = sizeof(dm);
    dm.dmPelsWidth = g_GameWidth;
    dm.dmPelsHeight = g_GameHeight;
    dm.dmBitsPerPel = 32;
    dm.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL;
    LONG result = Original_ChangeDisplaySettingsA(&dm, CDS_FULLSCREEN);
    WModeLog("[WindowedMode] SwitchToFullscreen - ChangeDisplaySettings result: %d", result);

    // Move window to cover full screen
    SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, g_GameWidth, g_GameHeight,
                 SWP_FRAMECHANGED | SWP_SHOWWINDOW);

    // IMPORTANT: Notify D3D resources AFTER display mode change is complete
    // This allows ID3DXSprite and other resources to reinitialize
    WModeLog("[WindowedMode] SwitchToFullscreen - Triggering PostSetSize callbacks...");
    TriggerPostSetSize(g_GameWidth, g_GameHeight);

    WModeLog("[WindowedMode] SwitchToFullscreen - Complete");
}

void SwitchToWindowed() {
    WModeLog("SwitchToWindowed called");

    // Check if game D3D application is available
    if (g_CD3DApplication && g_CD3DApplication->m_pd3dDevice) {
        // m_bWindowed offset calculation:
        // CD3DApplication layout: vtable(4) + m_d3dEnumeration(varies) + pad(4) + m_d3dSettings(varies) + m_bWindowed
        // Based on ECSRO analysis, m_bWindowed is typically at a fixed offset
        // We can read it using the game's IsWindowed state or m_d3dpp.Windowed
        bool isWindowed = g_CD3DApplication->m_d3dpp.Windowed ? true : false;

        WModeLog("SwitchToWindowed - Current D3D windowed state: %s", isWindowed ? "true" : "false");

        if (!isWindowed) {
            // Currently in fullscreen, call native ToggleFullscreen via vtable
            // ToggleFullscreen is virtual - we can call it via function pointer
            WModeLog("SwitchToWindowed - Calling native ToggleFullscreen()");

            // ToggleFullscreen function pointer - call via thiscall convention
            // The vtable index for ToggleFullscreen can vary, so we use hardcoded address
            // From d3dapp.cpp, ToggleFullscreen is at a known address we can call
            typedef HRESULT(__thiscall * ToggleFullscreenFn)(void *);
            // Use the game's native function address (from IDA analysis of CD3DApplication::ToggleFullscreen)
            // This is typically at the same offset as in d3dapp.cpp
            ToggleFullscreenFn pToggleFullscreen = (ToggleFullscreenFn) 0x008AD370;// ECSRO address

            HRESULT hr = pToggleFullscreen(g_CD3DApplication);
            WModeLog("SwitchToWindowed - ToggleFullscreen returned: 0x%08X", hr);

            if (SUCCEEDED(hr)) {
                // Now apply our windowed mode hooks
                HWND hWnd = g_CD3DApplication->m_hWnd;
                if (hWnd && IsWindow(hWnd)) {
                    g_hMainWnd = hWnd;

                    // Re-hook WndProc for windowed mode handling
                    g_OriginalWndProc = (WNDPROC) (Original_SetWindowLongA ? Original_SetWindowLongA : SetWindowLongA)(hWnd, GWL_WNDPROC, (LONG) Hooked_WndProc);
                    SetTimer(hWnd, 999, 500, NULL);

                    // Apply our fixed window style
                    (Original_SetWindowLongA ? Original_SetWindowLongA : SetWindowLongA)(hWnd, GWL_STYLE, FIXED_STYLE);
                    (Original_SetWindowLongA ? Original_SetWindowLongA : SetWindowLongA)(hWnd, GWL_EXSTYLE, FIXED_EXSTYLE);
                    SetWindowPos(hWnd, NULL, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER);
                    FixWindowSize(hWnd, g_GameWidth, g_GameHeight);
                    SetWindowPos(hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);

                    WModeLog("SwitchToWindowed - Windowed mode hooks applied");
                }
            }
            return;
        }
    }

    // Fallback: manual switching if no D3D application or already windowed
    WModeLog("SwitchToWindowed - Using fallback manual switching");

    HWND hWnd = FindGameWindow();
    if (!hWnd) {
        WModeLog("SwitchToWindowed - No game window found!");
        return;
    }
    WModeLog("SwitchToWindowed - Window: %p, Size: %dx%d", hWnd, g_GameWidth, g_GameHeight);

    // IMPORTANT: Notify D3D resources BEFORE changing display mode
    TriggerPreSetSize(g_GameWidth, g_GameHeight);

    // Restore display settings to normal
    LONG result = Original_ChangeDisplaySettingsA(NULL, 0);
    WModeLog("SwitchToWindowed - ChangeDisplaySettings(NULL) result: %d", result);

    // Re-hook WndProc for windowed mode handling
    g_OriginalWndProc = (WNDPROC) (Original_SetWindowLongA ? Original_SetWindowLongA : SetWindowLongA)(hWnd, GWL_WNDPROC, (LONG) Hooked_WndProc);
    SetTimer(hWnd, 999, 500, NULL);

    // Restore windowed style
    (Original_SetWindowLongA ? Original_SetWindowLongA : SetWindowLongA)(hWnd, GWL_STYLE, FIXED_STYLE);
    (Original_SetWindowLongA ? Original_SetWindowLongA : SetWindowLongA)(hWnd, GWL_EXSTYLE, FIXED_EXSTYLE);

    SetWindowPos(hWnd, NULL, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER);
    FixWindowSize(hWnd, g_GameWidth, g_GameHeight);
    SetWindowPos(hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);

    // Notify D3D resources AFTER display mode change is complete
    TriggerPostSetSize(g_GameWidth, g_GameHeight);

    WModeLog("SwitchToWindowed - Complete");
}

// Load initial setting from file at startup
static void LoadInitialSetting() {
    WModeLog("LoadInitialSetting called");

    char gameDir[MAX_PATH];
    GetModuleFileNameA(NULL, gameDir, MAX_PATH);
    char *lastSlash = strrchr(gameDir, '\\');
    if (lastSlash) *(lastSlash + 1) = '\0';

    char buffer[MAX_PATH];
    sprintf_s(buffer, "%sSetting\\SRCSet.dat", gameDir);
    WModeLog("LoadInitialSetting - Reading from: %s", buffer);

    FILE *file = fopen(buffer, "rb");
    if (file == NULL) {
        // Default to windowed mode enabled
        g_WindowedModeEnabled = true;
        WModeLog("LoadInitialSetting - File not found, defaulting to WINDOWED (g_WindowedModeEnabled=true)");
        return;
    }

    bool isChecked = true;
    fread(&isChecked, sizeof(bool), 1, file);
    fclose(file);

    // isChecked = true means windowed mode enabled
    g_WindowedModeEnabled = isChecked;
    WModeLog("LoadInitialSetting - Loaded value: %s (g_WindowedModeEnabled=%s)",
             isChecked ? "CHECKED" : "UNCHECKED",
             g_WindowedModeEnabled ? "true" : "false");
}

namespace WindowedMode {
    void InstallHooks() {
        // Load saved setting first
        LoadInitialSetting();

        HMODULE hUser32 = GetModuleHandleA("user32.dll");
        if (!hUser32) return;
#define HOOK(name)                                                                 \
    {                                                                              \
        void *p = (void *) GetProcAddress(hUser32, #name);                         \
        if (p) Original_##name = (name##_t) SimpleDetour(p, (void *) Hook_##name); \
    }
        HOOK(ChangeDisplaySettingsA);
        HOOK(CreateWindowExA);
        HOOK(CreateWindowExW);
        HOOK(SetWindowLongA);
        HOOK(SetWindowLongW);
        HOOK(MoveWindow);
        HOOK(GetCursorPos);
        HOOK(SetCursorPos);
        HOOK(ScreenToClient);
        HOOK(ClientToScreen);
        HOOK(GetClientRect);
        HOOK(GetWindowRect);
        HOOK(AdjustWindowRect);
        HOOK(AdjustWindowRectEx);
    }

    void SetEnabled(bool enabled) {
        WindowedMode_SetEnabled(enabled);
    }

    bool IsEnabled() {
        return WindowedMode_IsEnabled();
    }

    // Register callback for browser position updates during drag
    void RegisterBrowserPositionCallback(BrowserPositionCallback callback) {
        g_BrowserPositionCallback = callback;
    }

    // Unregister callback
    void UnregisterBrowserPositionCallback() {
        g_BrowserPositionCallback = NULL;
    }
}// namespace WindowedMode