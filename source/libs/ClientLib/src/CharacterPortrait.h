#pragma once
#include <windows.h>
#include <d3d9.h>

// TODO: Load PNG face image based on model ID
// Model ID ranges:
// - Chinese Male: ~1907
// - Chinese Female: ~1908  
// - European Male: ~14709
// - European Female: ~14714

namespace NativeRender {
    enum GlobalAddresses {
        ADDR_PLAYER_PTR = 0x00A0465C
    };
    
    // Player model ID is at offset +0x160
    enum { PLAYER_MODEL_ID_OFFSET = 0x160 };
}

class CharacterPortrait
{
public:
    CharacterPortrait() 
        : m_bInitialized(false), m_nModelID(0)
    {}
    
    ~CharacterPortrait() {}

    bool Initialize(int width, int height) {
        m_bInitialized = true;
        return true;
    }

    void Destroy() {}

    void Update() {
        if (!m_bInitialized) return;
        if (!IsPlayerSpawned()) return;
        
        UpdateModelID();
    }

    // TODO: Return loaded PNG texture based on model ID
    IDirect3DTexture9* GetTexture() const { 
        return NULL;
    }

    bool IsReady() const { return m_nModelID != 0; }
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

    bool m_bInitialized;
    int m_nModelID;
};

inline CharacterPortrait& GetCharacterPortrait() {
    static CharacterPortrait s_Instance;
    return s_Instance;
}
