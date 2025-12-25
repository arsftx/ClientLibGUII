#pragma once

#include <Windows.h>
#include <d3d9.h>
#include <d3dx9.h>

#define MAX_DAMAGE_ENTRIES 32

struct DamageEntry {
    DWORD targetID;
    DWORD damage;
    DWORD attackType;
    DWORD timestamp;
    float alpha;
    float offsetY;
    float posX, posY, posZ;
    bool isIncoming;  // true = we're taking damage (RED), false = we're dealing damage (WHITE)
};

class CustomDamageRenderer {
public:
    static CustomDamageRenderer& Instance();
    
    bool Initialize();
    void Shutdown();
    void AddDamage(DWORD targetID, DWORD damage, DWORD attackType, DWORD attackerID = 0);
    void Render(IDirect3DDevice9* pDevice);
    
    // Device reset handlers - call these on resolution change/alt-tab
    void OnDeviceLost();   // Before Reset()
    void OnDeviceReset();  // After Reset()
    
private:
    CustomDamageRenderer();
    ~CustomDamageRenderer();
    
    bool LoadDigitTextures();
    bool GetTextureSize(IDirect3DBaseTexture9* pTexture, DWORD& width, DWORD& height);
    bool WorldToScreen(IDirect3DDevice9* pDevice, float worldX, float worldY, float worldZ, float& screenX, float& screenY);
    void DrawDigitSprite(IDirect3DDevice9* pDevice, int digit, float x, float y, float scale, D3DCOLOR color, bool isIncoming = false);
    void DrawDamageNumber(IDirect3DDevice9* pDevice, int x, int y, DWORD damage, DWORD attackType, bool isIncoming = false);
    void DrawCriticalText(IDirect3DDevice9* pDevice, int x, int y, bool isIncoming = false);
    void DrawBlockingText(IDirect3DDevice9* pDevice, int x, int y, bool isIncoming = false);
    
private:
    bool m_initialized;
    bool m_texturesLoaded;
    
    ID3DXSprite* m_pSprite;
    
    // Digit textures (0-9) loaded via GameString - WHITE for outgoing damage
    IDirect3DBaseTexture9* m_digitTextures[10];
    IDirect3DBaseTexture9* m_digitShadowTextures[10];
    
    // Enemy digit textures (1-9) for incoming damage - RED
    IDirect3DBaseTexture9* m_enemyDigitTextures[10];
    IDirect3DBaseTexture9* m_enemyShadowTextures[10];
    
    // Special text textures (from original game) - OUTGOING damage
    IDirect3DBaseTexture9* m_criticalTexture;
    IDirect3DBaseTexture9* m_criticalShadowTexture;
    IDirect3DBaseTexture9* m_blockingTexture;
    IDirect3DBaseTexture9* m_blockingShadowTexture;
    
    // Enemy special text textures - INCOMING damage (RED)
    IDirect3DBaseTexture9* m_criticalEnemyTexture;
    IDirect3DBaseTexture9* m_criticalEnemyShadowTexture;
    IDirect3DBaseTexture9* m_blockingEnemyTexture;
    IDirect3DBaseTexture9* m_blockingEnemyShadowTexture;
    
    DamageEntry m_damageQueue[MAX_DAMAGE_ENTRIES];
    int m_damageCount;
};

// External functions for setting local player ID (call from character spawn packet handler)
void CustomDamageRenderer_SetLocalPlayerID(DWORD playerID);
DWORD CustomDamageRenderer_GetLocalPlayerID();
