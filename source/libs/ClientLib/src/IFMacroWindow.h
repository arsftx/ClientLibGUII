#pragma once

#include "IFMainFrame.h"
#include "IFStatic.h"
#include "IFButton.h"
#include "IFFrame.h"
#include "IFNormalTile.h"

#define GDR_MACRO_WINDOW 19700

// Control IDs
#define GDR_MW_FRAME 5100
#define GDR_MW_BG_TILE 5101
#define GDR_MW_ROW1_BG 5110
#define GDR_MW_ROW1_ICONBG 5111
#define GDR_MW_ROW1_ICON 5112
#define GDR_MW_ROW1_LABEL 5113
#define GDR_MW_ROW1_SETTINGS 5114
#define GDR_MW_ROW2_BG 5120
#define GDR_MW_ROW2_ICONBG 5121
#define GDR_MW_ROW2_ICON 5122
#define GDR_MW_ROW2_LABEL 5123
#define GDR_MW_ROW2_SETTINGS 5124
#define GDR_MW_ROW3_BG 5130
#define GDR_MW_ROW3_ICONBG 5131
#define GDR_MW_ROW3_ICON 5132
#define GDR_MW_ROW3_LABEL 5133
#define GDR_MW_ROW3_SETTINGS 5134
#define GDR_MW_ROW4_BG 5140
#define GDR_MW_ROW4_ICONBG 5141
#define GDR_MW_ROW4_ICON 5142
#define GDR_MW_ROW4_LABEL 5143
#define GDR_MW_ROW4_SETTINGS 5144

class CIFMacroWindow : public CIFMainFrame
{
    GFX_DECLARE_DYNCREATE(CIFMacroWindow)
    GFX_DECLARE_MESSAGE_MAP(CIFMacroWindow)

public:
    CIFMacroWindow();
    ~CIFMacroWindow();

    bool OnCreate(long ln) override;
    void OnUpdate() override;
    int OnMouseMove(int a1, int x, int y) override;
    void ResetPosition();

    // Toggle state getters
    bool IsAutoPotionEnabled() const { return m_autoPotionEnabled; }
    bool IsPetAutoPotionEnabled() const { return m_petAutoPotionEnabled; }
    bool IsPetFilterEnabled() const { return m_petFilterEnabled; }

private:
    // Button click handlers
    void On_AutoPotion_Toggle();
    void On_AutoPotion_Settings();
    void On_PetAutoPotion_Toggle();
    void On_PetAutoPotion_Settings();
    void On_AutoAttack_Toggle();
    void On_AutoAttack_Settings();
    void On_PetFilter_Toggle();
    void On_PetFilter_Settings();

    // Update icon textures based on state
    void UpdateIconStates();

private:
    // Toggle states
    bool m_autoPotionEnabled;
    bool m_petAutoPotionEnabled;
    bool m_petFilterEnabled;

public:
    // Auto Attack enabled (public for death handler access)
    bool m_autoAttackEnabled;

private:
    // Icon buttons (for texture switching)
    CIFButton* m_btnAutoPotionIcon;
    CIFButton* m_btnPetAutoPotionIcon;
    CIFButton* m_btnAutoAttackIcon;
    CIFButton* m_btnPetFilterIcon;

    // Labels
    CIFStatic* m_lblAutoPotion;
    CIFStatic* m_lblPetAutoPotion;
    CIFStatic* m_lblAutoAttack;
    CIFStatic* m_lblPetFilter;

public:
    // Static function for button click routing
    static void HandleButtonClick(int buttonId);
};

// Global frame pointer
extern CIFMainFrame* MacroWindowMainFrame;

// Global MacroWindow instance pointer (for death handler etc.)
extern CIFMacroWindow* g_pMacroWindow;

// Helper functions
void ShowMacroWindow(bool show);
bool IsMacroWindowVisible();
