#include "IFMacroWindow.h"
#include "GInterface.h"
#include "Game.h"
#include "IFAutoHuntSettings.h"
#include "IFAutoPotion.h"
#include "IFNormalTile.h"
#include "IFPetAutoPotion.h"
#include "IFPetFilterSettings.h"
#include "AutoBuffController.h"
#include "AutoTargetController.h"
#include "AutoAttackSkillController.h"
#include "AutoMoveController.h"
#include "ReturnToTownController.h"
#include <BSLib/multibyte.h>


// D3D9 Viewport Resolution
extern int g_D3DViewportWidth;
extern int g_D3DViewportHeight;

// Global macro window frame
CIFMainFrame *MacroWindowMainFrame = NULL;

// Pointer to the CIFMacroWindow instance for button handler routing
CIFMacroWindow *g_pMacroWindow = NULL;

GFX_IMPLEMENT_DYNCREATE(CIFMacroWindow, CIFMainFrame)

GFX_BEGIN_MESSAGE_MAP(CIFMacroWindow, CIFMainFrame)
ONG_COMMAND(GDR_MW_ROW1_ICON, &On_AutoPotion_Toggle)
ONG_COMMAND(GDR_MW_ROW1_SETTINGS, &On_AutoPotion_Settings)
ONG_COMMAND(GDR_MW_ROW2_ICON, &On_PetAutoPotion_Toggle)
ONG_COMMAND(GDR_MW_ROW2_SETTINGS, &On_PetAutoPotion_Settings)
ONG_COMMAND(GDR_MW_ROW3_ICON, &On_AutoAttack_Toggle)
ONG_COMMAND(GDR_MW_ROW3_SETTINGS, &On_AutoAttack_Settings)
ONG_COMMAND(GDR_MW_ROW4_ICON, &On_PetFilter_Toggle)
ONG_COMMAND(GDR_MW_ROW4_SETTINGS, &On_PetFilter_Settings)
GFX_END_MESSAGE_MAP()

CIFMacroWindow::CIFMacroWindow()
    : m_autoPotionEnabled(false), m_petAutoPotionEnabled(false), m_autoAttackEnabled(false), m_petFilterEnabled(false), m_btnAutoPotionIcon(NULL), m_btnPetAutoPotionIcon(NULL), m_btnAutoAttackIcon(NULL), m_btnPetFilterIcon(NULL), m_lblAutoPotion(NULL), m_lblPetAutoPotion(NULL), m_lblAutoAttack(NULL), m_lblPetFilter(NULL) {
    g_pMacroWindow = this;
}

CIFMacroWindow::~CIFMacroWindow() {
    g_pMacroWindow = NULL;
}

bool CIFMacroWindow::OnCreate(long ln) {
    // Create a SEPARATE main frame
    // CRITICAL: Create OFF-SCREEN to prevent blocking clicks in upper-left corner!
    RECT MacroWindowRect = {-500, -500, 360, 300};
    MacroWindowMainFrame = (CIFMainFrame *) CGWnd::CreateInstance(g_pCGInterface, GFX_RUNTIME_CLASS(CIFMainFrame), MacroWindowRect, 2002, 0);

    if (!MacroWindowMainFrame) {
        return false;
    }

    MacroWindowMainFrame->SetText("Macro");
    MacroWindowMainFrame->TB_Func_12("interface\\frame\\mframe_wnd_", 1, 0);
    MacroWindowMainFrame->SetGWndSize(360, 300);

    wnd_rect sz;

    sz.pos.x = 13;
    sz.pos.y = 40;
    sz.size.width = 333;
    sz.size.height = 248;
    CIFFrame *frame = (CIFFrame *) CGWnd::CreateInstance(MacroWindowMainFrame, GFX_RUNTIME_CLASS(CIFFrame), sz, GDR_MW_FRAME, 0);
    if (frame) {
        frame->TB_Func_12("interface\\frame\\frame_sub_", 1, 0);
        frame->SetGWndSize(333, 248);
    }

    // Background tile
    sz.pos.x = 23;
    sz.pos.y = 50;
    sz.size.width = 313;
    sz.size.height = 228;
    CIFNormalTile *tile = (CIFNormalTile *) CGWnd::CreateInstance(MacroWindowMainFrame, GFX_RUNTIME_CLASS(CIFNormalTile), sz, GDR_MW_BG_TILE, 0);
    if (tile) {
        tile->TB_Func_12("interface\\ifcommon\\bg_tile\\com_bg_tile_a.ddj", 1, 0);
        tile->SetGWndSize(313, 228);
    }

    // ========== ROW 1: Auto Potion ==========
    sz.pos.x = 23;
    sz.pos.y = 50;
    sz.size.width = 313;
    sz.size.height = 56;
    CIFStatic *row1_bg = (CIFStatic *) CGWnd::CreateInstance(MacroWindowMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, GDR_MW_ROW1_BG, 0);
    if (row1_bg) {
        row1_bg->TB_Func_12("interface\\macro\\macro_box.ddj", 1, 1);
        row1_bg->SetGWndSize(313, 56);
    }

    sz.pos.x = 140;
    sz.pos.y = 57;
    sz.size.width = 76;
    sz.size.height = 40;
    CIFStatic *row1_iconbg = (CIFStatic *) CGWnd::CreateInstance(MacroWindowMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, GDR_MW_ROW1_ICONBG, 0);
    if (row1_iconbg) {
        row1_iconbg->TB_Func_12("interface\\macro\\macro_iconbg.ddj", 1, 1);
        row1_iconbg->SetGWndSize(76, 40);
    }

    sz.pos.x = 162;
    sz.pos.y = 61;
    sz.size.width = 32;
    sz.size.height = 32;
    m_btnAutoPotionIcon = (CIFButton *) CGWnd::CreateInstance(MacroWindowMainFrame, GFX_RUNTIME_CLASS(CIFButton), sz, GDR_MW_ROW1_ICON, 0);
    if (m_btnAutoPotionIcon) {
        m_btnAutoPotionIcon->TB_Func_12("interface\\macro\\macro_icon_potion_off.ddj", 1, 1);
        m_btnAutoPotionIcon->SetGWndSize(32, 32);
    }

    sz.pos.x = 40;
    sz.pos.y = 72;
    sz.size.width = 85;
    sz.size.height = 13;
    m_lblAutoPotion = (CIFStatic *) CGWnd::CreateInstance(MacroWindowMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, GDR_MW_ROW1_LABEL, 0);
    if (m_lblAutoPotion) {
        m_lblAutoPotion->SetText("Auto Potion");
        m_lblAutoPotion->SetGWndSize(85, 13);
    }

    sz.pos.x = 230;
    sz.pos.y = 66;
    sz.size.width = 90;
    sz.size.height = 23;
    CIFButton *btn_row1_settings = (CIFButton *) CGWnd::CreateInstance(MacroWindowMainFrame, GFX_RUNTIME_CLASS(CIFButton), sz, GDR_MW_ROW1_SETTINGS, 0);
    if (btn_row1_settings) {
        btn_row1_settings->SetText("Settings");
        btn_row1_settings->TB_Func_12("interface\\ifcommon\\com_mid_button.ddj", 1, 1);
        btn_row1_settings->SetGWndSize(90, 23);
    }

    // ========== ROW 2: Pet Auto Potion ==========
    sz.pos.x = 23;
    sz.pos.y = 106;
    sz.size.width = 313;
    sz.size.height = 56;
    CIFStatic *row2_bg = (CIFStatic *) CGWnd::CreateInstance(MacroWindowMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, GDR_MW_ROW2_BG, 0);
    if (row2_bg) {
        row2_bg->TB_Func_12("interface\\macro\\macro_box.ddj", 1, 1);
        row2_bg->SetGWndSize(313, 56);
    }

    sz.pos.x = 140;
    sz.pos.y = 111;
    sz.size.width = 76;
    sz.size.height = 40;
    CIFStatic *row2_iconbg = (CIFStatic *) CGWnd::CreateInstance(MacroWindowMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, GDR_MW_ROW2_ICONBG, 0);
    if (row2_iconbg) {
        row2_iconbg->TB_Func_12("interface\\macro\\macro_iconbg.ddj", 1, 1);
        row2_iconbg->SetGWndSize(76, 40);
    }

    sz.pos.x = 162;
    sz.pos.y = 115;
    sz.size.width = 32;
    sz.size.height = 32;
    m_btnPetAutoPotionIcon = (CIFButton *) CGWnd::CreateInstance(MacroWindowMainFrame, GFX_RUNTIME_CLASS(CIFButton), sz, GDR_MW_ROW2_ICON, 0);
    if (m_btnPetAutoPotionIcon) {
        m_btnPetAutoPotionIcon->TB_Func_12("interface\\macro\\macro_icon_potion_off.ddj", 1, 1);
        m_btnPetAutoPotionIcon->SetGWndSize(32, 32);
    }

    sz.pos.x = 40;
    sz.pos.y = 123;
    sz.size.width = 85;
    sz.size.height = 13;
    m_lblPetAutoPotion = (CIFStatic *) CGWnd::CreateInstance(MacroWindowMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, GDR_MW_ROW2_LABEL, 0);
    if (m_lblPetAutoPotion) {
        m_lblPetAutoPotion->SetText("Pet Potion");
        m_lblPetAutoPotion->SetGWndSize(85, 13);
    }

    sz.pos.x = 230;
    sz.pos.y = 120;
    sz.size.width = 90;
    sz.size.height = 23;
    CIFButton *btn_row2_settings = (CIFButton *) CGWnd::CreateInstance(MacroWindowMainFrame, GFX_RUNTIME_CLASS(CIFButton), sz, GDR_MW_ROW2_SETTINGS, 0);
    if (btn_row2_settings) {
        btn_row2_settings->SetText("Settings");
        btn_row2_settings->TB_Func_12("interface\\ifcommon\\com_mid_button.ddj", 1, 1);
        btn_row2_settings->SetGWndSize(90, 23);
    }

    // ========== ROW 3: Auto Attack ==========
    sz.pos.x = 23;
    sz.pos.y = 162;
    sz.size.width = 313;
    sz.size.height = 56;
    CIFStatic *row3_bg = (CIFStatic *) CGWnd::CreateInstance(MacroWindowMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, GDR_MW_ROW3_BG, 0);
    if (row3_bg) {
        row3_bg->TB_Func_12("interface\\macro\\macro_box.ddj", 1, 1);
        row3_bg->SetGWndSize(313, 56);
    }

    sz.pos.x = 140;
    sz.pos.y = 165;
    sz.size.width = 76;
    sz.size.height = 40;
    CIFStatic *row3_iconbg = (CIFStatic *) CGWnd::CreateInstance(MacroWindowMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, GDR_MW_ROW3_ICONBG, 0);
    if (row3_iconbg) {
        row3_iconbg->TB_Func_12("interface\\macro\\macro_iconbg.ddj", 1, 1);
        row3_iconbg->SetGWndSize(76, 40);
    }

    sz.pos.x = 162;
    sz.pos.y = 169;
    sz.size.width = 32;
    sz.size.height = 32;
    m_btnAutoAttackIcon = (CIFButton *) CGWnd::CreateInstance(MacroWindowMainFrame, GFX_RUNTIME_CLASS(CIFButton), sz, GDR_MW_ROW3_ICON, 0);
    if (m_btnAutoAttackIcon) {
        m_btnAutoAttackIcon->TB_Func_12("interface\\macro\\macro_icon_hunting_off.ddj", 1, 1);
        m_btnAutoAttackIcon->SetGWndSize(32, 32);
    }

    sz.pos.x = 40;
    sz.pos.y = 174;
    sz.size.width = 85;
    sz.size.height = 13;
    m_lblAutoAttack = (CIFStatic *) CGWnd::CreateInstance(MacroWindowMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, GDR_MW_ROW3_LABEL, 0);
    if (m_lblAutoAttack) {
        m_lblAutoAttack->SetText("Auto Attack");
        m_lblAutoAttack->SetGWndSize(85, 13);
    }

    sz.pos.x = 230;
    sz.pos.y = 174;
    sz.size.width = 90;
    sz.size.height = 23;
    CIFButton *btn_row3_settings = (CIFButton *) CGWnd::CreateInstance(MacroWindowMainFrame, GFX_RUNTIME_CLASS(CIFButton), sz, GDR_MW_ROW3_SETTINGS, 0);
    if (btn_row3_settings) {
        btn_row3_settings->SetText("Settings");
        btn_row3_settings->TB_Func_12("interface\\ifcommon\\com_mid_button.ddj", 1, 1);
        btn_row3_settings->SetGWndSize(90, 23);
    }

    // ========== ROW 4: Pet Filter ==========
    sz.pos.x = 23;
    sz.pos.y = 218;
    sz.size.width = 313;
    sz.size.height = 56;
    CIFStatic *row4_bg = (CIFStatic *) CGWnd::CreateInstance(MacroWindowMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, GDR_MW_ROW4_BG, 0);
    if (row4_bg) {
        row4_bg->TB_Func_12("interface\\macro\\macro_box.ddj", 1, 1);
        row4_bg->SetGWndSize(313, 56);
    }

    sz.pos.x = 140;
    sz.pos.y = 221;
    sz.size.width = 76;
    sz.size.height = 40;
    CIFStatic *row4_iconbg = (CIFStatic *) CGWnd::CreateInstance(MacroWindowMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, GDR_MW_ROW4_ICONBG, 0);
    if (row4_iconbg) {
        row4_iconbg->TB_Func_12("interface\\macro\\macro_iconbg.ddj", 1, 1);
        row4_iconbg->SetGWndSize(76, 40);
    }

    sz.pos.x = 162;
    sz.pos.y = 225;
    sz.size.width = 32;
    sz.size.height = 32;
    m_btnPetFilterIcon = (CIFButton *) CGWnd::CreateInstance(MacroWindowMainFrame, GFX_RUNTIME_CLASS(CIFButton), sz, GDR_MW_ROW4_ICON, 0);
    if (m_btnPetFilterIcon) {
        m_btnPetFilterIcon->TB_Func_12("interface\\macro\\macro_icon_hunting_off.ddj", 1, 1);
        m_btnPetFilterIcon->SetGWndSize(32, 32);
    }

    sz.pos.x = 40;
    sz.pos.y = 230;
    sz.size.width = 85;
    sz.size.height = 13;
    m_lblPetFilter = (CIFStatic *) CGWnd::CreateInstance(MacroWindowMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, GDR_MW_ROW4_LABEL, 0);
    if (m_lblPetFilter) {
        m_lblPetFilter->SetText("Pet Filter");
        m_lblPetFilter->SetGWndSize(85, 13);
    }

    sz.pos.x = 230;
    sz.pos.y = 230;
    sz.size.width = 90;
    sz.size.height = 23;
    CIFButton *btn_row4_settings = (CIFButton *) CGWnd::CreateInstance(MacroWindowMainFrame, GFX_RUNTIME_CLASS(CIFButton), sz, GDR_MW_ROW4_SETTINGS, 0);
    if (btn_row4_settings) {
        btn_row4_settings->SetText("Settings");
        btn_row4_settings->TB_Func_12("interface\\ifcommon\\com_mid_button.ddj", 1, 1);
        btn_row4_settings->SetGWndSize(90, 23);
    }

    // Hide window by default
    MacroWindowMainFrame->ShowGWnd(false);

    return true;
}

void CIFMacroWindow::OnUpdate() {
    if (!MacroWindowMainFrame || !MacroWindowMainFrame->IsVisible()) return;

    // Sync icon states with checkbox states every frame
    UpdateIconStates();

    // Check for button clicks using mouse state
    static bool s_wasMouseDown = false;
    bool isMouseDown = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;

    // Detect mouse click (transition from pressed to released)
    if (s_wasMouseDown && !isMouseDown) {
        POINT pt;
        GetCursorPos(&pt);

        extern HWND g_hMainWnd;
        if (g_hMainWnd) {
            ScreenToClient(g_hMainWnd, &pt);

            int frameX = *reinterpret_cast<int *>(reinterpret_cast<char *>(MacroWindowMainFrame) + 0x3C);
            int frameY = *reinterpret_cast<int *>(reinterpret_cast<char *>(MacroWindowMainFrame) + 0x40);

            int localX = pt.x - frameX;
            int localY = pt.y - frameY;

            // Row 1 Toggle Icon: x=162, y=61, w=32, h=32
            if (localX >= 162 && localX < 194 && localY >= 61 && localY < 93) {
                On_AutoPotion_Toggle();
            }
            // Row 1 Settings: x=230, y=66, w=90, h=23
            else if (localX >= 230 && localX < 320 && localY >= 66 && localY < 89) {
                On_AutoPotion_Settings();
            }
            // Row 2 Toggle Icon: x=162, y=115, w=32, h=32
            else if (localX >= 162 && localX < 194 && localY >= 115 && localY < 147) {
                On_PetAutoPotion_Toggle();
            }
            // Row 2 Settings: x=230, y=120, w=90, h=23
            else if (localX >= 230 && localX < 320 && localY >= 120 && localY < 143) {
                On_PetAutoPotion_Settings();
            }
            // Row 3 Toggle Icon: x=162, y=169, w=32, h=32
            else if (localX >= 162 && localX < 194 && localY >= 169 && localY < 201) {
                On_AutoAttack_Toggle();
            }
            // Row 3 Settings: x=230, y=174, w=90, h=23
            else if (localX >= 230 && localX < 320 && localY >= 174 && localY < 197) {
                On_AutoAttack_Settings();
            }
            // Row 4 Toggle Icon: x=162, y=225, w=32, h=32
            else if (localX >= 162 && localX < 194 && localY >= 225 && localY < 257) {
                On_PetFilter_Toggle();
            }
            // Row 4 Settings: x=230, y=230, w=90, h=23
            else if (localX >= 230 && localX < 320 && localY >= 230 && localY < 253) {
                On_PetFilter_Settings();
            }
        }
    }

    s_wasMouseDown = isMouseDown;
}

int CIFMacroWindow::OnMouseMove(int a1, int x, int y) {
    return 0;
}

void CIFMacroWindow::ResetPosition() {
    if (!MacroWindowMainFrame) return;

    int screenWidth = g_D3DViewportWidth > 0 ? g_D3DViewportWidth : GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = g_D3DViewportHeight > 0 ? g_D3DViewportHeight : GetSystemMetrics(SM_CYSCREEN);

    MacroWindowMainFrame->MoveGWnd((screenWidth - 360) / 2, (screenHeight - 300) / 4);
}

void CIFMacroWindow::On_AutoPotion_Toggle() {
    // Toggle the HP checkbox in IFAutoPotion
    extern CIFAutoPotion *g_pCIFAutoPotion;
    if (g_pCIFAutoPotion && g_pCIFAutoPotion->m_hp) {
        bool currentState = g_pCIFAutoPotion->m_hp->GetCheckedState_MAYBE();
        g_pCIFAutoPotion->m_hp->FUN_00656d50(!currentState);
        g_pCIFAutoPotion->On_SaveAutoPotion();
    }

    UpdateIconStates();
}

void CIFMacroWindow::On_AutoPotion_Settings() {
    extern CIFMainFrame *AutoPotionMainFrame;
    extern CIFMainFrame *PetAutoPotionMainFrame;
    extern CIFMainFrame *AutoHuntSettingsMainFrame;
    extern CIFMainFrame *PetFilterSettingsMainFrame;
    
    // Close all other settings windows first
    if (PetAutoPotionMainFrame) PetAutoPotionMainFrame->ShowGWnd(false);
    if (AutoHuntSettingsMainFrame) AutoHuntSettingsMainFrame->ShowGWnd(false);
    if (PetFilterSettingsMainFrame) PetFilterSettingsMainFrame->ShowGWnd(false);
    
    if (AutoPotionMainFrame && MacroWindowMainFrame) {
        // Position windows side-by-side
        int screenWidth = g_D3DViewportWidth > 0 ? g_D3DViewportWidth : GetSystemMetrics(SM_CXSCREEN);
        int screenHeight = g_D3DViewportHeight > 0 ? g_D3DViewportHeight : GetSystemMetrics(SM_CYSCREEN);
        int macroWidth = 360;
        int settingsWidth = 400;
        int gap = 10;
        int totalWidth = macroWidth + gap + settingsWidth;
        int startX = (screenWidth - totalWidth) / 2;
        int posY = (screenHeight - 300) / 4;
        
        MacroWindowMainFrame->MoveGWnd(startX, posY);
        AutoPotionMainFrame->MoveGWnd(startX + macroWidth + gap, posY);
        AutoPotionMainFrame->ShowGWnd(true);
        AutoPotionMainFrame->BringToFront();
    }
}

void CIFMacroWindow::On_PetAutoPotion_Toggle() {
    // Toggle ALL checkboxes in IFPetAutoPotion (master on/off)
    extern CIFPetAutoPotion *g_pCIFPetAutoPotion;
    if (g_pCIFPetAutoPotion) {
        // Check if ANY checkbox is currently enabled
        bool anyEnabled = false;
        if (g_pCIFPetAutoPotion->m_chkAttackHP && g_pCIFPetAutoPotion->m_chkAttackHP->GetCheckedState_MAYBE()) anyEnabled = true;
        if (g_pCIFPetAutoPotion->m_chkAttackHGP && g_pCIFPetAutoPotion->m_chkAttackHGP->GetCheckedState_MAYBE()) anyEnabled = true;
        if (g_pCIFPetAutoPotion->m_chkAttackCure && g_pCIFPetAutoPotion->m_chkAttackCure->GetCheckedState_MAYBE()) anyEnabled = true;
        if (g_pCIFPetAutoPotion->m_chkTransportHP && g_pCIFPetAutoPotion->m_chkTransportHP->GetCheckedState_MAYBE()) anyEnabled = true;
        if (g_pCIFPetAutoPotion->m_chkTransportCure && g_pCIFPetAutoPotion->m_chkTransportCure->GetCheckedState_MAYBE()) anyEnabled = true;

        // Toggle: if any enabled -> disable all, if none enabled -> enable all
        bool newState = !anyEnabled;

        // Set all checkboxes to new state
        if (g_pCIFPetAutoPotion->m_chkAttackHP) g_pCIFPetAutoPotion->m_chkAttackHP->FUN_00656d50(newState);
        if (g_pCIFPetAutoPotion->m_chkAttackHGP) g_pCIFPetAutoPotion->m_chkAttackHGP->FUN_00656d50(newState);
        if (g_pCIFPetAutoPotion->m_chkAttackCure) g_pCIFPetAutoPotion->m_chkAttackCure->FUN_00656d50(newState);
        if (g_pCIFPetAutoPotion->m_chkTransportHP) g_pCIFPetAutoPotion->m_chkTransportHP->FUN_00656d50(newState);
        if (g_pCIFPetAutoPotion->m_chkTransportCure) g_pCIFPetAutoPotion->m_chkTransportCure->FUN_00656d50(newState);

        // Update internal states
        g_pCIFPetAutoPotion->m_attackPetHP = newState;
        g_pCIFPetAutoPotion->m_attackPetHGP = newState;
        g_pCIFPetAutoPotion->m_attackPetCure = newState;
        g_pCIFPetAutoPotion->m_transportPetHP = newState;
        g_pCIFPetAutoPotion->m_transportPetCure = newState;

        g_pCIFPetAutoPotion->SaveConfig();
    }

    UpdateIconStates();
}

void CIFMacroWindow::On_PetAutoPotion_Settings() {
    extern CIFMainFrame *AutoPotionMainFrame;
    extern CIFMainFrame *PetAutoPotionMainFrame;
    extern CIFMainFrame *AutoHuntSettingsMainFrame;
    extern CIFMainFrame *PetFilterSettingsMainFrame;
    
    // Close all other settings windows first
    if (AutoPotionMainFrame) AutoPotionMainFrame->ShowGWnd(false);
    if (AutoHuntSettingsMainFrame) AutoHuntSettingsMainFrame->ShowGWnd(false);
    if (PetFilterSettingsMainFrame) PetFilterSettingsMainFrame->ShowGWnd(false);
    
    if (PetAutoPotionMainFrame && MacroWindowMainFrame) {
        // Position windows side-by-side
        int screenWidth = g_D3DViewportWidth > 0 ? g_D3DViewportWidth : GetSystemMetrics(SM_CXSCREEN);
        int screenHeight = g_D3DViewportHeight > 0 ? g_D3DViewportHeight : GetSystemMetrics(SM_CYSCREEN);
        int macroWidth = 360;
        int settingsWidth = 400;
        int gap = 10;
        int totalWidth = macroWidth + gap + settingsWidth;
        int startX = (screenWidth - totalWidth) / 2;
        int posY = (screenHeight - 300) / 4;
        
        MacroWindowMainFrame->MoveGWnd(startX, posY);
        PetAutoPotionMainFrame->MoveGWnd(startX + macroWidth + gap, posY);
        PetAutoPotionMainFrame->ShowGWnd(true);
        PetAutoPotionMainFrame->BringToFront();
    }
}

void CIFMacroWindow::On_AutoAttack_Toggle() {
    m_autoAttackEnabled = !m_autoAttackEnabled;
    
    // Sync AutoBuffController with this toggle
    AutoBuffController::SetEnabled(m_autoAttackEnabled);
    
    // Sync AutoTargetController with this toggle
    AutoTargetController::SetEnabled(m_autoAttackEnabled);
    
    // Sync AutoAttackSkillController with this toggle
    AutoAttackSkillController::SetEnabled(m_autoAttackEnabled);
    
    // Sync AutoMoveController (patrol) with this toggle
    AutoMoveController::SetEnabled(m_autoAttackEnabled);
    
    // Sync ReturnToTownController (death/low item detection)
    ReturnToTownController::SetEnabled(m_autoAttackEnabled);
    
    UpdateIconStates();
}


void CIFMacroWindow::On_AutoAttack_Settings() {
    extern CIFMainFrame *AutoPotionMainFrame;
    extern CIFMainFrame *PetAutoPotionMainFrame;
    extern CIFMainFrame *AutoHuntSettingsMainFrame;
    extern CIFMainFrame *PetFilterSettingsMainFrame;
    extern CIFAutoHuntSettings *g_pCIFAutoHuntSettings;
    
    // Close all other settings windows first
    if (AutoPotionMainFrame) AutoPotionMainFrame->ShowGWnd(false);
    if (PetAutoPotionMainFrame) PetAutoPotionMainFrame->ShowGWnd(false);
    if (PetFilterSettingsMainFrame) PetFilterSettingsMainFrame->ShowGWnd(false);
    
    if (AutoHuntSettingsMainFrame && MacroWindowMainFrame) {
        // Refresh learned skills when window is opened
        if (g_pCIFAutoHuntSettings) {
            g_pCIFAutoHuntSettings->RefreshLearnedSkills();
        }
        
        // Position windows side-by-side
        int screenWidth = g_D3DViewportWidth > 0 ? g_D3DViewportWidth : GetSystemMetrics(SM_CXSCREEN);
        int screenHeight = g_D3DViewportHeight > 0 ? g_D3DViewportHeight : GetSystemMetrics(SM_CYSCREEN);
        int macroWidth = 360;
        int settingsWidth = 600; // AutoHuntSettings is wider
        int gap = 10;
        int totalWidth = macroWidth + gap + settingsWidth;
        int startX = (screenWidth - totalWidth) / 2;
        int posY = (screenHeight - 300) / 4;
        
        MacroWindowMainFrame->MoveGWnd(startX, posY);
        AutoHuntSettingsMainFrame->MoveGWnd(startX + macroWidth + gap, posY);
        AutoHuntSettingsMainFrame->ShowGWnd(true);
        AutoHuntSettingsMainFrame->BringToFront();
    }
}

void CIFMacroWindow::On_PetFilter_Toggle() {
    // Toggle the SwitchPetFilter in IFPetFilterSettings
    CIFPetFilterSettings::SwitchPetFilter = !CIFPetFilterSettings::SwitchPetFilter;
    m_petFilterEnabled = CIFPetFilterSettings::SwitchPetFilter;

    // Enable/Disable the PetFilter system
    extern void SetPetFilterEnabled(bool enabled);
    SetPetFilterEnabled(m_petFilterEnabled);

    // Update radio button visuals in settings window if it exists
    if (g_pCIFPetFilterSettings) {
        g_pCIFPetFilterSettings->UpdateRadioButtons(m_petFilterEnabled);
    }

    UpdateIconStates();
}

void CIFMacroWindow::On_PetFilter_Settings() {
    extern CIFMainFrame *AutoPotionMainFrame;
    extern CIFMainFrame *PetAutoPotionMainFrame;
    extern CIFMainFrame *AutoHuntSettingsMainFrame;
    extern CIFMainFrame *PetFilterSettingsMainFrame;
    
    // Close all other settings windows first
    if (AutoPotionMainFrame) AutoPotionMainFrame->ShowGWnd(false);
    if (PetAutoPotionMainFrame) PetAutoPotionMainFrame->ShowGWnd(false);
    if (AutoHuntSettingsMainFrame) AutoHuntSettingsMainFrame->ShowGWnd(false);
    
    if (PetFilterSettingsMainFrame && MacroWindowMainFrame) {
        // Position windows side-by-side
        int screenWidth = g_D3DViewportWidth > 0 ? g_D3DViewportWidth : GetSystemMetrics(SM_CXSCREEN);
        int screenHeight = g_D3DViewportHeight > 0 ? g_D3DViewportHeight : GetSystemMetrics(SM_CYSCREEN);
        int macroWidth = 360;
        int settingsWidth = 400;
        int gap = 10;
        int totalWidth = macroWidth + gap + settingsWidth;
        int startX = (screenWidth - totalWidth) / 2;
        int posY = (screenHeight - 300) / 4;
        
        MacroWindowMainFrame->MoveGWnd(startX, posY);
        PetFilterSettingsMainFrame->MoveGWnd(startX + macroWidth + gap, posY);
        PetFilterSettingsMainFrame->ShowGWnd(true);
        PetFilterSettingsMainFrame->BringToFront();
    }
}

void CIFMacroWindow::UpdateIconStates() {
    // Read Auto Potion (character) HP checkbox state
    extern CIFAutoPotion *g_pCIFAutoPotion;
    bool autoPotionHPEnabled = false;
    if (g_pCIFAutoPotion && g_pCIFAutoPotion->m_hp) {
        autoPotionHPEnabled = g_pCIFAutoPotion->m_hp->GetCheckedState_MAYBE();
    }

    // Read Pet Auto Potion - check if ANY checkbox is enabled
    extern CIFPetAutoPotion *g_pCIFPetAutoPotion;
    bool petAutoPotionAnyEnabled = false;
    if (g_pCIFPetAutoPotion) {
        if (g_pCIFPetAutoPotion->m_chkAttackHP && g_pCIFPetAutoPotion->m_chkAttackHP->GetCheckedState_MAYBE()) petAutoPotionAnyEnabled = true;
        if (g_pCIFPetAutoPotion->m_chkAttackHGP && g_pCIFPetAutoPotion->m_chkAttackHGP->GetCheckedState_MAYBE()) petAutoPotionAnyEnabled = true;
        if (g_pCIFPetAutoPotion->m_chkAttackCure && g_pCIFPetAutoPotion->m_chkAttackCure->GetCheckedState_MAYBE()) petAutoPotionAnyEnabled = true;
        if (g_pCIFPetAutoPotion->m_chkTransportHP && g_pCIFPetAutoPotion->m_chkTransportHP->GetCheckedState_MAYBE()) petAutoPotionAnyEnabled = true;
        if (g_pCIFPetAutoPotion->m_chkTransportCure && g_pCIFPetAutoPotion->m_chkTransportCure->GetCheckedState_MAYBE()) petAutoPotionAnyEnabled = true;
    }

    // Update Auto Potion icon based on HP checkbox state
    if (m_btnAutoPotionIcon) {
        if (autoPotionHPEnabled)
            m_btnAutoPotionIcon->TB_Func_12("interface\\macro\\macro_icon_potion_on.ddj", 1, 1);
        else
            m_btnAutoPotionIcon->TB_Func_12("interface\\macro\\macro_icon_potion_off.ddj", 1, 1);
    }

    // Update Pet Auto Potion icon based on ANY checkbox state
    if (m_btnPetAutoPotionIcon) {
        if (petAutoPotionAnyEnabled)
            m_btnPetAutoPotionIcon->TB_Func_12("interface\\macro\\macro_icon_potion_on.ddj", 1, 1);
        else
            m_btnPetAutoPotionIcon->TB_Func_12("interface\\macro\\macro_icon_potion_off.ddj", 1, 1);
    }

    // Auto Attack icon (still uses internal flag)
    if (m_btnAutoAttackIcon) {
        if (m_autoAttackEnabled)
            m_btnAutoAttackIcon->TB_Func_12("interface\\macro\\macro_icon_hunting_on.ddj", 1, 1);
        else
            m_btnAutoAttackIcon->TB_Func_12("interface\\macro\\macro_icon_hunting_off.ddj", 1, 1);
    }

    // Pet Filter icon - sync with SwitchPetFilter from IFPetFilterSettings
    if (m_btnPetFilterIcon) {
        // Read from IFPetFilterSettings::SwitchPetFilter
        bool petFilterEnabled = CIFPetFilterSettings::SwitchPetFilter;
        m_petFilterEnabled = petFilterEnabled;// Keep in sync

        if (petFilterEnabled)
            m_btnPetFilterIcon->TB_Func_12("interface\\macro\\macro_icon_hunting_on.ddj", 1, 1);
        else
            m_btnPetFilterIcon->TB_Func_12("interface\\macro\\macro_icon_hunting_off.ddj", 1, 1);
    }
}

// Static function to handle button click - called from global click handlers
void CIFMacroWindow::HandleButtonClick(int buttonId) {
    if (!g_pMacroWindow) return;

    switch (buttonId) {
        case GDR_MW_ROW1_ICON:
            g_pMacroWindow->On_AutoPotion_Toggle();
            break;
        case GDR_MW_ROW1_SETTINGS:
            g_pMacroWindow->On_AutoPotion_Settings();
            break;
        case GDR_MW_ROW2_ICON:
            g_pMacroWindow->On_PetAutoPotion_Toggle();
            break;
        case GDR_MW_ROW2_SETTINGS:
            g_pMacroWindow->On_PetAutoPotion_Settings();
            break;
        case GDR_MW_ROW3_ICON:
            g_pMacroWindow->On_AutoAttack_Toggle();
            break;
        case GDR_MW_ROW3_SETTINGS:
            g_pMacroWindow->On_AutoAttack_Settings();
            break;
        case GDR_MW_ROW4_ICON:
            g_pMacroWindow->On_PetFilter_Toggle();
            break;
        case GDR_MW_ROW4_SETTINGS:
            g_pMacroWindow->On_PetFilter_Settings();
            break;
    }
}

// Helper functions
void ShowMacroWindow(bool show) {
    if (MacroWindowMainFrame) {
        MacroWindowMainFrame->ShowGWnd(show);
        if (show) {
            MacroWindowMainFrame->BringToFront();
        }
    }
}

bool IsMacroWindowVisible() {
    return MacroWindowMainFrame ? MacroWindowMainFrame->IsVisible() : false;
}