#include "CustomGUISession.h"
#include <imgui/imgui.h>
#include "imgui/examples/imgui_impl_dx9.h"
#include "imgui/examples/imgui_impl_win32.h"
#ifdef IMGUI_ENABLE_FREETYPE
#include "imgui/misc/freetype/imgui_freetype.h"
#endif
#include <GInterface.h>
#include <GFX3DFunction/GFXVideo3d.h>
#include "../hooks/Hooks.h"

// External ImGui Win32 handler declaration
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Forward declarations
static void CustomGUI_OnEndScene();
static LRESULT CALLBACK CustomGUI_WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
static void CustomGUI_OnPreSetSize(int width, int height);
static void CustomGUI_OnPostSetSize(int width, int height);

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
    m_pStateBlock = NULL; // <--- StateBlock başlangıç değeri
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

    // StateBlock temizliği
    if (m_pStateBlock) {
        m_pStateBlock->Release();
        m_pStateBlock = NULL;
    }
}

bool CustomGUISession::Initialize() {
    if (m_bHookRegistered) return true;

    // Register hooks
    OnEndScene(CustomGUI_OnEndScene);
    OnWndProc(CustomGUI_WndProc);
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

        // 1. Create ImGui context
        ImGui::CreateContext();

        // 2. Initialize Win32 FIRST (Input handling)
        ImGui_ImplWin32_Init(g_CD3DApplication->m_hWnd);

        // 3. Configure ImGui style
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;  // Don't change game cursor

        // 4. Load Fonts (DX9 Init'ten ÖNCE yapılmalı!)
        ImFontConfig fontConfig;

        // --- KESKİNLİK AYARLARI ---
        fontConfig.OversampleH = 1; // 1 = Piksel piksel (Blur yok)
        fontConfig.OversampleV = 1;
        fontConfig.PixelSnapH = true;

        // Yazıyı biraz daha etli/kalın göstermek için
        fontConfig.RasterizerMultiply = 1.2f;

        // Silkroad'ın orijinal Font.ttf dosyasını kullan
        ImFont* font = io.Fonts->AddFontFromFileTTF("Fonts\\Font.ttf", 13.0f, &fontConfig);
        if (!font) {
            // Bulamazsa Windows'tan Tahoma al (Arial yerine Tahoma daha uygundur)
            io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\tahoma.ttf", 13.0f, &fontConfig);
        }

#ifdef IMGUI_ENABLE_FREETYPE
        // 5. Build Atlas with FreeType (MonoHinting for crisp edges)
        unsigned int flags = ImGuiFreeType::MonoHinting;
        ImGuiFreeType::BuildFontAtlas(io.Fonts, flags);
#endif

        // 6. Initialize DX9 LAST (Bu en son yapılmalı ki font texture'ı düzgün yüklensin)
        ImGui_ImplDX9_Init(g_CD3DApplication->m_pd3dDevice);

        // Dark theme for overlay
        ImGui::StyleColorsDark();

        m_bImGuiInitialized = true;
        return true;

    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

bool CustomGUISession::BeginFrame() {
    if (m_bInFrame) return false;

    __try {
        if (!g_CD3DApplication) return false;
        if (!g_CD3DApplication->m_pd3dDevice) return false;
        if (!g_CD3DApplication->m_hWnd) return false;

        // IsLost yerine TestCooperativeLevel daha güvenlidir
        IDirect3DDevice9* pDevice = g_CD3DApplication->m_pd3dDevice;
        HRESULT hr = pDevice->TestCooperativeLevel();
        if (hr != D3D_OK) return false;

        // Skip if device is resetting
        if (m_bDeviceResetting) return false;

        // CRITICAL: Player Pointer Check
        // Hardcoded pointer yerine oyunun hazır olup olmadığını kontrol ediyoruz
        static const DWORD ADDR_PLAYER_PTR = 0x00A0465C;
        DWORD pPlayerDW = 0;

        // Güvenli okuma
        __try {
            if (IsBadReadPtr((void*)ADDR_PLAYER_PTR, 4)) return false;
            pPlayerDW = *(DWORD*)ADDR_PLAYER_PTR;
        }
        __except (1) { return false; }

        if (pPlayerDW == 0) return false;

        // Now safe to initialize ImGui
        if (!EnsureImGuiInitialized()) return false;

        // Start new ImGui frame
        ImGui_ImplDX9_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        m_bInFrame = true;
        return true;

    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

void CustomGUISession::EndFrame() {
    if (!m_bInFrame) return;

    __try {
        ImGui::EndFrame();
        ImGui::Render();

        IDirect3DDevice9* pDevice = g_CD3DApplication->m_pd3dDevice;
        if (pDevice) {
            // --- STATE BLOCK KORUMASI (Grafik Bozulmasını Önler) ---
            // 1. StateBlock yoksa oluştur
            if (m_pStateBlock == NULL) {
                pDevice->CreateStateBlock(D3DSBT_ALL, &m_pStateBlock);
            }

            // 2. Mevcut grafik ayarlarını kaydet
            if (m_pStateBlock) {
                m_pStateBlock->Capture();
            }

            // 3. ImGui Çizimini Yap
            ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

            // 4. Grafik ayarlarını geri yükle (Restore)
            if (m_pStateBlock) {
                m_pStateBlock->Apply();
            }
        }

        m_bInFrame = false;

    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        m_bInFrame = false;
    }
}

int CustomGUISession::RegisterRenderCallback(RenderCallback callback) {
    if (!callback) return -1;
    if (m_callbackCount >= MAX_CALLBACKS) return -1;

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

// Track device lost state
static bool s_CustomGUIDeviceLost = false;

static void CustomGUI_OnEndScene() {
    if (!g_CD3DApplication) return;
    if (!g_CD3DApplication->m_pd3dDevice) return;

    IDirect3DDevice9* pDevice = g_CD3DApplication->m_pd3dDevice;
    CustomGUISession& session = CustomGUISession::Instance();

    HRESULT hr = pDevice->TestCooperativeLevel();

    if (hr == D3DERR_DEVICELOST) {
        if (!s_CustomGUIDeviceLost) {
            s_CustomGUIDeviceLost = true;
            session.OnDeviceLost();
        }
        return;
    }

    if (hr == D3DERR_DEVICENOTRESET) {
        if (!s_CustomGUIDeviceLost) {
            s_CustomGUIDeviceLost = true;
            session.OnDeviceLost();
        }
        return;
    }

    if (hr == D3D_OK || hr == S_FALSE) {
        if (s_CustomGUIDeviceLost) {
            s_CustomGUIDeviceLost = false;
            session.OnDeviceReset();
        }

        if (g_CD3DApplication->IsLost()) return;

        if (!session.BeginFrame()) return;

        // Call callbacks
        for (int i = 0; i < CustomGUISession::MAX_CALLBACKS; i++) {
            CustomGUISession::RenderCallback callback = session.m_callbacks[i];
            if (callback != NULL) {
                __try {
                    callback();
                }
                __except (EXCEPTION_EXECUTE_HANDLER) {
                }
            }
        }

        session.EndFrame();
    }
}

static LRESULT CALLBACK CustomGUI_WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (!CustomGUISession::Instance().IsReady()) {
        return 0;
    }

    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam)) {
        return 1;
    }

    ImGuiIO& io = ImGui::GetIO();

    if (io.WantCaptureMouse) {
        switch (msg) {
        case WM_LBUTTONDOWN: case WM_LBUTTONUP: case WM_LBUTTONDBLCLK:
        case WM_RBUTTONDOWN: case WM_RBUTTONUP: case WM_RBUTTONDBLCLK:
        case WM_MBUTTONDOWN: case WM_MBUTTONUP: case WM_MBUTTONDBLCLK:
        case WM_MOUSEWHEEL: case WM_MOUSEMOVE:
            return 1;
        }
    }

    if (io.WantCaptureKeyboard) {
        switch (msg) {
        case WM_KEYDOWN: case WM_KEYUP: case WM_CHAR:
        case WM_SYSKEYDOWN: case WM_SYSKEYUP:
            return 1;
        }
    }

    return 0;
}

// ============================================================================
// D3D Device Reset Handling
// ============================================================================

void CustomGUISession::OnDeviceLost() {
    m_bDeviceResetting = true;

    if (m_bImGuiInitialized) {
        // StateBlock release (Memory leak önlemek için çok önemli)
        if (m_pStateBlock) {
            m_pStateBlock->Release();
            m_pStateBlock = NULL;
        }

        ImGui_ImplDX9_InvalidateDeviceObjects();
    }
}

void CustomGUISession::OnDeviceReset() {
    if (m_bImGuiInitialized) {
        ImGui_ImplDX9_CreateDeviceObjects();
    }

    m_bDeviceResetting = false;
}

static void CustomGUI_OnPreSetSize(int width, int height) {
    CustomGUISession::Instance().OnDeviceLost();
}

static void CustomGUI_OnPostSetSize(int width, int height) {
    CustomGUISession::Instance().OnDeviceReset();
}