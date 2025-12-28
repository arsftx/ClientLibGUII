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
#include "UIRenderer.h"
#include "SROProgressBar.h"
#include "NativeBarRenderer.h"

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

// Helper function to send Hwan activation packet (separate to avoid SEH/object unwinding conflict)
static void SendHwanActivationPacket() {
    NEWMSG(0x70A7)
    pReq << (BYTE)1;
    SENDMSG()
}

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

// Debug logging disabled for production
static void LogMsg(const char* fmt, ...) {
    // No-op - logging disabled
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
    
    // Initialize CustomMinimap (test minimap) - lazy loading of textures
    extern void InitializeCustomMinimap();
    InitializeCustomMinimap();
    
    // Initialize CustomWorldMap (full world map overlay)
    extern void InitializeCustomWorldMap();
    InitializeCustomWorldMap();
    
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
    m_pSprite = NULL;  // Native DirectX sprite renderer
    
    // Animation state initialization
    m_displayHP = 0.0f;
    m_displayMP = 0.0f;
    m_lastHP = 0;
    m_damageFlashTimer = 0.0f;
    
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
    
    // Load Character Icon texture (S button replacement)
    GameString charIconStr = {0, 0, 0};
    const char* charIconPath = "newui\\playerminiinfo\\icon_character.ddj";
    CreateGameString(&charIconStr, charIconPath);
    LogMsg("[PlayerMiniInfo] Loading: %s", charIconPath);
    
    IDirect3DBaseTexture9* pCharIconTex = LoadGameTexture(&charIconStr);
    if (pCharIconTex) {
        m_texCharIcon.pTexture = (IDirect3DTexture9*)pCharIconTex;
        D3DSURFACE_DESC desc;
        if (SUCCEEDED(m_texCharIcon.pTexture->GetLevelDesc(0, &desc))) {
            m_texCharIcon.width = desc.Width;
            m_texCharIcon.height = desc.Height;
            LogMsg("[PlayerMiniInfo] CharIcon texture loaded: %dx%d", desc.Width, desc.Height);
        }
    } else {
        LogMsg("[PlayerMiniInfo] FAILED to load CharIcon texture!");
    }
    
    // Load Stat Icon texture (RS button replacement)
    GameString statIconStr = {0, 0, 0};
    const char* statIconPath = "newui\\playerminiinfo\\icon_stat.ddj";
    CreateGameString(&statIconStr, statIconPath);
    LogMsg("[PlayerMiniInfo] Loading: %s", statIconPath);
    
    IDirect3DBaseTexture9* pStatIconTex = LoadGameTexture(&statIconStr);
    if (pStatIconTex) {
        m_texStatIcon.pTexture = (IDirect3DTexture9*)pStatIconTex;
        D3DSURFACE_DESC desc;
        if (SUCCEEDED(m_texStatIcon.pTexture->GetLevelDesc(0, &desc))) {
            m_texStatIcon.width = desc.Width;
            m_texStatIcon.height = desc.Height;
            LogMsg("[PlayerMiniInfo] StatIcon texture loaded: %dx%d", desc.Width, desc.Height);
        }
    } else {
        LogMsg("[PlayerMiniInfo] FAILED to load StatIcon texture!");
    }
    
    // Load Hwan Icon texture (shows when hwan = 5)
    GameString hwanIconStr = {0, 0, 0};
    const char* hwanIconPath = "newui\\playerminiinfo\\icon_hwan.ddj";
    CreateGameString(&hwanIconStr, hwanIconPath);
    LogMsg("[PlayerMiniInfo] Loading: %s", hwanIconPath);
    
    IDirect3DBaseTexture9* pHwanIconTex = LoadGameTexture(&hwanIconStr);
    if (pHwanIconTex) {
        m_texHwanIcon.pTexture = (IDirect3DTexture9*)pHwanIconTex;
        D3DSURFACE_DESC desc;
        if (SUCCEEDED(m_texHwanIcon.pTexture->GetLevelDesc(0, &desc))) {
            m_texHwanIcon.width = desc.Width;
            m_texHwanIcon.height = desc.Height;
            LogMsg("[PlayerMiniInfo] HwanIcon texture loaded: %dx%d", desc.Width, desc.Height);
        }
    } else {
        LogMsg("[PlayerMiniInfo] FAILED to load HwanIcon texture!");
    }
    
    // Load Stats Popup background texture
    GameString statsPopupBgStr = {0, 0, 0};
    const char* statsPopupBgPath = "newui\\window\\window_decorated.ddj";
    CreateGameString(&statsPopupBgStr, statsPopupBgPath);
    LogMsg("[PlayerMiniInfo] Loading: %s", statsPopupBgPath);
    
    IDirect3DBaseTexture9* pStatsPopupBgTex = LoadGameTexture(&statsPopupBgStr);
    if (pStatsPopupBgTex) {
        m_texStatsPopupBg.pTexture = (IDirect3DTexture9*)pStatsPopupBgTex;
        D3DSURFACE_DESC desc;
        if (SUCCEEDED(m_texStatsPopupBg.pTexture->GetLevelDesc(0, &desc))) {
            m_texStatsPopupBg.width = desc.Width;
            m_texStatsPopupBg.height = desc.Height;
            LogMsg("[PlayerMiniInfo] StatsPopupBg texture loaded: %dx%d", desc.Width, desc.Height);
        }
    } else {
        LogMsg("[PlayerMiniInfo] FAILED to load StatsPopupBg texture!");
    }
    
    // Load Title Bar texture
    GameString titleBarStr = {0, 0, 0};
    const char* titleBarPath = "newui\\window\\title_standart_1.ddj";
    CreateGameString(&titleBarStr, titleBarPath);
    LogMsg("[PlayerMiniInfo] Loading: %s", titleBarPath);
    
    IDirect3DBaseTexture9* pTitleBarTex = LoadGameTexture(&titleBarStr);
    if (pTitleBarTex) {
        m_texTitleBar.pTexture = (IDirect3DTexture9*)pTitleBarTex;
        D3DSURFACE_DESC desc;
        if (SUCCEEDED(m_texTitleBar.pTexture->GetLevelDesc(0, &desc))) {
            m_texTitleBar.width = desc.Width;
            m_texTitleBar.height = desc.Height;
            LogMsg("[PlayerMiniInfo] TitleBar texture loaded: %dx%d", desc.Width, desc.Height);
        }
    } else {
        LogMsg("[PlayerMiniInfo] FAILED to load TitleBar texture!");
    }
    
    // Load Exit Button textures (3 states: normal, hovered, pressed)
    const char* exitBtnPaths[] = {
        "newui\\buttons\\buttonexit_standart_normal.ddj",
        "newui\\buttons\\buttonexit_standart_hovered.ddj",
        "newui\\buttons\\buttonexit_standart_pressed.ddj"
    };
    TextureInfo* exitBtnTextures[] = {
        &m_texExitBtnNormal, &m_texExitBtnHover, &m_texExitBtnPressed
    };
    
    for (int i = 0; i < 3; i++) {
        GameString btnStr = {0, 0, 0};
        CreateGameString(&btnStr, exitBtnPaths[i]);
        LogMsg("[PlayerMiniInfo] Loading: %s", exitBtnPaths[i]);
        
        IDirect3DBaseTexture9* pBtnTex = LoadGameTexture(&btnStr);
        if (pBtnTex) {
            exitBtnTextures[i]->pTexture = (IDirect3DTexture9*)pBtnTex;
            D3DSURFACE_DESC desc;
            if (SUCCEEDED(exitBtnTextures[i]->pTexture->GetLevelDesc(0, &desc))) {
                exitBtnTextures[i]->width = desc.Width;
                exitBtnTextures[i]->height = desc.Height;
                LogMsg("[PlayerMiniInfo] ExitBtn[%d] texture loaded: %dx%d", i, desc.Width, desc.Height);
            }
        } else {
            LogMsg("[PlayerMiniInfo] FAILED to load ExitBtn[%d] texture!", i);
        }
    }
    
    m_bTexturesLoaded = true;  // Mark as loaded to avoid retry spam
    
    LogMsg("[PlayerMiniInfo] Texture loading complete. Background=%p, HP=%p, MP=%p, Hwan=%p",
           m_texBackground.pTexture, m_texHpFill.pTexture, m_texMpFill.pTexture, m_texHwanFill.pTexture);
    
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
        
        // Initialize UIRenderer with DX9 device (for blend mode support)
        UIRenderer::Init(pDevice);
        
        // Initialize native sprite renderer for DirectX native quality
        InitNativeSprite(pDevice);
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
    
    // === SMOOTH BAR ANIMATION (Lerp) ===
    // Initialize displayed values if first frame
    if (m_displayHP <= 0.0f && currentHP > 0) {
        m_displayHP = (float)currentHP;
    }
    if (m_displayMP <= 0.0f && currentMP > 0) {
        m_displayMP = (float)currentMP;
    }
    
    // Smooth lerp towards actual values (FPS-independent)
    float deltaTime = ImGui::GetIO().DeltaTime;
    float lerpSpeed = 8.0f;  // Higher = faster transition
    
    // HP Lerp
    float hpDiff = (float)currentHP - m_displayHP;
    if (abs(hpDiff) < 1.0f) {
        m_displayHP = (float)currentHP;  // Snap to target if very close
    } else {
        m_displayHP += hpDiff * deltaTime * lerpSpeed;
    }
    
    // MP Lerp
    float mpDiff = (float)currentMP - m_displayMP;
    if (abs(mpDiff) < 1.0f) {
        m_displayMP = (float)currentMP;
    } else {
        m_displayMP += mpDiff * deltaTime * lerpSpeed;
    }
    
    // === DAMAGE FLASH DETECTION ===
    // If HP decreased, trigger red flash
    if (currentHP < m_lastHP && m_lastHP > 0) {
        m_damageFlashTimer = 0.5f;  // Start 0.5 second flash
    }
    m_lastHP = currentHP;
    
    // Decay flash timer
    if (m_damageFlashTimer > 0.0f) {
        m_damageFlashTimer -= deltaTime;
        if (m_damageFlashTimer < 0.0f) m_damageFlashTimer = 0.0f;
    }
    
    // Calculate percentages using ANIMATED display values
    float hpPercent = (maxHP > 0) ? m_displayHP / (float)maxHP : 0.0f;
    float mpPercent = (maxMP > 0) ? m_displayMP / (float)maxMP : 0.0f;
    
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
        
        __try {
            // Get actual window instance via IRM
            CIFPlayerMiniInfo* pNativeMiniInfo = NULL;
            if (g_pCGInterface) {
                pNativeMiniInfo = (CIFPlayerMiniInfo*)g_pCGInterface->m_IRM.GetResObj(GDR_PLAYER_MINI_INFO, 1);
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
                // Position buffs using slider values
                int buffX = (int)windowPos.x + (int)dbg_BuffX;
                int buffY = (int)windowPos.y + (int)dbg_BuffY;
                pMagicStateBoard->MoveGWnd(buffX, buffY);
            }
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            // Ignore exceptions
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
        static float dbg_CharIconX = 30.396f;  // Character icon X position
        static float dbg_CharIconY = 74.009f;    // Character icon Y position
        static float dbg_CharIconScale = 100.0f; // Character icon scale % (100 = original)
        static float dbg_StatIconX = 231.278f;  // Stat icon X position (RS button)
        static float dbg_StatIconY = 5.286f;    // Stat icon Y position
        static float dbg_StatIconScale = 100.0f; // Stat icon scale % (100 = original)
        static float dbg_HwanIconX = 252.423f;  // Hwan icon X position (to the right of hwan bar)
        static float dbg_HwanIconY = 70.0f;   // Hwan icon Y position
        static float dbg_HwanIconScale = 100.0f; // Hwan icon scale % (100 = original)
        static int dbg_IconNormalBright = 160;  // Normal brightness (0-255)
        static int dbg_IconHoverBright = 255;   // Hover brightness (0-255)
        static float dbg_PortraitFaceX = 48.0f;  // Portrait face X (same as portrait center)
        static float dbg_PortraitFaceY = 52.0f;  // Portrait face Y (same as portrait center)
        static float dbg_PortraitFaceW = 64.0f;  // Portrait face width
        static float dbg_PortraitFaceH = 64.0f;  // Portrait face height
        // NOTE: dbg_BuffX and dbg_BuffY are declared earlier (before MoveGWnd usage)
        
        // Debug window
        if (ImGui::Begin("MiniInfo Debug")) {
            ImGui::Text("Portrait Center (Frame)");
            ImGui::SliderFloat("Portrait X", &dbg_PortraitX, 0, 150);
            ImGui::SliderFloat("Portrait Y", &dbg_PortraitY, 0, 100);
            ImGui::Text("Portrait Face (Character Image)");
            ImGui::SliderFloat("Face X", &dbg_PortraitFaceX, 0, 150);
            ImGui::SliderFloat("Face Y", &dbg_PortraitFaceY, 0, 100);
            ImGui::SliderFloat("Face W", &dbg_PortraitFaceW, 16, 128);
            ImGui::SliderFloat("Face H", &dbg_PortraitFaceH, 16, 128);
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
            ImGui::Text("Buff Viewer (MagicStateBoard)");
            ImGui::SliderFloat("Buff X", &dbg_BuffX, 0, 800);
            ImGui::SliderFloat("Buff Y", &dbg_BuffY, 0, 200);
            ImGui::Separator();
            ImGui::Text("Character Icon");
            ImGui::SliderFloat("CharIcon X", &dbg_CharIconX, 0, 300);
            ImGui::SliderFloat("CharIcon Y", &dbg_CharIconY, 0, 100);
            ImGui::SliderFloat("CharIcon Scale %", &dbg_CharIconScale, 10, 200);
            ImGui::Separator();
            ImGui::Text("Stat Icon (RS)");
            ImGui::SliderFloat("StatIcon X", &dbg_StatIconX, 0, 300);
            ImGui::SliderFloat("StatIcon Y", &dbg_StatIconY, 0, 100);
            ImGui::SliderFloat("StatIcon Scale %", &dbg_StatIconScale, 10, 200);
            ImGui::Separator();
            ImGui::Text("Hwan Icon (shows when hwan=5)");
            ImGui::SliderFloat("HwanIcon X", &dbg_HwanIconX, 0, 300);
            ImGui::SliderFloat("HwanIcon Y", &dbg_HwanIconY, 0, 100);
            ImGui::SliderFloat("HwanIcon Scale %", &dbg_HwanIconScale, 10, 200);
            ImGui::Separator();
            ImGui::Text("Icon Hover Effect");
            ImGui::SliderInt("Normal Bright", &dbg_IconNormalBright, 0, 500);
            ImGui::SliderInt("Hover Bright", &dbg_IconHoverBright, 0, 500);
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
        
        // === Update character portrait (loads DDJ based on model ID) ===
        CharacterPortrait& portrait = GetCharacterPortrait();
        portrait.Update();
        
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
        
        // === BLUE AREAS: Stat and RS buttons ===
        // Stat button - use icon_character.ddj if loaded, otherwise [S] text
        // NOTE: Drawing is deferred to after portrait frame for top-layer rendering
        float statBtnX = windowPos.x + dbg_CharIconX * SCALE;
        float statBtnY = windowPos.y + dbg_CharIconY * SCALE;
        
        ImVec2 statBtnMin, statBtnMax;
        
        // First, calculate bounds for hover detection
        ImVec2 mousePos = ImGui::GetMousePos();
        bool statHovered = false;
        float charIconW = 0, charIconH = 0;
        
        if (m_texCharIcon.pTexture) {
            // Apply scale % to texture dimensions
            float iconScale = dbg_CharIconScale / 100.0f;
            charIconW = (float)m_texCharIcon.width * SCALE * iconScale;
            charIconH = (float)m_texCharIcon.height * SCALE * iconScale;
            statBtnMin = ImVec2(statBtnX, statBtnY);
            statBtnMax = ImVec2(statBtnX + charIconW, statBtnY + charIconH);
            
            // Check hover (draw will be done later)
            statHovered = (mousePos.x >= statBtnMin.x && mousePos.x <= statBtnMax.x &&
                          mousePos.y >= statBtnMin.y && mousePos.y <= statBtnMax.y);
        } else {
            // Fallback to text if texture not loaded
            const char* statBtnText = "[S]";
            ImVec2 statBtnSize = ImGui::CalcTextSize(statBtnText);
            statBtnMin = ImVec2(statBtnX - 2, statBtnY - 2);
            statBtnMax = ImVec2(statBtnX + statBtnSize.x + 2, statBtnY + statBtnSize.y + 2);
            
            statHovered = (mousePos.x >= statBtnMin.x && mousePos.x <= statBtnMax.x &&
                          mousePos.y >= statBtnMin.y && mousePos.y <= statBtnMax.y);
            
            // Draw fallback text here (no texture, so draw immediately)
            ImU32 textColor = statHovered ? IM_COL32(100, 200, 255, 255) : IM_COL32(150, 180, 255, 255);
            drawList->AddText(ImVec2(statBtnX + 1, statBtnY + 1), IM_COL32(0, 0, 0, 200), statBtnText);
            drawList->AddText(ImVec2(statBtnX, statBtnY), textColor, statBtnText);
        }
        
        // Handle click for Stat button
        if (statHovered && ImGui::IsMouseClicked(0)) {
            m_bShowStatsPopup = !m_bShowStatsPopup;
        }
        
        // RS button (only if stat points available) - use icon_stat.ddj if loaded
        // NOTE: Drawing is deferred to after portrait frame for top-layer rendering
        ImVec2 rsBtnMin, rsBtnMax;
        bool rsHovered = false;
        bool rsButtonActive = (statPoints > 0);
        
        if (rsButtonActive) {
            float rsBtnX = windowPos.x + dbg_StatIconX * SCALE;
            float rsBtnY = windowPos.y + dbg_StatIconY * SCALE;
            
            if (m_texStatIcon.pTexture) {
                // Apply scale % to texture dimensions
                float iconScale = dbg_StatIconScale / 100.0f;
                float rsIconW = (float)m_texStatIcon.width * SCALE * iconScale;
                float rsIconH = (float)m_texStatIcon.height * SCALE * iconScale;
                rsBtnMin = ImVec2(rsBtnX, rsBtnY);
                rsBtnMax = ImVec2(rsBtnX + rsIconW, rsBtnY + rsIconH);
                
                // Check hover (draw will be done later)
                rsHovered = (mousePos.x >= rsBtnMin.x && mousePos.x <= rsBtnMax.x &&
                            mousePos.y >= rsBtnMin.y && mousePos.y <= rsBtnMax.y);
            } else {
                // Fallback to text if texture not loaded
                const char* rsBtnText = "[RS]";
                ImVec2 rsBtnSize = ImGui::CalcTextSize(rsBtnText);
                rsBtnMin = ImVec2(rsBtnX - 2, rsBtnY - 2);
                rsBtnMax = ImVec2(rsBtnX + rsBtnSize.x + 2, rsBtnY + rsBtnSize.y + 2);
                
                rsHovered = (mousePos.x >= rsBtnMin.x && mousePos.x <= rsBtnMax.x &&
                            mousePos.y >= rsBtnMin.y && mousePos.y <= rsBtnMax.y);
                
                // Draw fallback text here (no texture, so draw immediately)
                ImU32 rsColor = rsHovered ? IM_COL32(255, 150, 150, 255) : IM_COL32(255, 100, 100, 255);
                drawList->AddText(ImVec2(rsBtnX + 1, rsBtnY + 1), IM_COL32(0, 0, 0, 200), rsBtnText);
                drawList->AddText(ImVec2(rsBtnX, rsBtnY), rsColor, rsBtnText);
            }
            
            // Handle click for RS button
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
        
        // === HP/MP/HWAN BARS - WITH SUBTLE DEPTH EFFECTS ===
        // Normal blending with subtle gloss/shadow for depth (not too bright)
        
        // HP Bar - Additive Glow (reduced brightness) + Gloss/Shadow
        if (m_texHpFill.pTexture && hpPercent > 0.0f) {
            float hpFillW = HP_BAR_WIDTH * hpPercent;
            ImVec2 hpMin = ImVec2(hpBarX, hpBarY);
            ImVec2 hpMax = ImVec2(hpBarX + hpFillW, hpBarY + HP_BAR_HEIGHT);
            ImVec2 uvMin = ImVec2(0.0f, 0.0f);
            ImVec2 uvMax = ImVec2(hpPercent, 1.0f);
            
            // Simple normal rendering (no glow)
            drawList->AddImage((ImTextureID)m_texHpFill.pTexture, hpMin, hpMax, uvMin, uvMax);
        }
        
        // MP Bar - Additive Glow (reduced brightness) + Gloss/Shadow
        float mpBarX = windowPos.x + MP_BAR_X;
        float mpBarY = windowPos.y + MP_BAR_Y;
        if (m_texMpFill.pTexture && mpPercent > 0.0f) {
            float mpFillW = MP_BAR_WIDTH * mpPercent;
            ImVec2 mpMin = ImVec2(mpBarX, mpBarY);
            ImVec2 mpMax = ImVec2(mpBarX + mpFillW, mpBarY + MP_BAR_HEIGHT);
            ImVec2 uvMin = ImVec2(0.0f, 0.0f);
            ImVec2 uvMax = ImVec2(mpPercent, 1.0f);
            
            // Simple normal rendering (no glow)
            drawList->AddImage((ImTextureID)m_texMpFill.pTexture, mpMin, mpMax, uvMin, uvMax);
        }
        
        // Hwan Bar - Additive with reduced brightness
        if (hwanPoint > 0 && m_texHwanFill.pTexture) {
            float hwanPercent = (float)hwanPoint / 5.0f;
            float hwanBarX = windowPos.x + HWAN_BAR_X;
            float hwanBarY = windowPos.y + HWAN_BAR_Y;
            float hwanFillW = HWAN_BAR_WIDTH * hwanPercent;
            ImVec2 hwanMin = ImVec2(hwanBarX, hwanBarY);
            ImVec2 hwanMax = ImVec2(hwanBarX + hwanFillW, hwanBarY + HWAN_BAR_HEIGHT);
            ImVec2 uvMin = ImVec2(0.0f, 0.0f);
            ImVec2 uvMax = ImVec2(hwanPercent, 1.0f);
            
            // === HWAN PULSE EFFECT (when full) ===
            if (hwanPoint == 5) {
                // Sinusoidal pulse - breathing effect when hwan is full
                float time = (float)GetTickCount() / 1000.0f;
                int pulseAlpha = 200 + (int)(sinf(time * 5.0f) * 55.0f);  // 145-255 range
                if (pulseAlpha > 255) pulseAlpha = 255;
                if (pulseAlpha < 145) pulseAlpha = 145;
                
                // Draw with pulsing brightness
                ImU32 hwanPulseColor = IM_COL32(255, 255, 255, pulseAlpha);
                drawList->AddImage((ImTextureID)m_texHwanFill.pTexture, hwanMin, hwanMax, uvMin, uvMax, hwanPulseColor);
            } else {
                // Normal rendering when not full
                drawList->AddImage((ImTextureID)m_texHwanFill.pTexture, hwanMin, hwanMax, uvMin, uvMax);
            }
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
        
        // MP text overlay: current/max (mpBarX, mpBarY already defined above)
        char mpText[64];
        sprintf(mpText, "%d / %d", currentMP, maxMP);
        ImVec2 mpTextSize = ImGui::CalcTextSize(mpText);
        float mpTextX = mpBarX + (MP_BAR_WIDTH - mpTextSize.x) * 0.5f;
        float mpTextY = mpBarY + (MP_BAR_HEIGHT - mpTextSize.y) * 0.5f;
        
        // Shadow
        drawList->AddText(ImVec2(mpTextX + 1, mpTextY + 1), IM_COL32(0, 0, 0, 200), mpText);
        // White text
        drawList->AddText(ImVec2(mpTextX, mpTextY), IM_COL32(255, 255, 255, 255), mpText);
        
        // Hwan button will be drawn at the very end
        
        // === LAYER: PORTRAIT BG (on top of bars, at X=2, Y=2) - WITH DEPTH ===
        if (m_texPortraitBg.pTexture) {
            float portraitBgW = (float)m_texPortraitBg.width * SCALE;
            float portraitBgH = (float)m_texPortraitBg.height * SCALE;
            ImVec2 pBgMin = ImVec2(windowPos.x + 2.0f * SCALE, windowPos.y + 2.0f * SCALE);
            ImVec2 pBgMax = ImVec2(pBgMin.x + portraitBgW, pBgMin.y + portraitBgH);
            
            // Drop Shadow - clip to texture bounds so it doesn't leak outside
            drawList->PushClipRect(pBgMin, ImVec2(pBgMax.x + 3, pBgMax.y + 3), true);
            drawList->AddImage((ImTextureID)m_texPortraitBg.pTexture, 
                ImVec2(pBgMin.x + 2, pBgMin.y + 2), 
                ImVec2(pBgMax.x + 2, pBgMax.y + 2),
                ImVec2(0, 0), ImVec2(1, 1), 
                IM_COL32(0, 0, 0, 100));
            drawList->PopClipRect();
            
            // Main texture
            drawList->AddImage((ImTextureID)m_texPortraitBg.pTexture, pBgMin, pBgMax);
        }
        
        // === DAMAGE FLASH EFFECT (RED SHADOW BEHIND MAIN FRAME) ===
        // Draw red shadow behind the main background when damaged (like drop shadow but red)
        if (m_damageFlashTimer > 0.0f && m_texBackground.pTexture) {
            int flashAlpha = (int)(m_damageFlashTimer * 300.0f);
            if (flashAlpha > 150) flashAlpha = 150;
            if (flashAlpha < 0) flashAlpha = 0;
            
            // Use main background texture as offset red shadow
            float shadowOffset = 5.0f;
            ImVec2 shadowMin = ImVec2(windowPos.x + shadowOffset, windowPos.y + shadowOffset);
            ImVec2 shadowMax = ImVec2(windowPos.x + bgWidth + shadowOffset, windowPos.y + bgHeight + shadowOffset);
            
            drawList->PushClipRectFullScreen();
            drawList->AddImage((ImTextureID)m_texBackground.pTexture, shadowMin, shadowMax,
                ImVec2(0, 0), ImVec2(1, 1), IM_COL32(255, 50, 50, flashAlpha));
            drawList->PopClipRect();
        }
        
        // === LAYER: CHARACTER PORTRAIT FACE (on top of BG) ===
        // Note: 'portrait' already declared above (GetCharacterPortrait)
        IDirect3DTexture9* pPortraitTex = portrait.GetTexture();
        if (pPortraitTex) {
            // Use debug slider values for portrait face position and size
            float portraitW = dbg_PortraitFaceW * SCALE;
            float portraitH = dbg_PortraitFaceH * SCALE;
            float portraitX = windowPos.x + dbg_PortraitFaceX * SCALE - portraitW * 0.5f;
            float portraitY = windowPos.y + dbg_PortraitFaceY * SCALE - portraitH * 0.5f;
            ImVec2 pMin = ImVec2(portraitX, portraitY);
            ImVec2 pMax = ImVec2(portraitX + portraitW, portraitY + portraitH);
            drawList->AddImage((ImTextureID)pPortraitTex, pMin, pMax);
        }
        
        // === LAYER: PORTRAIT FRAME (top layer, at X=0, Y=0) - WITH DEPTH ===
        if (m_texPortraitFrame.pTexture) {
            float portraitFrameW = (float)m_texPortraitFrame.width * SCALE;
            float portraitFrameH = (float)m_texPortraitFrame.height * SCALE;
            // Pixel-perfect alignment (integer coordinates)
            ImVec2 pFrameMin = ImVec2((float)(int)windowPos.x, (float)(int)windowPos.y);
            ImVec2 pFrameMax = ImVec2((float)(int)(pFrameMin.x + portraitFrameW), (float)(int)(pFrameMin.y + portraitFrameH));
            
            // Drop Shadow - clip to texture bounds
            drawList->PushClipRect(pFrameMin, ImVec2(pFrameMax.x + 3, pFrameMax.y + 3), true);
            drawList->AddImage((ImTextureID)m_texPortraitFrame.pTexture, 
                ImVec2(pFrameMin.x + 2, pFrameMin.y + 2), 
                ImVec2(pFrameMax.x + 2, pFrameMax.y + 2),
                ImVec2(0, 0), ImVec2(1, 1), 
                IM_COL32(0, 0, 0, 100));
            drawList->PopClipRect();
            
            // Main frame texture (slightly dimmed for SRO authentic look)
            drawList->AddImage((ImTextureID)m_texPortraitFrame.pTexture, pFrameMin, pFrameMax,
                ImVec2(0, 0), ImVec2(1, 1), IM_COL32(220, 220, 220, 255));
            
            // Metallic highlight (very subtle)
            drawList->AddCircleFilled(
                ImVec2(pFrameMin.x + portraitFrameW * 0.25f, pFrameMin.y + portraitFrameH * 0.25f),
                portraitFrameW * 0.2f,
                IM_COL32(255, 255, 255, 8));
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
        
        // === DEFERRED CHARACTER ICON DRAW (on top of portrait frame) ===
        if (m_texCharIcon.pTexture) {
            // Use debug values for brightness
            int bright = statHovered ? dbg_IconHoverBright : dbg_IconNormalBright;
            // Clamp to 255 for IM_COL32
            if (bright > 255) bright = 255;
            ImU32 iconColor = IM_COL32(bright, bright, bright, 255);
            drawList->AddImage((ImTextureID)m_texCharIcon.pTexture, statBtnMin, statBtnMax, ImVec2(0, 0), ImVec2(1, 1), iconColor);
        }
        
        // === DEFERRED STAT ICON DRAW (on top of portrait frame) ===
        if (rsButtonActive && m_texStatIcon.pTexture) {
            // Use debug values for brightness (same as character icon)
            int rsBright = rsHovered ? dbg_IconHoverBright : dbg_IconNormalBright;
            // Clamp to 255 for IM_COL32
            if (rsBright > 255) rsBright = 255;
            ImU32 rsIconColor = IM_COL32(rsBright, rsBright, rsBright, 255);
            drawList->AddImage((ImTextureID)m_texStatIcon.pTexture, rsBtnMin, rsBtnMax, ImVec2(0, 0), ImVec2(1, 1), rsIconColor);
        }
        
        // === DEFERRED HWAN ICON DRAW (only when hwan = 5) ===
        if (hwanPoint == 5 && m_texHwanIcon.pTexture) {
            float hwanIconX = windowPos.x + dbg_HwanIconX * SCALE;
            float hwanIconY = windowPos.y + dbg_HwanIconY * SCALE;
            // Apply scale % to texture dimensions
            float iconScale = dbg_HwanIconScale / 100.0f;
            float hwanIconW = (float)m_texHwanIcon.width * SCALE * iconScale;
            float hwanIconH = (float)m_texHwanIcon.height * SCALE * iconScale;
            
            ImVec2 hwanIconMin = ImVec2(hwanIconX, hwanIconY);
            ImVec2 hwanIconMax = ImVec2(hwanIconX + hwanIconW, hwanIconY + hwanIconH);
            
            // Check hover
            bool hwanIconHovered = (mousePos.x >= hwanIconMin.x && mousePos.x <= hwanIconMax.x &&
                                   mousePos.y >= hwanIconMin.y && mousePos.y <= hwanIconMax.y);
            
            // Use debug values for brightness (same as character icon)
            int hwanBright = hwanIconHovered ? dbg_IconHoverBright : dbg_IconNormalBright;
            if (hwanBright > 255) hwanBright = 255;
            ImU32 hwanIconColor = IM_COL32(hwanBright, hwanBright, hwanBright, 255);
            drawList->AddImage((ImTextureID)m_texHwanIcon.pTexture, hwanIconMin, hwanIconMax, ImVec2(0, 0), ImVec2(1, 1), hwanIconColor);
            
            // Handle click for Hwan icon - activate Hwan/Berserk mode
            if (hwanIconHovered && ImGui::IsMouseClicked(0)) {
                SendHwanActivationPacket();
                LogMsg("[PlayerMiniInfo] Hwan activation packet sent (0x70A7)");
            }
        }
        
        // Self-target: Click anywhere on the PlayerMiniInfo background to target self
        // Use entire window bounds for click detection
        ImVec2 windowMin = windowPos;
        ImVec2 windowMax = ImVec2(windowPos.x + bgWidth, windowPos.y + bgHeight);
        
        if (mousePos.x >= windowMin.x && mousePos.x <= windowMax.x &&
            mousePos.y >= windowMin.y && mousePos.y <= windowMax.y &&
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
    // === DEBUG CONTROLS FOR STATS POPUP ===
    static float dbg_StatsPopupX = 100.0f;
    static float dbg_StatsPopupY = 100.0f;
    static float dbg_StatsPopupW = 300.0f;
    static float dbg_StatsPopupH = 400.0f;
    static float dbg_CloseBtnX = 270.0f;  // Close button X position (relative to popup)
    static float dbg_CloseBtnY = 5.0f;    // Close button Y position (relative to popup)
    static float dbg_CloseBtnW = 24.0f;   // Close button width
    static float dbg_CloseBtnH = 24.0f;   // Close button height
    static float dbg_TitleBarX = 58.621f;    // Title bar X position
    static float dbg_TitleBarY = -11.207f;   // Title bar Y position
    static float dbg_TitleBarW = 183.966f;   // Title bar width
    static float dbg_TitleBarH = 27.379f;    // Title bar height
    static float dbg_TitleTextX = 111.724f;  // Title text X position
    static float dbg_TitleTextY = -3.793f;   // Title text Y position
    static float dbg_TitleTextSize = 14.0f;  // Title text font size
    static bool bShowDebug = false;          // Debug window visibility toggle
    
    // Toggle debug window with keyboard shortcut (F9)
    if (GetAsyncKeyState(VK_F9) & 1) {
        bShowDebug = !bShowDebug;
    }
    
    // Debug window for stats popup controls (hidden by default)
    if (bShowDebug) {
        if (ImGui::Begin("Stats Popup Debug", &bShowDebug)) {
            ImGui::Text("Position");
            ImGui::SliderFloat("X##popup", &dbg_StatsPopupX, 0, 800);
            ImGui::SliderFloat("Y##popup", &dbg_StatsPopupY, 0, 600);
            
            ImGui::Separator();
            ImGui::Text("Resolution");
            ImGui::SliderFloat("Width##popup", &dbg_StatsPopupW, 100, 800);
            ImGui::SliderFloat("Height##popup", &dbg_StatsPopupH, 100, 800);
            
            ImGui::Separator();
            ImGui::Text("Close Button");
            ImGui::SliderFloat("Btn X##close", &dbg_CloseBtnX, 0, 400);
            ImGui::SliderFloat("Btn Y##close", &dbg_CloseBtnY, 0, 100);
            ImGui::SliderFloat("Btn W##close", &dbg_CloseBtnW, 10, 100);
            ImGui::SliderFloat("Btn H##close", &dbg_CloseBtnH, 10, 100);
            
            ImGui::Separator();
            ImGui::Text("Title Bar");
            ImGui::SliderFloat("Title X##title", &dbg_TitleBarX, -50, 400);
            ImGui::SliderFloat("Title Y##title", &dbg_TitleBarY, -50, 100);
            ImGui::SliderFloat("Title W##title", &dbg_TitleBarW, 50, 400);
            ImGui::SliderFloat("Title H##title", &dbg_TitleBarH, 10, 100);
            
            ImGui::Separator();
            ImGui::Text("Title Text");
            ImGui::SliderFloat("Text X##text", &dbg_TitleTextX, -50, 300);
            ImGui::SliderFloat("Text Y##text", &dbg_TitleTextY, -50, 50);
            ImGui::SliderFloat("Text Size##text", &dbg_TitleTextSize, 8, 32);
        }
        ImGui::End();
    }
    
    // === RENDER STATS POPUP WINDOW ===
    // NoCollapse, NoResize, NoTitleBar, NoBackground - allow moving only
    ImGuiWindowFlags popupFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | 
                                   ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground;
    
    // Only set position/size on first use - allow user to move/resize after
    ImGui::SetNextWindowPos(ImVec2(dbg_StatsPopupX, dbg_StatsPopupY), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(dbg_StatsPopupW, dbg_StatsPopupH));
    
    if (ImGui::Begin("Character Stats", &m_bShowStatsPopup, popupFlags)) {
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 windowPos = ImGui::GetWindowPos();
        
        // Draw background texture with depth effects
        if (m_texStatsPopupBg.pTexture) {
            // Pixel-perfect alignment
            ImVec2 bgMin = ImVec2((float)(int)windowPos.x, (float)(int)windowPos.y);
            ImVec2 bgMax = ImVec2((float)(int)(windowPos.x + dbg_StatsPopupW), (float)(int)(windowPos.y + dbg_StatsPopupH));
            
            // 1. Drop Shadow
            drawList->AddImage((ImTextureID)m_texStatsPopupBg.pTexture, 
                ImVec2(bgMin.x + 4, bgMin.y + 4), 
                ImVec2(bgMax.x + 4, bgMax.y + 4),
                ImVec2(0, 0), ImVec2(1, 1), 
                IM_COL32(0, 0, 0, 150));
            
            // 2. Main background (slightly dimmed for SRO look)
            drawList->AddImage((ImTextureID)m_texStatsPopupBg.pTexture, bgMin, bgMax,
                ImVec2(0, 0), ImVec2(1, 1), IM_COL32(220, 220, 220, 255));
        }
        
        // Draw title bar texture - push full screen clip rect to allow drawing outside window
        if (m_texTitleBar.pTexture) {
            drawList->PushClipRectFullScreen();
            
            // Pixel-perfect alignment
            float titleX = (float)(int)(windowPos.x + dbg_TitleBarX);
            float titleY = (float)(int)(windowPos.y + dbg_TitleBarY);
            ImVec2 titleMin = ImVec2(titleX, titleY);
            ImVec2 titleMax = ImVec2((float)(int)(titleX + dbg_TitleBarW), (float)(int)(titleY + dbg_TitleBarH));
            
            // Main title bar (slightly dimmed for SRO look)
            drawList->AddImage((ImTextureID)m_texTitleBar.pTexture, titleMin, titleMax,
                ImVec2(0, 0), ImVec2(1, 1), IM_COL32(220, 220, 220, 255));
            
            // Draw "Character Stats" text using debug position and size
            const char* titleText = "Character Stats";
            ImFont* font = ImGui::GetFont();
            float textX = windowPos.x + dbg_TitleTextX;
            float textY = windowPos.y + dbg_TitleTextY;
            // Shadow
            drawList->AddText(font, dbg_TitleTextSize, ImVec2(textX + 1, textY + 1), IM_COL32(0, 0, 0, 200), titleText);
            // Text
            drawList->AddText(font, dbg_TitleTextSize, ImVec2(textX, textY), IM_COL32(255, 255, 200, 255), titleText);
            
            drawList->PopClipRect();
        }
        
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
        
        // Display stats in formatted layout (add padding for background)
        ImGui::SetCursorPos(ImVec2(20, 30));
        ImGui::BeginGroup();
        
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
        ImGui::Text("Stat Points:"); ImGui::NextColumn(); ImGui::Text("%d", statPoints); ImGui::NextColumn();
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
        
        ImGui::EndGroup();
        
        // Close button using 3-state textures (normal, hovered, pressed)
        {
            float btnX = windowPos.x + dbg_CloseBtnX;
            float btnY = windowPos.y + dbg_CloseBtnY;
            ImVec2 btnMin = ImVec2(btnX, btnY);
            ImVec2 btnMax = ImVec2(btnX + dbg_CloseBtnW, btnY + dbg_CloseBtnH);
            
            // Check hover and pressed state
            ImVec2 mousePos = ImGui::GetMousePos();
            bool btnHovered = (mousePos.x >= btnMin.x && mousePos.x <= btnMax.x &&
                              mousePos.y >= btnMin.y && mousePos.y <= btnMax.y);
            bool btnPressed = btnHovered && ImGui::IsMouseDown(0);
            
            // Select texture based on state
            IDirect3DTexture9* pBtnTex = NULL;
            if (btnPressed && m_texExitBtnPressed.pTexture) {
                pBtnTex = m_texExitBtnPressed.pTexture;
            } else if (btnHovered && m_texExitBtnHover.pTexture) {
                pBtnTex = m_texExitBtnHover.pTexture;
            } else if (m_texExitBtnNormal.pTexture) {
                pBtnTex = m_texExitBtnNormal.pTexture;
            }
            
            // Draw button
            if (pBtnTex) {
                drawList->AddImage((ImTextureID)pBtnTex, btnMin, btnMax);
            }
            
            // Handle click
            if (btnHovered && ImGui::IsMouseClicked(0)) {
                m_bShowStatsPopup = false;
            }
        }
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

// =============================================================================
// DirectX Native Sprite Rendering Functions
// =============================================================================

void CustomPlayerMiniInfo::InitNativeSprite(IDirect3DDevice9* pDevice) {
    if (m_pSprite) return;  // Already initialized
    if (!pDevice) return;
    
    HRESULT hr = D3DXCreateSprite(pDevice, &m_pSprite);
    if (SUCCEEDED(hr)) {
        LogMsg("[PlayerMiniInfo] Native sprite renderer created successfully");
    } else {
        LogMsg("[PlayerMiniInfo] Failed to create native sprite renderer: 0x%08X", hr);
        m_pSprite = NULL;
    }
}

void CustomPlayerMiniInfo::ReleaseNativeSprite() {
    if (m_pSprite) {
        m_pSprite->Release();
        m_pSprite = NULL;
        LogMsg("[PlayerMiniInfo] Native sprite renderer released");
    }
}

void CustomPlayerMiniInfo::RenderNativeSprite(IDirect3DTexture9* pTexture, 
    float x, float y, float w, float h, D3DCOLOR color) {
    if (!m_pSprite || !pTexture) return;
    
    // Get original texture size
    D3DSURFACE_DESC desc;
    if (FAILED(pTexture->GetLevelDesc(0, &desc))) return;
    
    // Calculate scale to fit desired size
    float scaleX = w / (float)desc.Width;
    float scaleY = h / (float)desc.Height;
    D3DXVECTOR2 scaling(scaleX, scaleY);
    D3DXVECTOR2 position(x, y);
    
    m_pSprite->Draw(
        (LPDIRECT3DTEXTURE9)pTexture,
        NULL,           // Full texture
        &scaling,       // Scaling
        NULL,           // Rotation center
        0.0f,           // Rotation angle
        &position,      // Position
        color           // Tint color
    );
}

void CustomPlayerMiniInfo::RenderNativeSpriteUV(IDirect3DTexture9* pTexture, 
    float x, float y, float w, float h, float uvMaxX, D3DCOLOR color) {
    if (!m_pSprite || !pTexture) return;
    if (uvMaxX <= 0.0f) return;
    
    // Get original texture size
    D3DSURFACE_DESC desc;
    if (FAILED(pTexture->GetLevelDesc(0, &desc))) return;
    
    // Create source rect for UV clipping (for progress bars)
    RECT srcRect;
    srcRect.left = 0;
    srcRect.top = 0;
    srcRect.right = (LONG)(desc.Width * uvMaxX);
    srcRect.bottom = desc.Height;
    
    // Calculate scale - target width is partial
    float actualWidth = w * uvMaxX;
    float scaleX = actualWidth / (float)srcRect.right;
    float scaleY = h / (float)desc.Height;
    D3DXVECTOR2 scaling(scaleX, scaleY);
    D3DXVECTOR2 position(x, y);
    
    m_pSprite->Draw(
        (LPDIRECT3DTEXTURE9)pTexture,
        &srcRect,       // Source rect for UV clipping
        &scaling,       // Scaling
        NULL,           // Rotation center
        0.0f,           // Rotation angle
        &position,      // Position
        color           // Tint color
    );
}
