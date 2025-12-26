#pragma once
/**
 * @file NativeBarRenderer.h
 * @brief Native DirectX bar rendering after ImGui (EndScene hook)
 * 
 * ImGui calculates positions, this renders bars with native D3D quality.
 * Similar pattern to CustomDamageRenderer.
 */

#include <d3d9.h>
#include <d3dx9.h>

// Bar render data - filled by ImGui, rendered by EndScene hook
struct BarRenderData {
    bool valid;
    IDirect3DTexture9* pTexture;
    float x, y, width, height;
    float fillPercent;  // 0.0 - 1.0 for progress bars
    DWORD color;
};

// Maximum bars to render
#define MAX_NATIVE_BARS 8

class NativeBarRenderer {
public:
    static NativeBarRenderer& Instance();
    
    // Initialize - call once (registers EndScene hook)
    void Initialize();
    
    // Queue a bar for native rendering (called from ImGui)
    void QueueBar(IDirect3DTexture9* pTexture, float x, float y, float w, float h, float fillPercent, DWORD color = 0xFFFFFFFF);
    
    // Clear all queued bars (called at start of each frame)
    void ClearBars();
    
    // Render all queued bars (called from EndScene hook)
    void Render(IDirect3DDevice9* pDevice);
    
    // Device reset handlers
    void OnDeviceLost();
    void OnDeviceReset();
    
private:
    NativeBarRenderer();
    ~NativeBarRenderer();
    
    bool m_initialized;
    ID3DXSprite* m_pSprite;
    
    BarRenderData m_bars[MAX_NATIVE_BARS];
    int m_barCount;
};
