#pragma once

#include <windows.h>

namespace WindowedMode {
    void InstallHooks();
    void SetEnabled(bool enabled);
    bool IsEnabled();
    
    // Callback for browser position updates during drag
    // This is called from Hooked_WndProc during WM_MOUSEMOVE while dragging
    typedef void (*BrowserPositionCallback)();
    void RegisterBrowserPositionCallback(BrowserPositionCallback callback);
    void UnregisterBrowserPositionCallback();
}
