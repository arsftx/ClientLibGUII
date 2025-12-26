#pragma once
/**
 * @file SROProgressBar.h
 * @brief SRO-style progress bar with glow and depth effects
 * 
 * Uses UIRenderer for advanced rendering with:
 * - Additive blending for glow
 * - Gloss overlay for glass effect
 * - Inner shadow for depth
 * 
 * Compatible with VS2005
 */

#include "UIRenderer.h"
#include <imgui/imgui.h>

class SROProgressBar {
public:
    ImVec2 m_pos;           // Position
    ImVec2 m_size;          // Full size
    void* m_textureFill;    // Fill texture (colored part)
    float m_percent;        // 0.0 to 1.0
    
    // Visual settings
    bool m_enableGlow;      // Additive blend for glow
    bool m_enableGloss;     // Glass/gloss overlay
    bool m_enableShadow;    // Inner shadow
    int m_glossIntensity;   // Gloss brightness (0-255)
    int m_shadowIntensity;  // Shadow darkness (0-255)
    ImU32 m_tintColor;      // Color tint for the fill
    
    SROProgressBar() {
        m_pos = ImVec2(0, 0);
        m_size = ImVec2(100, 10);
        m_textureFill = NULL;
        m_percent = 1.0f;
        m_enableGlow = true;
        m_enableGloss = true;
        m_enableShadow = true;
        m_glossIntensity = 60;
        m_shadowIntensity = 80;
        m_tintColor = IM_COL32(255, 255, 255, 255);
    }
    
    void Render(ImDrawList* drawList) {
        if (!m_textureFill) return;
        if (m_percent <= 0.0f) return;
        
        // Clamp percent
        float percent = m_percent;
        if (percent > 1.0f) percent = 1.0f;
        
        // Calculate current fill width
        float currentWidth = m_size.x * percent;
        ImVec2 fillSize = ImVec2(currentWidth, m_size.y);
        
        // UV coordinates for partial texture
        ImVec2 uvMin = ImVec2(0, 0);
        ImVec2 uvMax = ImVec2(percent, 1.0f);
        ImVec2 pMin = m_pos;
        ImVec2 pMax = ImVec2(m_pos.x + currentWidth, m_pos.y + m_size.y);
        
        // === LAYER 1: GLOW FILL (Additive Blend) ===
        if (m_enableGlow) {
            UIRenderer::DrawImageUV(
                drawList,
                m_textureFill,
                pMin, pMax,
                uvMin, uvMax,
                BLEND_ADDITIVE,
                m_tintColor
            );
        } else {
            // Normal blend
            UIRenderer::DrawImageUV(
                drawList,
                m_textureFill,
                pMin, pMax,
                uvMin, uvMax,
                BLEND_NORMAL,
                m_tintColor
            );
        }
        
        // === LAYER 2: GLOSS OVERLAY (Glass effect) ===
        if (m_enableGloss) {
            UIRenderer::DrawGlossOverlay(drawList, m_pos, fillSize, m_glossIntensity);
        }
        
        // === LAYER 3: INNER SHADOW (Depth effect) ===
        if (m_enableShadow) {
            UIRenderer::DrawInnerShadow(drawList, m_pos, fillSize, m_shadowIntensity);
        }
    }
    
    // Quick setup method
    void Setup(ImVec2 pos, ImVec2 size, void* texture, float percent) {
        m_pos = pos;
        m_size = size;
        m_textureFill = texture;
        m_percent = percent;
    }
};

// Specialized bars with preset colors
class SROHealthBar : public SROProgressBar {
public:
    SROHealthBar() : SROProgressBar() {
        m_tintColor = IM_COL32(255, 100, 100, 255); // Red tint
        m_glossIntensity = 50;
    }
};

class SROManaBar : public SROProgressBar {
public:
    SROManaBar() : SROProgressBar() {
        m_tintColor = IM_COL32(100, 150, 255, 255); // Blue tint
        m_glossIntensity = 50;
    }
};

class SROHwanBar : public SROProgressBar {
public:
    SROHwanBar() : SROProgressBar() {
        m_tintColor = IM_COL32(255, 200, 100, 255); // Orange/Gold tint
        m_glossIntensity = 70;
    }
};
