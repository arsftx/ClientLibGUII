#include "CustomDamageRenderer.h"
#include <stdio.h>
#include <stdarg.h>
#include <GFX3DFunction/GFXVideo3d.h>
#include "hooks/Hooks.h"
#include <ICPlayer.h>

#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "d3d9.lib")

typedef void* (__cdecl *tFindEntityByID)(DWORD entityID);
static tFindEntityByID FindEntityByID = (tFindEntityByID)0x00664F50;

// =============================================================================
// Game's Native String Structure
// =============================================================================
// Game uses: {data_ptr, end_ptr, capacity_ptr} - NOT std::string!
struct GameString {
    char* data;      // [0] pointer to allocated data
    char* end;       // [4] data + length
    char* capacity;  // [8] data + buffer_size
};

// sub_406190: String constructor (thiscall)
// ECX = destination GameString*
// arg1 = source string start
// arg2 = source string end
typedef void (__thiscall *tStringConstruct)(GameString* pThis, const char* start, const char* end);
static tStringConstruct StringConstruct = (tStringConstruct)0x00406190;

// sub_4064A0: Memory allocator for strings
typedef char* (__cdecl *tStringAlloc)(size_t size);
static tStringAlloc StringAlloc = (tStringAlloc)0x004064A0;

// sub_409E10: Texture loader (cdecl)
// Takes GameString* and returns texture
typedef IDirect3DBaseTexture9* (__cdecl *tLoadTexture)(GameString* pPath);
static tLoadTexture LoadGameTexture = (tLoadTexture)0x00409E10;

// Helper: Create GameString from const char*
static void CreateGameString(GameString* pStr, const char* text) {
    size_t len = strlen(text);
    StringConstruct(pStr, text, text + len);
}

// Helper: Destroy GameString (free memory)
static void DestroyGameString(GameString* pStr) {
    // sub_406480 or just leave it (game manages memory)
    pStr->data = NULL;
    pStr->end = NULL;
    pStr->capacity = NULL;
}

// DEBUG LOGS DISABLED FOR PERFORMANCE - Uncomment to enable
static void LogMsg(const char* fmt, ...) {
    // Disabled for performance
    return;
    /*
    FILE* fp = fopen("ClientLog.txt", "a");
    if (fp) {
        va_list args;
        va_start(args, fmt);
        vfprintf(fp, fmt, args);
        fprintf(fp, "\n");
        va_end(args);
        fclose(fp);
    }
    */
}

// =============================================================================
// Global Local Player ID - Set from character spawn packet (0x3013)
// =============================================================================
static DWORD g_LocalPlayerID = 0;

// External function to set local player ID (called from CustomOpcodeHandler on spawn)
void CustomDamageRenderer_SetLocalPlayerID(DWORD playerID) {
    g_LocalPlayerID = playerID;
    LogMsg("[CustomDamageRenderer] Local Player ID set to: %u", playerID);
}

// Get cached local player ID
DWORD CustomDamageRenderer_GetLocalPlayerID() {
    return g_LocalPlayerID;
}

// =============================================================================
// Singleton
// =============================================================================
CustomDamageRenderer& CustomDamageRenderer::Instance() {
    static CustomDamageRenderer instance;
    return instance;
}

CustomDamageRenderer::CustomDamageRenderer() 
    : m_initialized(false), m_pSprite(NULL), m_damageCount(0), m_texturesLoaded(false),
      m_criticalTexture(NULL), m_criticalShadowTexture(NULL),
      m_blockingTexture(NULL), m_blockingShadowTexture(NULL),
      m_criticalEnemyTexture(NULL), m_criticalEnemyShadowTexture(NULL),
      m_blockingEnemyTexture(NULL), m_blockingEnemyShadowTexture(NULL) {
    memset(m_damageQueue, 0, sizeof(m_damageQueue));
    memset(m_digitTextures, 0, sizeof(m_digitTextures));
    memset(m_digitShadowTextures, 0, sizeof(m_digitShadowTextures));
    memset(m_enemyDigitTextures, 0, sizeof(m_enemyDigitTextures));
    memset(m_enemyShadowTextures, 0, sizeof(m_enemyShadowTextures));
}

CustomDamageRenderer::~CustomDamageRenderer() {
    Shutdown();
}

// Track device lost state for proper reset handling
static bool s_DamageRendererDeviceLost = false;

static void DamageRenderer_OnEndScene() {
    if (!g_CD3DApplication) return;
    if (!g_CD3DApplication->m_pd3dDevice) return;
    
    IDirect3DDevice9* pDevice = g_CD3DApplication->m_pd3dDevice;
    
    // Check device cooperative level to detect lost/reset state
    // This catches device lost situations that occur when:
    // - Running programs as administrator in background
    // - Opening certain fullscreen/exclusive mode applications
    // - Display mode changes from external sources
    HRESULT hr = pDevice->TestCooperativeLevel();
    
    if (hr == D3DERR_DEVICELOST) {
        // Device is lost, we cannot render
        // Mark as lost so we can handle reset properly
        if (!s_DamageRendererDeviceLost) {
            s_DamageRendererDeviceLost = true;
            // Release D3DPOOL_DEFAULT resources before device is reset
            CustomDamageRenderer::Instance().OnDeviceLost();
        }
        return;
    }
    
    if (hr == D3DERR_DEVICENOTRESET) {
        // Device is ready to be reset but hasn't been reset yet
        // Keep resources released
        if (!s_DamageRendererDeviceLost) {
            s_DamageRendererDeviceLost = true;
            CustomDamageRenderer::Instance().OnDeviceLost();
        }
        return;
    }
    
    if (hr == D3D_OK || hr == S_FALSE) {
        // Device is operational
        // If we were lost before, restore resources now that device is reset
        if (s_DamageRendererDeviceLost) {
            s_DamageRendererDeviceLost = false;
            CustomDamageRenderer::Instance().OnDeviceReset();
        }
        
        // Additional safety check using game's IsLost flag
        if (g_CD3DApplication->IsLost()) return;
        
        CustomDamageRenderer::Instance().Render(pDevice);
    }
}

static void DamageRenderer_OnPreSetSize(int width, int height) {
    CustomDamageRenderer::Instance().OnDeviceLost();
}

static void DamageRenderer_OnPostSetSize(int width, int height) {
    CustomDamageRenderer::Instance().OnDeviceReset();
}

bool CustomDamageRenderer::Initialize() {
    if (m_initialized) return true;
    
    LogMsg("[CustomDamageRenderer] Registering with hook system...");
    OnEndScene(DamageRenderer_OnEndScene);
    OnPreSetSize(DamageRenderer_OnPreSetSize);
    OnPostSetSize(DamageRenderer_OnPostSetSize);
    
    LogMsg("[CustomDamageRenderer] Registered successfully!");
    m_initialized = true;
    return true;
}

void CustomDamageRenderer::Shutdown() {
    if (m_pSprite) {
        m_pSprite->Release();
        m_pSprite = NULL;
    }
    m_initialized = false;
}

void CustomDamageRenderer::OnDeviceLost() {
    // ID3DXSprite uses D3DPOOL_DEFAULT resources - must notify before Reset()
    if (m_pSprite) {
        m_pSprite->OnLostDevice();
    }
}

void CustomDamageRenderer::OnDeviceReset() {
    // Restore ID3DXSprite after device Reset()
    if (m_pSprite) {
        m_pSprite->OnResetDevice();
    }
}

// =============================================================================
// Load Digit Textures using Game's Native String
// =============================================================================
bool CustomDamageRenderer::LoadDigitTextures() {
    if (m_texturesLoaded) return true;
    
    LogMsg("[CustomDamageRenderer] Loading digit textures with GameString...");
    
    bool allLoaded = true;
    char pathBuf[256];
    
    for (int i = 0; i < 10; i++) {
        GameString gameStr = {0, 0, 0};
        
        // Main texture
        sprintf(pathBuf, "interface\\hitcount\\hitcount_%d.ddj", i);
        CreateGameString(&gameStr, pathBuf);
        
        LogMsg("[CustomDamageRenderer] Loading: %s (GameString: data=%p, end=%p, cap=%p)", 
            pathBuf, gameStr.data, gameStr.end, gameStr.capacity);
        
        m_digitTextures[i] = LoadGameTexture(&gameStr);
        
        if (!m_digitTextures[i]) {
            LogMsg("[CustomDamageRenderer] Failed to load digit %d", i);
            allLoaded = false;
        } else {
            LogMsg("[CustomDamageRenderer] Loaded digit %d: %p", i, m_digitTextures[i]);
        }
        
        // Shadow texture
        sprintf(pathBuf, "interface\\hitcount\\hitcount_%d_shadow.ddj", i);
        CreateGameString(&gameStr, pathBuf);
        m_digitShadowTextures[i] = LoadGameTexture(&gameStr);
    }
    
    // Load critical text texture (from original game)
    GameString critStr = {0, 0, 0};
    CreateGameString(&critStr, "interface\\hitcount\\critical.ddj");
    m_criticalTexture = LoadGameTexture(&critStr);
    LogMsg("[CustomDamageRenderer] Loaded critical texture: %p", m_criticalTexture);
    
    CreateGameString(&critStr, "interface\\hitcount\\critical_shadow.ddj");
    m_criticalShadowTexture = LoadGameTexture(&critStr);
    
    // Load blocking text texture (for outgoing - when enemy blocks our attack)
    CreateGameString(&critStr, "interface\\hitcount\\blocking_player.ddj");
    m_blockingTexture = LoadGameTexture(&critStr);
    LogMsg("[CustomDamageRenderer] Loaded blocking_player texture: %p", m_blockingTexture);
    
    CreateGameString(&critStr, "interface\\hitcount\\blocking_player_shadow.ddj");
    m_blockingShadowTexture = LoadGameTexture(&critStr);
    
    m_texturesLoaded = allLoaded;
    
    // Load enemy digit textures (hitcount_enemy_0.ddj to hitcount_enemy_9.ddj for incoming damage)
    LogMsg("[CustomDamageRenderer] Loading enemy digit textures...");
    for (int i = 0; i <= 9; i++) {
        GameString enemyStr = {0, 0, 0};
        
        sprintf(pathBuf, "interface\\hitcount\\hitcount_enemy_%d.ddj", i);
        CreateGameString(&enemyStr, pathBuf);
        m_enemyDigitTextures[i] = LoadGameTexture(&enemyStr);
        
        if (m_enemyDigitTextures[i]) {
            LogMsg("[CustomDamageRenderer] Loaded enemy digit %d: %p", i, m_enemyDigitTextures[i]);
        } else {
            LogMsg("[CustomDamageRenderer] Failed to load enemy digit %d", i);
        }
        
        // Shadow texture for enemy digits
        sprintf(pathBuf, "interface\\hitcount\\hitcount_enemy_%d_shadow.ddj", i);
        CreateGameString(&enemyStr, pathBuf);
        m_enemyShadowTextures[i] = LoadGameTexture(&enemyStr);
    }
    
    // Load enemy critical texture (for incoming critical damage)
    GameString enemyCritStr = {0, 0, 0};
    CreateGameString(&enemyCritStr, "interface\\hitcount\\critical_enemy.ddj");
    m_criticalEnemyTexture = LoadGameTexture(&enemyCritStr);
    LogMsg("[CustomDamageRenderer] Loaded critical_enemy texture: %p", m_criticalEnemyTexture);
    
    CreateGameString(&enemyCritStr, "interface\\hitcount\\critical_enemy_shadow.ddj");
    m_criticalEnemyShadowTexture = LoadGameTexture(&enemyCritStr);
    
    // Load enemy blocking texture (for when we block incoming attacks)
    CreateGameString(&enemyCritStr, "interface\\hitcount\\blocking_enemy.ddj");
    m_blockingEnemyTexture = LoadGameTexture(&enemyCritStr);
    LogMsg("[CustomDamageRenderer] Loaded blocking_enemy texture: %p", m_blockingEnemyTexture);
    
    CreateGameString(&enemyCritStr, "interface\\hitcount\\blocking_enemy_shadow.ddj");
    m_blockingEnemyShadowTexture = LoadGameTexture(&enemyCritStr);
    
    LogMsg("[CustomDamageRenderer] Texture loading %s", allLoaded ? "SUCCESS" : "PARTIAL");
    return allLoaded;
}

void CustomDamageRenderer::AddDamage(DWORD targetID, DWORD damage, DWORD attackType, DWORD attackerID) {
    if (m_damageCount >= MAX_DAMAGE_ENTRIES) {
        for (int i = 0; i < m_damageCount - 1; i++) {
            m_damageQueue[i] = m_damageQueue[i + 1];
        }
        m_damageCount--;
    }
    
    DamageEntry* entry = &m_damageQueue[m_damageCount];
    entry->targetID = targetID;
    entry->damage = damage;
    entry->attackType = attackType;
    entry->timestamp = GetTickCount();
    entry->alpha = 1.0f;
    entry->offsetY = 0.0f;
    entry->posX = 0; entry->posY = 0; entry->posZ = 0;
    
    // ==========================================================================
    // INCOMING DAMAGE DETECTION - Entity Map Traversal
    // ==========================================================================
    // Entity map at 0xA0462C (from IDA analysis of FindEntityByID)
    // Node structure: +0x08=left, +0x0C=right, +0x10=entityID, +0x14=objectPtr
    // Incoming = player is TARGET (RED texture)
    // Outgoing = player is ATTACKER (WHITE texture)
    
    entry->isIncoming = false;  // Default: outgoing (WHITE)
    
    DWORD localPlayerID = 0;
    static DWORD s_cachedPlayerObj = 0;  // Track player object to detect character changes
    
    __try {
        // Get player object from g_pCICPlayer
        DWORD* pPlayerPtr = (DWORD*)0xA0465C;
        DWORD playerObj = *pPlayerPtr;
        
        // Invalidate cache if player object changed (character switch/relog)
        if (playerObj != s_cachedPlayerObj) {
            s_cachedPlayerObj = playerObj;
            g_LocalPlayerID = 0;  // Force re-lookup
            LogMsg("[CustomDamageRenderer] Player object changed to 0x%X, re-fetching ID", playerObj);
        }
        
        if (playerObj != 0 && g_LocalPlayerID == 0) {
            // Traverse entity map to find player's entity ID
            DWORD mapRoot = *(DWORD*)0xA0462C;
            if (mapRoot != 0) {
                DWORD node = *(DWORD*)(mapRoot + 4);  // First node
                
                // Simple traversal - check up to 1000 nodes
                int maxIter = 1000;
                DWORD stack[64];
                int stackIdx = 0;
                
                while ((node != 0 || stackIdx > 0) && maxIter-- > 0) {
                    // Go left as far as possible
                    while (node != 0 && node != mapRoot && maxIter-- > 0) {
                        if (stackIdx < 64) stack[stackIdx++] = node;
                        node = *(DWORD*)(node + 0x08);  // left child
                    }
                    
                    if (stackIdx > 0) {
                        node = stack[--stackIdx];
                        
                        // Check if this node's object matches player
                        DWORD nodeObj = *(DWORD*)(node + 0x14);
                        if (nodeObj == playerObj) {
                            localPlayerID = *(DWORD*)(node + 0x10);
                            g_LocalPlayerID = localPlayerID;
                            LogMsg("[CustomDamageRenderer] FOUND Player ID via MAP: %u (obj=0x%X)", localPlayerID, playerObj);
                            break;
                        }
                        
                        node = *(DWORD*)(node + 0x0C);  // right child
                    }
                }
            }
        }
        
        // Use cached ID
        if (localPlayerID == 0) {
            localPlayerID = g_LocalPlayerID;
        }
        
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        LogMsg("[CustomDamageRenderer] Exception in map traversal!");
    }
    
    // Determine incoming vs outgoing based on local player ID
    if (localPlayerID != 0) {
        if (targetID == localPlayerID) {
            entry->isIncoming = true;   // Player is being hit -> RED
        } else if (attackerID == localPlayerID) {
            entry->isIncoming = false;  // Player is attacking -> WHITE
        }
    }
    
    LogMsg("[CustomDamageRenderer] LocalPlayerID=%u, TargetID=%u, AttackerID=%u, Incoming=%s",
        localPlayerID, targetID, attackerID, entry->isIncoming ? "YES" : "NO");
    
    // ==========================================================================
    // VERTICAL STACKING - New damages appear below existing ones
    // ==========================================================================
    // Find how many active damages are on the same target and offset this one
    float stackOffset = 0.0f;
    const float STACK_HEIGHT = 35.0f;  // Pixels between stacked damages
    const DWORD STACK_TIME_WINDOW = 1500; // Only stack if within 1.5 seconds
    
    DWORD currentTime = GetTickCount();
    for (int i = 0; i < m_damageCount; i++) {
        DamageEntry& existing = m_damageQueue[i];
        // Check if same target and recent enough to stack
        if (existing.targetID == targetID && 
            (currentTime - existing.timestamp) < STACK_TIME_WINDOW) {
            stackOffset += STACK_HEIGHT;
        }
    }
    entry->offsetY = stackOffset;  // Store vertical offset for this entry
    
    // Get entity position for damage drawing
    void* pEntity = FindEntityByID(targetID);
    if (pEntity) {
        entry->posX = *(float*)((BYTE*)pEntity + 0x84);
        entry->posY = *(float*)((BYTE*)pEntity + 0x88);
        entry->posZ = *(float*)((BYTE*)pEntity + 0x8C);
    }
    
    m_damageCount++;
    LogMsg("[CustomDamageRenderer] Added: Target=%u, Dmg=%u, Type=%u, Incoming=%d, StackOffset=%.0f, Queue=%d", 
        targetID, damage, attackType, entry->isIncoming ? 1 : 0, stackOffset, m_damageCount);
}

bool CustomDamageRenderer::WorldToScreen(IDirect3DDevice9* pDevice, 
    float worldX, float worldY, float worldZ, float& screenX, float& screenY) {
    
    D3DVECTOR worldPos = {worldX, worldY, worldZ};
    D3DVECTOR screenPos = {0, 0, 0};
    
    CGFXVideo3d* gfx = CGFXVideo3d::get();
    if (gfx && gfx->Project(worldPos, screenPos) > 0) {
        screenX = screenPos.x;
        screenY = screenPos.y;
        return true;
    }
    return false;
}

bool CustomDamageRenderer::GetTextureSize(IDirect3DBaseTexture9* pTexture, DWORD& width, DWORD& height) {
    if (!pTexture) return false;
    
    IDirect3DTexture9* pTex2D = (IDirect3DTexture9*)pTexture;
    D3DSURFACE_DESC desc;
    if (SUCCEEDED(pTex2D->GetLevelDesc(0, &desc))) {
        width = desc.Width;
        height = desc.Height;
        return true;
    }
    return false;
}

void CustomDamageRenderer::DrawDigitSprite(IDirect3DDevice9* pDevice, int digit, 
    float x, float y, float scale, D3DCOLOR color, bool isIncoming) {
    
    if (digit < 0 || digit > 9) return;
    if (!m_pSprite) return;
    
    // Select texture based on incoming flag
    // isIncoming=true: Player is being hit -> RED (enemy textures)
    // isIncoming=false: Player is attacking -> use normal textures with color tint
    IDirect3DBaseTexture9* pDigitTex = isIncoming ? m_enemyDigitTextures[digit] : m_digitTextures[digit];
    IDirect3DBaseTexture9* pShadowTex = isIncoming ? m_enemyShadowTextures[digit] : m_digitShadowTextures[digit];
    
    // Determine draw color
    D3DCOLOR drawColor = 0xFFFFFFFF;  // Default WHITE
    
    // Fallback to normal textures if enemy textures not loaded
    if (!pDigitTex) {
        pDigitTex = m_digitTextures[digit];
        if (isIncoming) drawColor = 0xFFFF3A3A;  // Tint RED for incoming fallback
    } else if (isIncoming) {
        drawColor = 0xFFFFFFFF;  // Enemy textures are already red, use WHITE to show native color
    } else {
        // Outgoing: use the color parameter (WHITE/CYAN/YELLOW based on attack type)
        drawColor = color;
    }
    if (!pShadowTex) pShadowTex = m_digitShadowTextures[digit];
    
    if (!pDigitTex) return;
    
    D3DXVECTOR2 scaling(scale, scale);
    D3DXVECTOR2 translation(x, y);
    
    // Draw shadow first
    if (pShadowTex) {
        D3DXVECTOR2 shadowTranslation(x + 2, y + 2);
        m_pSprite->Draw(
            (LPDIRECT3DTEXTURE9)pShadowTex,
            NULL, &scaling, NULL, 0.0f, &shadowTranslation,
            D3DCOLOR_ARGB(180, 0, 0, 0)
        );
    }
    
    // Draw digit with appropriate color
    m_pSprite->Draw(
        (LPDIRECT3DTEXTURE9)pDigitTex,
        NULL, &scaling, NULL, 0.0f, &translation, drawColor
    );
}

void CustomDamageRenderer::DrawCriticalText(IDirect3DDevice9* pDevice, int x, int y, bool isIncoming) {
    // Draw "Critical" using game's native critical.ddj sprite texture
    // isIncoming=true: Player hit -> RED, isIncoming=false: Player attacks -> WHITE
    IDirect3DBaseTexture9* pTexture = isIncoming ? m_criticalEnemyTexture : m_criticalTexture;
    IDirect3DBaseTexture9* pShadowTexture = isIncoming ? m_criticalEnemyShadowTexture : m_criticalShadowTexture;
    
    // Fallback to normal critical texture if enemy version not available
    if (!pTexture) pTexture = m_criticalTexture;
    if (!pShadowTexture) pShadowTexture = m_criticalShadowTexture;
    
    if (!pTexture || !m_pSprite) return;
    
    // Get viewport for resolution-based scaling (same as damage numbers)
    D3DVIEWPORT9 viewport;
    pDevice->GetViewport(&viewport);
    
    const float REF_WIDTH = 1600.0f;
    const float REF_HEIGHT = 1200.0f;
    float resScaleX = (float)viewport.Width / REF_WIDTH;
    float resScaleY = (float)viewport.Height / REF_HEIGHT;
    float scale = (resScaleX + resScaleY) * 0.5f;  // Resolution-based scale
    
    DWORD texWidth = 64, texHeight = 16;
    GetTextureSize(pTexture, texWidth, texHeight);
    
    D3DXVECTOR2 scaling(scale, scale);
    
    // Center the critical text
    float startX = x - (texWidth * scale * 0.5f);
    D3DXVECTOR2 translation(startX, (float)y);
    
    // Draw shadow first
    if (pShadowTexture) {
        D3DXVECTOR2 shadowTranslation(startX + 2, (float)y + 2);
        m_pSprite->Draw(
            (LPDIRECT3DTEXTURE9)pShadowTexture,
            NULL, &scaling, NULL, 0.0f, &shadowTranslation,
            D3DCOLOR_ARGB(180, 0, 0, 0)
        );
    }
    
    // Draw "Critical" text
    // Outgoing: CYAN (#78CAD2), Incoming: use native red from enemy texture
    D3DCOLOR critColor = isIncoming ? 0xFFFFFFFF : 0xFF78CAD2;
    m_pSprite->Draw(
        (LPDIRECT3DTEXTURE9)pTexture,
        NULL, &scaling, NULL, 0.0f, &translation,
        critColor
    );
}

void CustomDamageRenderer::DrawBlockingText(IDirect3DDevice9* pDevice, int x, int y, bool isIncoming) {
    // Draw "Blocking" using game's native blocking.ddj sprite texture
    // isIncoming=true: Player hit -> RED, isIncoming=false: Player attacks -> WHITE
    IDirect3DBaseTexture9* pTexture = isIncoming ? m_blockingEnemyTexture : m_blockingTexture;
    IDirect3DBaseTexture9* pShadowTexture = isIncoming ? m_blockingEnemyShadowTexture : m_blockingShadowTexture;
    
    // Fallback to normal blocking texture if enemy version not available
    if (!pTexture) pTexture = m_blockingTexture;
    if (!pShadowTexture) pShadowTexture = m_blockingShadowTexture;
    
    if (!pTexture || !m_pSprite) return;
    
    // Get viewport for resolution-based scaling
    D3DVIEWPORT9 viewport;
    pDevice->GetViewport(&viewport);
    
    const float REF_WIDTH = 1600.0f;
    const float REF_HEIGHT = 1200.0f;
    float resScaleX = (float)viewport.Width / REF_WIDTH;
    float resScaleY = (float)viewport.Height / REF_HEIGHT;
    float scale = (resScaleX + resScaleY) * 0.5f;
    
    DWORD texWidth = 64, texHeight = 16;
    GetTextureSize(pTexture, texWidth, texHeight);
    
    D3DXVECTOR2 scaling(scale, scale);
    
    // Center the blocking text
    float startX = x - (texWidth * scale * 0.5f);
    D3DXVECTOR2 translation(startX, (float)y);
    
    // Draw shadow first
    if (pShadowTexture) {
        D3DXVECTOR2 shadowTranslation(startX + 2, (float)y + 2);
        m_pSprite->Draw(
            (LPDIRECT3DTEXTURE9)pShadowTexture,
            NULL, &scaling, NULL, 0.0f, &shadowTranslation,
            D3DCOLOR_ARGB(180, 0, 0, 0)
        );
    }
    
    // Draw "Blocking" text
    // Outgoing: YELLOW (#F4E04D), Incoming: use native red from enemy texture
    D3DCOLOR blockColor = isIncoming ? 0xFFFFFFFF : 0xFFF4E04D;
    m_pSprite->Draw(
        (LPDIRECT3DTEXTURE9)pTexture,
        NULL, &scaling, NULL, 0.0f, &translation,
        blockColor
    );
}

void CustomDamageRenderer::DrawDamageNumber(IDirect3DDevice9* pDevice, 
    int x, int y, DWORD damage, DWORD attackType, bool isIncoming) {
    
    // Get viewport for resolution-based scaling
    D3DVIEWPORT9 viewport;
    pDevice->GetViewport(&viewport);
    
    // Original game uses 1600x1200 reference resolution (from IDA):
    // flt_94FFC4 = 0.000625 = 1/1600 (width multiplier)
    // flt_94FFC0 = 0.000833 = 1/1200 (height multiplier)
    // Scale = (CurrentRes / ReferenceRes) * BaseScale
    const float REF_WIDTH = 1600.0f;
    const float REF_HEIGHT = 1200.0f;
    
    float resScaleX = (float)viewport.Width / REF_WIDTH;
    float resScaleY = (float)viewport.Height / REF_HEIGHT;
    float resScale = (resScaleX + resScaleY) * 0.5f;  // Average for uniform scale
    
    // Base scale - reduced to 0.7 for smaller, cleaner text
    float baseScale = 0.7f;
    D3DCOLOR color = 0xFFFFFFFF;  // Default: White (outgoing damage)
    // Bit flags - Note: GameServer sends 0x20/0x40 but packet processing shifts them to 0x100/0x200
    bool isCritical = (attackType & 0x200) != 0;  // Bit 9 = critical flag
    bool isBlock = (attackType & 0x100) != 0;     // Bit 8 = block flag
    
    // Note: BLOCK/CRITICAL logging removed to prevent spam (was logging every frame)
    
    // Color based on damage type
    // Outgoing: WHITE (normal), CYAN #78CAD2 (critical), YELLOW #F4E04D (block)
    // Incoming: RED (all types) - uses enemy textures
    if (isIncoming) {
        color = 0xFFFFFFFF;  // WHITE - enemy textures are already red
    } else if (isCritical) {
        baseScale = 0.8f;  // Slightly larger for critical
        color = 0xFF78CAD2;  // Critical: CYAN #78CAD2
    } else if (isBlock) {
        color = 0xFFF4E04D;  // Block: YELLOW #F4E04D
    }
    
    // Final scale = resolution factor * base scale
    float scale = resScale * baseScale;
    
    char text[16];
    sprintf(text, "%u", damage);
    int numDigits = strlen(text);
    
    DWORD digitWidth = 24, digitHeight = 32;
    if (m_digitTextures[0]) {
        GetTextureSize(m_digitTextures[0], digitWidth, digitHeight);
    }
    
    float scaledWidth = digitWidth * scale;
    float scaledHeight = digitHeight * scale;
    float totalWidth = scaledWidth * numDigits;
    float startX = x - totalWidth * 0.5f;
    
    // For blocks: only show "Blocking" text, no damage number
    if (isBlock) {
        // Draw just the block text centered at x,y
        DrawBlockingText(pDevice, x, y, isIncoming);
    }
    // For critical: show damage number + "Critical" text below
    else if (isCritical) {
        // Draw damage number
        for (int i = 0; i < numDigits; i++) {
            int digit = text[i] - '0';
            float digitX = startX + scaledWidth * i;
            DrawDigitSprite(pDevice, digit, digitX, (float)y, scale, color, isIncoming);
        }
        // Draw critical text below
        int critTextY = y + (int)scaledHeight + 5;
        DrawCriticalText(pDevice, x, critTextY, isIncoming);
    }
    // Normal damage: just show damage number
    else {
        for (int i = 0; i < numDigits; i++) {
            int digit = text[i] - '0';
            float digitX = startX + scaledWidth * i;
            DrawDigitSprite(pDevice, digit, digitX, (float)y, scale, color, isIncoming);
        }
    }
}

void CustomDamageRenderer::Render(IDirect3DDevice9* pDevice) {
    if (m_damageCount == 0) return;
    
    D3DVIEWPORT9 viewport;
    pDevice->GetViewport(&viewport);
    
    if (viewport.Width < 400 || viewport.Height < 300) {
        return;
    }
    
    // Load textures if not loaded
    if (!m_texturesLoaded) {
        LoadDigitTextures();
    }
    
    // Create sprite if needed
    if (!m_pSprite) {
        if (FAILED(D3DXCreateSprite(pDevice, &m_pSprite))) {
            LogMsg("[CustomDamageRenderer] Failed to create sprite!");
            return;
        }
        LogMsg("[CustomDamageRenderer] Sprite created: %p", m_pSprite);
    }
    
    DWORD currentTime = GetTickCount();
    const DWORD DAMAGE_LIFETIME = 2000;
    const float FLOAT_SPEED = 40.0f;
    
    // Setup alpha blending for transparent textures
    pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    
    m_pSprite->Begin();
    
    int drawnCount = 0;
    
    for (int i = 0; i < m_damageCount; ) {
        DamageEntry* entry = &m_damageQueue[i];
        DWORD elapsed = currentTime - entry->timestamp;
        
        if (elapsed > DAMAGE_LIFETIME) {
            for (int j = i; j < m_damageCount - 1; j++) {
                m_damageQueue[j] = m_damageQueue[j + 1];
            }
            m_damageCount--;
            continue;
        }
        
        // Animation: float up over time (world units)
        float animOffset = (elapsed / 1000.0f) * FLOAT_SPEED;
        
        // Get entity's current world position
        void* pEntity = FindEntityByID(entry->targetID);
        if (pEntity) {
            entry->posX = *(float*)((BYTE*)pEntity + 0x84);
            entry->posY = *(float*)((BYTE*)pEntity + 0x88);
            entry->posZ = *(float*)((BYTE*)pEntity + 0x8C);
        }
        
        float screenX, screenY;
        bool onScreen = false;
        
        if (entry->posX != 0 || entry->posY != 0 || entry->posZ != 0) {
            // Original game positioning from IDA analysis:
            // 1. sub_652A40 adds BONE offset from skeleton (mob's head/chest position)
            // 2. GenDigits adds flt_94AFC4 = 20.0 on top of that
            // Since we can't easily call bone lookup, we approximate with larger offset
            // Position above character's head
            const float WORLD_Y_OFFSET = 8.0f;  // Above head in world units
            
            // Add world-space offset BEFORE projection
            float worldY = entry->posY + WORLD_Y_OFFSET;
            
            onScreen = WorldToScreen(pDevice, entry->posX, worldY, entry->posZ, screenX, screenY);
        }
        
        int finalX, finalY;
        
        if (onScreen) {
            // Apply animation offset in screen space (floating up effect)
            // Also apply vertical stacking offset for multiple damages on same target
            finalX = (int)screenX + (i % 3 - 1) * 8;  // Small horizontal scatter
            finalY = (int)(screenY - animOffset + entry->offsetY);  // Float up + stack down
        } else {
            // Fallback: center of screen
            finalX = (int)(viewport.Width / 2) + (drawnCount * 60) - ((m_damageCount - 1) * 30);
            finalY = (int)(viewport.Height / 4 - animOffset + entry->offsetY);
        }
        
        DrawDamageNumber(pDevice, finalX, finalY, entry->damage, entry->attackType, entry->isIncoming);
        drawnCount++;
        ++i;
    }
    
    m_pSprite->End();
}
