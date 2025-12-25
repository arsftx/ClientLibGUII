#include "IFPetAutoPotion.h"
#include "IFNormalTile.h"
#include "GInterface.h"
#include "Game.h"
#include "ICPlayer.h"
#include <BSLib/multibyte.h>
#include <cstdio>
#include <ctime>
#include <sys/stat.h>
#include <direct.h>

extern int g_D3DViewportWidth;
extern int g_D3DViewportHeight;

// Helper function to get executable directory
static std::string GetPetAutoPotionExeDir() {
    char buffer[MAX_PATH];
    GetModuleFileNameA(NULL, buffer, MAX_PATH);
    std::string exePath(buffer);
    size_t lastSlash = exePath.find_last_of("\\/");
    return exePath.substr(0, lastSlash + 1);
}

// Helper function to check if file exists
static bool DoesPetAutoPotionFileExist(const std::string& name) {
    struct stat buffer;
    return (stat(name.c_str(), &buffer) == 0);
}

// Logging helper for PetAutoPotion - disabled
static void LogPetAutoPotion(const char* format, ...) {
    // Logging disabled
}

bool g_PetAutoPotionEnabled = false;
CIFMainFrame* PetAutoPotionMainFrame = NULL;
CIFPetAutoPotion* g_pCIFPetAutoPotion = NULL;

#define GDR_PAP_FRAME 1
#define GDR_PAP_BG_TILE 2
#define GDR_PAP_ATTACK_SECTION 10
#define GDR_PAP_ATTACK_HP_CHK 11
#define GDR_PAP_ATTACK_HGP_CHK 12
#define GDR_PAP_ATTACK_CURE_CHK 13
#define GDR_PAP_TRANSPORT_SECTION 20
#define GDR_PAP_TRANSPORT_HP_CHK 21
#define GDR_PAP_TRANSPORT_CURE_CHK 22
#define GDR_PAP_BTN_CONFIRM 30
#define GDR_PAP_BTN_CANCEL 31

GFX_IMPLEMENT_DYNCREATE(CIFPetAutoPotion, CIFMainFrame)

GFX_BEGIN_MESSAGE_MAP(CIFPetAutoPotion, CIFMainFrame)
GFX_END_MESSAGE_MAP()

CIFPetAutoPotion::CIFPetAutoPotion()
    : m_attackPetHP(false)
    , m_attackPetHGP(false)
    , m_attackPetCure(false)
    , m_attackHPThreshold(50)
    , m_attackHGPThreshold(50)
    , m_transportPetHP(false)
    , m_transportPetCure(false)
    , m_transportHPThreshold(50)
    , m_lblAttackPet(NULL)
    , m_chkAttackHP(NULL)
    , m_chkAttackHGP(NULL)
    , m_chkAttackCure(NULL)
    , m_lblAttackHP(NULL)
    , m_lblAttackHGP(NULL)
    , m_lblAttackCure(NULL)
    , m_sliderAttackHP(NULL)
    , m_sliderAttackHGP(NULL)
    , m_lblAttackHPValue(NULL)
    , m_lblAttackHGPValue(NULL)
    , m_lblTransportPet(NULL)
    , m_chkTransportHP(NULL)
    , m_chkTransportCure(NULL)
    , m_lblTransportHP(NULL)
    , m_lblTransportCure(NULL)
    , m_sliderTransportHP(NULL)
    , m_lblTransportHPValue(NULL)
    , m_btnConfirm(NULL)
    , m_btnCancel(NULL)
    , m_sliderBgAttackHP(NULL)
    , m_sliderBgAttackHGP(NULL)
    , m_sliderBgTransportHP(NULL)
    , m_attackHPSliderDisabled(false)
    , m_attackHGPSliderDisabled(false)
    , m_transportHPSliderDisabled(false)
    , m_lastAttackHPSlider(-1)
    , m_lastAttackHGPSlider(-1)
    , m_lastTransportHPSlider(-1)
{
    LogPetAutoPotion("Constructor called - initializing Pet Auto Potion system");
    g_pCIFPetAutoPotion = this;  // Store global pointer for ICCos access
}

CIFPetAutoPotion::~CIFPetAutoPotion()
{
}

bool CIFPetAutoPotion::OnCreate(long ln)
{
    // Window: 394x490 to fit 3 sections with frames + buttons
    RECT rect = {50, 50, 394, 490};
    PetAutoPotionMainFrame = (CIFMainFrame*)CGWnd::CreateInstance(g_pCGInterface, GFX_RUNTIME_CLASS(CIFMainFrame), rect, 2003, 0);
    PetAutoPotionMainFrame->SetText("Pet Auto Potion");
    PetAutoPotionMainFrame->TB_Func_12("interface\\frame\\mframe_wnd_", 1, 0);
    PetAutoPotionMainFrame->SetGWndSize(394, 490);

    wnd_rect sz;

    // Main Frame
    sz.pos.x = 15; sz.pos.y = 43; sz.size.width = 364; sz.size.height = 435;
    CIFFrame* pFrame = (CIFFrame*)CGWnd::CreateInstance(PetAutoPotionMainFrame, GFX_RUNTIME_CLASS(CIFFrame), sz, GDR_PAP_FRAME, 0);
    if (pFrame) { pFrame->TB_Func_12("interface\\inventory\\int_window_", 1, 0); pFrame->SetGWndSize(364, 435); }

    // Background tile
    sz.pos.x = 30; sz.pos.y = 54; sz.size.width = 335; sz.size.height = 414;
    CIFNormalTile* pTile = (CIFNormalTile*)CGWnd::CreateInstance(PetAutoPotionMainFrame, GFX_RUNTIME_CLASS(CIFNormalTile), sz, GDR_PAP_BG_TILE, 0);
    if (pTile) { pTile->TB_Func_12("interface\\ifcommon\\bg_tile\\com_bg_tile_b.ddj", 1, 0); pTile->SetGWndSize(335, 414); }

    // ==================== SECTION 1: Attack Pet (y=57) ====================
    sz.pos.x = 22; sz.pos.y = 57; sz.size.width = 124; sz.size.height = 28;
    CIFStatic* tabSta1 = (CIFStatic*)CGWnd::CreateInstance(PetAutoPotionMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 10, 0);
    if (tabSta1) { tabSta1->TB_Func_12("interface\\option\\opt_video_tab.ddj", 1, 0); tabSta1->SetGWndSize(124, 28); }
    
    sz.pos.x = 32; sz.pos.y = 66; sz.size.width = 102; sz.size.height = 11;
    m_lblAttackPet = (CIFStatic*)CGWnd::CreateInstance(PetAutoPotionMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, GDR_PAP_ATTACK_SECTION, 0);
    if (m_lblAttackPet) { m_lblAttackPet->SetText("Attack Pet"); m_lblAttackPet->SetTextColor(D3DCOLOR_XRGB(239, 218, 164)); }
    
    sz.pos.x = 22; sz.pos.y = 84; sz.size.width = 350; sz.size.height = 118;
    CIFFrame* sec1Frame = (CIFFrame*)CGWnd::CreateInstance(PetAutoPotionMainFrame, GFX_RUNTIME_CLASS(CIFFrame), sz, 11, 0);
    if (sec1Frame) { sec1Frame->TB_Func_12("interface\\inventory\\int_window_", 1, 0); sec1Frame->SetGWndSize(350, 118); }
    
    // --- HP Row ---
    sz.pos.x = 32; sz.pos.y = 95; sz.size.width = 16; sz.size.height = 16;
    m_chkAttackHP = (CIFCheckBox*)CGWnd::CreateInstance(PetAutoPotionMainFrame, GFX_RUNTIME_CLASS(CIFCheckBox), sz, GDR_PAP_ATTACK_HP_CHK, 0);
    
    sz.pos.x = 55; sz.pos.y = 93; sz.size.width = 24; sz.size.height = 24;
    m_lblAttackHP = (CIFStatic*)CGWnd::CreateInstance(PetAutoPotionMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 101, 0);
    if (m_lblAttackHP) m_lblAttackHP->SetText("HP");
    
    sz.pos.x = 90; sz.pos.y = 95; sz.size.width = 28; sz.size.height = 20;
    m_lblAttackHPValue = (CIFStatic*)CGWnd::CreateInstance(PetAutoPotionMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 102, 0);
    if (m_lblAttackHPValue) { m_lblAttackHPValue->SetText("50"); m_lblAttackHPValue->TB_Func_12("interface\\guild\\gil_bar04.ddj", 1, 0); m_lblAttackHPValue->SetGWndSize(28, 20); }
    
    sz.pos.x = 121; sz.pos.y = 93; sz.size.width = 16; sz.size.height = 24;
    CIFStatic* pct1 = (CIFStatic*)CGWnd::CreateInstance(PetAutoPotionMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 103, 0);
    if (pct1) pct1->SetText("%");

    sz.pos.x = 33; sz.pos.y = 117; sz.size.width = 328; sz.size.height = 24;
    m_sliderBgAttackHP = (CIFStatic*)CGWnd::CreateInstance(PetAutoPotionMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 104, 0);
    if (m_sliderBgAttackHP) { m_sliderBgAttackHP->TB_Func_12("interface\\recovery\\re_selectbar.ddj", 1, 0); m_sliderBgAttackHP->SetGWndSize(328, 22); }

    sz.pos.x = 55; sz.pos.y = 120; sz.size.width = 267; sz.size.height = 16;
    m_sliderAttackHP = (CIFHScroll_Option*)CGWnd::CreateInstance(PetAutoPotionMainFrame, GFX_RUNTIME_CLASS(CIFHScroll_Option), sz, 105, 0);
    if (m_sliderAttackHP) {
        m_sliderAttackHP->Set2E0(100); m_sliderAttackHP->Set2E4(100); m_sliderAttackHP->Set2E8(1); m_sliderAttackHP->Set2F0(1);
        m_sliderAttackHP->Get2F4()->SetGWndSize(24, 20); m_sliderAttackHP->Get2F8()->SetGWndSize(24, 20);
        m_sliderAttackHP->SetHScrollBar(267, 0, 100, 1); m_sliderAttackHP->SetGWndSize(267, 20);
    }

    // --- HGP Row ---
    sz.pos.x = 32; sz.pos.y = 145; sz.size.width = 16; sz.size.height = 16;
    m_chkAttackHGP = (CIFCheckBox*)CGWnd::CreateInstance(PetAutoPotionMainFrame, GFX_RUNTIME_CLASS(CIFCheckBox), sz, GDR_PAP_ATTACK_HGP_CHK, 0);
    
    sz.pos.x = 55; sz.pos.y = 143; sz.size.width = 30; sz.size.height = 24;
    m_lblAttackHGP = (CIFStatic*)CGWnd::CreateInstance(PetAutoPotionMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 201, 0);
    if (m_lblAttackHGP) m_lblAttackHGP->SetText("HGP");
    
    sz.pos.x = 90; sz.pos.y = 145; sz.size.width = 28; sz.size.height = 20;
    m_lblAttackHGPValue = (CIFStatic*)CGWnd::CreateInstance(PetAutoPotionMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 202, 0);
    if (m_lblAttackHGPValue) { m_lblAttackHGPValue->SetText("50"); m_lblAttackHGPValue->TB_Func_12("interface\\guild\\gil_bar04.ddj", 1, 0); m_lblAttackHGPValue->SetGWndSize(28, 20); }
    
    sz.pos.x = 121; sz.pos.y = 143; sz.size.width = 16; sz.size.height = 24;
    CIFStatic* pct2 = (CIFStatic*)CGWnd::CreateInstance(PetAutoPotionMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 203, 0);
    if (pct2) pct2->SetText("%");

    sz.pos.x = 33; sz.pos.y = 167; sz.size.width = 328; sz.size.height = 24;
    m_sliderBgAttackHGP = (CIFStatic*)CGWnd::CreateInstance(PetAutoPotionMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 204, 0);
    if (m_sliderBgAttackHGP) { m_sliderBgAttackHGP->TB_Func_12("interface\\recovery\\re_selectbar.ddj", 1, 0); m_sliderBgAttackHGP->SetGWndSize(328, 22); }

    sz.pos.x = 55; sz.pos.y = 170; sz.size.width = 267; sz.size.height = 16;
    m_sliderAttackHGP = (CIFHScroll_Option*)CGWnd::CreateInstance(PetAutoPotionMainFrame, GFX_RUNTIME_CLASS(CIFHScroll_Option), sz, 205, 0);
    if (m_sliderAttackHGP) {
        m_sliderAttackHGP->Set2E0(100); m_sliderAttackHGP->Set2E4(100); m_sliderAttackHGP->Set2E8(1); m_sliderAttackHGP->Set2F0(1);
        m_sliderAttackHGP->Get2F4()->SetGWndSize(24, 20); m_sliderAttackHGP->Get2F8()->SetGWndSize(24, 20);
        m_sliderAttackHGP->SetHScrollBar(267, 0, 100, 1); m_sliderAttackHGP->SetGWndSize(267, 20);
    }

    // ==================== SECTION 2: Transport Pet (y=208) ====================
    sz.pos.x = 22; sz.pos.y = 208; sz.size.width = 124; sz.size.height = 28;
    CIFStatic* tabSta2 = (CIFStatic*)CGWnd::CreateInstance(PetAutoPotionMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 12, 0);
    if (tabSta2) { tabSta2->TB_Func_12("interface\\option\\opt_video_tab.ddj", 1, 0); tabSta2->SetGWndSize(124, 28); }
    
    sz.pos.x = 32; sz.pos.y = 217; sz.size.width = 102; sz.size.height = 11;
    m_lblTransportPet = (CIFStatic*)CGWnd::CreateInstance(PetAutoPotionMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, GDR_PAP_TRANSPORT_SECTION, 0);
    if (m_lblTransportPet) { m_lblTransportPet->SetText("Transport Pet"); m_lblTransportPet->SetTextColor(D3DCOLOR_XRGB(239, 218, 164)); }
    
    sz.pos.x = 22; sz.pos.y = 235; sz.size.width = 350; sz.size.height = 68;
    CIFFrame* sec2Frame = (CIFFrame*)CGWnd::CreateInstance(PetAutoPotionMainFrame, GFX_RUNTIME_CLASS(CIFFrame), sz, 13, 0);
    if (sec2Frame) { sec2Frame->TB_Func_12("interface\\inventory\\int_window_", 1, 0); sec2Frame->SetGWndSize(350, 68); }
    
    // --- HP Row ---
    sz.pos.x = 32; sz.pos.y = 248; sz.size.width = 16; sz.size.height = 16;
    m_chkTransportHP = (CIFCheckBox*)CGWnd::CreateInstance(PetAutoPotionMainFrame, GFX_RUNTIME_CLASS(CIFCheckBox), sz, GDR_PAP_TRANSPORT_HP_CHK, 0);
    
    sz.pos.x = 55; sz.pos.y = 246; sz.size.width = 24; sz.size.height = 24;
    m_lblTransportHP = (CIFStatic*)CGWnd::CreateInstance(PetAutoPotionMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 401, 0);
    if (m_lblTransportHP) m_lblTransportHP->SetText("HP");
    
    sz.pos.x = 90; sz.pos.y = 248; sz.size.width = 28; sz.size.height = 20;
    m_lblTransportHPValue = (CIFStatic*)CGWnd::CreateInstance(PetAutoPotionMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 402, 0);
    if (m_lblTransportHPValue) { m_lblTransportHPValue->SetText("50"); m_lblTransportHPValue->TB_Func_12("interface\\guild\\gil_bar04.ddj", 1, 0); m_lblTransportHPValue->SetGWndSize(28, 20); }
    
    sz.pos.x = 121; sz.pos.y = 246; sz.size.width = 16; sz.size.height = 24;
    CIFStatic* pct3 = (CIFStatic*)CGWnd::CreateInstance(PetAutoPotionMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 403, 0);
    if (pct3) pct3->SetText("%");

    sz.pos.x = 33; sz.pos.y = 270; sz.size.width = 328; sz.size.height = 24;
    m_sliderBgTransportHP = (CIFStatic*)CGWnd::CreateInstance(PetAutoPotionMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 404, 0);
    if (m_sliderBgTransportHP) { m_sliderBgTransportHP->TB_Func_12("interface\\recovery\\re_selectbar.ddj", 1, 0); m_sliderBgTransportHP->SetGWndSize(328, 22); }

    sz.pos.x = 55; sz.pos.y = 273; sz.size.width = 267; sz.size.height = 16;
    m_sliderTransportHP = (CIFHScroll_Option*)CGWnd::CreateInstance(PetAutoPotionMainFrame, GFX_RUNTIME_CLASS(CIFHScroll_Option), sz, 405, 0);
    if (m_sliderTransportHP) {
        m_sliderTransportHP->Set2E0(100); m_sliderTransportHP->Set2E4(100); m_sliderTransportHP->Set2E8(1); m_sliderTransportHP->Set2F0(1);
        m_sliderTransportHP->Get2F4()->SetGWndSize(24, 20); m_sliderTransportHP->Get2F8()->SetGWndSize(24, 20);
        m_sliderTransportHP->SetHScrollBar(267, 0, 100, 1); m_sliderTransportHP->SetGWndSize(267, 20);
    }

    // ==================== SECTION 3: Cure (y=310) ====================
    sz.pos.x = 22; sz.pos.y = 310; sz.size.width = 124; sz.size.height = 28;
    CIFStatic* tabSta3 = (CIFStatic*)CGWnd::CreateInstance(PetAutoPotionMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 14, 0);
    if (tabSta3) { tabSta3->TB_Func_12("interface\\option\\opt_video_tab.ddj", 1, 0); tabSta3->SetGWndSize(124, 28); }
    
    sz.pos.x = 32; sz.pos.y = 319; sz.size.width = 102; sz.size.height = 11;
    CIFStatic* lblCure = (CIFStatic*)CGWnd::CreateInstance(PetAutoPotionMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 15, 0);
    if (lblCure) { lblCure->SetText("Cure"); lblCure->SetTextColor(D3DCOLOR_XRGB(239, 218, 164)); }
    
    sz.pos.x = 22; sz.pos.y = 337; sz.size.width = 350; sz.size.height = 62;
    CIFFrame* sec3Frame = (CIFFrame*)CGWnd::CreateInstance(PetAutoPotionMainFrame, GFX_RUNTIME_CLASS(CIFFrame), sz, 16, 0);
    if (sec3Frame) { sec3Frame->TB_Func_12("interface\\inventory\\int_window_", 1, 0); sec3Frame->SetGWndSize(350, 62); }
    
    // Abnormal Status Attack Pet
    sz.pos.x = 32; sz.pos.y = 352; sz.size.width = 16; sz.size.height = 16;
    m_chkAttackCure = (CIFCheckBox*)CGWnd::CreateInstance(PetAutoPotionMainFrame, GFX_RUNTIME_CLASS(CIFCheckBox), sz, GDR_PAP_ATTACK_CURE_CHK, 0);
    
    sz.pos.x = 55; sz.pos.y = 352; sz.size.width = 180; sz.size.height = 20;
    m_lblAttackCure = (CIFStatic*)CGWnd::CreateInstance(PetAutoPotionMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 301, 0);
    if (m_lblAttackCure) m_lblAttackCure->SetText("Abnormal Status Attack Pet");

    // Abnormal Status Transport
    sz.pos.x = 32; sz.pos.y = 374; sz.size.width = 16; sz.size.height = 16;
    m_chkTransportCure = (CIFCheckBox*)CGWnd::CreateInstance(PetAutoPotionMainFrame, GFX_RUNTIME_CLASS(CIFCheckBox), sz, GDR_PAP_TRANSPORT_CURE_CHK, 0);
    
    sz.pos.x = 55; sz.pos.y = 374; sz.size.width = 180; sz.size.height = 20;
    m_lblTransportCure = (CIFStatic*)CGWnd::CreateInstance(PetAutoPotionMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 501, 0);
    if (m_lblTransportCure) m_lblTransportCure->SetText("Abnormal Status Transport");

    // ==================== Buttons: Confirm & Cancel (y=420) ====================
    sz.pos.x = 115; sz.pos.y = 420; sz.size.width = 76; sz.size.height = 24;
    m_btnConfirm = (CIFButton*)CGWnd::CreateInstance(PetAutoPotionMainFrame, GFX_RUNTIME_CLASS(CIFButton), sz, 40, 0);
    if (m_btnConfirm) {
        m_btnConfirm->TB_Func_12("interface\\ifcommon\\com_button.ddj", 1, 1);
        m_btnConfirm->SetText("Confirm");
        m_btnConfirm->SetGWndSize(76, 24);
    }
    
    sz.pos.x = 204; sz.pos.y = 420; sz.size.width = 76; sz.size.height = 24;
    m_btnCancel = (CIFButton*)CGWnd::CreateInstance(PetAutoPotionMainFrame, GFX_RUNTIME_CLASS(CIFButton), sz, 41, 0);
    if (m_btnCancel) {
        m_btnCancel->TB_Func_12("interface\\ifcommon\\com_button.ddj", 1, 1);
        m_btnCancel->SetText("Cancel");
        m_btnCancel->SetGWndSize(76, 24);
    }

    // Set defaults
    if (m_chkAttackHP) m_chkAttackHP->FUN_00656d50(m_attackPetHP);
    if (m_chkAttackHGP) m_chkAttackHGP->FUN_00656d50(m_attackPetHGP);
    if (m_chkAttackCure) m_chkAttackCure->FUN_00656d50(m_attackPetCure);
    if (m_chkTransportHP) m_chkTransportHP->FUN_00656d50(m_transportPetHP);
    if (m_chkTransportCure) m_chkTransportCure->FUN_00656d50(m_transportPetCure);

    if (m_sliderAttackHP) m_sliderAttackHP->Set2EC(m_attackHPThreshold);
    if (m_sliderAttackHGP) m_sliderAttackHGP->Set2EC(m_attackHGPThreshold);
    if (m_sliderTransportHP) m_sliderTransportHP->Set2EC(m_transportHPThreshold);

    PetAutoPotionMainFrame->ShowGWnd(false);
    
    LogPetAutoPotion("OnCreate completed - loading configuration from .dat file");
    LoadConfig();
    
    // Initialize checkbox-slider synchronization
    On_CheckBoxScrollBar();
    
    LogPetAutoPotion("UI initialization complete - Attack HP: %s (%d%%), Attack HGP: %s (%d%%), Transport HP: %s (%d%%)",
        m_attackPetHP ? "ON" : "OFF", m_attackHPThreshold,
        m_attackPetHGP ? "ON" : "OFF", m_attackHGPThreshold,
        m_transportPetHP ? "ON" : "OFF", m_transportHPThreshold);
    
    return true;
}

void CIFPetAutoPotion::OnUpdate()
{
    CIFMainFrame::OnUpdate();
    
    // Update checkbox-slider synchronization (enable/disable sliders based on checkboxes)
    On_CheckBoxScrollBar();
    
    bool needsSave = false;
    
    // Attack HP slider - sync with text and detect changes
    if (m_sliderAttackHP && m_lblAttackHPValue) {
        int currentVal = m_sliderAttackHP->Get2EC();
        char buf[16]; sprintf(buf, "%d", currentVal);
        m_lblAttackHPValue->SetText(buf);
        
        // Check for slider value change
        if (m_lastAttackHPSlider != -1 && m_lastAttackHPSlider != currentVal) {
            LogPetAutoPotion("Attack HP slider changed: %d -> %d", m_lastAttackHPSlider, currentVal);
            m_attackHPThreshold = currentVal;
            needsSave = true;
        }
        m_lastAttackHPSlider = currentVal;
    }
    
    // Attack HGP slider - sync with text and detect changes
    if (m_sliderAttackHGP && m_lblAttackHGPValue) {
        int currentVal = m_sliderAttackHGP->Get2EC();
        char buf[16]; sprintf(buf, "%d", currentVal);
        m_lblAttackHGPValue->SetText(buf);
        
        // Check for slider value change
        if (m_lastAttackHGPSlider != -1 && m_lastAttackHGPSlider != currentVal) {
            LogPetAutoPotion("Attack HGP slider changed: %d -> %d", m_lastAttackHGPSlider, currentVal);
            m_attackHGPThreshold = currentVal;
            needsSave = true;
        }
        m_lastAttackHGPSlider = currentVal;
    }
    
    // Transport HP slider - sync with text and detect changes
    if (m_sliderTransportHP && m_lblTransportHPValue) {
        int currentVal = m_sliderTransportHP->Get2EC();
        char buf[16]; sprintf(buf, "%d", currentVal);
        m_lblTransportHPValue->SetText(buf);
        
        // Check for slider value change
        if (m_lastTransportHPSlider != -1 && m_lastTransportHPSlider != currentVal) {
            LogPetAutoPotion("Transport HP slider changed: %d -> %d", m_lastTransportHPSlider, currentVal);
            m_transportHPThreshold = currentVal;
            needsSave = true;
        }
        m_lastTransportHPSlider = currentVal;
    }
    
    // Check for checkbox state changes
    static bool lastAttackHP = true, lastAttackHGP = true, lastAttackCure = true;
    static bool lastTransportHP = true, lastTransportCure = true;
    static bool firstCheckboxCheck = true;
    
    if (m_chkAttackHP) {
        bool current = m_chkAttackHP->GetCheckedState_MAYBE();
        if (!firstCheckboxCheck && current != lastAttackHP) {
            LogPetAutoPotion("Attack HP checkbox changed: %s -> %s", lastAttackHP ? "ON" : "OFF", current ? "ON" : "OFF");
            m_attackPetHP = current;
            needsSave = true;
        }
        lastAttackHP = current;
    }
    if (m_chkAttackHGP) {
        bool current = m_chkAttackHGP->GetCheckedState_MAYBE();
        if (!firstCheckboxCheck && current != lastAttackHGP) {
            LogPetAutoPotion("Attack HGP checkbox changed: %s -> %s", lastAttackHGP ? "ON" : "OFF", current ? "ON" : "OFF");
            m_attackPetHGP = current;
            needsSave = true;
        }
        lastAttackHGP = current;
    }
    if (m_chkAttackCure) {
        bool current = m_chkAttackCure->GetCheckedState_MAYBE();
        if (!firstCheckboxCheck && current != lastAttackCure) {
            LogPetAutoPotion("Attack Cure checkbox changed: %s -> %s", lastAttackCure ? "ON" : "OFF", current ? "ON" : "OFF");
            m_attackPetCure = current;
            needsSave = true;
        }
        lastAttackCure = current;
    }
    if (m_chkTransportHP) {
        bool current = m_chkTransportHP->GetCheckedState_MAYBE();
        if (!firstCheckboxCheck && current != lastTransportHP) {
            LogPetAutoPotion("Transport HP checkbox changed: %s -> %s", lastTransportHP ? "ON" : "OFF", current ? "ON" : "OFF");
            m_transportPetHP = current;
            needsSave = true;
        }
        lastTransportHP = current;
    }
    if (m_chkTransportCure) {
        bool current = m_chkTransportCure->GetCheckedState_MAYBE();
        if (!firstCheckboxCheck && current != lastTransportCure) {
            LogPetAutoPotion("Transport Cure checkbox changed: %s -> %s", lastTransportCure ? "ON" : "OFF", current ? "ON" : "OFF");
            m_transportPetCure = current;
            needsSave = true;
        }
        lastTransportCure = current;
    }
    firstCheckboxCheck = false;
    
    // Auto-save if any setting changed
    if (needsSave) {
        LogPetAutoPotion("Auto-saving configuration due to setting change");
        SaveConfig();
    }
    
    // Button click detection
    static bool wasMouseDown = false;
    bool isMouseDown = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
    
    if (wasMouseDown && !isMouseDown && PetAutoPotionMainFrame && PetAutoPotionMainFrame->IsVisible()) {
        POINT pt; GetCursorPos(&pt);
        extern HWND g_hMainWnd;
        if (g_hMainWnd) {
            ScreenToClient(g_hMainWnd, &pt);
            int fx = *reinterpret_cast<int*>(reinterpret_cast<char*>(PetAutoPotionMainFrame) + 0x3C);
            int fy = *reinterpret_cast<int*>(reinterpret_cast<char*>(PetAutoPotionMainFrame) + 0x40);
            int lx = pt.x - fx, ly = pt.y - fy;
            
            // Confirm button (y=420)
            if (lx >= 115 && lx < 191 && ly >= 420 && ly < 444) {
                LogPetAutoPotion("Confirm button clicked - saving and closing");
                SaveConfig();
                PetAutoPotionMainFrame->ShowGWnd(false);
            }
            // Cancel button (y=420)
            if (lx >= 204 && lx < 280 && ly >= 420 && ly < 444) {
                LogPetAutoPotion("Cancel button clicked - closing without save");
                PetAutoPotionMainFrame->ShowGWnd(false);
            }
        }
    }
    wasMouseDown = isMouseDown;
}

int CIFPetAutoPotion::OnMouseMove(int a1, int x, int y)
{
    return 0;
}

void CIFPetAutoPotion::ResetPosition()
{
    if (!PetAutoPotionMainFrame) return;
    int screenWidth = g_D3DViewportWidth > 0 ? g_D3DViewportWidth : GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = g_D3DViewportHeight > 0 ? g_D3DViewportHeight : GetSystemMetrics(SM_CYSCREEN);
    PetAutoPotionMainFrame->MoveGWnd((screenWidth - 394) / 2, (screenHeight - 490) / 4);
}

void CIFPetAutoPotion::On_BtnClickConfirm()
{
    LogPetAutoPotion("Confirm button clicked - saving settings");
    
    m_attackPetHP = m_chkAttackHP->GetCheckedState_MAYBE();
    m_attackPetHGP = m_chkAttackHGP->GetCheckedState_MAYBE();
    m_attackPetCure = m_chkAttackCure->GetCheckedState_MAYBE();
    m_transportPetHP = m_chkTransportHP->GetCheckedState_MAYBE();
    m_transportPetCure = m_chkTransportCure->GetCheckedState_MAYBE();
    
    m_attackHPThreshold = m_sliderAttackHP->Get2EC();
    m_attackHGPThreshold = m_sliderAttackHGP->Get2EC();
    m_transportHPThreshold = m_sliderTransportHP->Get2EC();
    
    SaveConfig();
    
    LogPetAutoPotion("Settings confirmed - Attack HP: %s (%d%%), Attack HGP: %s (%d%%), Transport HP: %s (%d%%)",
        m_attackPetHP ? "ON" : "OFF", m_attackHPThreshold,
        m_attackPetHGP ? "ON" : "OFF", m_attackHGPThreshold,
        m_transportPetHP ? "ON" : "OFF", m_transportHPThreshold);
    
    PetAutoPotionMainFrame->ShowGWnd(false);
}

void CIFPetAutoPotion::On_BtnClickCancel()
{
    LogPetAutoPotion("Cancel button clicked - reloading saved settings");
    LoadConfig();
    PetAutoPotionMainFrame->ShowGWnd(false);
}

void CIFPetAutoPotion::On_CheckBoxScrollBar()
{
    // Attack HP slider enable/disable based on checkbox
    if (m_sliderBgAttackHP && m_sliderAttackHP && m_chkAttackHP) {
        if (!m_chkAttackHP->GetCheckedState_MAYBE()) {
            if (!m_attackHPSliderDisabled) {
                m_attackHPSliderDisabled = true;
                m_sliderBgAttackHP->TB_Func_12("interface\\recovery\\re_selectbar_disable.ddj", 0, 0);
                m_sliderAttackHP->Get2F4()->ShowGWnd(false);
                m_sliderAttackHP->Get2F8()->ShowGWnd(false);
                m_sliderAttackHP->Get2FC()->ShowGWnd(false);
                LogPetAutoPotion("Attack HP slider DISABLED (checkbox unchecked)");
            }
        } else {
            if (m_attackHPSliderDisabled) {
                m_attackHPSliderDisabled = false;
                m_sliderBgAttackHP->TB_Func_12("interface\\recovery\\re_selectbar.ddj", 0, 0);
                m_sliderAttackHP->Get2F4()->ShowGWnd(true);
                m_sliderAttackHP->Get2F8()->ShowGWnd(true);
                m_sliderAttackHP->Get2FC()->ShowGWnd(true);
                m_sliderAttackHP->Get2F4()->BringToFront();
                m_sliderAttackHP->Get2F8()->BringToFront();
                m_sliderAttackHP->Get2FC()->BringToFront();
                LogPetAutoPotion("Attack HP slider ENABLED (checkbox checked)");
            }
        }
    }
    
    // Attack HGP slider enable/disable based on checkbox
    if (m_sliderBgAttackHGP && m_sliderAttackHGP && m_chkAttackHGP) {
        if (!m_chkAttackHGP->GetCheckedState_MAYBE()) {
            if (!m_attackHGPSliderDisabled) {
                m_attackHGPSliderDisabled = true;
                m_sliderBgAttackHGP->TB_Func_12("interface\\recovery\\re_selectbar_disable.ddj", 0, 0);
                m_sliderAttackHGP->Get2F4()->ShowGWnd(false);
                m_sliderAttackHGP->Get2F8()->ShowGWnd(false);
                m_sliderAttackHGP->Get2FC()->ShowGWnd(false);
                LogPetAutoPotion("Attack HGP slider DISABLED (checkbox unchecked)");
            }
        } else {
            if (m_attackHGPSliderDisabled) {
                m_attackHGPSliderDisabled = false;
                m_sliderBgAttackHGP->TB_Func_12("interface\\recovery\\re_selectbar.ddj", 0, 0);
                m_sliderAttackHGP->Get2F4()->ShowGWnd(true);
                m_sliderAttackHGP->Get2F8()->ShowGWnd(true);
                m_sliderAttackHGP->Get2FC()->ShowGWnd(true);
                m_sliderAttackHGP->Get2F4()->BringToFront();
                m_sliderAttackHGP->Get2F8()->BringToFront();
                m_sliderAttackHGP->Get2FC()->BringToFront();
                LogPetAutoPotion("Attack HGP slider ENABLED (checkbox checked)");
            }
        }
    }
    
    // Transport HP slider enable/disable based on checkbox
    if (m_sliderBgTransportHP && m_sliderTransportHP && m_chkTransportHP) {
        if (!m_chkTransportHP->GetCheckedState_MAYBE()) {
            if (!m_transportHPSliderDisabled) {
                m_transportHPSliderDisabled = true;
                m_sliderBgTransportHP->TB_Func_12("interface\\recovery\\re_selectbar_disable.ddj", 0, 0);
                m_sliderTransportHP->Get2F4()->ShowGWnd(false);
                m_sliderTransportHP->Get2F8()->ShowGWnd(false);
                m_sliderTransportHP->Get2FC()->ShowGWnd(false);
                LogPetAutoPotion("Transport HP slider DISABLED (checkbox unchecked)");
            }
        } else {
            if (m_transportHPSliderDisabled) {
                m_transportHPSliderDisabled = false;
                m_sliderBgTransportHP->TB_Func_12("interface\\recovery\\re_selectbar.ddj", 0, 0);
                m_sliderTransportHP->Get2F4()->ShowGWnd(true);
                m_sliderTransportHP->Get2F8()->ShowGWnd(true);
                m_sliderTransportHP->Get2FC()->ShowGWnd(true);
                m_sliderTransportHP->Get2F4()->BringToFront();
                m_sliderTransportHP->Get2F8()->BringToFront();
                m_sliderTransportHP->Get2FC()->BringToFront();
                LogPetAutoPotion("Transport HP slider ENABLED (checkbox checked)");
            }
        }
    }
}

bool CIFPetAutoPotion::SaveConfig()
{
    std::string executableDir = GetPetAutoPotionExeDir();
    
    // Get character name for per-character settings
    const char* charName = "Default";
    if (g_pCICPlayerEcsro && g_pCICPlayerEcsro->charname[0] != '\0') {
        charName = g_pCICPlayerEcsro->charname;
    }
    
    char buffer[MAX_PATH];
    sprintf_s(buffer, sizeof(buffer), "%s\\Setting\\PetAutoPotion_%s.dat",
        executableDir.c_str(), charName);
    
    // Create Setting directory if it doesn't exist
    char settingDir[MAX_PATH];
    sprintf_s(settingDir, sizeof(settingDir), "%s\\Setting", executableDir.c_str());
    _mkdir(settingDir);
    
    FILE* f = fopen(buffer, "wb+");
    if (!f) {
        LogPetAutoPotion("ERROR: Failed to open file for writing: %s", buffer);
        return false;
    }
    
    // Write checkbox states (5 bools)
    bool checkboxData[5] = {
        m_attackPetHP,
        m_attackPetHGP,
        m_attackPetCure,
        m_transportPetHP,
        m_transportPetCure
    };
    size_t writtenBool = fwrite(checkboxData, sizeof(bool), 5, f);
    
    // Write slider values (3 ints)
    int sliderData[3] = {
        m_attackHPThreshold,
        m_attackHGPThreshold,
        m_transportHPThreshold
    };
    size_t writtenInt = fwrite(sliderData, sizeof(int), 3, f);
    
    fclose(f);
    
    if (writtenBool != 5 || writtenInt != 3) {
        LogPetAutoPotion("ERROR: Failed to write all data to %s", buffer);
        return false;
    }
    
    LogPetAutoPotion("Configuration saved to: %s", buffer);
    LogPetAutoPotion("  Checkboxes: AttackHP=%s, AttackHGP=%s, AttackCure=%s, TransportHP=%s, TransportCure=%s",
        m_attackPetHP ? "ON" : "OFF",
        m_attackPetHGP ? "ON" : "OFF",
        m_attackPetCure ? "ON" : "OFF",
        m_transportPetHP ? "ON" : "OFF",
        m_transportPetCure ? "ON" : "OFF");
    LogPetAutoPotion("  Thresholds: AttackHP=%d%%, AttackHGP=%d%%, TransportHP=%d%%",
        m_attackHPThreshold, m_attackHGPThreshold, m_transportHPThreshold);
    
    return true;
}

bool CIFPetAutoPotion::LoadConfig()
{
    std::string executableDir = GetPetAutoPotionExeDir();
    
    // Get character name for per-character settings
    const char* charName = "Default";
    if (g_pCICPlayerEcsro && g_pCICPlayerEcsro->charname[0] != '\0') {
        charName = g_pCICPlayerEcsro->charname;
    }
    
    char buffer[MAX_PATH];
    sprintf_s(buffer, sizeof(buffer), "%s\\Setting\\PetAutoPotion_%s.dat",
        executableDir.c_str(), charName);
    
    FILE* f = fopen(buffer, "rb");
    if (!f) {
        LogPetAutoPotion("Configuration file not found: %s - using defaults", buffer);
        
        // Set default values - all checkboxes OFF, sliders at 50%
        m_attackPetHP = false;
        m_attackPetHGP = false;
        m_attackPetCure = false;
        m_transportPetHP = false;
        m_transportPetCure = false;
        m_attackHPThreshold = 50;
        m_attackHGPThreshold = 50;
        m_transportHPThreshold = 50;
        
        LogPetAutoPotion("Using defaults: All checkboxes OFF, all sliders at 50%%");
        
        // Apply defaults to UI
        if (m_chkAttackHP) m_chkAttackHP->FUN_00656d50(m_attackPetHP);
        if (m_chkAttackHGP) m_chkAttackHGP->FUN_00656d50(m_attackPetHGP);
        if (m_chkAttackCure) m_chkAttackCure->FUN_00656d50(m_attackPetCure);
        if (m_chkTransportHP) m_chkTransportHP->FUN_00656d50(m_transportPetHP);
        if (m_chkTransportCure) m_chkTransportCure->FUN_00656d50(m_transportPetCure);
        
        if (m_sliderAttackHP) m_sliderAttackHP->Set2EC(m_attackHPThreshold);
        if (m_sliderAttackHGP) m_sliderAttackHGP->Set2EC(m_attackHGPThreshold);
        if (m_sliderTransportHP) m_sliderTransportHP->Set2EC(m_transportHPThreshold);
        
        // Save defaults to create the file
        SaveConfig();
        return true;
    }
    
    // Read checkbox states (5 bools)
    bool checkboxData[5];
    if (fread(checkboxData, sizeof(bool), 5, f) != 5) {
        LogPetAutoPotion("ERROR: Failed to read checkbox data from %s", buffer);
        fclose(f);
        return false;
    }
    
    // Read slider values (3 ints)
    int sliderData[3];
    if (fread(sliderData, sizeof(int), 3, f) != 3) {
        LogPetAutoPotion("ERROR: Failed to read slider data from %s", buffer);
        fclose(f);
        return false;
    }
    
    fclose(f);
    
    // Apply loaded values to member variables
    m_attackPetHP = checkboxData[0];
    m_attackPetHGP = checkboxData[1];
    m_attackPetCure = checkboxData[2];
    m_transportPetHP = checkboxData[3];
    m_transportPetCure = checkboxData[4];
    
    m_attackHPThreshold = sliderData[0];
    m_attackHGPThreshold = sliderData[1];
    m_transportHPThreshold = sliderData[2];
    
    // Apply to UI controls
    if (m_chkAttackHP) m_chkAttackHP->FUN_00656d50(m_attackPetHP);
    if (m_chkAttackHGP) m_chkAttackHGP->FUN_00656d50(m_attackPetHGP);
    if (m_chkAttackCure) m_chkAttackCure->FUN_00656d50(m_attackPetCure);
    if (m_chkTransportHP) m_chkTransportHP->FUN_00656d50(m_transportPetHP);
    if (m_chkTransportCure) m_chkTransportCure->FUN_00656d50(m_transportPetCure);
    
    if (m_sliderAttackHP) {
        m_sliderAttackHP->Set2EC(m_attackHPThreshold);
        m_sliderAttackHP->SetHCorrectScrollBar(m_attackHPThreshold);
    }
    if (m_sliderAttackHGP) {
        m_sliderAttackHGP->Set2EC(m_attackHGPThreshold);
        m_sliderAttackHGP->SetHCorrectScrollBar(m_attackHGPThreshold);
    }
    if (m_sliderTransportHP) {
        m_sliderTransportHP->Set2EC(m_transportHPThreshold);
        m_sliderTransportHP->SetHCorrectScrollBar(m_transportHPThreshold);
    }
    
    LogPetAutoPotion("Configuration loaded from: %s", buffer);
    LogPetAutoPotion("  Checkboxes: AttackHP=%s, AttackHGP=%s, AttackCure=%s, TransportHP=%s, TransportCure=%s",
        m_attackPetHP ? "ON" : "OFF",
        m_attackPetHGP ? "ON" : "OFF",
        m_attackPetCure ? "ON" : "OFF",
        m_transportPetHP ? "ON" : "OFF",
        m_transportPetCure ? "ON" : "OFF");
    LogPetAutoPotion("  Thresholds: AttackHP=%d%%, AttackHGP=%d%%, TransportHP=%d%%",
        m_attackHPThreshold, m_attackHGPThreshold, m_transportHPThreshold);
    
    return true;
}
