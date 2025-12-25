#include "CustomGUISession.h"
#include <imgui/imgui.h>
#include "imgui/examples/imgui_impl_dx9.h"
#include "imgui/examples/imgui_impl_win32.h"
#include <GInterface.h>
#include <GFX3DFunction/GFXVideo3d.h>
#include "../hooks/Hooks.h"

// External ImGui Win32 handler declaration
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Forward declarations
static void CustomGUI_OnEndScene();
static LRESULT CALLBACK CustomGUI_WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Singleton instance
CustomGUISession& CustomGUISession::Instance() {
    static CustomGUISession instance;
    return instance;
}

CustomGUISession::CustomGUISession() {
    m_bHookRegistered = false;
    m_bImGuiInitialized = false;
    m_bInFrame = false;
    m_bDeviceResetting = false;
    m_callbackCount = 0;
    
    // Initialize callback array
    for (int i = 0; i < MAX_CALLBACKS; i++) {
        m_callbacks[i] = NULL;
    }
}

CustomGUISession::~CustomGUISession() {
    // Cleanup ImGui if initialized
    if (m_bImGuiInitialized) {
        ImGui_ImplDX9_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
    }
}

// Forward declarations for SetSize hooks
static void CustomGUI_OnPreSetSize(int width, int height);
static void CustomGUI_OnPostSetSize(int width, int height);

bool CustomGUISession::Initialize() {
    if (m_bHookRegistered) return true;
    
    // Register our EndScene callback
    OnEndScene(CustomGUI_OnEndScene);
    
    // Register WndProc hook to capture mouse/keyboard input
    OnWndProc(CustomGUI_WndProc);
    
    // Register SetSize hooks for D3D device reset handling
    OnPreSetSize(CustomGUI_OnPreSetSize);
    OnPostSetSize(CustomGUI_OnPostSetSize);
    
    m_bHookRegistered = true;
    return true;
}

bool CustomGUISession::EnsureImGuiInitialized() {
    if (m_bImGuiInitialized) return true;
    
    __try {
        // Need D3D device and window handle
        if (!g_CD3DApplication) return false;
        if (!g_CD3DApplication->m_pd3dDevice) return false;
        if (!g_CD3DApplication->m_hWnd) return false; 
        
        // Create ImGui context
        ImGui::CreateContext();
        
        // Initialize platform/renderer backends
        ImGui_ImplWin32_Init(g_CD3DApplication->m_hWnd);
        ImGui_ImplDX9_Init(g_CD3DApplication->m_pd3dDevice);
        
        // Configure ImGui style
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;  // Don't change game cursor
        
        // Dark theme for overlay
        ImGui::StyleColorsDark();
        
        m_bImGuiInitialized = true;
        return true;
        
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

bool CustomGUISession::BeginFrame() {
    if (m_bInFrame) return false;  // Already in frame
    
    __try {
        // Basic device checks
        if (!g_CD3DApplication) return false;
        if (!g_CD3DApplication->m_pd3dDevice) return false;
        if (!g_CD3DApplication->m_hWnd) return false;
        if (g_CD3DApplication->IsLost()) return false;
        
        IDirect3DDevice9* pDevice = g_CD3DApplication->m_pd3dDevice;
        
        // Check device cooperative level
        HRESULT hr = pDevice->TestCooperativeLevel();
        if (hr != D3D_OK) return false;  // Device not ready
        
        // Skip if device is resetting
        if (m_bDeviceResetting) return false;
        
        // CRITICAL: Don't initialize ImGui until player is in-game
        // This ensures D3D device has been stable for a while
        static const DWORD ADDR_PLAYER_PTR = 0x00A0465C;
        DWORD pPlayerDW = *(DWORD*)ADDR_PLAYER_PTR;
        if (pPlayerDW == 0) return false;  // Player not loaded yet
        
        // Additional check: player must have valid MaxHP
        int maxHP = *(int*)(pPlayerDW + 0x358);  // MaxHP offset
        if (maxHP <= 0) return false;  // Player data not valid yet
        
        // Now safe to initialize ImGui
        if (!EnsureImGuiInitialized()) return false;
        
        // Start new ImGui frame
        ImGui_ImplDX9_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        
        m_bInFrame = true;
        return true;
        
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

void CustomGUISession::EndFrame() {
    if (!m_bInFrame) return;
    
    __try {
        ImGui::EndFrame();
        ImGui::Render();
        ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
        
        m_bInFrame = false;
        
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        m_bInFrame = false;
    }
}

int CustomGUISession::RegisterRenderCallback(RenderCallback callback) {
    if (!callback) return -1;
    if (m_callbackCount >= MAX_CALLBACKS) return -1;
    
    // Find first empty slot
    for (int i = 0; i < MAX_CALLBACKS; i++) {
        if (m_callbacks[i] == NULL) {
            m_callbacks[i] = callback;
            m_callbackCount++;
            return i;
        }
    }
    return -1;
}

void CustomGUISession::UnregisterRenderCallback(int id) {
    if (id < 0 || id >= MAX_CALLBACKS) return;
    if (m_callbacks[id] != NULL) {
        m_callbacks[id] = NULL;
        m_callbackCount--;
    }
}
// Track device lost state for proper reset handling
static bool s_CustomGUIDeviceLost = false;

// Static OnEndScene callback - calls all registered render callbacks
static void CustomGUI_OnEndScene() {
    // Basic checks
    if (!g_CD3DApplication) return;
    if (!g_CD3DApplication->m_pd3dDevice) return;
    
    IDirect3DDevice9* pDevice = g_CD3DApplication->m_pd3dDevice;
    CustomGUISession& session = CustomGUISession::Instance();
    
    // Check device cooperative level to detect lost/reset state
    // This catches device lost situations that occur when:
    // - Running programs as administrator in background
    // - Opening certain fullscreen/exclusive mode applications
    // - Display mode changes from external sources
    HRESULT hr = pDevice->TestCooperativeLevel();
    
    if (hr == D3DERR_DEVICELOST) {
        // Device is lost, we cannot render
        if (!s_CustomGUIDeviceLost) {
            s_CustomGUIDeviceLost = true;
            session.OnDeviceLost();
        }
        return;
    }
    
    if (hr == D3DERR_DEVICENOTRESET) {
        // Device is ready to be reset but hasn't been reset yet
        if (!s_CustomGUIDeviceLost) {
            s_CustomGUIDeviceLost = true;
            session.OnDeviceLost();
        }
        return;
    }
    
    if (hr == D3D_OK || hr == S_FALSE) {
        // Device is operational
        // If we were lost before, restore resources now that device is reset
        if (s_CustomGUIDeviceLost) {
            s_CustomGUIDeviceLost = false;
            session.OnDeviceReset();
        }
        
        // Additional safety check using game's IsLost flag
        if (g_CD3DApplication->IsLost()) return;
        
        // Start ImGui frame
        if (!session.BeginFrame()) return;
        
        // Call all registered render callbacks
        for (int i = 0; i < CustomGUISession::MAX_CALLBACKS; i++) {
            CustomGUISession::RenderCallback callback = session.m_callbacks[i];
            if (callback != NULL) {
                __try {
                    callback();
                } __except(EXCEPTION_EXECUTE_HANDLER) {
                    // Ignore callback errors
                }
            }
        }
        
        // End ImGui frame
        session.EndFrame();
    }
}

// WndProc callback - handles mouse/keyboard input for ImGui
static LRESULT CALLBACK CustomGUI_WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    // If ImGui not initialized, let game handle it
    if (!CustomGUISession::Instance().IsReady()) {
        return 0;  // 0 = let game process this message
    }
    
    // Let ImGui process the message first
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam)) {
        return 1;  // 1 = message consumed by ImGui, block game
    }
    
    // Check if ImGui wants to capture mouse
    ImGuiIO& io = ImGui::GetIO();
    
    // If mouse is over any ImGui window, block mouse input to game
    if (io.WantCaptureMouse) {
        switch (msg) {
            case WM_LBUTTONDOWN:
            case WM_LBUTTONUP:
            case WM_LBUTTONDBLCLK:
            case WM_RBUTTONDOWN:
            case WM_RBUTTONUP:
            case WM_RBUTTONDBLCLK:
            case WM_MBUTTONDOWN:
            case WM_MBUTTONUP:
            case WM_MBUTTONDBLCLK:
            case WM_MOUSEWHEEL:
            case WM_MOUSEMOVE:
                return 1;  // Block game from receiving mouse input
        }
    }
    
    // If ImGui wants keyboard, block keyboard input to game
    if (io.WantCaptureKeyboard) {
        switch (msg) {
            case WM_KEYDOWN:
            case WM_KEYUP:
            case WM_CHAR:
            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP:
                return 1;  // Block game from receiving keyboard input
        }
    }
    
    return 0;  // Let game process this message
}

// ============================================================================
// D3D Device Reset Handling
// ============================================================================

void CustomGUISession::OnDeviceLost() {
    // Set flag to block rendering during reset
    m_bDeviceResetting = true;
    
    // Release ImGui D3D resources before device Reset()
    if (m_bImGuiInitialized) {
        ImGui_ImplDX9_InvalidateDeviceObjects();
    }
}

void CustomGUISession::OnDeviceReset() {
    // Recreate ImGui D3D resources after device Reset()
    if (m_bImGuiInitialized) {
        ImGui_ImplDX9_CreateDeviceObjects();
    }
    
    // Clear flag to resume rendering
    m_bDeviceResetting = false;
}

// Static hook callbacks for SetSize
static void CustomGUI_OnPreSetSize(int width, int height) {
    CustomGUISession::Instance().OnDeviceLost();
}

static void CustomGUI_OnPostSetSize(int width, int height) {
    CustomGUISession::Instance().OnDeviceReset();
}
