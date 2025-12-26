#pragma once
/**
 * @file UIRenderer.h
 * @brief Custom UI Renderer with DirectX Blend Modes for SRO-quality graphics
 * 
 * This class provides ImGui with DX9 blend state control for:
 * - Additive Blending (glow/neon effects)
 * - Multiply Blending (shadows)
 * - Gloss/Glass overlays for depth
 * 
 * Compatible with VS2005
 */

#include <imgui/imgui.h>
#include <d3d9.h>

// Blend Modes - Control how textures are rendered
enum BlendModeType {
    BLEND_NORMAL = 0,   // Standard alpha blend (flat, matte)
    BLEND_ADDITIVE,     // Glow effect (colors add up, black disappears)
    BLEND_MULTIPLY      // Darken (for shadows)
};

class UIRenderer {
private:
    static IDirect3DDevice9* s_pd3dDevice;

    // ImGui callback to change DX9 render state between draw calls
    static void RenderStateCallback(const ImDrawList* parent_list, const ImDrawCmd* cmd) {
        BlendModeType mode = (BlendModeType)(int)(size_t)cmd->UserCallbackData;
        
        if (!s_pd3dDevice) return;

        // === POINT FILTERING - Keskin kenarlar için (bulanıklığı önler) ===
        s_pd3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
        s_pd3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);

        if (mode == BLEND_ADDITIVE) {
            // ADDITIVE MODE - SRO Glow Effect
            // Colors add together. Black becomes transparent. Colors glow.
            s_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
            s_pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
            s_pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE); // Key: ONE instead of INVSRCALPHA
        }
        else if (mode == BLEND_MULTIPLY) {
            // MULTIPLY MODE - Darken effect for shadows
            s_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
            s_pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_DESTCOLOR);
            s_pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);
        }
        else {
            // NORMAL MODE - Standard ImGui blending
            s_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
            s_pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
            s_pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
        }
    }

public:
    // Initialize with DX9 device
    static void Init(IDirect3DDevice9* device) {
        s_pd3dDevice = device;
    }
    
    static IDirect3DDevice9* GetDevice() {
        return s_pd3dDevice;
    }

    // Draw image with blend mode control
    static void DrawImage(ImDrawList* drawList, void* texture, ImVec2 pos, ImVec2 size, 
                         BlendModeType mode = BLEND_NORMAL, ImU32 color = 0xFFFFFFFF) {
        if (!texture) return;
        
        if (mode != BLEND_NORMAL) {
            // Switch to requested blend mode
            drawList->AddCallback(RenderStateCallback, (void*)(size_t)mode);
        }

        // Draw the image
        drawList->AddImage((ImTextureID)texture, pos, 
                          ImVec2(pos.x + size.x, pos.y + size.y), 
                          ImVec2(0, 0), ImVec2(1, 1), color);

        if (mode != BLEND_NORMAL) {
            // Reset to normal blend mode
            drawList->AddCallback(RenderStateCallback, (void*)(size_t)BLEND_NORMAL);
        }
    }
    
    // Draw image with UV coords and blend mode
    static void DrawImageUV(ImDrawList* drawList, void* texture, 
                           ImVec2 pMin, ImVec2 pMax, ImVec2 uvMin, ImVec2 uvMax,
                           BlendModeType mode = BLEND_NORMAL, ImU32 color = 0xFFFFFFFF) {
        if (!texture) return;
        
        if (mode != BLEND_NORMAL) {
            drawList->AddCallback(RenderStateCallback, (void*)(size_t)mode);
        }

        drawList->AddImage((ImTextureID)texture, pMin, pMax, uvMin, uvMax, color);

        if (mode != BLEND_NORMAL) {
            drawList->AddCallback(RenderStateCallback, (void*)(size_t)BLEND_NORMAL);
        }
    }

    // Glass/Gloss effect overlay - adds depth to flat elements
    // Draws a white-to-transparent gradient on top of an element
    static void DrawGlossOverlay(ImDrawList* drawList, ImVec2 pos, ImVec2 size, 
                                 int intensity = 100) {
        ImVec2 pMin = pos;
        ImVec2 pMax = ImVec2(pos.x + size.x, pos.y + (size.y * 0.5f)); // Top half only
        
        // White to transparent gradient (cylinder highlight effect)
        drawList->AddRectFilledMultiColor(
            pMin, pMax,
            IM_COL32(255, 255, 255, intensity),  // Top Left (bright)
            IM_COL32(255, 255, 255, intensity),  // Top Right
            IM_COL32(255, 255, 255, 0),          // Bottom Right (transparent)
            IM_COL32(255, 255, 255, 0)           // Bottom Left
        );
    }
    
    // Inner shadow effect - adds depth to bottom of elements
    static void DrawInnerShadow(ImDrawList* drawList, ImVec2 pos, ImVec2 size,
                               int intensity = 100) {
        // Shadow on bottom 30% of element
        ImVec2 pMin = ImVec2(pos.x, pos.y + size.y * 0.7f);
        ImVec2 pMax = ImVec2(pos.x + size.x, pos.y + size.y);
        
        drawList->AddRectFilledMultiColor(
            pMin, pMax,
            IM_COL32(0, 0, 0, 0), IM_COL32(0, 0, 0, 0),          // Top (transparent)
            IM_COL32(0, 0, 0, intensity), IM_COL32(0, 0, 0, intensity)  // Bottom (dark)
        );
    }
    
    // Outer glow effect around an element
    static void DrawOuterGlow(ImDrawList* drawList, ImVec2 pos, ImVec2 size,
                             ImU32 glowColor, int glowSize = 4) {
        // Draw multiple expanding rectangles with decreasing opacity
        for (int i = glowSize; i >= 1; i--) {
            int alpha = (int)(40.0f / (float)i);
            ImU32 color = (glowColor & 0x00FFFFFF) | ((alpha & 0xFF) << 24);
            
            ImVec2 pMin = ImVec2(pos.x - i, pos.y - i);
            ImVec2 pMax = ImVec2(pos.x + size.x + i, pos.y + size.y + i);
            drawList->AddRect(pMin, pMax, color, 0, 0, 1.0f);
        }
    }
};

// Static member definition
// Note: This should be in a .cpp file, but for header-only approach:
#ifndef UIRENDERER_IMPL
#define UIRENDERER_IMPL
IDirect3DDevice9* UIRenderer::s_pd3dDevice = NULL;
#endif
