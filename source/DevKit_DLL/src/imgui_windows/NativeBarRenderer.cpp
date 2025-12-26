#include "NativeBarRenderer.h"
#include "../hooks/Hooks.h"
#include <GFX3DFunction/GFXVideo3d.h>
#include <stdio.h>

#pragma comment(lib, "d3dx9.lib")

// =============================================================================
// EndScene Hook Callback
// =============================================================================
static void NativeBar_OnEndScene() {
    if (!g_CD3DApplication) return;
    if (!g_CD3DApplication->m_pd3dDevice) return;
    
    IDirect3DDevice9* pDevice = g_CD3DApplication->m_pd3dDevice;
    
    // Check device state
    HRESULT hr = pDevice->TestCooperativeLevel();
    if (hr != D3D_OK && hr != S_FALSE) return;
    if (g_CD3DApplication->IsLost()) return;
    
    NativeBarRenderer::Instance().Render(pDevice);
}

static void NativeBar_OnPreSetSize(int w, int h) {
    NativeBarRenderer::Instance().OnDeviceLost();
}

static void NativeBar_OnPostSetSize(int w, int h) {
    NativeBarRenderer::Instance().OnDeviceReset();
}

// =============================================================================
// Singleton
// =============================================================================
NativeBarRenderer& NativeBarRenderer::Instance() {
    static NativeBarRenderer instance;
    return instance;
}

NativeBarRenderer::NativeBarRenderer() 
    : m_initialized(false), m_pSprite(NULL), m_barCount(0) {
    memset(m_bars, 0, sizeof(m_bars));
}

NativeBarRenderer::~NativeBarRenderer() {
    if (m_pSprite) {
        m_pSprite->Release();
        m_pSprite = NULL;
    }
}

void NativeBarRenderer::Initialize() {
    if (m_initialized) return;
    
    // Register with hook system
    OnEndScene(NativeBar_OnEndScene);
    OnPreSetSize(NativeBar_OnPreSetSize);
    OnPostSetSize(NativeBar_OnPostSetSize);
    
    m_initialized = true;
}

void NativeBarRenderer::OnDeviceLost() {
    if (m_pSprite) {
        m_pSprite->OnLostDevice();
    }
}

void NativeBarRenderer::OnDeviceReset() {
    if (m_pSprite) {
        m_pSprite->OnResetDevice();
    }
}

void NativeBarRenderer::QueueBar(IDirect3DTexture9* pTexture, 
    float x, float y, float w, float h, float fillPercent, DWORD color) {
    if (m_barCount >= MAX_NATIVE_BARS) return;
    if (!pTexture) return;
    if (fillPercent <= 0.0f) return;
    
    BarRenderData& bar = m_bars[m_barCount++];
    bar.valid = true;
    bar.pTexture = pTexture;
    bar.x = x;
    bar.y = y;
    bar.width = w;
    bar.height = h;
    bar.fillPercent = fillPercent;
    bar.color = color;
}

void NativeBarRenderer::ClearBars() {
    m_barCount = 0;
}

void NativeBarRenderer::Render(IDirect3DDevice9* pDevice) {
    if (m_barCount == 0) return;
    
    // Create sprite if needed
    if (!m_pSprite) {
        if (FAILED(D3DXCreateSprite(pDevice, &m_pSprite))) {
            return;
        }
    }
    
    // Setup alpha blending
    pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    
    m_pSprite->Begin();
    
    for (int i = 0; i < m_barCount; i++) {
        BarRenderData& bar = m_bars[i];
        if (!bar.valid || !bar.pTexture) continue;
        
        // Get texture size
        D3DSURFACE_DESC desc;
        if (FAILED(bar.pTexture->GetLevelDesc(0, &desc))) continue;
        
        // Create source rect for UV clipping (progress bar effect)
        RECT srcRect;
        srcRect.left = 0;
        srcRect.top = 0;
        srcRect.right = (LONG)(desc.Width * bar.fillPercent);
        srcRect.bottom = desc.Height;
        
        if (srcRect.right <= 0) continue;
        
        // Calculate scale to fit desired size
        float actualWidth = bar.width * bar.fillPercent;
        float scaleX = actualWidth / (float)srcRect.right;
        float scaleY = bar.height / (float)desc.Height;
        D3DXVECTOR2 scaling(scaleX, scaleY);
        D3DXVECTOR2 position(bar.x, bar.y);
        
        m_pSprite->Draw(
            bar.pTexture,
            &srcRect,
            &scaling,
            NULL,
            0.0f,
            &position,
            bar.color
        );
    }
    
    m_pSprite->End();
    
    // Clear bars for next frame
    m_barCount = 0;
}
