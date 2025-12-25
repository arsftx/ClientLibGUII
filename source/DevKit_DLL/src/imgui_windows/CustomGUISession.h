#pragma once
/**
 * @file CustomGUISession.h
 * @brief Shared ImGui session manager for custom overlay windows
 * 
 * This singleton manages ImGui context initialization and frame lifecycle.
 * All custom GUI windows (CustomPlayerMiniInfo, future windows) should use
 * this instead of creating their own ImGui sessions.
 * 
 * Usage:
 *   // In DllMain or initialization:
 *   CustomGUISession::Instance().Initialize();
 *   
 *   // In each window's render callback:
 *   if (!CustomGUISession::Instance().BeginFrame()) return;
 *   // ... render ImGui windows ...
 *   CustomGUISession::Instance().EndFrame();
 */

#include <d3d9.h>

class CustomGUISession {
public:
    // Singleton access
    static CustomGUISession& Instance();
    
    // Initialize - registers the EndScene hook
    bool Initialize();
    
    // Called automatically by OnEndScene hook
    // Returns true if ImGui is ready for rendering
    bool BeginFrame();
    void EndFrame();
    
    // Check if ImGui is initialized and ready
    bool IsReady() const { return m_bImGuiInitialized; }
    
    // Register a render callback - called every frame when ImGui is ready
    // Returns callback ID for unregistering
    typedef void (*RenderCallback)();
    int RegisterRenderCallback(RenderCallback callback);
    void UnregisterRenderCallback(int id);
    
private:
    CustomGUISession();
    ~CustomGUISession();
    
    // Disable copy
    CustomGUISession(const CustomGUISession&);
    CustomGUISession& operator=(const CustomGUISession&);
    
    // Friend for OnEndScene callback to access callbacks array
    friend void CustomGUI_OnEndScene();
    
    // Internal ImGui initialization
    bool EnsureImGuiInitialized();
    
    // State
    bool m_bHookRegistered;
    bool m_bImGuiInitialized;
    bool m_bInFrame;  // True between BeginFrame and EndFrame
    
    // Render callbacks (simple array for VS2005 compatibility)
    static const int MAX_CALLBACKS = 16;
    RenderCallback m_callbacks[MAX_CALLBACKS];
    int m_callbackCount;
};

// Global accessor macro for convenience
#define g_CustomGUI CustomGUISession::Instance()
