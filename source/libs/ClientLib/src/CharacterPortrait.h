#pragma once
#include <windows.h>
#include <d3d9.h>
#include <stdio.h>

namespace NativeRender {
    enum GlobalAddresses {
        ADDR_PLAYER_PTR = 0x00A0465C
    };
    
    // Player model ID is at offset +0x160
    enum { PLAYER_MODEL_ID_OFFSET = 0x160 };
}

// Game's native string structure for texture loading
struct PortraitGameString {
    char* data;
    char* end;
    char* capacity;
};

// Forward declare game functions (same as CustomPlayerMiniInfo.cpp)
typedef void (__thiscall *tPortraitStringConstruct)(PortraitGameString* pThis, const char* start, const char* end);
typedef IDirect3DBaseTexture9* (__cdecl *tPortraitLoadTexture)(PortraitGameString* pPath);

class CharacterPortrait
{
public:
    CharacterPortrait() 
        : m_bInitialized(false), m_nModelID(0), m_nLoadedModelID(0), m_pTexture(NULL)
    {}
    
    ~CharacterPortrait() {}

    bool Initialize(int width, int height) {
        m_bInitialized = true;
        return true;
    }

    void Destroy() {
        m_pTexture = NULL;  // Game manages texture memory
    }

    void Update() {
        if (!m_bInitialized) return;
        if (!IsPlayerSpawned()) return;
        
        UpdateModelID();
        
        // Load texture if model ID changed
        if (m_nModelID != m_nLoadedModelID && m_nModelID != 0) {
            LoadPortraitTexture();
        }
    }

    IDirect3DTexture9* GetTexture() const { 
        return m_pTexture;
    }

    bool IsReady() const { return m_pTexture != NULL; }
    int GetWidth() const { return 64; }
    int GetHeight() const { return 64; }
    int GetModelID() const { return m_nModelID; }

private:
    bool IsPlayerSpawned() const {
        __try {
            DWORD* pPlayerPtr = (DWORD*)NativeRender::ADDR_PLAYER_PTR;
            if (!pPlayerPtr || !*pPlayerPtr) return false;
            int hp = *(int*)(*pPlayerPtr + 0x358);
            return hp > 0;
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            return false;
        }
    }

    void UpdateModelID() {
        using namespace NativeRender;
        
        __try {
            DWORD* pPlayerPtr = (DWORD*)ADDR_PLAYER_PTR;
            if (!pPlayerPtr || !*pPlayerPtr) return;
            DWORD pPlayer = *pPlayerPtr;
            
            // Model ID at offset +0x160
            m_nModelID = *(int*)(pPlayer + PLAYER_MODEL_ID_OFFSET);
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            m_nModelID = 0;
        }
    }

    void LoadPortraitTexture() {
        // Build path: newui\playerminiinfo\portrait\{modelID}.ddj
        char path[256];
        sprintf(path, "newui\\playerminiinfo\\portrait\\%d.ddj", m_nModelID);
        
        // Get game's texture loading functions
        tPortraitStringConstruct StringConstruct = (tPortraitStringConstruct)0x00406190;
        tPortraitLoadTexture LoadGameTexture = (tPortraitLoadTexture)0x00409E10;
        
        __try {
            PortraitGameString pathStr = {0, 0, 0};
            size_t len = strlen(path);
            StringConstruct(&pathStr, path, path + len);
            
            IDirect3DBaseTexture9* pTex = LoadGameTexture(&pathStr);
            if (pTex) {
                m_pTexture = (IDirect3DTexture9*)pTex;
                m_nLoadedModelID = m_nModelID;
            }
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            // Failed to load texture
        }
    }

    bool m_bInitialized;
    int m_nModelID;
    int m_nLoadedModelID;
    IDirect3DTexture9* m_pTexture;
};

inline CharacterPortrait& GetCharacterPortrait() {
    static CharacterPortrait s_Instance;
    return s_Instance;
}
