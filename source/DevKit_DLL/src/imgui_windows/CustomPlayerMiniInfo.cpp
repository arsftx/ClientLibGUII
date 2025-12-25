#include "CustomPlayerMiniInfo.h"
#include "CustomGUISession.h"
#include "../hooks/PlayerMiniInfo_Hook.h"
#include <imgui/imgui.h>
#include <GInterface.h>
#include <Game.h>
#include <ICPlayer.h>
#include <IFMainPopup.h>
#include <IFPlayerInfo.h>
#include <CharacterDependentData.h>
#include <GlobalDataManager.h>
#include <CharacterPortrait.h>
#include <BSLib/multibyte.h>
#include <ClientNet/MsgStreamBuffer.h>
#include <memory/hook.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <GFX3DFunction/GFXVideo3d.h>
#include <d3dx9.h>
#include <IFPlayerMiniInfo.h>

// Opcode for selecting target (self-target on portrait click)
#define OPCODE_SELECT_TARGET 0x7045

// Address constants from ECSRO client
static const DWORD ADDR_PLAYER_PTR = 0x00A0465C;
static const DWORD ADDR_MAINPOPUP = 0xC5DD24;

// Native target function (from CIFPlayerMiniInfo::sub_519060)
typedef void (__thiscall *SelectTargetFn)(void* pMainPopup, DWORD uniqueID);
static const DWORD ADDR_SUB_4EA8D0 = 0x4EA8D0;

// Native visibility check (from CIFPlayerMiniInfo render - sub_89F430)
typedef bool (__thiscall *IsWindowVisibleFn)(void* pThis);
static const DWORD ADDR_SUB_89F430 = 0x89F430;

// Loading Manager global pointer (from sub_5ABE80 -> dword_A00524)
// Loading flag is at offset +0xBC (from sub_5E0370 analysis)
static const DWORD ADDR_LOADING_MANAGER = 0xA00524;
static const DWORD OFFSET_LOADING_FLAG = 0xBC;

// Native PlayerMiniInfo global pointer (from sub_517DC0 -> dword_9FF7B0)
// Visibility flag is at offset +0x5D (from sub_89F430 analysis)
static const DWORD ADDR_NATIVE_PLAYERMINIINFO = 0x9FF7B0;
static const DWORD OFFSET_VISIBILITY = 0x5D;

// =============================================================================
// Game's Native String Structure (same as CustomDamageRenderer)
// =============================================================================
struct GameString {
    char* data;      // [0] pointer to allocated data
    char* end;       // [4] data + length
    char* capacity;  // [8] data + buffer_size
};

// sub_406190: String constructor (thiscall)
typedef void (__thiscall *tStringConstruct)(GameString* pThis, const char* start, const char* end);
static tStringConstruct StringConstruct = (tStringConstruct)0x00406190;

// sub_409E10: Texture loader (cdecl)
typedef IDirect3DBaseTexture9* (__cdecl *tLoadTexture)(GameString* pPath);
static tLoadTexture LoadGameTexture = (tLoadTexture)0x00409E10;

// Helper: Create GameString from const char*
static void CreateGameString(GameString* pStr, const char* text) {
    size_t len = strlen(text);
    StringConstruct(pStr, text, text + len);
}

// Debug logging for texture loading
static void LogMsg(const char* fmt, ...) {
    FILE* fp = fopen("PlayerMiniInfoLog.txt", "a");
    if (fp) {
        va_list args;
        va_start(args, fmt);
        vfprintf(fp, fmt, args);
        fprintf(fp, "\n");
        va_end(args);
        fclose(fp);
    }
}

// Helper to check if UI should be visible (not during loading/teleport)
static bool IsUIVisible() {
    // Check basic player validity first
    DWORD* pPlayerPtr = (DWORD*)ADDR_PLAYER_PTR;
    if (!pPlayerPtr || !*pPlayerPtr) return false;
    
    DWORD* pMainPopupPtr = (DWORD*)ADDR_MAINPOPUP;
    if (!pMainPopupPtr || !*pMainPopupPtr) return false;
    
    // Check loading state from Loading Manager
    DWORD loadingManager = *(DWORD*)ADDR_LOADING_MANAGER;
    if (loadingManager != 0) {
        BYTE isLoading = *(BYTE*)(loadingManager + OFFSET_LOADING_FLAG);
        if (isLoading != 0) {
            return false;  // Loading in progress, hide our UI
        }
    }
    
    return true;
}

// Helper function for self-target using native game function
static void SendSelfTargetPacket() {
    DWORD playerAddr = *(DWORD*)ADDR_PLAYER_PTR;
    if (!playerAddr) return;
    
    DWORD playerUniqueID = *(DWORD*)(playerAddr + 0xE0);
    if (playerUniqueID == 0) return;
    
    DWORD* pMainPopup = (DWORD*)ADDR_MAINPOPUP;
    if (!pMainPopup || !*pMainPopup) return;
    
    // Call native target function directly (instant, no delay!)
    SelectTargetFn selectTarget = (SelectTargetFn)ADDR_SUB_4EA8D0;
    selectTarget((void*)*pMainPopup, playerUniqueID);
}

// Static initialization tracking
static bool s_PlayerInfoInitialized = false;

// Singleton instance
CustomPlayerMiniInfo& CustomPlayerMiniInfo::Instance() {
    static CustomPlayerMiniInfo instance;
    return instance;
}

// Static render callback for CustomGUISession
static void PlayerMiniInfo_RenderCallback() {
    CustomPlayerMiniInfo::Instance().Render();
}

// Initialize - registers with CustomGUISession
bool CustomPlayerMiniInfo::Initialize() {
    if (s_PlayerInfoInitialized) return true;
    
    // Initialize character portrait renderer
    GetCharacterPortrait().Initialize(64, 64);
    
    // Register our render callback with the shared GUI session
    int callbackId = g_CustomGUI.RegisterRenderCallback(PlayerMiniInfo_RenderCallback);
    if (callbackId < 0) return false;
    
    s_PlayerInfoInitialized = true;
    return true;
} 

CustomPlayerMiniInfo::CustomPlayerMiniInfo() {
    // Initialize all members
    m_bShow = true;
    m_bEnabled = true;
    m_bShowStats = false;
    m_bShowStatsPopup = false;
    m_fPosX = 10.0f;
    m_fPosY = 10.0f;
    m_fAnimatedHP = 1.0f;
    m_fAnimatedMP = 1.0f;
    m_fAnimationSpeed = 0.05f;
    m_pCachedMiniInfo = NULL;
    m_bTexturesLoaded = false;
    
    // Initialize texture structures
    m_texBackground.pTexture = NULL; m_texBackground.width = 0; m_texBackground.height = 0;
    m_texHpFill.pTexture = NULL; m_texHpFill.width = 0; m_texHpFill.height = 0;
    m_texMpFill.pTexture = NULL; m_texMpFill.width = 0; m_texMpFill.height = 0;
    m_texHwanFill.pTexture = NULL; m_texHwanFill.width = 0; m_texHwanFill.height = 0;
    
    // Default colors
    m_colors.hpBar[0] = 0.8f; m_colors.hpBar[1] = 0.2f; m_colors.hpBar[2] = 0.2f; m_colors.hpBar[3] = 1.0f;
    m_colors.mpBar[0] = 0.2f; m_colors.mpBar[1] = 0.4f; m_colors.mpBar[2] = 0.8f; m_colors.mpBar[3] = 1.0f;
    m_colors.hpCritical[0] = 1.0f; m_colors.hpCritical[1] = 0.0f; m_colors.hpCritical[2] = 0.0f; m_colors.hpCritical[3] = 1.0f;
    m_colors.background[0] = 0.1f; m_colors.background[1] = 0.1f; m_colors.background[2] = 0.15f; m_colors.background[3] = 0.85f;
    m_colors.text[0] = 1.0f; m_colors.text[1] = 1.0f; m_colors.text[2] = 1.0f; m_colors.text[3] = 1.0f;
    m_colors.levelText[0] = 1.0f; m_colors.levelText[1] = 0.85f; m_colors.levelText[2] = 0.0f; m_colors.levelText[3] = 1.0f;
}

// =============================================================================
// Texture Loading - Load DDJ textures from Media.pk2 using game's native loader
// =============================================================================
bool CustomPlayerMiniInfo::LoadTextures() {
    if (m_bTexturesLoaded) return true;
    
    LogMsg("[PlayerMiniInfo] LoadTextures() called...");
    
    // Load background texture using game's native loader
    GameString bgStr = {0, 0, 0};
    const char* bgPath = "newui\\playerminiinfo\\mainbackground.ddj";
    CreateGameString(&bgStr, bgPath);
    
    LogMsg("[PlayerMiniInfo] Loading: %s", bgPath);
    
    IDirect3DBaseTexture9* pBgTex = LoadGameTexture(&bgStr);
    LogMsg("[PlayerMiniInfo] LoadGameTexture returned: %p", pBgTex);
    
    if (pBgTex) {
        m_texBackground.pTexture = (IDirect3DTexture9*)pBgTex;
        D3DSURFACE_DESC desc;
        if (SUCCEEDED(m_texBackground.pTexture->GetLevelDesc(0, &desc))) {
            m_texBackground.width = desc.Width;
            m_texBackground.height = desc.Height;
            LogMsg("[PlayerMiniInfo] Background texture loaded: %dx%d", desc.Width, desc.Height);
        }
    } else {
        LogMsg("[PlayerMiniInfo] FAILED to load background texture!");
    }
    
    // Load portrait background texture (drawn after bars, at X=2, Y=2)
    GameString portraitBgStr = {0, 0, 0};
    const char* portraitBgPath = "newui\\playerminiinfo\\portraitbg.ddj";
    CreateGameString(&portraitBgStr, portraitBgPath);
    LogMsg("[PlayerMiniInfo] Loading: %s", portraitBgPath);
    
    IDirect3DBaseTexture9* pPortraitBgTex = LoadGameTexture(&portraitBgStr);
    if (pPortraitBgTex) {
        m_texPortraitBg.pTexture = (IDirect3DTexture9*)pPortraitBgTex;
        D3DSURFACE_DESC desc;
        if (SUCCEEDED(m_texPortraitBg.pTexture->GetLevelDesc(0, &desc))) {
            m_texPortraitBg.width = desc.Width;
            m_texPortraitBg.height = desc.Height;
            LogMsg("[PlayerMiniInfo] Portrait BG texture loaded: %dx%d", desc.Width, desc.Height);
        }
    } else {
        LogMsg("[PlayerMiniInfo] FAILED to load portrait BG texture!");
    }
    
    // Load portrait frame texture (top layer, drawn at X=0, Y=0)
    GameString portraitFrameStr = {0, 0, 0};
    const char* portraitFramePath = "newui\\playerminiinfo\\portraitframe.ddj";
    CreateGameString(&portraitFrameStr, portraitFramePath);
    LogMsg("[PlayerMiniInfo] Loading: %s", portraitFramePath);
    
    IDirect3DBaseTexture9* pPortraitFrameTex = LoadGameTexture(&portraitFrameStr);
    if (pPortraitFrameTex) {
        m_texPortraitFrame.pTexture = (IDirect3DTexture9*)pPortraitFrameTex;
        D3DSURFACE_DESC desc;
        if (SUCCEEDED(m_texPortraitFrame.pTexture->GetLevelDesc(0, &desc))) {
            m_texPortraitFrame.width = desc.Width;
            m_texPortraitFrame.height = desc.Height;
            LogMsg("[PlayerMiniInfo] Portrait Frame texture loaded: %dx%d", desc.Width, desc.Height);
        }
    } else {
        LogMsg("[PlayerMiniInfo] FAILED to load portrait Frame texture!");
    }
    
    // Load level frame texture (behind level text)
    GameString levelFrameStr = {0, 0, 0};
    const char* levelFramePath = "newui\\playerminiinfo\\levelframe.ddj";
    CreateGameString(&levelFrameStr, levelFramePath);
    LogMsg("[PlayerMiniInfo] Loading: %s", levelFramePath);
    
    IDirect3DBaseTexture9* pLevelFrameTex = LoadGameTexture(&levelFrameStr);
    if (pLevelFrameTex) {
        m_texLevelFrame.pTexture = (IDirect3DTexture9*)pLevelFrameTex;
        D3DSURFACE_DESC desc;
        if (SUCCEEDED(m_texLevelFrame.pTexture->GetLevelDesc(0, &desc))) {
            m_texLevelFrame.width = desc.Width;
            m_texLevelFrame.height = desc.Height;
            LogMsg("[PlayerMiniInfo] Level Frame texture loaded: %dx%d", desc.Width, desc.Height);
        }
    } else {
        LogMsg("[PlayerMiniInfo] FAILED to load level Frame texture!");
    }
    
    // Load HP bar fill texture
    GameString hpStr = {0, 0, 0};
    const char* hpPath = "newui\\playerminiinfo\\hpbar_health.ddj";
    CreateGameString(&hpStr, hpPath);
    LogMsg("[PlayerMiniInfo] Loading: %s", hpPath);
    
    IDirect3DBaseTexture9* pHpTex = LoadGameTexture(&hpStr);
    LogMsg("[PlayerMiniInfo] HP texture returned: %p", pHpTex);
    
    if (pHpTex) {
        m_texHpFill.pTexture = (IDirect3DTexture9*)pHpTex;
        D3DSURFACE_DESC desc;
        if (SUCCEEDED(m_texHpFill.pTexture->GetLevelDesc(0, &desc))) {
            m_texHpFill.width = desc.Width;
            m_texHpFill.height = desc.Height;
            LogMsg("[PlayerMiniInfo] HP texture loaded: %dx%d", desc.Width, desc.Height);
        }
    } else {
        LogMsg("[PlayerMiniInfo] FAILED to load HP texture!");
    }
    
    // Load MP bar fill texture
    GameString mpStr = {0, 0, 0};
    const char* mpPath = "newui\\playerminiinfo\\hpbar_mana.ddj";
    CreateGameString(&mpStr, mpPath);
    LogMsg("[PlayerMiniInfo] Loading: %s", mpPath);
    
    IDirect3DBaseTexture9* pMpTex = LoadGameTexture(&mpStr);
    LogMsg("[PlayerMiniInfo] MP texture returned: %p", pMpTex);
    
    if (pMpTex) {
        m_texMpFill.pTexture = (IDirect3DTexture9*)pMpTex;
        D3DSURFACE_DESC desc;
        if (SUCCEEDED(m_texMpFill.pTexture->GetLevelDesc(0, &desc))) {
            m_texMpFill.width = desc.Width;
            m_texMpFill.height = desc.Height;
            LogMsg("[PlayerMiniInfo] MP texture loaded: %dx%d", desc.Width, desc.Height);
        }
    } else {
        LogMsg("[PlayerMiniInfo] FAILED to load MP texture!");
    }
    
    // Load Hwan bar fill texture
    GameString hwanStr = {0, 0, 0};
    const char* hwanPath = "newui\\playerminiinfo\\hpbar_hwan.ddj";
    CreateGameString(&hwanStr, hwanPath);
    LogMsg("[PlayerMiniInfo] Loading: %s", hwanPath);
    
    IDirect3DBaseTexture9* pHwanTex = LoadGameTexture(&hwanStr);
    LogMsg("[PlayerMiniInfo] Hwan texture returned: %p", pHwanTex);
    
    if (pHwanTex) {
        m_texHwanFill.pTexture = (IDirect3DTexture9*)pHwanTex;
        D3DSURFACE_DESC desc;
        if (SUCCEEDED(m_texHwanFill.pTexture->GetLevelDesc(0, &desc))) {
            m_texHwanFill.width = desc.Width;
            m_texHwanFill.height = desc.Height;
            LogMsg("[PlayerMiniInfo] Hwan texture loaded: %dx%d", desc.Width, desc.Height);
        }
    } else {
        LogMsg("[PlayerMiniInfo] FAILED to load Hwan texture!");
    }
    
    m_bTexturesLoaded = true;  // Mark as loaded to avoid retry spam
    
    LogMsg("[PlayerMiniInfo] Texture loading complete. Background=%p, HP=%p, MP=%p, Hwan=%p",
           m_texBackground.pTexture, m_texHpFill.pTexture, 
           m_texMpFill.pTexture, m_texHwanFill.pTexture);
    
    return (m_texBackground.pTexture != NULL);
}

void CustomPlayerMiniInfo::ReleaseTextures() {
    // Note: Game manages texture memory, we don't release manually
    m_texBackground.pTexture = NULL;
    m_texHpFill.pTexture = NULL;
    m_texMpFill.pTexture = NULL;
    m_texHwanFill.pTexture = NULL;
    m_bTexturesLoaded = false;
}

void CustomPlayerMiniInfo::OnDeviceLost() {
    // Release textures on device lost
    ReleaseTextures();
}

void CustomPlayerMiniInfo::OnDeviceReset() {
    // Reload textures after device reset
    m_bTexturesLoaded = false;
}

void CustomPlayerMiniInfo::MenuItem() {
    if (ImGui::BeginMenu("Player Info")) {
        bool enabled = m_bEnabled;
        if (ImGui::MenuItem("Enable Custom UI", NULL, &enabled)) {
            if (enabled) {
                Enable();
            } else {
                Disable();
            }
        }
        
        ImGui::MenuItem("Show Stats Panel", NULL, &m_bShowStats);
        
        ImGui::EndMenu();
    }
}

void CustomPlayerMiniInfo::Render() {
    if (!m_bShow) return;
    
    // === D3D DEVICE LOST/RESET HANDLING ===
    // Check device state to avoid D3D errors on alt-tab or program switch
    if (g_CD3DApplication && g_CD3DApplication->m_pd3dDevice) {
        IDirect3DDevice9* pDevice = g_CD3DApplication->m_pd3dDevice;
        HRESULT hr = pDevice->TestCooperativeLevel();
        
        if (hr == D3DERR_DEVICELOST || hr == D3DERR_DEVICENOTRESET) {
            // Device is lost or needs reset, skip rendering
            return;
        }
        
        // Also check game's internal IsLost flag
        if (g_CD3DApplication->IsLost()) {
            return;
        }
    }
    
    // Check if UI should be visible (not during loading/teleport)
    if (!IsUIVisible()) {
        return;
    }
    
    // Load textures on first render
    if (!m_bTexturesLoaded) {
        LoadTextures();
    }
    
    // Hide native PlayerMiniInfo rendering and reposition it
    // BuffViewer calculates its position relative to PlayerMiniInfo
    // So HideNativePlayerMiniInfo() also repositions to control buff position
    if (m_bEnabled) {
        HideNativePlayerMiniInfo();
    }
    
    // Get player pointer safely
    DWORD pPlayerDW = 0;
    __try {
        pPlayerDW = *(DWORD*)ADDR_PLAYER_PTR;
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        return;
    }
    
    if (pPlayerDW == 0) {
        return;  // Player not loaded yet
    }
    
    CICPlayerEcsro* pPlayerEcsro = (CICPlayerEcsro*)pPlayerDW;
    
    // Read player data safely
    int currentHP = 0, maxHP = 1, currentMP = 0, maxMP = 1;
    BYTE level = 0, hwanPoint = 0;
    short strength = 0, intelligence = 0, statPoints = 0;
    char* charName = NULL;
    
    __try {
        currentHP = pPlayerEcsro->RemaingHP;
        maxHP = pPlayerEcsro->MaxHP;
        currentMP = pPlayerEcsro->RemaingMP;
        maxMP = pPlayerEcsro->MaxMP;
        level = pPlayerEcsro->Level;
        strength = pPlayerEcsro->Strength;
        intelligence = pPlayerEcsro->Intelligence;
        hwanPoint = pPlayerEcsro->HwanPoint;
        charName = pPlayerEcsro->charname;
        statPoints = pPlayerEcsro->StatPoints;
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        maxHP = 1;
        maxMP = 1;
    }
    
    // Calculate percentages
    float hpPercent = (maxHP > 0) ? (float)currentHP / (float)maxHP : 0.0f;
    float mpPercent = (maxMP > 0) ? (float)currentMP / (float)maxMP : 0.0f;
    
    // Clamp percentages
    if (hpPercent < 0.0f) hpPercent = 0.0f;
    if (hpPercent > 1.0f) hpPercent = 1.0f;
    if (mpPercent < 0.0f) mpPercent = 0.0f;
    if (mpPercent > 1.0f) mpPercent = 1.0f;
    
    // === NEW LAYOUT BASED ON PNG DESIGN ===
    // Window flags - no titlebar, no resize, no move, transparent background
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar |
                              ImGuiWindowFlags_NoResize |
                              ImGuiWindowFlags_NoScrollbar |
                              ImGuiWindowFlags_NoScrollWithMouse |
                              ImGuiWindowFlags_NoMove |
                              ImGuiWindowFlags_NoBackground;
    
    // =======================================================================
    // SCALE FACTOR - Adjust this to resize the entire UI proportionally
    // 1.0 = original size, 0.5 = half size, 0.3 = 30% size, etc.
    // =======================================================================
    const float SCALE = 1.0f;  // <-- CHANGE THIS VALUE TO RESIZE!
    
    // Use background texture dimensions if loaded, otherwise use defaults
    float bgWidth = m_texBackground.pTexture ? (float)m_texBackground.width * SCALE : 500.0f * SCALE;
    float bgHeight = m_texBackground.pTexture ? (float)m_texBackground.height * SCALE : 120.0f * SCALE;
    
    // Fixed position - top left
    ImGui::SetNextWindowPos(ImVec2(3.0f, 3.0f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(bgWidth + 10, bgHeight + 10), ImGuiCond_Always);
    
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    
    if (ImGui::Begin("##CustomPlayerMiniInfo", NULL, flags)) {
        ImVec2 windowPos = ImGui::GetWindowPos();
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        
        // === BUFF POSITION DEBUG SLIDERS (declared early for use in MoveGWnd) ===
        static float dbg_BuffX = 70.948f;  // Buff viewer X offset from window
        static float dbg_BuffY = 94.801f;  // Buff viewer Y offset from window
        
        // === SYNC NATIVE WINDOW POSITION FOR BUFF ICONS ===
        // Use CGInterface IRM to get actual window instances
        // GDR_PLAYER_MINI_INFO = ID 11 (from ginterface.txt)
        #define GDR_PLAYER_MINI_INFO 11
        
        // Debug log - write to file once per second max
        static DWORD lastLogTime = 0;
        DWORD currentTime = GetTickCount();
        bool shouldLog = (currentTime - lastLogTime > 1000);
        
        __try {
            // Get actual window instance via IRM
            CIFPlayerMiniInfo* pNativeMiniInfo = NULL;
            if (g_pCGInterface) {
                pNativeMiniInfo = (CIFPlayerMiniInfo*)g_pCGInterface->m_IRM.GetResObj(GDR_PLAYER_MINI_INFO, 1);
            }
            
            if (shouldLog) {
                lastLogTime = currentTime;
                FILE* f = fopen("buff_debug.txt", "a");
                if (f) {
                    fprintf(f, "[%d] ImGui pos: %.1f, %.1f\n", currentTime, windowPos.x, windowPos.y);
                    fprintf(f, "  g_pCGInterface: %p\n", g_pCGInterface);
                    fprintf(f, "  MiniInfo via IRM (ID 11): %p\n", pNativeMiniInfo);
                    
                    if (pNativeMiniInfo) {
                        // Read position from offset 0x3C and 0x40 
                        int miniX = *(int*)((DWORD)pNativeMiniInfo + 0x3C);
                        int miniY = *(int*)((DWORD)pNativeMiniInfo + 0x40);
                        fprintf(f, "  MiniInfo X/Y (0x3C/0x40): %d, %d\n", miniX, miniY);
                    }
                    
                    // Check MagicStateBoard (ID 22) - might be buff display
                    #define GDR_MAGICSTATEBOARD 22
                    CIFWnd* pMagicStateBoard = g_pCGInterface->m_IRM.GetResObj(GDR_MAGICSTATEBOARD, 1);
                    fprintf(f, "  MagicStateBoard (ID 22): %p\n", pMagicStateBoard);
                    if (pMagicStateBoard) {
                        int msbX = *(int*)((DWORD)pMagicStateBoard + 0x3C);
                        int msbY = *(int*)((DWORD)pMagicStateBoard + 0x40);
                        fprintf(f, "  MagicStateBoard X/Y: %d, %d\n", msbX, msbY);
                    }
                    fprintf(f, "\n");
                    fclose(f);
                }
            }
            
            if (pNativeMiniInfo) {
                // Move native window to match ImGui position
                pNativeMiniInfo->MoveGWnd((int)windowPos.x, (int)windowPos.y);
            }
            
            // Move MagicStateBoard (BUFF VIEWER - Active Buff Container) relative to our window
            // GDR_MAGICSTATEBOARD (ID 22) controls player buff icons position
            #ifndef GDR_MAGICSTATEBOARD
            #define GDR_MAGICSTATEBOARD 22
            #endif
            CIFWnd* pMagicStateBoard = g_pCGInterface->m_IRM.GetResObj(GDR_MAGICSTATEBOARD, 1);
            if (pMagicStateBoard) {
                // Position buffs using debug slider values (adjustable at runtime)
                int buffX = (int)windowPos.x + (int)dbg_BuffX;
                int buffY = (int)windowPos.y + (int)dbg_BuffY;
                pMagicStateBoard->MoveGWnd(buffX, buffY);
            }
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            // Ignore exceptions
            FILE* f = fopen("buff_debug.txt", "a");
            if (f) {
                fprintf(f, "[%d] EXCEPTION in buff sync!\n", currentTime);
                fclose(f);
            }
        }
        
        // === LAYER 1: DRAW MAINFRAME BACKGROUND ===
        if (m_texBackground.pTexture) {
            ImVec2 bgMin = windowPos;
            ImVec2 bgMax = ImVec2(windowPos.x + bgWidth, windowPos.y + bgHeight);
            drawList->AddImage((ImTextureID)m_texBackground.pTexture, bgMin, bgMax);
        }
        
        // === DEBUG: Runtime position adjustment ===
        // TODO: Remove this debug window after finding correct values
        static float dbg_PortraitX = 48.0f;
        static float dbg_PortraitY = 52.0f;
        static float dbg_LevelX = 93.0f;
        static float dbg_LevelY = 24.5f;
        static float dbg_LevelFrameX = 92.0f;
        static float dbg_LevelFrameY = 25.0f;
        static float dbg_NameX = 164.5f;
        static float dbg_NameY = 23.4f;
        static float dbg_ButtonX1 = 200.0f;
        static float dbg_ButtonX2 = 230.0f;
        static float dbg_ButtonY = 3.0f;
        static float dbg_HpBarX = 92.0f;
        static float dbg_HpBarY = 43.4f;
        static float dbg_HpBarW = 175.0f;
        static float dbg_HpBarH = 10.0f;
        static float dbg_MpBarX = 88.0f;
        static float dbg_MpBarY = 59.0f;
        static float dbg_MpBarW = 175.0f;
        static float dbg_MpBarH = 10.0f;
        static float dbg_HwanBarX = 81.9f;
        static float dbg_HwanBarY = 72.5f;
        static float dbg_HwanBarW = 177.6f;
        static float dbg_HwanBarH = 8.9f;
        // NOTE: dbg_BuffX and dbg_BuffY are declared earlier (before MoveGWnd usage)
        
        // Debug window
        if (ImGui::Begin("MiniInfo Debug")) {
            ImGui::Text("Portrait");
            ImGui::SliderFloat("Portrait X", &dbg_PortraitX, 0, 150);
            ImGui::SliderFloat("Portrait Y", &dbg_PortraitY, 0, 100);
            ImGui::Separator();
            ImGui::Text("Level Text");
            ImGui::SliderFloat("LvlTxt X", &dbg_LevelX, 0, 150);
            ImGui::SliderFloat("LvlTxt Y", &dbg_LevelY, 0, 100);
            ImGui::Text("Level Frame");
            ImGui::SliderFloat("LvlFrm X", &dbg_LevelFrameX, 0, 150);
            ImGui::SliderFloat("LvlFrm Y", &dbg_LevelFrameY, 0, 100);
            ImGui::Separator();
            ImGui::Text("Name");
            ImGui::SliderFloat("Name X", &dbg_NameX, 0, 280);
            ImGui::SliderFloat("Name Y", &dbg_NameY, 0, 100);
            ImGui::Separator();
            ImGui::Text("Buttons");
            ImGui::SliderFloat("Btn1 X", &dbg_ButtonX1, 0, 280);
            ImGui::SliderFloat("Btn2 X", &dbg_ButtonX2, 0, 280);
            ImGui::SliderFloat("Btn Y", &dbg_ButtonY, 0, 50);
            ImGui::Separator();
            ImGui::Text("HP Bar");
            ImGui::SliderFloat("HP X", &dbg_HpBarX, 0, 200);
            ImGui::SliderFloat("HP Y", &dbg_HpBarY, 0, 100);
            ImGui::SliderFloat("HP W", &dbg_HpBarW, 50, 200);
            ImGui::SliderFloat("HP H", &dbg_HpBarH, 5, 30);
            ImGui::Separator();
            ImGui::Text("MP Bar");
            ImGui::SliderFloat("MP X", &dbg_MpBarX, 0, 200);
            ImGui::SliderFloat("MP Y", &dbg_MpBarY, 0, 100);
            ImGui::SliderFloat("MP W", &dbg_MpBarW, 50, 200);
            ImGui::SliderFloat("MP H", &dbg_MpBarH, 5, 30);
            ImGui::Separator();
            ImGui::Text("Hwan Bar");
            ImGui::SliderFloat("Hwan X", &dbg_HwanBarX, 0, 200);
            ImGui::SliderFloat("Hwan Y", &dbg_HwanBarY, 0, 100);
            ImGui::SliderFloat("Hwan W", &dbg_HwanBarW, 50, 200);
            ImGui::SliderFloat("Hwan H", &dbg_HwanBarH, 5, 30);
            ImGui::Separator();
            ImGui::Text("Buff Viewer (MagicStateBoard)");
            ImGui::SliderFloat("Buff X", &dbg_BuffX, 0, 800);
            ImGui::SliderFloat("Buff Y", &dbg_BuffY, 0, 200);
        }
        ImGui::End();
        
        // Use debug values (apply SCALE)
        const float PORTRAIT_CENTER_X = dbg_PortraitX * SCALE;
        const float PORTRAIT_CENTER_Y = dbg_PortraitY * SCALE;
        const float LEVEL_X = dbg_LevelX * SCALE;
        const float LEVEL_Y = dbg_LevelY * SCALE;
        const float NAME_X = dbg_NameX * SCALE;
        const float NAME_Y = dbg_NameY * SCALE;
        const float BUTTON_X1 = dbg_ButtonX1 * SCALE;
        const float BUTTON_X2 = dbg_ButtonX2 * SCALE;
        const float BUTTON_Y = dbg_ButtonY * SCALE;
        
        const float HP_BAR_X = dbg_HpBarX * SCALE;
        const float HP_BAR_Y = dbg_HpBarY * SCALE;
        const float HP_BAR_WIDTH = dbg_HpBarW * SCALE;
        const float HP_BAR_HEIGHT = dbg_HpBarH * SCALE;
        
        const float MP_BAR_X = dbg_MpBarX * SCALE;
        const float MP_BAR_Y = dbg_MpBarY * SCALE;
        const float MP_BAR_WIDTH = dbg_MpBarW * SCALE;
        const float MP_BAR_HEIGHT = dbg_MpBarH * SCALE;
        
        const float HWAN_BAR_X = dbg_HwanBarX * SCALE;
        const float HWAN_BAR_Y = dbg_HwanBarY * SCALE;
        const float HWAN_BAR_WIDTH = dbg_HwanBarW * SCALE;
        const float HWAN_BAR_HEIGHT = dbg_HwanBarH * SCALE;
        
        // === Model ID will be drawn at the very end (on top of everything) ===
        CharacterPortrait& portrait = GetCharacterPortrait();
        portrait.Update();
        int modelID = portrait.GetModelID();
        
        // === RED AREA: Level number only ===
        char levelText[16];
        sprintf(levelText, "%d", level);
        
        // Use slightly larger font for level (default + 1 pixels)
        ImFont* font = ImGui::GetFont();
        float levelFontSize = ImGui::GetFontSize() + 1.0f;
        ImVec2 levelTextSize = font->CalcTextSizeA(levelFontSize, FLT_MAX, 0.0f, levelText);
        float levelTextX = windowPos.x + LEVEL_X - levelTextSize.x * 0.5f;
        float levelTextY = windowPos.y + LEVEL_Y - levelTextSize.y * 0.5f;
        
        // Level frame and text are now drawn at the end (after portrait frame)
        
        // === PINK AREA: Character name ===
        if (charName && charName[0] != '\0') {
            float nameX = windowPos.x + NAME_X;
            float nameY = windowPos.y + NAME_Y;
            
            // Shadow
            drawList->AddText(ImVec2(nameX + 1, nameY + 1), IM_COL32(0, 0, 0, 200), charName);
            // White name text
            drawList->AddText(ImVec2(nameX, nameY), IM_COL32(255, 255, 255, 255), charName);
        }
        
        // === BLUE AREAS: Stat and RS buttons (text for now) ===
        // Stat button [S]
        const char* statBtnText = "[S]";
        float statBtnX = windowPos.x + BUTTON_X1;
        float statBtnY = windowPos.y + BUTTON_Y;
        
        ImVec2 statBtnSize = ImGui::CalcTextSize(statBtnText);
        ImVec2 statBtnMin = ImVec2(statBtnX - 2, statBtnY - 2);
        ImVec2 statBtnMax = ImVec2(statBtnX + statBtnSize.x + 2, statBtnY + statBtnSize.y + 2);
        
        // Check hover/click for Stat button
        ImVec2 mousePos = ImGui::GetMousePos();
        bool statHovered = (mousePos.x >= statBtnMin.x && mousePos.x <= statBtnMax.x &&
                           mousePos.y >= statBtnMin.y && mousePos.y <= statBtnMax.y);
        
        ImU32 statColor = statHovered ? IM_COL32(100, 200, 255, 255) : IM_COL32(150, 180, 255, 255);
        drawList->AddText(ImVec2(statBtnX + 1, statBtnY + 1), IM_COL32(0, 0, 0, 200), statBtnText);
        drawList->AddText(ImVec2(statBtnX, statBtnY), statColor, statBtnText);
        
        if (statHovered && ImGui::IsMouseClicked(0)) {
            m_bShowStatsPopup = !m_bShowStatsPopup;
        }
        
        // RS button (only if stat points available)
        if (statPoints > 0) {
            const char* rsBtnText = "[RS]";
            float rsBtnX = windowPos.x + BUTTON_X2;
            float rsBtnY = windowPos.y + BUTTON_Y;
            
            ImVec2 rsBtnSize = ImGui::CalcTextSize(rsBtnText);
            ImVec2 rsBtnMin = ImVec2(rsBtnX - 2, rsBtnY - 2);
            ImVec2 rsBtnMax = ImVec2(rsBtnX + rsBtnSize.x + 2, rsBtnY + rsBtnSize.y + 2);
            
            bool rsHovered = (mousePos.x >= rsBtnMin.x && mousePos.x <= rsBtnMax.x &&
                             mousePos.y >= rsBtnMin.y && mousePos.y <= rsBtnMax.y);
            
            ImU32 rsColor = rsHovered ? IM_COL32(255, 150, 150, 255) : IM_COL32(255, 100, 100, 255);
            drawList->AddText(ImVec2(rsBtnX + 1, rsBtnY + 1), IM_COL32(0, 0, 0, 200), rsBtnText);
            drawList->AddText(ImVec2(rsBtnX, rsBtnY), rsColor, rsBtnText);
            
            if (rsHovered && ImGui::IsMouseClicked(0)) {
                // Open character window
                __try {
                    if (g_pCGInterface) {
                        typedef int (__thiscall *ToggleCharWindowFn)(void*);
                        ToggleCharWindowFn pToggle = (ToggleCharWindowFn)0x4FC780;
                        pToggle(g_pCGInterface);
                    }
                } __except(EXCEPTION_EXECUTE_HANDLER) {
                }
            }
        }
        
        // === HP BAR with texture and text overlay ===
        float hpBarX = windowPos.x + HP_BAR_X;
        float hpBarY = windowPos.y + HP_BAR_Y;
        float hpFillWidth = HP_BAR_WIDTH * hpPercent;
        
        if (m_texHpFill.pTexture && hpPercent > 0.0f) {
            // Draw HP fill with UV clipping based on percentage
            ImVec2 hpMin = ImVec2(hpBarX, hpBarY);
            ImVec2 hpMax = ImVec2(hpBarX + hpFillWidth, hpBarY + HP_BAR_HEIGHT);
            ImVec2 uvMin = ImVec2(0.0f, 0.0f);
            ImVec2 uvMax = ImVec2(hpPercent, 1.0f);
            drawList->AddImage((ImTextureID)m_texHpFill.pTexture, hpMin, hpMax, uvMin, uvMax);
        }
        
        // HP text overlay: current/max
        char hpText[64];
        sprintf(hpText, "%d / %d", currentHP, maxHP);
        ImVec2 hpTextSize = ImGui::CalcTextSize(hpText);
        float hpTextX = hpBarX + (HP_BAR_WIDTH - hpTextSize.x) * 0.5f;
        float hpTextY = hpBarY + (HP_BAR_HEIGHT - hpTextSize.y) * 0.5f;
        
        // Shadow
        drawList->AddText(ImVec2(hpTextX + 1, hpTextY + 1), IM_COL32(0, 0, 0, 200), hpText);
        // White text
        drawList->AddText(ImVec2(hpTextX, hpTextY), IM_COL32(255, 255, 255, 255), hpText);
        
        // === MP BAR with texture and text overlay ===
        float mpBarX = windowPos.x + MP_BAR_X;
        float mpBarY = windowPos.y + MP_BAR_Y;
        float mpFillWidth = MP_BAR_WIDTH * mpPercent;
        
        if (m_texMpFill.pTexture && mpPercent > 0.0f) {
            ImVec2 mpMin = ImVec2(mpBarX, mpBarY);
            ImVec2 mpMax = ImVec2(mpBarX + mpFillWidth, mpBarY + MP_BAR_HEIGHT);
            ImVec2 uvMin = ImVec2(0.0f, 0.0f);
            ImVec2 uvMax = ImVec2(mpPercent, 1.0f);
            drawList->AddImage((ImTextureID)m_texMpFill.pTexture, mpMin, mpMax, uvMin, uvMax);
        }
        
        // MP text overlay: current/max
        char mpText[64];
        sprintf(mpText, "%d / %d", currentMP, maxMP);
        ImVec2 mpTextSize = ImGui::CalcTextSize(mpText);
        float mpTextX = mpBarX + (MP_BAR_WIDTH - mpTextSize.x) * 0.5f;
        float mpTextY = mpBarY + (MP_BAR_HEIGHT - mpTextSize.y) * 0.5f;
        
        // Shadow
        drawList->AddText(ImVec2(mpTextX + 1, mpTextY + 1), IM_COL32(0, 0, 0, 200), mpText);
        // White text
        drawList->AddText(ImVec2(mpTextX, mpTextY), IM_COL32(255, 255, 255, 255), mpText);
        
        // === HWAN BAR (if any) ===
        if (hwanPoint > 0 && m_texHwanFill.pTexture) {
            float hwanPercent = (float)hwanPoint / 5.0f;  // Max 5 hwan points
            float hwanBarX = windowPos.x + HWAN_BAR_X;
            float hwanBarY = windowPos.y + HWAN_BAR_Y;
            float hwanFillWidth = HWAN_BAR_WIDTH * hwanPercent;
            
            ImVec2 hwanMin = ImVec2(hwanBarX, hwanBarY);
            ImVec2 hwanMax = ImVec2(hwanBarX + hwanFillWidth, hwanBarY + HWAN_BAR_HEIGHT);
            ImVec2 uvMin = ImVec2(0.0f, 0.0f);
            ImVec2 uvMax = ImVec2(hwanPercent, 1.0f);
            drawList->AddImage((ImTextureID)m_texHwanFill.pTexture, hwanMin, hwanMax, uvMin, uvMax);
            // Hwan text removed - no text overlay needed
        }
        
        // Hwan button will be drawn at the very end
        
        // === LAYER: PORTRAIT BG (on top of bars, at X=2, Y=2) ===
        if (m_texPortraitBg.pTexture) {
            float portraitBgW = (float)m_texPortraitBg.width * SCALE;
            float portraitBgH = (float)m_texPortraitBg.height * SCALE;
            ImVec2 pBgMin = ImVec2(windowPos.x + 2.0f * SCALE, windowPos.y + 2.0f * SCALE);
            ImVec2 pBgMax = ImVec2(pBgMin.x + portraitBgW, pBgMin.y + portraitBgH);
            drawList->AddImage((ImTextureID)m_texPortraitBg.pTexture, pBgMin, pBgMax);
        }
        
        // === LAYER: PORTRAIT FRAME (top layer, at X=0, Y=0) ===
        if (m_texPortraitFrame.pTexture) {
            float portraitFrameW = (float)m_texPortraitFrame.width * SCALE;
            float portraitFrameH = (float)m_texPortraitFrame.height * SCALE;
            ImVec2 pFrameMin = ImVec2(windowPos.x, windowPos.y);
            ImVec2 pFrameMax = ImVec2(pFrameMin.x + portraitFrameW, pFrameMin.y + portraitFrameH);
            drawList->AddImage((ImTextureID)m_texPortraitFrame.pTexture, pFrameMin, pFrameMax);
        }
        
        // === LAYER: LEVEL FRAME (on top of portrait, separate position from level text) ===
        if (m_texLevelFrame.pTexture) {
            float lvlFrameW = (float)m_texLevelFrame.width * SCALE;
            float lvlFrameH = (float)m_texLevelFrame.height * SCALE;
            // Use separate level frame position (dbg_LevelFrameX/Y)
            float lvlFrameX = windowPos.x + dbg_LevelFrameX * SCALE - lvlFrameW * 0.5f;
            float lvlFrameY = windowPos.y + dbg_LevelFrameY * SCALE - lvlFrameH * 0.5f;
            ImVec2 lvlFrameMin = ImVec2(lvlFrameX, lvlFrameY);
            ImVec2 lvlFrameMax = ImVec2(lvlFrameX + lvlFrameW, lvlFrameY + lvlFrameH);
            drawList->AddImage((ImTextureID)m_texLevelFrame.pTexture, lvlFrameMin, lvlFrameMax);
        }
        
        // Level text on top of level frame (recalculate since we're in render scope)
        {
            char lvlText[16];
            sprintf(lvlText, "%d", level);
            ImFont* lvlFont = ImGui::GetFont();
            float lvlFontSize = ImGui::GetFontSize() + 1.0f;
            ImVec2 lvlTextSize = lvlFont->CalcTextSizeA(lvlFontSize, FLT_MAX, 0.0f, lvlText);
            float lvlTextX = windowPos.x + LEVEL_X - lvlTextSize.x * 0.5f;
            float lvlTextY = windowPos.y + LEVEL_Y - lvlTextSize.y * 0.5f;
            
            // Shadow
            drawList->AddText(lvlFont, lvlFontSize, ImVec2(lvlTextX + 1, lvlTextY + 1), IM_COL32(0, 0, 0, 200), lvlText);
            // Gold level text
            drawList->AddText(lvlFont, lvlFontSize, ImVec2(lvlTextX, lvlTextY), IM_COL32(255, 215, 0, 255), lvlText);
        }
        
        // === MODEL ID (on top of everything) ===
        {
            char modelText[32];
            sprintf(modelText, "ID: %d", modelID);
            ImVec2 modelTextSize = ImGui::CalcTextSize(modelText);
            float modelTextX = windowPos.x + PORTRAIT_CENTER_X - modelTextSize.x * 0.5f;
            float modelTextY = windowPos.y + PORTRAIT_CENTER_Y - modelTextSize.y * 0.5f;
            
            // Shadow
            drawList->AddText(ImVec2(modelTextX + 1, modelTextY + 1), IM_COL32(0, 0, 0, 200), modelText);
            // Text
            drawList->AddText(ImVec2(modelTextX, modelTextY), IM_COL32(200, 200, 255, 255), modelText);
        }
        
        // Self-target: Click on portrait area to target self
        ImVec2 portraitMin = ImVec2(windowPos.x + PORTRAIT_CENTER_X - 60, windowPos.y + PORTRAIT_CENTER_Y - 60);
        ImVec2 portraitMax = ImVec2(windowPos.x + PORTRAIT_CENTER_X + 60, windowPos.y + PORTRAIT_CENTER_Y + 60);
        
        if (mousePos.x >= portraitMin.x && mousePos.x <= portraitMax.x &&
            mousePos.y >= portraitMin.y && mousePos.y <= portraitMax.y &&
            ImGui::IsMouseClicked(0)) {
            SendSelfTargetPacket();
        }
    }
    ImGui::End();
    
    ImGui::PopStyleVar();
    
    // Render stats popup if open
    if (m_bShowStatsPopup) {
        RenderStatsPopup(pPlayerEcsro);
    }
}

void CustomPlayerMiniInfo::RenderStatsPopup(CICPlayerEcsro* pPlayer) {
    ImGuiWindowFlags popupFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize;
    
    if (ImGui::Begin("Character Stats", &m_bShowStatsPopup, popupFlags)) {
        // Read basic stats safely
        BYTE level = 0;
        short str = 0, intel = 0, statPoints = 0;
        int currentHP = 0, maxHP = 1, currentMP = 0, maxMP = 1;
        
        // Combat stats (from combat stats struct at 0xA01510)
        int phyAtkMin = 0, phyAtkMax = 0;
        int magAtkMin = 0, magAtkMax = 0;
        int phyDef = 0, magDef = 0;
        int phyBalance = 0, magBalance = 0;
        int hitRatio = 0, parryRatio = 0;
        
        __try {
            // Basic stats from player struct
            level = pPlayer->Level;
            str = pPlayer->Strength;
            intel = pPlayer->Intelligence;
            statPoints = pPlayer->StatPoints;
            currentHP = pPlayer->RemaingHP;
            maxHP = pPlayer->MaxHP;
            currentMP = pPlayer->RemaingMP;
            maxMP = pPlayer->MaxMP;
            
            // Combat stats - read from global combat stats struct
            DWORD pCombatStats = 0xA01510 + 0xFC;  // 0xA0160C
            if (pCombatStats != 0) {
                phyAtkMin = *(int*)(pCombatStats + 0x00);
                phyAtkMax = *(int*)(pCombatStats + 0x04);
                magAtkMin = *(int*)(pCombatStats + 0x08);
                magAtkMax = *(int*)(pCombatStats + 0x0C);
                phyDef = *(short*)(pCombatStats + 0x10);
                magDef = *(short*)(pCombatStats + 0x12);  
                hitRatio = *(short*)(pCombatStats + 0x14);
                parryRatio = *(short*)(pCombatStats + 0x16);
                
                if (level > 0) {
                    double phyBalanceCalc = (double)(str + 2 * level + 14) * 100.0 
                                          / ((double)(level - 1) * 7.0 + 56.0) / 0.857142857;
                    phyBalance = (int)phyBalanceCalc;
                    if (phyBalance > 120) phyBalance = 120;
                    
                    double magBalanceCalc = (double)intel * 100.0 
                                          / ((double)(5 * (level + 7))) / 0.8;
                    magBalance = (int)magBalanceCalc;
                    if (magBalance > 120) magBalance = 120;
                }
            }
        } __except(EXCEPTION_EXECUTE_HANDLER) {
        }
        
        // Display stats in formatted layout
        ImGui::TextColored(ImVec4(1.0f, 0.9f, 0.4f, 1.0f), "=== Character Stats ===");
        ImGui::Separator();
        
        ImGui::Columns(2, NULL, false);
        ImGui::Text("Level:"); ImGui::NextColumn(); ImGui::Text("%d", level); ImGui::NextColumn();
        ImGui::Text("HP:"); ImGui::NextColumn(); ImGui::Text("%d / %d", currentHP, maxHP); ImGui::NextColumn();
        ImGui::Text("MP:"); ImGui::NextColumn(); ImGui::Text("%d / %d", currentMP, maxMP); ImGui::NextColumn();
        ImGui::Columns(1);
        
        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.5f, 1.0f, 0.5f, 1.0f), "=== Base Stats ===");
        
        ImGui::Columns(2, NULL, false);
        ImGui::Text("STR:"); ImGui::NextColumn(); ImGui::Text("%d", str); ImGui::NextColumn();
        ImGui::Text("INT:"); ImGui::NextColumn(); ImGui::Text("%d", intel); ImGui::NextColumn();
        ImGui::Text("Remaining Stat Points:"); ImGui::NextColumn(); ImGui::Text("%d", statPoints); ImGui::NextColumn();
        ImGui::Columns(1);
        
        ImGui::Separator();
        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.5f, 1.0f), "=== Combat Stats ===");
        
        ImGui::Columns(2, NULL, false);
        ImGui::Text("Phy. Atk:"); ImGui::NextColumn(); ImGui::Text("%d ~ %d", phyAtkMin, phyAtkMax); ImGui::NextColumn();
        ImGui::Text("Phy. Def:"); ImGui::NextColumn(); ImGui::Text("%d", phyDef); ImGui::NextColumn();
        ImGui::Text("Mag. Atk:"); ImGui::NextColumn(); ImGui::Text("%d ~ %d", magAtkMin, magAtkMax); ImGui::NextColumn();
        ImGui::Text("Mag. Def:"); ImGui::NextColumn(); ImGui::Text("%d", magDef); ImGui::NextColumn();
        ImGui::Separator();
        ImGui::Text("Phy. Balance:"); ImGui::NextColumn(); ImGui::Text("%d%%", phyBalance); ImGui::NextColumn();
        ImGui::Text("Mag. Balance:"); ImGui::NextColumn(); ImGui::Text("%d%%", magBalance); ImGui::NextColumn();
        ImGui::Text("Hit Ratio:"); ImGui::NextColumn(); ImGui::Text("%d", hitRatio); ImGui::NextColumn();
        ImGui::Text("Parry Ratio:"); ImGui::NextColumn(); ImGui::Text("%d", parryRatio); ImGui::NextColumn();
        ImGui::Columns(1);
    }
    ImGui::End();
}

void CustomPlayerMiniInfo::Enable() {
    m_bEnabled = true;
}

void CustomPlayerMiniInfo::Disable() {
    m_bEnabled = false;
}

bool CustomPlayerMiniInfo::IsEnabled() const {
    return m_bEnabled;
}

void CustomPlayerMiniInfo::SetPosition(float x, float y) {
    m_fPosX = x;
    m_fPosY = y;
}

void CustomPlayerMiniInfo::GetPosition(float& x, float& y) const {
    x = m_fPosX;
    y = m_fPosY;
}

// Stub implementations - TODO: implement if needed
void CustomPlayerMiniInfo::RenderHPBar(CICPlayerEcsro* pPlayer) {}
void CustomPlayerMiniInfo::RenderMPBar(CICPlayerEcsro* pPlayer) {}
void CustomPlayerMiniInfo::RenderHwanBar(CICPlayerEcsro* pPlayer) {}
void CustomPlayerMiniInfo::RenderLevelInfo(CICPlayerEcsro* pPlayer) {}
void CustomPlayerMiniInfo::RenderZerkPoints(CIFPlayerMiniInfo* pMiniInfo) {}
void CustomPlayerMiniInfo::RenderStatsPanel(CICPlayerEcsro* pPlayer) {}
void CustomPlayerMiniInfo::HideOriginalGUI() {}
void CustomPlayerMiniInfo::ShowOriginalGUI() {}
CIFPlayerMiniInfo* CustomPlayerMiniInfo::GetOriginalPlayerMiniInfo() { return NULL; }
