#include "IFAutoHuntSettings.h"
#include "GInterface.h"
#include "Game.h"
#include "ICPlayer.h"
#include "IFNormalTile.h"
#include "LearnedSkillManager.h"
#include <direct.h>
#include <string>
#include <cstdio>

extern int g_D3DViewportWidth;
extern int g_D3DViewportHeight;

CIFMainFrame *AutoHuntSettingsMainFrame = NULL;
CIFAutoHuntSettings *g_pCIFAutoHuntSettings = NULL;


GFX_IMPLEMENT_DYNCREATE(CIFAutoHuntSettings, CIFMainFrame)
GFX_BEGIN_MESSAGE_MAP(CIFAutoHuntSettings, CIFMainFrame)
ONG_COMMAND(60, &CIFAutoHuntSettings::OnClick_Confirm)
ONG_COMMAND(61, &CIFAutoHuntSettings::OnClick_Cancel)
GFX_END_MESSAGE_MAP()

static std::string GetAutoHuntExeDir() {
    char buffer[MAX_PATH];
    GetModuleFileNameA(NULL, buffer, MAX_PATH);
    std::string exePath(buffer);
    size_t lastSlash = exePath.find_last_of("\\/");
    return exePath.substr(0, lastSlash + 1);
}

// Window dimensions
static const int WINDOW_WIDTH = 600;
static const int WINDOW_HEIGHT = 520;
static const int CONTENT_MARGIN = 15;
static const int PANEL_GAP = 10;

CIFAutoHuntSettings::CIFAutoHuntSettings()
    : m_activeTab(TAB_AUTO_HUNT),
      m_tabAutoHunt(NULL), m_tabAttackSkills(NULL), m_tabBuffSkills(NULL),
      // Hunt panel
      m_huntTabHeader(NULL), m_huntTabLabel(NULL), m_huntSection1Frame(NULL),
      m_rangeTabHeader(NULL), m_rangeTabLabel(NULL), m_rangeSectionFrame(NULL), m_rangeSection2Frame(NULL),
      m_chkAutoBerserk(NULL), m_lblAutoBerserk(NULL),
      m_chkReturnToTownOnDeath(NULL), m_lblReturnToTownOnDeath(NULL),
      m_chkTownOnLowHP(NULL), m_lblTownOnLowHP(NULL),
      m_chkTownOnLowMP(NULL), m_lblTownOnLowMP(NULL),
      m_chkTownOnLowPetHP(NULL), m_lblTownOnLowPetHP(NULL),
      m_chkTownOnLowArrow(NULL), m_lblTownOnLowArrow(NULL),
      m_chkTownOnLowDurability(NULL), m_lblTownOnLowDurability(NULL),
      m_chkGoBackCenter(NULL), m_lblGoBackCenter(NULL),
      m_chkWalkAround(NULL), m_lblWalkAround(NULL),
      m_rangeSliderBg(NULL), m_rangeSlider(NULL), m_rangeValueText(NULL),
      m_rangeValue(100),  // Default range 100 (= 1000 game units)
      // Attack skills panel
      m_atkInfoLabel(NULL), m_atkAcquiredFrame(NULL), m_atkAcquiredLabel(NULL),
      m_atkAcquiredOutline(NULL), m_atkAcquiredLattice(NULL), m_atkAcquiredScroll(NULL),
      m_atkToUseFrame(NULL), m_atkToUseLabel(NULL),
      m_atkToUseOutline(NULL), m_atkToUseLattice(NULL), m_atkToUseScroll(NULL),
      // Buff skills panel
      m_buffInfoLabel(NULL), m_buffAcquiredFrame(NULL), m_buffAcquiredLabel(NULL),
      m_buffAcquiredOutline(NULL), m_buffAcquiredLattice(NULL), m_buffAcquiredScroll(NULL),
      m_buffToUseFrame(NULL), m_buffToUseLabel(NULL),
      m_buffToUseOutline(NULL), m_buffToUseLattice(NULL), m_buffToUseScroll(NULL),
      // Buttons
      m_btnConfirm(NULL), m_btnCancel(NULL),
      // Window position tracking
      m_lastWindowX(-1), m_lastWindowY(-1) {
    g_pCIFAutoHuntSettings = this;

    // Initialize slot arrays to NULL
    memset(m_atkAcquiredSlots, 0, sizeof(m_atkAcquiredSlots));
    memset(m_atkToUseSlots, 0, sizeof(m_atkToUseSlots));
    memset(m_buffAcquiredSlots, 0, sizeof(m_buffAcquiredSlots));
    memset(m_buffToUseSlots, 0, sizeof(m_buffToUseSlots));

    // Initialize skill ID tracking (Skills to Use)
    memset(m_atkSelectedSkillIds, 0, sizeof(m_atkSelectedSkillIds));
    memset(m_buffSelectedSkillIds, 0, sizeof(m_buffSelectedSkillIds));
    m_atkSelectedCount = 0;
    m_buffSelectedCount = 0;

    // Scroll offset for Acquired Skills (vectors init automatically)
    m_atkScrollOffset = 0;
    m_buffScrollOffset = 0;
    m_lastAtkScrollOffset = -1;  // Force initial render
    m_lastBuffScrollOffset = -1;

    // Initialize manual drag state
    m_isDragging = false;
    m_dragSourceSlotIndex = -1;
    m_dragSkillId = 0;
    m_dragIconPath[0] = '\0';
    m_dragIsAttackSkill = true;
    m_dragFromToUsePanel = false;
}

CIFAutoHuntSettings::~CIFAutoHuntSettings() {
    g_pCIFAutoHuntSettings = NULL;
}

bool CIFAutoHuntSettings::OnCreate(long ln) {
    RECT rect = {50, 50, WINDOW_WIDTH, WINDOW_HEIGHT};
    AutoHuntSettingsMainFrame = (CIFMainFrame *) CGWnd::CreateInstance(g_pCGInterface, GFX_RUNTIME_CLASS(CIFMainFrame), rect, 2005, 0);
    if (!AutoHuntSettingsMainFrame) return false;

    AutoHuntSettingsMainFrame->SetText("Auto Hunt Settings");
    AutoHuntSettingsMainFrame->TB_Func_12("interface\\frame\\mframe_wnd_", 1, 0);
    AutoHuntSettingsMainFrame->SetGWndSize(WINDOW_WIDTH, WINDOW_HEIGHT);

    wnd_rect sz;

    // ========== TAB BUTTONS (3 tabs) ==========
    int tabY = 43;
    int tabWidth = 100;
    int tabHeight = 24;
    int tabSpacing = 102;

    sz.pos.x = CONTENT_MARGIN;
    sz.pos.y = tabY;
    sz.size.width = tabWidth;
    sz.size.height = tabHeight;
    m_tabAutoHunt = (CIFStatic *) CGWnd::CreateInstance(AutoHuntSettingsMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 100, 0);
    if (m_tabAutoHunt) {
        m_tabAutoHunt->TB_Func_12("interface\\ifcommon\\com_long_tab_on.ddj", 0, 0);
        m_tabAutoHunt->SetText("Auto Hunt");
        m_tabAutoHunt->m_FontTexture.sub_8B4750(14);
    }

    sz.pos.x = CONTENT_MARGIN + tabSpacing;
    m_tabAttackSkills = (CIFStatic *) CGWnd::CreateInstance(AutoHuntSettingsMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 101, 0);
    if (m_tabAttackSkills) {
        m_tabAttackSkills->TB_Func_12("interface\\ifcommon\\com_long_tab_off.ddj", 0, 0);
        m_tabAttackSkills->SetText("Attack Skills");
        m_tabAttackSkills->m_FontTexture.sub_8B4750(14);
    }

    sz.pos.x = CONTENT_MARGIN + tabSpacing * 2;
    m_tabBuffSkills = (CIFStatic *) CGWnd::CreateInstance(AutoHuntSettingsMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 102, 0);
    if (m_tabBuffSkills) {
        m_tabBuffSkills->TB_Func_12("interface\\ifcommon\\com_long_tab_off.ddj", 0, 0);
        m_tabBuffSkills->SetText("Buff Skills");
        m_tabBuffSkills->m_FontTexture.sub_8B4750(14);
    }

    // ========== MAIN CONTENT FRAME ==========
    int contentWidth = WINDOW_WIDTH - (CONTENT_MARGIN * 2);
    int contentHeight = WINDOW_HEIGHT - 110;

    sz.pos.x = CONTENT_MARGIN;
    sz.pos.y = 65;
    sz.size.width = contentWidth;
    sz.size.height = contentHeight;
    CIFFrame *pFrame = (CIFFrame *) CGWnd::CreateInstance(AutoHuntSettingsMainFrame, GFX_RUNTIME_CLASS(CIFFrame), sz, 1, 0);
    if (pFrame) {
        pFrame->TB_Func_12("interface\\inventory\\int_window_", 1, 0);
        pFrame->SetGWndSize(contentWidth, contentHeight);
    }

    // Background tile
    sz.pos.x = CONTENT_MARGIN + 15;
    sz.pos.y = 76;
    sz.size.width = contentWidth - 30;
    sz.size.height = contentHeight - 22;
    CIFNormalTile *pTile = (CIFNormalTile *) CGWnd::CreateInstance(AutoHuntSettingsMainFrame, GFX_RUNTIME_CLASS(CIFNormalTile), sz, 2, 0);
    if (pTile) {
        pTile->TB_Func_12("interface\\ifcommon\\bg_tile\\com_bg_tile_b.ddj", 1, 0);
        pTile->SetGWndSize(contentWidth - 30, contentHeight - 22);
    }

    // Create all panels
    CreateAutoHuntPanel();
    CreateAttackSkillsPanel();
    CreateBuffSkillsPanel();

    // ========== Buttons ==========
    int btnY = WINDOW_HEIGHT - 35;  // Moved down for better centering
    int btnWidth = 76;
    int btnGap = 8;
    int totalBtnWidth = btnWidth * 2 + btnGap;
    int btnStartX = (WINDOW_WIDTH - totalBtnWidth) / 2;  // Center both buttons
    
    sz.pos.x = btnStartX;
    sz.pos.y = btnY;
    sz.size.width = btnWidth;
    sz.size.height = 24;
    m_btnConfirm = (CIFButton *) CGWnd::CreateInstance(AutoHuntSettingsMainFrame, GFX_RUNTIME_CLASS(CIFButton), sz, 60, 0);
    if (m_btnConfirm) {
        m_btnConfirm->TB_Func_12("interface\\ifcommon\\com_button.ddj", 1, 1);
        m_btnConfirm->SetText("Confirm");
        m_btnConfirm->SetGWndSize(btnWidth, 24);
    }

    sz.pos.x = btnStartX + btnWidth + btnGap;
    m_btnCancel = (CIFButton *) CGWnd::CreateInstance(AutoHuntSettingsMainFrame, GFX_RUNTIME_CLASS(CIFButton), sz, 61, 0);
    if (m_btnCancel) {
        m_btnCancel->TB_Func_12("interface\\ifcommon\\com_button.ddj", 1, 1);
        m_btnCancel->SetText("Cancel");
        m_btnCancel->SetGWndSize(btnWidth, 24);
    }

    AutoHuntSettingsMainFrame->ShowGWnd(false);
    SwitchToTab(TAB_AUTO_HUNT);
    LoadConfig();
    return true;
}

void CIFAutoHuntSettings::CreateAutoHuntPanel() {
    wnd_rect sz;
    int contentWidth = WINDOW_WIDTH - (CONTENT_MARGIN * 2) - 30;

    // Section 1: Hunt Settings
    sz.pos.x = CONTENT_MARGIN + 7;
    sz.pos.y = 79;
    sz.size.width = 124;
    sz.size.height = 28;
    m_huntTabHeader = (CIFStatic *) CGWnd::CreateInstance(AutoHuntSettingsMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 10, 0);
    if (m_huntTabHeader) {
        m_huntTabHeader->TB_Func_12("interface\\option\\opt_video_tab.ddj", 1, 0);
        m_huntTabHeader->SetGWndSize(124, 28);
    }

    sz.pos.x = CONTENT_MARGIN + 17;
    sz.pos.y = 88;
    sz.size.width = 102;
    sz.size.height = 11;
    m_huntTabLabel = (CIFStatic *) CGWnd::CreateInstance(AutoHuntSettingsMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 11, 0);
    if (m_huntTabLabel) {
        m_huntTabLabel->SetText("Hunt Settings");
        m_huntTabLabel->SetTextColor(D3DCOLOR_XRGB(239, 218, 164));
    }

    sz.pos.x = CONTENT_MARGIN + 7;
    sz.pos.y = 102;
    sz.size.width = contentWidth;
    sz.size.height = 200;
    m_huntSection1Frame = (CIFFrame *) CGWnd::CreateInstance(AutoHuntSettingsMainFrame, GFX_RUNTIME_CLASS(CIFFrame), sz, 12, 0);
    if (m_huntSection1Frame) {
        m_huntSection1Frame->TB_Func_12("interface\\inventory\\int_window_", 1, 0);
        m_huntSection1Frame->SetGWndSize(contentWidth, 200);
    }

    // Checkboxes - aligned like Pet Auto Potion
    int col1X = CONTENT_MARGIN + 17;   // Checkbox X position (with left padding)
    int labelX = CONTENT_MARGIN + 45;  // Label X position (fixed absolute value for all labels)
    int col2X = CONTENT_MARGIN + 210;
    int col3X = CONTENT_MARGIN + 400;
    int startY = 117;
    int rowHeight = 26;

    // Row 1 - Auto Berserk
    sz.pos.x = col1X;
    sz.pos.y = startY;
    sz.size.width = 16;
    sz.size.height = 16;
    m_chkAutoBerserk = (CIFCheckBox *) CGWnd::CreateInstance(AutoHuntSettingsMainFrame, GFX_RUNTIME_CLASS(CIFCheckBox), sz, 20, 0);
    sz.pos.x = CONTENT_MARGIN + 31;  // Manual alignment for "Auto Berserk"
    sz.pos.y = startY + 2;
    sz.size.width = 100;
    sz.size.height = 11;
    m_lblAutoBerserk = (CIFStatic *) CGWnd::CreateInstance(AutoHuntSettingsMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 21, 0);
    if (m_lblAutoBerserk) m_lblAutoBerserk->SetText("Auto Berserk");

    // Row 2 - Return to Town on Death
    sz.pos.x = col1X;
    sz.pos.y = startY + rowHeight;
    sz.size.width = 16;
    sz.size.height = 16;
    m_chkReturnToTownOnDeath = (CIFCheckBox *) CGWnd::CreateInstance(AutoHuntSettingsMainFrame, GFX_RUNTIME_CLASS(CIFCheckBox), sz, 22, 0);
    sz.pos.x = CONTENT_MARGIN + 38;  // Manual alignment for "Return to Town on Death"
    sz.pos.y = startY + rowHeight + 2;
    sz.size.width = 150;
    sz.size.height = 11;
    m_lblReturnToTownOnDeath = (CIFStatic *) CGWnd::CreateInstance(AutoHuntSettingsMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 23, 0);
    if (m_lblReturnToTownOnDeath) m_lblReturnToTownOnDeath->SetText("Return to Town on Death");

    // Row 3 - Town on Low HP (<20)
    sz.pos.x = col1X;
    sz.pos.y = startY + rowHeight * 2;
    sz.size.width = 16;
    sz.size.height = 16;
    m_chkTownOnLowHP = (CIFCheckBox *) CGWnd::CreateInstance(AutoHuntSettingsMainFrame, GFX_RUNTIME_CLASS(CIFCheckBox), sz, 24, 0);
    sz.pos.x = CONTENT_MARGIN + 33;  // Manual alignment for "Town on Low HP"
    sz.pos.y = startY + rowHeight * 2 + 2;
    sz.size.width = 150;
    sz.size.height = 11;
    m_lblTownOnLowHP = (CIFStatic *) CGWnd::CreateInstance(AutoHuntSettingsMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 25, 0);
    if (m_lblTownOnLowHP) m_lblTownOnLowHP->SetText("Town on Low HP (<20)");

    // Row 4 - Town on Low MP (<20)
    sz.pos.x = col1X;
    sz.pos.y = startY + rowHeight * 3;
    sz.size.width = 16;
    sz.size.height = 16;
    m_chkTownOnLowMP = (CIFCheckBox *) CGWnd::CreateInstance(AutoHuntSettingsMainFrame, GFX_RUNTIME_CLASS(CIFCheckBox), sz, 26, 0);
    sz.pos.x = CONTENT_MARGIN + 33;  // Manual alignment for "Town on Low MP"
    sz.pos.y = startY + rowHeight * 3 + 2;
    sz.size.width = 150;
    sz.size.height = 11;
    m_lblTownOnLowMP = (CIFStatic *) CGWnd::CreateInstance(AutoHuntSettingsMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 27, 0);
    if (m_lblTownOnLowMP) m_lblTownOnLowMP->SetText("Town on Low MP (<20)");

    // Row 5 - Town on Low Pet HP (<20)
    sz.pos.x = col1X;
    sz.pos.y = startY + rowHeight * 4;
    sz.size.width = 16;
    sz.size.height = 16;
    m_chkTownOnLowPetHP = (CIFCheckBox *) CGWnd::CreateInstance(AutoHuntSettingsMainFrame, GFX_RUNTIME_CLASS(CIFCheckBox), sz, 28, 0);
    sz.pos.x = CONTENT_MARGIN + 43;  // Manual alignment for "Town on Low Pet HP"
    sz.pos.y = startY + rowHeight * 4 + 2;
    sz.size.width = 150;
    sz.size.height = 11;
    m_lblTownOnLowPetHP = (CIFStatic *) CGWnd::CreateInstance(AutoHuntSettingsMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 29, 0);
    if (m_lblTownOnLowPetHP) m_lblTownOnLowPetHP->SetText("Town on Low Pet HP (<20)");

    // Row 6 - Town on Low Arrow (<50)
    sz.pos.x = col1X;
    sz.pos.y = startY + rowHeight * 5;
    sz.size.width = 16;
    sz.size.height = 16;
    m_chkTownOnLowArrow = (CIFCheckBox *) CGWnd::CreateInstance(AutoHuntSettingsMainFrame, GFX_RUNTIME_CLASS(CIFCheckBox), sz, 30, 0);
    sz.pos.x = CONTENT_MARGIN + 41;  // Manual alignment for "Town on Low Arrow"
    sz.pos.y = startY + rowHeight * 5 + 2;
    sz.size.width = 150;
    sz.size.height = 11;
    m_lblTownOnLowArrow = (CIFStatic *) CGWnd::CreateInstance(AutoHuntSettingsMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 31, 0);
    if (m_lblTownOnLowArrow) m_lblTownOnLowArrow->SetText("Town on Low Arrow (<50)");

    // Row 7 - Town on Low Durability (<5)
    sz.pos.x = col1X;
    sz.pos.y = startY + rowHeight * 6;
    sz.size.width = 16;
    sz.size.height = 16;
    m_chkTownOnLowDurability = (CIFCheckBox *) CGWnd::CreateInstance(AutoHuntSettingsMainFrame, GFX_RUNTIME_CLASS(CIFCheckBox), sz, 32, 0);
    sz.pos.x = CONTENT_MARGIN + 35;
    sz.pos.y = startY + rowHeight * 6 + 2;
    sz.size.width = 150;
    sz.size.height = 11;
    m_lblTownOnLowDurability = (CIFStatic *) CGWnd::CreateInstance(AutoHuntSettingsMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 33, 0);
    if (m_lblTownOnLowDurability) m_lblTownOnLowDurability->SetText("Town on Low Dura (<5)");

    // Section 2: Range
    int rangeY = 358;  // Adjusted for new Row 7 (Town on Low Durability)
    sz.pos.x = CONTENT_MARGIN + 7;
    sz.pos.y = rangeY;
    sz.size.width = 124;
    sz.size.height = 28;
    m_rangeTabHeader = (CIFStatic *) CGWnd::CreateInstance(AutoHuntSettingsMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 40, 0);
    if (m_rangeTabHeader) {
        m_rangeTabHeader->TB_Func_12("interface\\option\\opt_video_tab.ddj", 1, 0);
        m_rangeTabHeader->SetGWndSize(124, 28);
    }

    sz.pos.x = CONTENT_MARGIN + 17;
    sz.pos.y = rangeY + 9;
    sz.size.width = 102;
    sz.size.height = 11;
    m_rangeTabLabel = (CIFStatic *) CGWnd::CreateInstance(AutoHuntSettingsMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 41, 0);
    if (m_rangeTabLabel) {
        m_rangeTabLabel->SetText("Range");
        m_rangeTabLabel->SetTextColor(D3DCOLOR_XRGB(239, 218, 164));
    }

    // Subframe - aligned with Range tab header on left
    int subframeWidth = 380;
    int subframeX = CONTENT_MARGIN + 7;  // Same as Range tab header
    sz.pos.x = subframeX;
    sz.pos.y = rangeY + 23;
    sz.size.width = subframeWidth;
    sz.size.height = 56;
    m_rangeSectionFrame = (CIFFrame *) CGWnd::CreateInstance(AutoHuntSettingsMainFrame, GFX_RUNTIME_CLASS(CIFFrame), sz, 42, 0);
    if (m_rangeSectionFrame) {
        m_rangeSectionFrame->TB_Func_12("interface\\inventory\\int_window_", 1, 0);
        m_rangeSectionFrame->SetGWndSize(subframeWidth, 90);
    }

    // Second subframe - to the right of Range slider subframe
    int subframe2Width = contentWidth - subframeWidth - 10;  // Fill remaining space
    int subframe2X = subframeX + subframeWidth + 10;
    sz.pos.x = subframe2X;
    sz.pos.y = rangeY + 23;
    sz.size.width = subframe2Width;
    sz.size.height = 56;
    m_rangeSection2Frame = (CIFFrame *) CGWnd::CreateInstance(AutoHuntSettingsMainFrame, GFX_RUNTIME_CLASS(CIFFrame), sz, 43, 0);
    if (m_rangeSection2Frame) {
        m_rangeSection2Frame->TB_Func_12("interface\\inventory\\int_window_", 1, 0);
        m_rangeSection2Frame->SetGWndSize(subframe2Width, 90);
    }

    // Go Back Center checkbox - inside second subframe (row 1)
    int sf2ContentX = subframe2X + 10;
    int sf2Row1Y = rangeY + 38;
    sz.pos.x = sf2ContentX;
    sz.pos.y = sf2Row1Y;
    sz.size.width = 16;
    sz.size.height = 16;
    m_chkGoBackCenter = (CIFCheckBox *) CGWnd::CreateInstance(AutoHuntSettingsMainFrame, GFX_RUNTIME_CLASS(CIFCheckBox), sz, 60, 0);
    sz.pos.x = sf2ContentX + 20;
    sz.pos.y = sf2Row1Y + 2;
    sz.size.width = 110;
    sz.size.height = 11;
    m_lblGoBackCenter = (CIFStatic *) CGWnd::CreateInstance(AutoHuntSettingsMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 61, 0);
    if (m_lblGoBackCenter) m_lblGoBackCenter->SetText("Go Back Center");

    // Walk Around checkbox - inside second subframe (row 2)
    int sf2Row2Y = sf2Row1Y + 28;
    sz.pos.x = sf2ContentX;
    sz.pos.y = sf2Row2Y;
    sz.size.width = 16;
    sz.size.height = 16;
    m_chkWalkAround = (CIFCheckBox *) CGWnd::CreateInstance(AutoHuntSettingsMainFrame, GFX_RUNTIME_CLASS(CIFCheckBox), sz, 62, 0);
    sz.pos.x = sf2ContentX + 20;
    sz.pos.y = sf2Row2Y + 2;
    sz.size.width = 110;
    sz.size.height = 11;
    m_lblWalkAround = (CIFStatic *) CGWnd::CreateInstance(AutoHuntSettingsMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 63, 0);
    if (m_lblWalkAround) m_lblWalkAround->SetText("Walk Around");

    // Range slider - centered in subframe
    int sliderBgWidth = 328;
    int sliderControlWidth = 267;
    int sliderX = subframeX + (subframeWidth - sliderBgWidth) / 2;  // Center in subframe
    int labelY = rangeY + 32;
    int sliderY = rangeY + 48;

    // "0" label
    sz.pos.x = sliderX + 22;
    sz.pos.y = labelY;
    sz.size.width = 25;
    sz.size.height = 11;
    CIFStatic* lblMin = (CIFStatic*) CGWnd::CreateInstance(AutoHuntSettingsMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 52, 0);
    if (lblMin) { lblMin->SetText("0"); lblMin->SetTextColor(D3DCOLOR_XRGB(200, 200, 200)); }

    // "500" label
    sz.pos.x = sliderX + sliderBgWidth - 45;
    sz.pos.y = labelY;
    sz.size.width = 35;
    sz.size.height = 11;
    CIFStatic* lblMax = (CIFStatic*) CGWnd::CreateInstance(AutoHuntSettingsMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 53, 0);
    if (lblMax) { lblMax->SetText("500"); lblMax->SetTextColor(D3DCOLOR_XRGB(200, 200, 200)); }

    // Background bar - EXACT PetAutoPotion (width=328, height=22)
    sz.pos.x = sliderX;
    sz.pos.y = sliderY;
    sz.size.width = 328;
    sz.size.height = 24;
    m_rangeSliderBg = (CIFStatic*) CGWnd::CreateInstance(AutoHuntSettingsMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 50, 0);
    if (m_rangeSliderBg) { 
        m_rangeSliderBg->TB_Func_12("interface\\recovery\\re_selectbar.ddj", 1, 0); 
        m_rangeSliderBg->SetGWndSize(328, 22); 
    }

    // Slider control - EXACT PetAutoPotion (x+22, y+3, width=267)
    sz.pos.x = sliderX + 22;
    sz.pos.y = sliderY + 3;
    sz.size.width = 267;
    sz.size.height = 16;
    m_rangeSlider = (CIFHScroll_Option*) CGWnd::CreateInstance(AutoHuntSettingsMainFrame, GFX_RUNTIME_CLASS(CIFHScroll_Option), sz, 51, 0);
    if (m_rangeSlider) {
        m_rangeSlider->Set2E0(500); m_rangeSlider->Set2E4(500); m_rangeSlider->Set2E8(0); m_rangeSlider->Set2F0(10);
        m_rangeSlider->Get2F4()->SetGWndSize(24, 20); m_rangeSlider->Get2F8()->SetGWndSize(24, 20);
        m_rangeSlider->SetHScrollBar(267, 0, 500, 10); m_rangeSlider->SetGWndSize(267, 20);
    }

    // Value text - centered below slider
    sz.pos.x = sliderX + (sliderBgWidth / 2) - 25;
    sz.pos.y = sliderY + 26;
    sz.size.width = 50;
    sz.size.height = 14;
    m_rangeValueText = (CIFStatic*) CGWnd::CreateInstance(AutoHuntSettingsMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 54, 0);
    if (m_rangeValueText) {
        m_rangeValueText->SetTextFormatted("%d", m_rangeValue);
        m_rangeValueText->SetTextColor(D3DCOLOR_XRGB(255, 255, 0));
    }
}

void CIFAutoHuntSettings::CreateAttackSkillsPanel() {
    wnd_rect sz;

    // Calculate centered panel positions
    int totalContentWidth = WINDOW_WIDTH - (CONTENT_MARGIN * 2) - 30;
    int panelWidth = (totalContentWidth - PANEL_GAP) / 2;
    int panelHeight = 300;
    int leftPanelX = CONTENT_MARGIN + 15;
    int rightPanelX = leftPanelX + panelWidth + PANEL_GAP;
    int panelY = 110;
    // Lattice: 6 cols × 32 = 192px, 8 rows × 32 = 256px
    int latticeWidth = SKILL_SLOT_COLS * SKILL_SLOT_SIZE;  // 6 * 32 = 192
    int latticeHeight = SKILL_SLOT_ROWS * SKILL_SLOT_SIZE; // 8 * 32 = 256

    // Info label (centered)
    sz.pos.x = CONTENT_MARGIN + 20;
    sz.pos.y = 82;
    sz.size.width = totalContentWidth;
    sz.size.height = 20;
    m_atkInfoLabel = (CIFStatic *) CGWnd::CreateInstance(AutoHuntSettingsMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 200, 0);
    if (m_atkInfoLabel) {
        m_atkInfoLabel->SetText("Assign attack skills to use automatically.");
        m_atkInfoLabel->SetTextColor(D3DCOLOR_XRGB(239, 218, 164));
        m_atkInfoLabel->ShowGWnd(false);
    }

    // Left panel: Acquired Skills Frame
    sz.pos.x = leftPanelX;
    sz.pos.y = panelY;
    sz.size.width = panelWidth;
    sz.size.height = panelHeight;
    m_atkAcquiredFrame = (CIFFrame *) CGWnd::CreateInstance(AutoHuntSettingsMainFrame, GFX_RUNTIME_CLASS(CIFFrame), sz, 201, 0);
    if (m_atkAcquiredFrame) {
        m_atkAcquiredFrame->TB_Func_12("interface\\inventory\\int_window_", 1, 0);
        m_atkAcquiredFrame->SetGWndSize(panelWidth, panelHeight);
        m_atkAcquiredFrame->ShowGWnd(false);
    }

    // Left panel label
    sz.pos.x = leftPanelX + 10;
    sz.pos.y = panelY + 8;
    sz.size.width = 120;
    sz.size.height = 15;
    m_atkAcquiredLabel = (CIFStatic *) CGWnd::CreateInstance(AutoHuntSettingsMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 202, 0);
    if (m_atkAcquiredLabel) {
        m_atkAcquiredLabel->SetText("Acquired Skills");
        m_atkAcquiredLabel->SetTextColor(D3DCOLOR_XRGB(200, 200, 200));
        m_atkAcquiredLabel->ShowGWnd(false);
    }

    // Left lattice outline (tile behind lattice)
    sz.pos.x = leftPanelX + 9;
    sz.pos.y = panelY + 26;
    sz.size.width = latticeWidth + 6;
    sz.size.height = latticeHeight + 4;
    m_atkAcquiredOutline = (CIFStretchWnd *) CGWnd::CreateInstance(AutoHuntSettingsMainFrame, GFX_RUNTIME_CLASS(CIFStretchWnd), sz, 209, 0);
    if (m_atkAcquiredOutline) {
        m_atkAcquiredOutline->LoadTexturesFromPrefix("interface\\ifcommon\\lattice_window\\com_lattice_outline_");
        m_atkAcquiredOutline->SetGWndSize(latticeWidth + 6, latticeHeight + 4);
        m_atkAcquiredOutline->ShowGWnd(false);
    }

    // Left lattice grid (6 cols x 8 rows, 32x32 cells)
    typedef void(__thiscall * VertexCalc_t)(void *);
    static VertexCalc_t vertexCalc = (VertexCalc_t) 0x56AFC0;

    sz.pos.x = leftPanelX + 12;
    sz.pos.y = panelY + 28;
    sz.size.width = latticeWidth;
    sz.size.height = latticeHeight;
    m_atkAcquiredLattice = (CIFLattice *) CGWnd::CreateInstance(AutoHuntSettingsMainFrame, GFX_RUNTIME_CLASS(CIFLattice), sz, 203, 0);
    if (m_atkAcquiredLattice) {
        m_atkAcquiredLattice->TB_Func_12("interface\\ifcommon\\lattice_window\\com_lattice_", 1, 0);
        m_atkAcquiredLattice->SetCornerSizes(32, 32);// Override texture-derived corner sizes
        m_atkAcquiredLattice->SetGWndSize(latticeWidth, latticeHeight);
        m_atkAcquiredLattice->SetGridSize(6, 8);
        m_atkAcquiredLattice->SetCellSize(32, 32);
        vertexCalc(m_atkAcquiredLattice);// Force vertex recalc
        m_atkAcquiredLattice->ShowGWnd(false);
    }

    // Right panel: Skills to Use Frame
    sz.pos.x = rightPanelX;
    sz.pos.y = panelY;
    sz.size.width = panelWidth;
    sz.size.height = panelHeight;
    m_atkToUseFrame = (CIFFrame *) CGWnd::CreateInstance(AutoHuntSettingsMainFrame, GFX_RUNTIME_CLASS(CIFFrame), sz, 204, 0);
    if (m_atkToUseFrame) {
        m_atkToUseFrame->TB_Func_12("interface\\inventory\\int_window_", 1, 0);
        m_atkToUseFrame->SetGWndSize(panelWidth, panelHeight);
        m_atkToUseFrame->ShowGWnd(false);
    }

    // Right panel label
    sz.pos.x = rightPanelX + 10;
    sz.pos.y = panelY + 8;
    sz.size.width = 100;
    sz.size.height = 15;
    m_atkToUseLabel = (CIFStatic *) CGWnd::CreateInstance(AutoHuntSettingsMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 205, 0);
    if (m_atkToUseLabel) {
        m_atkToUseLabel->SetText("Skills to Use");
        m_atkToUseLabel->SetTextColor(D3DCOLOR_XRGB(200, 200, 200));
        m_atkToUseLabel->ShowGWnd(false);
    }

    // Right lattice outline
    sz.pos.x = rightPanelX + 9;
    sz.pos.y = panelY + 26;
    sz.size.width = latticeWidth + 6;
    sz.size.height = latticeHeight + 4;
    m_atkToUseOutline = (CIFStretchWnd *) CGWnd::CreateInstance(AutoHuntSettingsMainFrame, GFX_RUNTIME_CLASS(CIFStretchWnd), sz, 210, 0);
    if (m_atkToUseOutline) {
        m_atkToUseOutline->LoadTexturesFromPrefix("interface\\ifcommon\\lattice_window\\com_lattice_outline_");
        m_atkToUseOutline->SetGWndSize(latticeWidth + 6, latticeHeight + 4);
        m_atkToUseOutline->ShowGWnd(false);
    }

    // Right lattice grid (6 cols x 8 rows)
    sz.pos.x = rightPanelX + 12;
    sz.pos.y = panelY + 28;
    sz.size.width = latticeWidth;
    sz.size.height = latticeHeight;
    m_atkToUseLattice = (CIFLattice *) CGWnd::CreateInstance(AutoHuntSettingsMainFrame, GFX_RUNTIME_CLASS(CIFLattice), sz, 206, 0);
    if (m_atkToUseLattice) {
        m_atkToUseLattice->TB_Func_12("interface\\ifcommon\\lattice_window\\com_lattice_", 1, 0);
        m_atkToUseLattice->SetCornerSizes(32, 32);// Override texture-derived corner sizes
        m_atkToUseLattice->SetGWndSize(latticeWidth, latticeHeight);
        m_atkToUseLattice->SetGridSize(6, 8);
        m_atkToUseLattice->SetCellSize(32, 32);
        vertexCalc(m_atkToUseLattice);// Force vertex recalc
        m_atkToUseLattice->ShowGWnd(false);
    }

    // ========== CREATE SKILL SLOTS ON TOP OF LATTICE ==========
    // Left panel slots (Acquired Skills) - CIFSlotWithHelp with slot type 0x49 for DRAG SOURCE
    int leftSlotBaseX = leftPanelX + 12;
    int leftSlotBaseY = panelY + 28;

    for (int i = 0; i < SKILL_SLOT_COUNT; i++) {
        int col = i % SKILL_SLOT_COLS;
        int row = i / SKILL_SLOT_COLS;

        sz.pos.x = leftSlotBaseX + (col * SKILL_SLOT_SIZE);
        sz.pos.y = leftSlotBaseY + (row * SKILL_SLOT_SIZE);
        sz.size.width = SKILL_SLOT_SIZE;
        sz.size.height = SKILL_SLOT_SIZE;

        // Use CIFSlotWithHelp with slot type 0x49 for drag SOURCE
        m_atkAcquiredSlots[i] = (CIFSlotWithHelp *) CGWnd::CreateInstance(
            AutoHuntSettingsMainFrame, GFX_RUNTIME_CLASS(CIFSlotWithHelp), sz, 220 + i, 0);

        if (m_atkAcquiredSlots[i]) {
            m_atkAcquiredSlots[i]->InitDropSupport();
            m_atkAcquiredSlots[i]->InitRenderData(sz.pos.x, sz.pos.y, sz.pos.x + SKILL_SLOT_SIZE, sz.pos.y + SKILL_SLOT_SIZE);
            m_atkAcquiredSlots[i]->SetSlotIndex(i);
            m_atkAcquiredSlots[i]->SetSlotType(0x49);// 🔥 Drag SOURCE type (like SkillBoard slots)
            m_atkAcquiredSlots[i]->ShowGWnd(false);
        }
    }

    // Right panel slots (Skills to Use)
    int rightSlotBaseX = rightPanelX + 12;
    int rightSlotBaseY = panelY + 28;

    for (int i = 0; i < SKILL_SLOT_COUNT; i++) {
        int col = i % SKILL_SLOT_COLS;
        int row = i / SKILL_SLOT_COLS;

        sz.pos.x = rightSlotBaseX + (col * SKILL_SLOT_SIZE);
        sz.pos.y = rightSlotBaseY + (row * SKILL_SLOT_SIZE);
        sz.size.width = SKILL_SLOT_SIZE;
        sz.size.height = SKILL_SLOT_SIZE;

        m_atkToUseSlots[i] = (CIFSlotWithHelp *) CGWnd::CreateInstance(
            AutoHuntSettingsMainFrame, GFX_RUNTIME_CLASS(CIFSlotWithHelp), sz, 270 + i, 0);

        if (m_atkToUseSlots[i]) {
            m_atkToUseSlots[i]->InitDropSupport();// Initialize drag-drop support
            m_atkToUseSlots[i]->InitRenderData(sz.pos.x, sz.pos.y, sz.pos.x + SKILL_SLOT_SIZE, sz.pos.y + SKILL_SLOT_SIZE);
            m_atkToUseSlots[i]->SetSlotIndex(i);
            m_atkToUseSlots[i]->SetSlotType(0);// Basic type (testing)
            m_atkToUseSlots[i]->SetSlotEnabled(true);
            m_atkToUseSlots[i]->InitSlotIndex2(i);
            m_atkToUseSlots[i]->InitSlotParam(0);
            m_atkToUseSlots[i]->SetSlotItemID(-1);// Mark as empty slot
            m_atkToUseSlots[i]->ShowGWnd(false);
        }
    }

    
    // ========== VERTICAL SCROLLBAR FOR ACQUIRED SKILLS ==========
    sz.pos.x = leftPanelX + 12 + latticeWidth + 4;
    sz.pos.y = panelY + 44;
    sz.size.width = 16;
    sz.size.height = 220;
    
    m_atkAcquiredScroll = (CIFVerticalScroll *) CGWnd::CreateInstance(
        AutoHuntSettingsMainFrame, GFX_RUNTIME_CLASS(CIFVerticalScroll), sz, 215, 0);
    if (m_atkAcquiredScroll) {
        m_atkAcquiredScroll->TB_Func_12("interface\\ifcommon\\com_scroll_bar.ddj", 0, 0);
        m_atkAcquiredScroll->SetGWndSize(16, 220);
        m_atkAcquiredScroll->SetScrollBarRange(220, 0, 100, 1);
        m_atkAcquiredScroll->SetVCorrectScrollBar(0);
        
        CIFButton* upBtn = m_atkAcquiredScroll->GetUpButton();
        CIFButton* downBtn = m_atkAcquiredScroll->GetDownButton();
        if (upBtn) upBtn->SetGWndSize(16, 16);
        if (downBtn) downBtn->SetGWndSize(16, 16);
        
        m_atkAcquiredScroll->ShowGWnd(false);
    }
    
    m_atkToUseScroll = NULL;
}

void CIFAutoHuntSettings::CreateBuffSkillsPanel() {
    wnd_rect sz;

    int totalContentWidth = WINDOW_WIDTH - (CONTENT_MARGIN * 2) - 30;
    int panelWidth = (totalContentWidth - PANEL_GAP) / 2;
    int panelHeight = 300;
    int leftPanelX = CONTENT_MARGIN + 15;
    int rightPanelX = leftPanelX + panelWidth + PANEL_GAP;
    int panelY = 110;
    // Lattice: 6 cols × 32 = 192px, 8 rows × 32 = 256px
    int latticeWidth = SKILL_SLOT_COLS * SKILL_SLOT_SIZE;  // 6 * 32 = 192
    int latticeHeight = SKILL_SLOT_ROWS * SKILL_SLOT_SIZE; // 8 * 32 = 256

    // Info label
    sz.pos.x = CONTENT_MARGIN + 20;
    sz.pos.y = 82;
    sz.size.width = totalContentWidth;
    sz.size.height = 20;
    m_buffInfoLabel = (CIFStatic *) CGWnd::CreateInstance(AutoHuntSettingsMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 300, 0);
    if (m_buffInfoLabel) {
        m_buffInfoLabel->SetText("Assign buff skills to use automatically.");
        m_buffInfoLabel->SetTextColor(D3DCOLOR_XRGB(239, 218, 164));
        m_buffInfoLabel->ShowGWnd(false);
    }

    // Left panel: Acquired Buffs Frame
    sz.pos.x = leftPanelX;
    sz.pos.y = panelY;
    sz.size.width = panelWidth;
    sz.size.height = panelHeight;
    m_buffAcquiredFrame = (CIFFrame *) CGWnd::CreateInstance(AutoHuntSettingsMainFrame, GFX_RUNTIME_CLASS(CIFFrame), sz, 301, 0);
    if (m_buffAcquiredFrame) {
        m_buffAcquiredFrame->TB_Func_12("interface\\inventory\\int_window_", 1, 0);
        m_buffAcquiredFrame->SetGWndSize(panelWidth, panelHeight);
        m_buffAcquiredFrame->ShowGWnd(false);
    }

    // Left panel label
    sz.pos.x = leftPanelX + 10;
    sz.pos.y = panelY + 8;
    sz.size.width = 120;
    sz.size.height = 15;
    m_buffAcquiredLabel = (CIFStatic *) CGWnd::CreateInstance(AutoHuntSettingsMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 302, 0);
    if (m_buffAcquiredLabel) {
        m_buffAcquiredLabel->SetText("Acquired Buffs");
        m_buffAcquiredLabel->SetTextColor(D3DCOLOR_XRGB(200, 200, 200));
        m_buffAcquiredLabel->ShowGWnd(false);
    }

    // Left lattice outline
    sz.pos.x = leftPanelX + 9;
    sz.pos.y = panelY + 26;
    sz.size.width = latticeWidth + 6;
    sz.size.height = latticeHeight + 4;
    m_buffAcquiredOutline = (CIFStretchWnd *) CGWnd::CreateInstance(AutoHuntSettingsMainFrame, GFX_RUNTIME_CLASS(CIFStretchWnd), sz, 311, 0);
    if (m_buffAcquiredOutline) {
        m_buffAcquiredOutline->LoadTexturesFromPrefix("interface\\ifcommon\\lattice_window\\com_lattice_outline_");
        m_buffAcquiredOutline->SetGWndSize(latticeWidth + 6, latticeHeight + 4);
        m_buffAcquiredOutline->ShowGWnd(false);
    }

    // Vertex calc for lattices
    typedef void(__thiscall * VertexCalc_t)(void *);
    static VertexCalc_t vertexCalc = (VertexCalc_t) 0x56AFC0;

    // Left lattice grid (6 cols x 8 rows)
    sz.pos.x = leftPanelX + 12;
    sz.pos.y = panelY + 28;
    sz.size.width = latticeWidth;
    sz.size.height = latticeHeight;
    m_buffAcquiredLattice = (CIFLattice *) CGWnd::CreateInstance(AutoHuntSettingsMainFrame, GFX_RUNTIME_CLASS(CIFLattice), sz, 303, 0);
    if (m_buffAcquiredLattice) {
        m_buffAcquiredLattice->TB_Func_12("interface\\ifcommon\\lattice_window\\com_lattice_", 1, 0);
        m_buffAcquiredLattice->SetCornerSizes(32, 32);// Override texture-derived corner sizes
        m_buffAcquiredLattice->SetGWndSize(latticeWidth, latticeHeight);
        m_buffAcquiredLattice->SetGridSize(6, 8);
        m_buffAcquiredLattice->SetCellSize(32, 32);
        vertexCalc(m_buffAcquiredLattice);// Force vertex recalc
        m_buffAcquiredLattice->ShowGWnd(false);
    }

    // Right panel: Buffs to Use Frame
    sz.pos.x = rightPanelX;
    sz.pos.y = panelY;
    sz.size.width = panelWidth;
    sz.size.height = panelHeight;
    m_buffToUseFrame = (CIFFrame *) CGWnd::CreateInstance(AutoHuntSettingsMainFrame, GFX_RUNTIME_CLASS(CIFFrame), sz, 304, 0);
    if (m_buffToUseFrame) {
        m_buffToUseFrame->TB_Func_12("interface\\inventory\\int_window_", 1, 0);
        m_buffToUseFrame->SetGWndSize(panelWidth, panelHeight);
        m_buffToUseFrame->ShowGWnd(false);
    }

    // Right panel label
    sz.pos.x = rightPanelX + 10;
    sz.pos.y = panelY + 8;
    sz.size.width = 100;
    sz.size.height = 15;
    m_buffToUseLabel = (CIFStatic *) CGWnd::CreateInstance(AutoHuntSettingsMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 305, 0);
    if (m_buffToUseLabel) {
        m_buffToUseLabel->SetText("Buffs to Use");
        m_buffToUseLabel->SetTextColor(D3DCOLOR_XRGB(200, 200, 200));
        m_buffToUseLabel->ShowGWnd(false);
    }

    // Right lattice outline
    sz.pos.x = rightPanelX + 9;
    sz.pos.y = panelY + 26;
    sz.size.width = latticeWidth + 6;
    sz.size.height = latticeHeight + 4;
    m_buffToUseOutline = (CIFStretchWnd *) CGWnd::CreateInstance(AutoHuntSettingsMainFrame, GFX_RUNTIME_CLASS(CIFStretchWnd), sz, 312, 0);
    if (m_buffToUseOutline) {
        m_buffToUseOutline->LoadTexturesFromPrefix("interface\\ifcommon\\lattice_window\\com_lattice_outline_");
        m_buffToUseOutline->SetGWndSize(latticeWidth + 6, latticeHeight + 4);
        m_buffToUseOutline->ShowGWnd(false);
    }

    // Right lattice grid (6 cols x 8 rows)
    sz.pos.x = rightPanelX + 12;
    sz.pos.y = panelY + 28;
    sz.size.width = latticeWidth;
    sz.size.height = latticeHeight;
    m_buffToUseLattice = (CIFLattice *) CGWnd::CreateInstance(AutoHuntSettingsMainFrame, GFX_RUNTIME_CLASS(CIFLattice), sz, 306, 0);
    if (m_buffToUseLattice) {
        m_buffToUseLattice->TB_Func_12("interface\\ifcommon\\lattice_window\\com_lattice_", 1, 0);
        m_buffToUseLattice->SetCornerSizes(32, 32);// Override texture-derived corner sizes
        m_buffToUseLattice->SetGWndSize(latticeWidth, latticeHeight);
        m_buffToUseLattice->SetGridSize(6, 8);
        m_buffToUseLattice->SetCellSize(32, 32);
        vertexCalc(m_buffToUseLattice);// Force vertex recalc
        m_buffToUseLattice->ShowGWnd(false);
    }

    // ========== CREATE BUFF SKILL SLOTS ON TOP OF LATTICE ==========
    // Left panel slots (Acquired Buffs) - CIFSlotWithHelp with slot type 0x49 for DRAG SOURCE
    int leftSlotBaseX = leftPanelX + 12;
    int leftSlotBaseY = panelY + 28;

    for (int i = 0; i < SKILL_SLOT_COUNT; i++) {
        int col = i % SKILL_SLOT_COLS;
        int row = i / SKILL_SLOT_COLS;

        sz.pos.x = leftSlotBaseX + (col * SKILL_SLOT_SIZE);
        sz.pos.y = leftSlotBaseY + (row * SKILL_SLOT_SIZE);
        sz.size.width = SKILL_SLOT_SIZE;
        sz.size.height = SKILL_SLOT_SIZE;

        // Use CIFSlotWithHelp with slot type 0x49 for drag SOURCE
        m_buffAcquiredSlots[i] = (CIFSlotWithHelp *) CGWnd::CreateInstance(
            AutoHuntSettingsMainFrame, GFX_RUNTIME_CLASS(CIFSlotWithHelp), sz, 320 + i, 0);

        if (m_buffAcquiredSlots[i]) {
            m_buffAcquiredSlots[i]->InitDropSupport();
            m_buffAcquiredSlots[i]->InitRenderData(sz.pos.x, sz.pos.y, sz.pos.x + SKILL_SLOT_SIZE, sz.pos.y + SKILL_SLOT_SIZE);
            m_buffAcquiredSlots[i]->SetSlotIndex(i);
            m_buffAcquiredSlots[i]->SetSlotType(0x49);// Drag SOURCE type (like SkillBoard slots)
            m_buffAcquiredSlots[i]->ShowGWnd(false);
        }
    }

    // Right panel slots (Buffs to Use)
    int rightSlotBaseX = rightPanelX + 12;
    int rightSlotBaseY = panelY + 28;

    for (int i = 0; i < SKILL_SLOT_COUNT; i++) {
        int col = i % SKILL_SLOT_COLS;
        int row = i / SKILL_SLOT_COLS;

        sz.pos.x = rightSlotBaseX + (col * SKILL_SLOT_SIZE);
        sz.pos.y = rightSlotBaseY + (row * SKILL_SLOT_SIZE);
        sz.size.width = SKILL_SLOT_SIZE;
        sz.size.height = SKILL_SLOT_SIZE;

        m_buffToUseSlots[i] = (CIFSlotWithHelp *) CGWnd::CreateInstance(
            AutoHuntSettingsMainFrame, GFX_RUNTIME_CLASS(CIFSlotWithHelp), sz, 370 + i, 0);

        if (m_buffToUseSlots[i]) {
            m_buffToUseSlots[i]->InitDropSupport();// Initialize drag-drop support
            m_buffToUseSlots[i]->InitRenderData(sz.pos.x, sz.pos.y, sz.pos.x + SKILL_SLOT_SIZE, sz.pos.y + SKILL_SLOT_SIZE);
            m_buffToUseSlots[i]->SetSlotIndex(i);
            m_buffToUseSlots[i]->SetSlotType(0);// Basic type (testing)
            m_buffToUseSlots[i]->SetSlotEnabled(true);
            m_buffToUseSlots[i]->InitSlotIndex2(i);
            m_buffToUseSlots[i]->InitSlotParam(0);
            m_buffToUseSlots[i]->SetSlotItemID(-1);// Mark as empty slot
            m_buffToUseSlots[i]->ShowGWnd(false);
        }
    }


    // ========== VERTICAL SCROLLBAR FOR ACQUIRED BUFFS ==========
    sz.pos.x = leftPanelX + 12 + latticeWidth + 4;
    sz.pos.y = panelY + 44;  // Same as attack panel
    sz.size.width = 16;
    sz.size.height = 220;  // Fixed height to fit lattice
    
    m_buffAcquiredScroll = (CIFVerticalScroll *) CGWnd::CreateInstance(
        AutoHuntSettingsMainFrame, GFX_RUNTIME_CLASS(CIFVerticalScroll), sz, 315, 0);
    if (m_buffAcquiredScroll) {
        m_buffAcquiredScroll->TB_Func_12("interface\\ifcommon\\com_scroll_bar.ddj", 0, 0);
        m_buffAcquiredScroll->SetGWndSize(16, 220);
        m_buffAcquiredScroll->SetScrollBarRange(220, 0, 100, 1);
        m_buffAcquiredScroll->SetVCorrectScrollBar(0);
        
        CIFButton* upBtn = m_buffAcquiredScroll->GetUpButton();
        CIFButton* downBtn = m_buffAcquiredScroll->GetDownButton();
        if (upBtn) upBtn->SetGWndSize(16, 16);
        if (downBtn) downBtn->SetGWndSize(16, 16);
        
        m_buffAcquiredScroll->ShowGWnd(false);
    }
    
    m_buffToUseScroll = NULL;
}

void CIFAutoHuntSettings::ShowAutoHuntElements(bool show) {
    if (m_huntTabHeader) m_huntTabHeader->ShowGWnd(show);
    if (m_huntTabLabel) m_huntTabLabel->ShowGWnd(show);
    if (m_huntSection1Frame) m_huntSection1Frame->ShowGWnd(show);
    if (m_rangeTabHeader) m_rangeTabHeader->ShowGWnd(show);
    if (m_rangeTabLabel) m_rangeTabLabel->ShowGWnd(show);
    if (m_rangeSectionFrame) m_rangeSectionFrame->ShowGWnd(show);
    if (m_rangeSection2Frame) m_rangeSection2Frame->ShowGWnd(show);

    // Only Auto Berserk remains in main Hunt Settings
    if (m_chkAutoBerserk) m_chkAutoBerserk->ShowGWnd(show);
    if (m_lblAutoBerserk) m_lblAutoBerserk->ShowGWnd(show);
    
    // Go Back Center and Walk Around are in the second subframe
    if (m_chkGoBackCenter) m_chkGoBackCenter->ShowGWnd(show);
    if (m_lblGoBackCenter) m_lblGoBackCenter->ShowGWnd(show);
    if (m_chkWalkAround) m_chkWalkAround->ShowGWnd(show);
    if (m_lblWalkAround) m_lblWalkAround->ShowGWnd(show);

    // Range slider controls
    if (m_rangeSliderBg) m_rangeSliderBg->ShowGWnd(show);
    if (m_rangeSlider) m_rangeSlider->ShowGWnd(show);
    if (m_rangeValueText) m_rangeValueText->ShowGWnd(show);
}

void CIFAutoHuntSettings::ShowAttackSkillsElements(bool show) {
    if (m_atkInfoLabel) m_atkInfoLabel->ShowGWnd(show);
    if (m_atkAcquiredFrame) m_atkAcquiredFrame->ShowGWnd(show);
    if (m_atkAcquiredLabel) m_atkAcquiredLabel->ShowGWnd(show);
    if (m_atkAcquiredOutline) m_atkAcquiredOutline->ShowGWnd(show);
    if (m_atkAcquiredLattice) m_atkAcquiredLattice->ShowGWnd(show);
    if (m_atkAcquiredScroll) m_atkAcquiredScroll->ShowGWnd(show);
    if (m_atkToUseFrame) m_atkToUseFrame->ShowGWnd(show);
    if (m_atkToUseLabel) m_atkToUseLabel->ShowGWnd(show);
    if (m_atkToUseOutline) m_atkToUseOutline->ShowGWnd(show);
    if (m_atkToUseLattice) m_atkToUseLattice->ShowGWnd(show);
    if (m_atkToUseScroll) m_atkToUseScroll->ShowGWnd(show);

    // Show/hide skill slots based on scroll offset
    int startIndex = m_atkScrollOffset * SKILL_SLOT_COLS;
    for (int i = 0; i < SKILL_SLOT_COUNT; i++) {
        int skillIndex = startIndex + i;
        bool hasSkill = skillIndex < (int)m_allAtkSkills.size();
        if (m_atkAcquiredSlots[i]) {
            m_atkAcquiredSlots[i]->ShowGWnd(show && hasSkill);
        }
        // Skills to Use: ALWAYS show (for drop targets)
        if (m_atkToUseSlots[i]) {
            m_atkToUseSlots[i]->ShowGWnd(show);
        }
    }
}

void CIFAutoHuntSettings::ShowBuffSkillsElements(bool show) {
    if (m_buffInfoLabel) m_buffInfoLabel->ShowGWnd(show);
    if (m_buffAcquiredFrame) m_buffAcquiredFrame->ShowGWnd(show);
    if (m_buffAcquiredLabel) m_buffAcquiredLabel->ShowGWnd(show);
    if (m_buffAcquiredOutline) m_buffAcquiredOutline->ShowGWnd(show);
    if (m_buffAcquiredLattice) m_buffAcquiredLattice->ShowGWnd(show);
    if (m_buffAcquiredScroll) m_buffAcquiredScroll->ShowGWnd(show);
    if (m_buffToUseFrame) m_buffToUseFrame->ShowGWnd(show);
    if (m_buffToUseLabel) m_buffToUseLabel->ShowGWnd(show);
    if (m_buffToUseOutline) m_buffToUseOutline->ShowGWnd(show);
    if (m_buffToUseLattice) m_buffToUseLattice->ShowGWnd(show);
    if (m_buffToUseScroll) m_buffToUseScroll->ShowGWnd(show);

    // Show/hide skill slots based on scroll offset
    int startIndex = m_buffScrollOffset * SKILL_SLOT_COLS;
    for (int i = 0; i < SKILL_SLOT_COUNT; i++) {
        int skillIndex = startIndex + i;
        bool hasSkill = skillIndex < (int)m_allBuffSkills.size();
        if (m_buffAcquiredSlots[i]) {
            m_buffAcquiredSlots[i]->ShowGWnd(show && hasSkill);
        }
        // Buffs to Use: ALWAYS show (for drop targets)
        if (m_buffToUseSlots[i]) {
            m_buffToUseSlots[i]->ShowGWnd(show);
        }
    }
}

void CIFAutoHuntSettings::SwitchToTab(int tabIndex) {
    m_activeTab = tabIndex;
    UpdateTabButtons();

    // Populate skill slots FIRST before showing elements (so cached counts are set)
    if (tabIndex == TAB_AUTO_ATTACK_SKILLS || tabIndex == TAB_AUTO_BUFF_SKILLS) {
        PopulateLearnedSkills();
    }

    // Show elements AFTER populating (so slot visibility uses correct cached counts)
    ShowAutoHuntElements(tabIndex == TAB_AUTO_HUNT);
    ShowAttackSkillsElements(tabIndex == TAB_AUTO_ATTACK_SKILLS);
    ShowBuffSkillsElements(tabIndex == TAB_AUTO_BUFF_SKILLS);
}

void CIFAutoHuntSettings::UpdateTabButtons() {
    if (m_tabAutoHunt) {
        m_tabAutoHunt->TB_Func_12(m_activeTab == TAB_AUTO_HUNT
                                      ? "interface\\ifcommon\\com_long_tab_on.ddj"
                                      : "interface\\ifcommon\\com_long_tab_off.ddj",
                                  0, 0);
    }
    if (m_tabAttackSkills) {
        m_tabAttackSkills->TB_Func_12(m_activeTab == TAB_AUTO_ATTACK_SKILLS
                                          ? "interface\\ifcommon\\com_long_tab_on.ddj"
                                          : "interface\\ifcommon\\com_long_tab_off.ddj",
                                      0, 0);
    }
    if (m_tabBuffSkills) {
        m_tabBuffSkills->TB_Func_12(m_activeTab == TAB_AUTO_BUFF_SKILLS
                                        ? "interface\\ifcommon\\com_long_tab_on.ddj"
                                        : "interface\\ifcommon\\com_long_tab_off.ddj",
                                    0, 0);
    }
}

// Helper: Update child window absolute position and recalculate vertices
// IDA Analysis: Child windows store coords at 0x3C (X), 0x40 (Y)
// vertexCalc (sub_56AFC0) reads these to calculate screen vertices
static void UpdateChildAndRecalc(void *child, int parentX, int parentY, int relX, int relY) {
    if (!child) return;

    // Update child's absolute screen position
    *reinterpret_cast<int *>(reinterpret_cast<char *>(child) + 0x3C) = parentX + relX;
    *reinterpret_cast<int *>(reinterpret_cast<char *>(child) + 0x40) = parentY + relY;

    // Call native vertexCalc (sub_56AFC0)
    typedef void(__thiscall * VertexCalc_t)(void *);
    static VertexCalc_t vertexCalc = (VertexCalc_t) 0x56AFC0;
    vertexCalc(child);
}

void CIFAutoHuntSettings::OnUpdate() {
    if (!AutoHuntSettingsMainFrame || !AutoHuntSettingsMainFrame->IsVisible()) return;

    // ========== RANGE SLIDER SYNC ==========
    // Read slider value and update text display
    if (m_rangeSlider && m_rangeValueText) {
        int sliderValue = m_rangeSlider->Get2EC();  // Get current slider position
        if (sliderValue != m_rangeValue) {
            m_rangeValue = sliderValue;
            m_rangeValueText->SetTextFormatted("%d", m_rangeValue);
            printf("[AutoHunt] Range changed: %d (game units: %d)\n", m_rangeValue, m_rangeValue * 10);
        }
    }

    // ========== DUPLICATE CLEANUP: Remove any duplicates in Skills to Use ==========
    // Native handlers may create duplicates - clean them up each frame
    // Attack skills
    for (int i = 0; i < SKILL_SLOT_COUNT; i++) {
        if (m_atkSelectedSkillIds[i] == 0) continue;
        for (int j = 0; j < i; j++) {
            if (m_atkSelectedSkillIds[j] == m_atkSelectedSkillIds[i]) {
                m_atkSelectedSkillIds[i] = 0;
                if (m_atkToUseSlots[i]) {
                    m_atkToUseSlots[i]->ClearIcon();
                    m_atkToUseSlots[i]->SetSlotItemID(0);
                }
                break;
            }
        }
    }
    // Buff skills
    for (int i = 0; i < SKILL_SLOT_COUNT; i++) {
        if (m_buffSelectedSkillIds[i] == 0) continue;
        for (int j = 0; j < i; j++) {
            if (m_buffSelectedSkillIds[j] == m_buffSelectedSkillIds[i]) {
                m_buffSelectedSkillIds[i] = 0;
                if (m_buffToUseSlots[i]) {
                    m_buffToUseSlots[i]->ClearIcon();
                    m_buffToUseSlots[i]->SetSlotItemID(0);
                }
                break;
            }
        }
    }

    // ========== SCROLL HANDLING FOR ACQUIRED SKILLS PANELS ==========
    // Check if we're on a skill tab
    bool isAttackTab = (m_activeTab == TAB_AUTO_ATTACK_SKILLS);
    bool isBuffTab = (m_activeTab == TAB_AUTO_BUFF_SKILLS);
    
    if (isAttackTab || isBuffTab) {
        // Layout constants
        const int totalContentWidth = WINDOW_WIDTH - (CONTENT_MARGIN * 2) - 30;
        const int panelWidth = (totalContentWidth - PANEL_GAP) / 2;
        const int leftPanelX = CONTENT_MARGIN + 15;
        const int panelY = 110;
        const int latticeWidth = SKILL_SLOT_COLS * SKILL_SLOT_SIZE;
        const int latticeHeight = SKILL_SLOT_ROWS * SKILL_SLOT_SIZE;
        
        // Scrollbar detection area - matches actual scrollbar position
        // Scrollbar is at: X = leftPanelX + 12 + latticeWidth + 8, Y = panelY + 28
        // Size: 16 x latticeHeight (256)
        int scrollX = leftPanelX + 12 + latticeWidth + 6;  // Slightly left of actual scrollbar for easier clicks
        int scrollY = panelY + 28;  // Same as scrollbar Y position
        int scrollWidth = 20;  // Wider detection area
        int scrollHeight = 220;  // Match scrollbar height (256)
        
        // Get mouse position in window-local coords
        POINT scrollPt;
        GetCursorPos(&scrollPt);
        extern HWND g_hMainWnd;
        if (g_hMainWnd) {
            ScreenToClient(g_hMainWnd, &scrollPt);
            int scrollFx = *reinterpret_cast<int *>(reinterpret_cast<char *>(AutoHuntSettingsMainFrame) + 0x3C);
            int scrollFy = *reinterpret_cast<int *>(reinterpret_cast<char *>(AutoHuntSettingsMainFrame) + 0x40);
            int scrollLx = scrollPt.x - scrollFx;
            int scrollLy = scrollPt.y - scrollFy;
            
            // Check for click on scrollbar up/down buttons
            static bool wasScrollClick = false;
            static DWORD lastScrollTime = 0;
            bool isScrollMouseDown = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
            DWORD scrollNow = GetTickCount();
            
            // Calculate max scroll offset (in rows)
            std::vector<CachedSkillInfo>& skillVec = isAttackTab ? m_allAtkSkills : m_allBuffSkills;
            int& scrollOffset = isAttackTab ? m_atkScrollOffset : m_buffScrollOffset;
            int totalSkills = (int)skillVec.size();
            int totalRows = (totalSkills + SKILL_SLOT_COLS - 1) / SKILL_SLOT_COLS;  // Ceiling div
            int maxOffset = (totalRows > SKILL_SLOT_ROWS) ? (totalRows - SKILL_SLOT_ROWS) : 0;
            
            // Scrollbar click detection
            if (scrollLx >= scrollX && scrollLx < scrollX + scrollWidth) {
                if (isScrollMouseDown && !wasScrollClick && (scrollNow - lastScrollTime) > 150) {
                    lastScrollTime = scrollNow;
                    wasScrollClick = true;
                    
                    // Up button (top 16 pixels of scrollbar)
                    if (scrollLy >= scrollY && scrollLy < scrollY + 16) {
                        if (scrollOffset > 0) {
                            scrollOffset--;
                        }
                    }
                    // Down button (bottom 16 pixels of scrollbar)
                    else if (scrollLy >= scrollY + scrollHeight - 16 && scrollLy < scrollY + scrollHeight) {
                        if (scrollOffset < maxOffset) {
                            scrollOffset++;
                        }
                    }
                    // Track area - page up/down
                    else if (scrollLy >= scrollY + 16 && scrollLy < scrollY + scrollHeight - 16) {
                        int midY = scrollY + scrollHeight / 2;
                        if (scrollLy < midY) {
                            scrollOffset = max(0, scrollOffset - SKILL_SLOT_ROWS);
                        } else {
                            scrollOffset = min(maxOffset, scrollOffset + SKILL_SLOT_ROWS);
                        }
                    }
                }
            }
            
            if (!isScrollMouseDown) {
                wasScrollClick = false;
            }
            
            // ⭐ Read native scrollbar current position and sync scroll offset
            // Now using CIFVerticalScroll directly (not via CIFScrollManager)
            CIFVerticalScroll* scrollbar = isAttackTab ? m_atkAcquiredScroll : m_buffAcquiredScroll;
            if (scrollbar && maxOffset > 0) {
                // Read current position that sub_427240 sets at +0x2EC
                int nativePosition = scrollbar->GetCurrentPosition();
                
                // Clamp to valid range
                if (nativePosition < 0) nativePosition = 0;
                if (nativePosition > maxOffset) nativePosition = maxOffset;
                
                // If native position changed, update our scroll offset
                if (nativePosition != scrollOffset) {
                    scrollOffset = nativePosition;
                }
            }
            
            // Re-render visible slots if scroll offset changed
            int& lastOffset = isAttackTab ? m_lastAtkScrollOffset : m_lastBuffScrollOffset;
            if (scrollOffset != lastOffset) {
                lastOffset = scrollOffset;
                
                // Update scrollbar using IDA-verified offsets
                // scrollbar is already declared above, reuse it
                if (scrollbar && maxOffset > 0) {
                    // IDA sub_427240: thumbOffset = (pos - min) * height / range
                    // So we must set: max (+0x2E0) and range (+0x2E4) for proper calculation
                    scrollbar->SetMaxValue(maxOffset);       // +0x2E0
                    scrollbar->SetRange_Direct(maxOffset);   // +0x2E4 = range (max - min = maxOffset - 0)
                    
                    // ⭐ SetVCorrectScrollBar (sub_427240) syncs thumb based on position
                    scrollbar->SetVCorrectScrollBar(scrollOffset);
                }
                
                // Update visible slots
                CIFSlotWithHelp** slots = isAttackTab ? m_atkAcquiredSlots : m_buffAcquiredSlots;
                int startIndex = scrollOffset * SKILL_SLOT_COLS;
                
                for (int i = 0; i < SKILL_SLOT_COUNT; i++) {
                    int skillIndex = startIndex + i;
                    if (slots[i]) {
                        if (skillIndex < (int)skillVec.size()) {
                            slots[i]->SetSlotItemID(skillVec[skillIndex].skillId);
                            slots[i]->TB_Func_12(skillVec[skillIndex].iconPath, 0, 0);
                            slots[i]->ShowGWnd(true);
                        } else {
                            slots[i]->SetSlotItemID(0);
                            slots[i]->ClearIcon();
                            slots[i]->ShowGWnd(false);
                        }
                    }
                }
            }
        }
    }

    // Get current parent window position
    int curX = *reinterpret_cast<int *>(reinterpret_cast<char *>(AutoHuntSettingsMainFrame) + 0x3C);
    int curY = *reinterpret_cast<int *>(reinterpret_cast<char *>(AutoHuntSettingsMainFrame) + 0x40);

    if (curX != m_lastWindowX || curY != m_lastWindowY) {
        m_lastWindowX = curX;
        m_lastWindowY = curY;

        // Layout constants (same as in CreateAttackSkillsPanel/CreateBuffSkillsPanel)
        const int totalContentWidth = WINDOW_WIDTH - (CONTENT_MARGIN * 2) - 30;
        const int panelWidth = (totalContentWidth - PANEL_GAP) / 2;
        const int panelHeight = 300;
        const int leftPanelX = CONTENT_MARGIN + 15;
        const int rightPanelX = leftPanelX + panelWidth + PANEL_GAP;
        const int panelY = 110;
        // Lattice must be exact multiple of cell size (32px)
        const int latticeWidth = SKILL_SLOT_COLS * SKILL_SLOT_SIZE; // 192
        const int latticeHeight = SKILL_SLOT_ROWS * SKILL_SLOT_SIZE;// 256

        // Recalc frame panels (CIFFrame - CIFTileWnd derivative)
        // These use their own internal relative positions, just call vertexCalc
        typedef void(__thiscall * VertexCalc_t)(void *);
        static VertexCalc_t vertexCalc = (VertexCalc_t) 0x56AFC0;

        if (m_huntSection1Frame) vertexCalc(m_huntSection1Frame);
        if (m_rangeSectionFrame) vertexCalc(m_rangeSectionFrame);
        if (m_atkAcquiredFrame) vertexCalc(m_atkAcquiredFrame);
        if (m_atkToUseFrame) vertexCalc(m_atkToUseFrame);
        if (m_buffAcquiredFrame) vertexCalc(m_buffAcquiredFrame);
        if (m_buffToUseFrame) vertexCalc(m_buffToUseFrame);

        // Fix CIFStretchWnd outline positions and recalc (relative positions from CreateAttackSkillsPanel)
        // Outline: leftPanelX + 9, panelY + 26 | rightPanelX + 9, panelY + 26
        UpdateChildAndRecalc(m_atkAcquiredOutline, curX, curY, leftPanelX + 9, panelY + 26);
        UpdateChildAndRecalc(m_atkToUseOutline, curX, curY, rightPanelX + 9, panelY + 26);
        UpdateChildAndRecalc(m_buffAcquiredOutline, curX, curY, leftPanelX + 9, panelY + 26);
        UpdateChildAndRecalc(m_buffToUseOutline, curX, curY, rightPanelX + 9, panelY + 26);

        // Fix CIFLattice positions and recalc (relative positions from CreateAttackSkillsPanel)
        // Lattice: leftPanelX + 12, panelY + 28 | rightPanelX + 12, panelY + 28
        UpdateChildAndRecalc(m_atkAcquiredLattice, curX, curY, leftPanelX + 12, panelY + 28);
        UpdateChildAndRecalc(m_atkToUseLattice, curX, curY, rightPanelX + 12, panelY + 28);
        UpdateChildAndRecalc(m_buffAcquiredLattice, curX, curY, leftPanelX + 12, panelY + 28);
        UpdateChildAndRecalc(m_buffToUseLattice, curX, curY, rightPanelX + 12, panelY + 28);
    }

    // ========== MANUAL DRAG/DROP DETECTION ==========
    // Native drag-drop system doesn't work for our custom slots, so we implement our own
    static bool wasMouseDown = false;
    static DWORD lastClickTime = 0;
    static int mouseDownX = 0, mouseDownY = 0;  // Track where mouse was pressed
    bool isMouseDown = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;

    DWORD now = GetTickCount();

    // Get cursor position in window-local coordinates
    POINT pt;
    GetCursorPos(&pt);
    extern HWND g_hMainWnd;
    int lx = 0, ly = 0;
    if (g_hMainWnd) {
        ScreenToClient(g_hMainWnd, &pt);
        int fx = *reinterpret_cast<int *>(reinterpret_cast<char *>(AutoHuntSettingsMainFrame) + 0x3C);
        int fy = *reinterpret_cast<int *>(reinterpret_cast<char *>(AutoHuntSettingsMainFrame) + 0x40);
        lx = pt.x - fx;
        ly = pt.y - fy;
    }

    // Layout constants for hit testing
    const int totalContentWidth = WINDOW_WIDTH - (CONTENT_MARGIN * 2) - 30;
    const int panelWidth = (totalContentWidth - PANEL_GAP) / 2;
    const int leftPanelX = CONTENT_MARGIN + 15;
    const int rightPanelX = leftPanelX + panelWidth + PANEL_GAP;
    const int panelY = 110;
    const int leftSlotBaseX = leftPanelX + 12;
    const int leftSlotBaseY = panelY + 28;
    const int rightSlotBaseX = rightPanelX + 12;
    const int rightSlotBaseY = panelY + 28;
    const int gridWidth = SKILL_SLOT_COLS * SKILL_SLOT_SIZE;
    const int gridHeight = SKILL_SLOT_ROWS * SKILL_SLOT_SIZE;


    // ========== MOUSE DOWN: Start drag from Acquired Skills OR Skills to Use ==========
    if (isMouseDown && !wasMouseDown && (now - lastClickTime) > 100) {
        lastClickTime = now;
        // If drag is already active (click-to-pick pattern), don't start new drag
        // Also DON'T update mouseDownX/Y - keep original position for distance check
        if (m_isDragging && m_dragSkillId != 0) {
            goto skip_drag_start;
        }
        
        // Only save position for NEW drags
        mouseDownX = lx;
        mouseDownY = ly;

        // Only on Attack Skills or Buff Skills tab
        bool isAttackTab = (m_activeTab == TAB_AUTO_ATTACK_SKILLS);
        bool isBuffTab = (m_activeTab == TAB_AUTO_BUFF_SKILLS);

        if (isAttackTab || isBuffTab) {
            // Check if click is in left panel (Acquired Skills) grid area
            if (lx >= leftSlotBaseX && lx < leftSlotBaseX + gridWidth &&
                ly >= leftSlotBaseY && ly < leftSlotBaseY + gridHeight) {

                int col = (lx - leftSlotBaseX) / SKILL_SLOT_SIZE;
                int row = (ly - leftSlotBaseY) / SKILL_SLOT_SIZE;
                int slotIndex = row * SKILL_SLOT_COLS + col;

                if (slotIndex >= 0 && slotIndex < SKILL_SLOT_COUNT) {
                    // Get skill data from vector with scroll offset
                    std::vector<CachedSkillInfo>& skillVec = isAttackTab ? m_allAtkSkills : m_allBuffSkills;
                    int scrollOffset = isAttackTab ? m_atkScrollOffset : m_buffScrollOffset;
                    int skillIndex = scrollOffset * SKILL_SLOT_COLS + slotIndex;

                    if (skillIndex < (int)skillVec.size() && skillVec[skillIndex].skillId != 0) {
                        // Start drag from Acquired Skills
                        m_isDragging = true;
                        m_dragSourceSlotIndex = slotIndex;
                        m_dragSkillId = skillVec[skillIndex].skillId;
                        strncpy(m_dragIconPath, skillVec[skillIndex].iconPath, 259);
                        m_dragIconPath[259] = '\0';
                        m_dragIsAttackSkill = isAttackTab;
                        m_dragFromToUsePanel = false;  // From Acquired, not ToUse
                    }
                }
            }
            // Check if click is in right panel (Skills to Use) grid area
            else if (lx >= rightSlotBaseX && lx < rightSlotBaseX + gridWidth &&
                     ly >= rightSlotBaseY && ly < rightSlotBaseY + gridHeight) {

                int col = (lx - rightSlotBaseX) / SKILL_SLOT_SIZE;
                int row = (ly - rightSlotBaseY) / SKILL_SLOT_SIZE;
                int slotIndex = row * SKILL_SLOT_COLS + col;

                if (slotIndex >= 0 && slotIndex < SKILL_SLOT_COUNT) {
                    // Get selected skill IDs
                    DWORD *selectedIds = isAttackTab ? m_atkSelectedSkillIds : m_buffSelectedSkillIds;
                    CIFSlotWithHelp **toUseSlots = isAttackTab ? m_atkToUseSlots : m_buffToUseSlots;
                    if (selectedIds[slotIndex] != 0 && toUseSlots[slotIndex]) {
                        // Start drag from Skills to Use (for removal)
                        m_isDragging = true;
                        m_dragSourceSlotIndex = slotIndex;
                        m_dragSkillId = selectedIds[slotIndex];
                        
                        // Get icon path from skill vector
                        std::vector<CachedSkillInfo>& skillVec = isAttackTab ? m_allAtkSkills : m_allBuffSkills;
                        m_dragIconPath[0] = '\0';
                        for (size_t j = 0; j < skillVec.size(); j++) {
                            if (skillVec[j].skillId == m_dragSkillId) {
                                strncpy(m_dragIconPath, skillVec[j].iconPath, 259);
                                m_dragIconPath[259] = '\0';
                                break;
                            }
                        }
                        
                        m_dragIsAttackSkill = isAttackTab;
                        m_dragFromToUsePanel = true;  // From ToUse panel (for removal)

                        
                    }
                }
            }
        }
    }

skip_drag_start:
    // ========== MOUSE UP: Handle drop or regular click ==========
    if (wasMouseDown && !isMouseDown) {
        // Check if we were dragging - handle drop
        if (m_isDragging && m_dragSkillId != 0) {
            // Only apply movement threshold for ToUse panel drags (skill removal)
            // Acquired -> ToUse drops should always work without distance check
            if (m_dragFromToUsePanel) {
                int dx = lx - mouseDownX;
                int dy = ly - mouseDownY;
                int distSq = dx * dx + dy * dy;
                const int MIN_DRAG_DIST_SQ = 100;  // 10 pixels squared
                
                if (distSq < MIN_DRAG_DIST_SQ) {
                    // User just clicked on ToUse skill (same position)
                    // Keep drag ACTIVE for click-to-pick, click-to-drop pattern
                    goto end_mouse_up;  // Skip this mouse up, wait for next click
                }
            }
            
            bool isAttackTab = (m_activeTab == TAB_AUTO_ATTACK_SKILLS);
            bool isBuffTab = (m_activeTab == TAB_AUTO_BUFF_SKILLS);
            // Only drop if on same tab type as drag source
            if ((isAttackTab && m_dragIsAttackSkill) || (isBuffTab && !m_dragIsAttackSkill)) {
                // Check if drop is in right panel (Skills to Use) grid area
                if (lx >= rightSlotBaseX && lx < rightSlotBaseX + gridWidth &&
                    ly >= rightSlotBaseY && ly < rightSlotBaseY + gridHeight) {

                    int col = (lx - rightSlotBaseX) / SKILL_SLOT_SIZE;
                    int row = (ly - rightSlotBaseY) / SKILL_SLOT_SIZE;
                    int targetSlotIndex = row * SKILL_SLOT_COLS + col;

                    if (targetSlotIndex >= 0 && targetSlotIndex < SKILL_SLOT_COUNT) {
                        // Get selected skills array
                        DWORD *selectedIds = isAttackTab ? m_atkSelectedSkillIds : m_buffSelectedSkillIds;
                        int *selectedCount = isAttackTab ? &m_atkSelectedCount : &m_buffSelectedCount;
                        CIFSlotWithHelp **targetSlots = isAttackTab ? m_atkToUseSlots : m_buffToUseSlots;

                        // CHECK FOR DUPLICATE: Skip if skill already exists in panel (unless same slot)
                        bool isDuplicate = false;
                        int existingSlot = -1;
                        for (int j = 0; j < SKILL_SLOT_COUNT; j++) {
                            if (selectedIds[j] == m_dragSkillId && j != targetSlotIndex) {
                                isDuplicate = true;
                                existingSlot = j;
                                break;
                            }
                        }
                        
                        if (isDuplicate) {
                            // If dragging from ToUse panel (reordering), clear the source slot
                            if (m_dragFromToUsePanel && m_dragSourceSlotIndex >= 0) {
                                selectedIds[m_dragSourceSlotIndex] = 0;
                                if (targetSlots[m_dragSourceSlotIndex]) {
                                    targetSlots[m_dragSourceSlotIndex]->ClearIcon();
                                    targetSlots[m_dragSourceSlotIndex]->TB_Func_12("", 0, 0);  // Clear TextBoard layer
                                    targetSlots[m_dragSourceSlotIndex]->SetSlotItemID(0);
                                }
                            } else {
                                // Reset drag and skip drop
                                m_isDragging = false;
                                m_dragSourceSlotIndex = -1;
                                m_dragSkillId = 0;
                                m_dragIconPath[0] = '\0';
                                m_dragFromToUsePanel = false;
                                goto end_mouse_up;  // Skip the drop
                            }
                        }

                        if (targetSlots[targetSlotIndex]) {
                            // Set icon on target slot
                            targetSlots[targetSlotIndex]->TB_Func_12(m_dragIconPath, 0, 0);
                            targetSlots[targetSlotIndex]->SetSlotItemID(m_dragSkillId);

                            selectedIds[targetSlotIndex] = m_dragSkillId;
                            
                            // If moving from ToUse panel to a DIFFERENT slot, clear the source
                            if (m_dragFromToUsePanel && m_dragSourceSlotIndex >= 0 && 
                                m_dragSourceSlotIndex != targetSlotIndex) {
                                selectedIds[m_dragSourceSlotIndex] = 0;
                                if (targetSlots[m_dragSourceSlotIndex]) {
                                    targetSlots[m_dragSourceSlotIndex]->ClearIcon();
                                    targetSlots[m_dragSourceSlotIndex]->TB_Func_12("", 0, 0);
                                    targetSlots[m_dragSourceSlotIndex]->SetSlotItemID(0);
                                }
                            }
                            
                            if (targetSlotIndex >= *selectedCount) {
                                *selectedCount = targetSlotIndex + 1;
                            }
                            // Only reset drag on SUCCESSFUL drop
                            m_isDragging = false;
                            m_dragSourceSlotIndex = -1;
                            m_dragSkillId = 0;
                            m_dragIconPath[0] = '\0';
                            m_dragFromToUsePanel = false;
                        }
                    }
                }
            }

            // Check if released OUTSIDE our window - remove skill if from ToUse panel
            if (lx < 0 || lx > WINDOW_WIDTH || ly < 0 || ly > WINDOW_HEIGHT) {
                if (m_dragFromToUsePanel) {
                    // Remove skill from Skills to Use panel
                    DWORD *selectedIds = m_dragIsAttackSkill ? m_atkSelectedSkillIds : m_buffSelectedSkillIds;
                    CIFSlotWithHelp **toUseSlots = m_dragIsAttackSkill ? m_atkToUseSlots : m_buffToUseSlots;
                    
                    // Clear the slot
                    if (m_dragSourceSlotIndex >= 0 && m_dragSourceSlotIndex < SKILL_SLOT_COUNT) {
                        selectedIds[m_dragSourceSlotIndex] = 0;
                        if (toUseSlots[m_dragSourceSlotIndex]) {
                            // Clear BOTH icon layers - slot icon and TextBoard icon
                            toUseSlots[m_dragSourceSlotIndex]->ClearIcon();
                            toUseSlots[m_dragSourceSlotIndex]->TB_Func_12("", 0, 0);  // Clear TextBoard layer
                            toUseSlots[m_dragSourceSlotIndex]->SetSlotItemID(0);
                        }
                    }
                }                
                // Reset drag state
                m_isDragging = false;
                m_dragSourceSlotIndex = -1;
                m_dragSkillId = 0;
                m_dragIconPath[0] = '\0';
                m_dragFromToUsePanel = false;
            }
        }

        // ========== Regular click handling (tabs, buttons, etc.) ==========
        // Tab button click areas
        int tabY = 43, tabHeight = 24, tabWidth = 100, tabSpacing = 102;

        if (ly >= tabY && ly < tabY + tabHeight) {
            if (lx >= CONTENT_MARGIN && lx < CONTENT_MARGIN + tabWidth) {
                SwitchToTab(TAB_AUTO_HUNT);
            } else if (lx >= CONTENT_MARGIN + tabSpacing && lx < CONTENT_MARGIN + tabSpacing + tabWidth) {
                SwitchToTab(TAB_AUTO_ATTACK_SKILLS);
            } else if (lx >= CONTENT_MARGIN + tabSpacing * 2 && lx < CONTENT_MARGIN + tabSpacing * 2 + tabWidth) {
                SwitchToTab(TAB_AUTO_BUFF_SKILLS);
            }
        }

        // Slider now handles range - no click handling needed here

        // Buttons - match coordinates from OnCreate
        int btnWidth = 76;
        int btnGap = 8;
        int totalBtnWidth = btnWidth * 2 + btnGap;
        int btnStartX = (WINDOW_WIDTH - totalBtnWidth) / 2;
        int btnY = WINDOW_HEIGHT - 35;

        if (ly >= btnY && ly < btnY + 24) {
            if (lx >= btnStartX && lx < btnStartX + btnWidth) {
                SaveConfig();
                AutoHuntSettingsMainFrame->ShowGWnd(false);
            } else if (lx >= btnStartX + btnWidth + btnGap && lx < btnStartX + totalBtnWidth) {
                AutoHuntSettingsMainFrame->ShowGWnd(false);
            }
        }
    }

end_mouse_up:
    wasMouseDown = isMouseDown;
}

int CIFAutoHuntSettings::OnMouseMove(int a1, int x, int y) { return 0; }

void CIFAutoHuntSettings::ResetPosition() {
    if (!AutoHuntSettingsMainFrame) return;
    int sw = g_D3DViewportWidth > 0 ? g_D3DViewportWidth : GetSystemMetrics(SM_CXSCREEN);
    int sh = g_D3DViewportHeight > 0 ? g_D3DViewportHeight : GetSystemMetrics(SM_CYSCREEN);
    AutoHuntSettingsMainFrame->MoveGWnd((sw - WINDOW_WIDTH) / 2, (sh - WINDOW_HEIGHT) / 4);
}

bool CIFAutoHuntSettings::IsAutoBerserkChecked() const { return m_chkAutoBerserk && m_chkAutoBerserk->GetCheckedState_MAYBE(); }
bool CIFAutoHuntSettings::IsReturnToTownOnDeathChecked() const { return m_chkReturnToTownOnDeath && m_chkReturnToTownOnDeath->GetCheckedState_MAYBE(); }
bool CIFAutoHuntSettings::IsTownOnLowHPChecked() const { return m_chkTownOnLowHP && m_chkTownOnLowHP->GetCheckedState_MAYBE(); }
bool CIFAutoHuntSettings::IsTownOnLowMPChecked() const { return m_chkTownOnLowMP && m_chkTownOnLowMP->GetCheckedState_MAYBE(); }
bool CIFAutoHuntSettings::IsTownOnLowPetHPChecked() const { return m_chkTownOnLowPetHP && m_chkTownOnLowPetHP->GetCheckedState_MAYBE(); }
bool CIFAutoHuntSettings::IsTownOnLowArrowChecked() const { return m_chkTownOnLowArrow && m_chkTownOnLowArrow->GetCheckedState_MAYBE(); }
bool CIFAutoHuntSettings::IsTownOnLowDurabilityChecked() const { return m_chkTownOnLowDurability && m_chkTownOnLowDurability->GetCheckedState_MAYBE(); }
bool CIFAutoHuntSettings::IsGoBackCenterChecked() const { return m_chkGoBackCenter && m_chkGoBackCenter->GetCheckedState_MAYBE(); }
bool CIFAutoHuntSettings::IsWalkAroundChecked() const { return m_chkWalkAround && m_chkWalkAround->GetCheckedState_MAYBE(); }

bool CIFAutoHuntSettings::SaveConfig() {
    std::string executableDir = GetAutoHuntExeDir();
    const char *charName = "Default";
    if (g_pCICPlayerEcsro && g_pCICPlayerEcsro->charname[0] != '\0') {
        charName = g_pCICPlayerEcsro->charname;
    }

    char buffer[MAX_PATH];
    sprintf_s(buffer, sizeof(buffer), "%s\\Setting\\AutoHunt_%s.dat", executableDir.c_str(), charName);
    char settingDir[MAX_PATH];
    sprintf_s(settingDir, sizeof(settingDir), "%s\\Setting", executableDir.c_str());
    _mkdir(settingDir);

    FILE *f = fopen(buffer, "wb+");
    if (!f) return false;

    // Version header for compatibility
    int version = 9;  // Version 9: added TownOnLowDurability checkbox
    fwrite(&version, sizeof(int), 1, f);

    // Checkbox data (9 checkboxes now)
    bool checkboxData[9] = {
        IsAutoBerserkChecked(), IsReturnToTownOnDeathChecked(), 
        IsTownOnLowHPChecked(), IsTownOnLowMPChecked(), IsTownOnLowPetHPChecked(),
        IsTownOnLowArrowChecked(), IsTownOnLowDurabilityChecked(), IsGoBackCenterChecked(), IsWalkAroundChecked()};
    fwrite(checkboxData, sizeof(bool), 9, f);
    fwrite(&m_rangeValue, sizeof(int), 1, f);  // Save slider value directly

    // Skill IDs (new in version 2)
    fwrite(&m_atkSelectedCount, sizeof(int), 1, f);
    fwrite(m_atkSelectedSkillIds, sizeof(DWORD), SKILL_SLOT_COUNT, f);
    fwrite(&m_buffSelectedCount, sizeof(int), 1, f);
    fwrite(m_buffSelectedSkillIds, sizeof(DWORD), SKILL_SLOT_COUNT, f);

    // ========== LOG SELECTED SKILLS WITH CHAIN IDs ==========
    typedef DWORD (__thiscall *GetSkillObj_t)(void*, DWORD);
    GetSkillObj_t GetSkillObj = (GetSkillObj_t)0x616790;
    void* pSkillMgr = (void*)0xA01010;
    
    printf("\n=== SELECTED ATTACK SKILLS (Chain IDs) ===\n");
    for (int i = 0; i < SKILL_SLOT_COUNT; i++) {
        DWORD skillId = m_atkSelectedSkillIds[i];
        if (skillId != 0) {
            DWORD chainId = 0;
            DWORD pSkillObj = GetSkillObj(pSkillMgr, skillId);
            if (pSkillObj) {
                chainId = *(DWORD*)(pSkillObj + 0x10C + 0x04);
            }
            printf("Attack Slot[%d]: Skill %d -> Chain %d\n", i, skillId, chainId);
        }
    }
    
    printf("\n=== SELECTED BUFF SKILLS (Chain IDs) ===\n");
    for (int i = 0; i < SKILL_SLOT_COUNT; i++) {
        DWORD skillId = m_buffSelectedSkillIds[i];
        if (skillId != 0) {
            DWORD chainId = 0;
            DWORD pSkillObj = GetSkillObj(pSkillMgr, skillId);
            if (pSkillObj) {
                chainId = *(DWORD*)(pSkillObj + 0x10C + 0x04);
            }
            printf("Buff Slot[%d]: Skill %d -> Chain %d\n", i, skillId, chainId);
        }
    }
    printf("=== END ===\n\n");
    fflush(stdout);

    fclose(f);
    return true;
}


bool CIFAutoHuntSettings::LoadConfig() {
    std::string executableDir = GetAutoHuntExeDir();
    const char *charName = "Default";
    if (g_pCICPlayerEcsro && g_pCICPlayerEcsro->charname[0] != '\0') {
        charName = g_pCICPlayerEcsro->charname;
    }

    char buffer[MAX_PATH];
    sprintf_s(buffer, sizeof(buffer), "%s\\Setting\\AutoHunt_%s.dat", executableDir.c_str(), charName);

    FILE *f = fopen(buffer, "rb");
    if (!f) {
        m_rangeValue = 100;  // Default 100 (= 1000 game units)
        if (m_rangeSlider) m_rangeSlider->Set2EC(m_rangeValue);
        if (m_rangeValueText) m_rangeValueText->SetTextFormatted("%d", m_rangeValue);
        // Set AutoBerserk default to true (ON)
        if (m_chkAutoBerserk) m_chkAutoBerserk->FUN_00656d50(true);
        // Set ReturnToTownOnDeath default to true (ON)
        if (m_chkReturnToTownOnDeath) m_chkReturnToTownOnDeath->FUN_00656d50(true);
        // Set WalkAround default to true (ON)
        if (m_chkWalkAround) m_chkWalkAround->FUN_00656d50(true);
        printf("[AutoHuntSettings] No config file found, using defaults (AutoBerserk=ON, ReturnToTown=ON, WalkAround=ON)\n");
        fflush(stdout);
        return true;
    }

    // Read version header
    int version = 0;
    if (fread(&version, sizeof(int), 1, f) != 1) {
        fclose(f);
        return false;
    }

    bool checkboxData[9];  // 9 checkboxes: AutoBerserk, ReturnToTownOnDeath, TownOnLowHP, TownOnLowMP, TownOnLowPetHP, TownOnLowArrow, TownOnLowDurability, GoBackCenter, WalkAround
    memset(checkboxData, 0, sizeof(checkboxData));
    checkboxData[8] = true;  // Default WalkAround to ON

    if (version == 1 || version == 2 || version == 3 || version == 4) {
        // Old versions - read old format then map to new
        bool oldData[10];
        memset(oldData, 0, sizeof(oldData));
        
        if (version == 1) {
            fread(oldData, sizeof(bool), 9, f);
            int rangeIndex = 0;
            fread(&rangeIndex, sizeof(int), 1, f);
            int indexToValue[] = {30, 50, 100, 150, 200};
            m_rangeValue = (rangeIndex >= 0 && rangeIndex < 5) ? indexToValue[rangeIndex] : 100;
        } else if (version == 2) {
            fread(oldData, sizeof(bool), 9, f);
            int rangeIndex = 0;
            fread(&rangeIndex, sizeof(int), 1, f);
            int indexToValue[] = {30, 50, 100, 150, 200};
            m_rangeValue = (rangeIndex >= 0 && rangeIndex < 5) ? indexToValue[rangeIndex] : 100;
            fread(&m_atkSelectedCount, sizeof(int), 1, f);
            fread(m_atkSelectedSkillIds, sizeof(DWORD), SKILL_SLOT_COUNT, f);
            fread(&m_buffSelectedCount, sizeof(int), 1, f);
            fread(m_buffSelectedSkillIds, sizeof(DWORD), SKILL_SLOT_COUNT, f);
        } else if (version == 3) {
            fread(oldData, sizeof(bool), 9, f);
            fread(&m_rangeValue, sizeof(int), 1, f);
            fread(&m_atkSelectedCount, sizeof(int), 1, f);
            fread(m_atkSelectedSkillIds, sizeof(DWORD), SKILL_SLOT_COUNT, f);
            fread(&m_buffSelectedCount, sizeof(int), 1, f);
            fread(m_buffSelectedSkillIds, sizeof(DWORD), SKILL_SLOT_COUNT, f);
        } else if (version == 4) {
            fread(oldData, sizeof(bool), 10, f);
            fread(&m_rangeValue, sizeof(int), 1, f);
            fread(&m_atkSelectedCount, sizeof(int), 1, f);
            fread(m_atkSelectedSkillIds, sizeof(DWORD), SKILL_SLOT_COUNT, f);
            fread(&m_buffSelectedCount, sizeof(int), 1, f);
            fread(m_buffSelectedSkillIds, sizeof(DWORD), SKILL_SLOT_COUNT, f);
        }
        
        // Map old data to new: [0]=AutoBerserk, [1]=ReturnToTownOnDeath, [2-4]=new (OFF), [5]=GoBackCenter, [6]=WalkAround
        checkboxData[0] = oldData[0];  // AutoBerserk
        checkboxData[1] = true;         // ReturnToTownOnDeath (new, default ON)
        checkboxData[2] = false;        // TownOnLowHP (new, default OFF)
        checkboxData[3] = false;        // TownOnLowMP (new, default OFF)
        checkboxData[4] = false;        // TownOnLowPetHP (new, default OFF)
        checkboxData[5] = oldData[1];  // GoBackCenter
        checkboxData[6] = (version == 4) ? oldData[9] : true;  // WalkAround (default ON for old versions)
        
        if (m_atkSelectedCount < 0 || m_atkSelectedCount > SKILL_SLOT_COUNT) m_atkSelectedCount = 0;
        if (m_buffSelectedCount < 0 || m_buffSelectedCount > SKILL_SLOT_COUNT) m_buffSelectedCount = 0;
    } else if (version == 5) {
        // Version 5: 3 checkboxes (AutoBerserk, GoBackCenter, WalkAround)
        bool oldCheckboxData[3];
        fread(oldCheckboxData, sizeof(bool), 3, f);
        fread(&m_rangeValue, sizeof(int), 1, f);
        fread(&m_atkSelectedCount, sizeof(int), 1, f);
        fread(m_atkSelectedSkillIds, sizeof(DWORD), SKILL_SLOT_COUNT, f);
        fread(&m_buffSelectedCount, sizeof(int), 1, f);
        fread(m_buffSelectedSkillIds, sizeof(DWORD), SKILL_SLOT_COUNT, f);
        
        // Map v5 to v7: add new checkboxes
        checkboxData[0] = oldCheckboxData[0];  // AutoBerserk
        checkboxData[1] = true;                 // ReturnToTownOnDeath (default ON)
        checkboxData[2] = false;                // TownOnLowHP (new, default OFF)
        checkboxData[3] = false;                // TownOnLowMP (new, default OFF)
        checkboxData[4] = false;                // TownOnLowPetHP (new, default OFF)
        checkboxData[5] = oldCheckboxData[1];  // GoBackCenter
        checkboxData[6] = oldCheckboxData[2];  // WalkAround
        
        if (m_atkSelectedCount < 0 || m_atkSelectedCount > SKILL_SLOT_COUNT) m_atkSelectedCount = 0;
        if (m_buffSelectedCount < 0 || m_buffSelectedCount > SKILL_SLOT_COUNT) m_buffSelectedCount = 0;
    } else if (version == 6) {
        // Version 6: 4 checkboxes (AutoBerserk, ReturnToTownOnDeath, GoBackCenter, WalkAround)
        bool oldCheckboxData[4];
        fread(oldCheckboxData, sizeof(bool), 4, f);
        fread(&m_rangeValue, sizeof(int), 1, f);
        fread(&m_atkSelectedCount, sizeof(int), 1, f);
        fread(m_atkSelectedSkillIds, sizeof(DWORD), SKILL_SLOT_COUNT, f);
        fread(&m_buffSelectedCount, sizeof(int), 1, f);
        fread(m_buffSelectedSkillIds, sizeof(DWORD), SKILL_SLOT_COUNT, f);
        
        // Map v6 to v7: add new checkboxes
        checkboxData[0] = oldCheckboxData[0];  // AutoBerserk
        checkboxData[1] = oldCheckboxData[1];  // ReturnToTownOnDeath
        checkboxData[2] = false;                // TownOnLowHP (new, default OFF)
        checkboxData[3] = false;                // TownOnLowMP (new, default OFF)
        checkboxData[4] = false;                // TownOnLowPetHP (new, default OFF)
        checkboxData[5] = oldCheckboxData[2];  // GoBackCenter
        checkboxData[6] = oldCheckboxData[3];  // WalkAround
        
        if (m_atkSelectedCount < 0 || m_atkSelectedCount > SKILL_SLOT_COUNT) m_atkSelectedCount = 0;
        if (m_buffSelectedCount < 0 || m_buffSelectedCount > SKILL_SLOT_COUNT) m_buffSelectedCount = 0;
    } else if (version == 7) {
        // Version 7: 7 checkboxes - map to 8 checkbox format
        bool oldData[7];
        fread(oldData, sizeof(bool), 7, f);
        fread(&m_rangeValue, sizeof(int), 1, f);
        fread(&m_atkSelectedCount, sizeof(int), 1, f);
        fread(m_atkSelectedSkillIds, sizeof(DWORD), SKILL_SLOT_COUNT, f);
        fread(&m_buffSelectedCount, sizeof(int), 1, f);
        fread(m_buffSelectedSkillIds, sizeof(DWORD), SKILL_SLOT_COUNT, f);
        
        // Map v7 to v8: Insert TownOnLowArrow=false at position 5
        checkboxData[0] = oldData[0];  // AutoBerserk
        checkboxData[1] = oldData[1];  // ReturnToTownOnDeath
        checkboxData[2] = oldData[2];  // TownOnLowHP
        checkboxData[3] = oldData[3];  // TownOnLowMP
        checkboxData[4] = oldData[4];  // TownOnLowPetHP
        checkboxData[5] = false;       // TownOnLowArrow (NEW - default disabled)
        checkboxData[6] = oldData[5];  // GoBackCenter
        checkboxData[7] = oldData[6];  // WalkAround
        
        if (m_atkSelectedCount < 0 || m_atkSelectedCount > SKILL_SLOT_COUNT) m_atkSelectedCount = 0;
        if (m_buffSelectedCount < 0 || m_buffSelectedCount > SKILL_SLOT_COUNT) m_buffSelectedCount = 0;
    } else if (version == 8) {
        // Version 8: 8 checkboxes - migrate to 9 checkbox format
        bool oldData[8];
        fread(oldData, sizeof(bool), 8, f);
        fread(&m_rangeValue, sizeof(int), 1, f);
        fread(&m_atkSelectedCount, sizeof(int), 1, f);
        fread(m_atkSelectedSkillIds, sizeof(DWORD), SKILL_SLOT_COUNT, f);
        fread(&m_buffSelectedCount, sizeof(int), 1, f);
        fread(m_buffSelectedSkillIds, sizeof(DWORD), SKILL_SLOT_COUNT, f);
        
        // Map v8 to v9: Insert TownOnLowDurability=false at position 6
        checkboxData[0] = oldData[0];  // AutoBerserk
        checkboxData[1] = oldData[1];  // ReturnToTownOnDeath
        checkboxData[2] = oldData[2];  // TownOnLowHP
        checkboxData[3] = oldData[3];  // TownOnLowMP
        checkboxData[4] = oldData[4];  // TownOnLowPetHP
        checkboxData[5] = oldData[5];  // TownOnLowArrow
        checkboxData[6] = false;       // TownOnLowDurability (NEW - default disabled)
        checkboxData[7] = oldData[6];  // GoBackCenter
        checkboxData[8] = oldData[7];  // WalkAround
        
        if (m_atkSelectedCount < 0 || m_atkSelectedCount > SKILL_SLOT_COUNT) m_atkSelectedCount = 0;
        if (m_buffSelectedCount < 0 || m_buffSelectedCount > SKILL_SLOT_COUNT) m_buffSelectedCount = 0;
    } else if (version == 9) {
        // Version 9: 9 checkboxes
        fread(checkboxData, sizeof(bool), 9, f);
        fread(&m_rangeValue, sizeof(int), 1, f);
        fread(&m_atkSelectedCount, sizeof(int), 1, f);
        fread(m_atkSelectedSkillIds, sizeof(DWORD), SKILL_SLOT_COUNT, f);
        fread(&m_buffSelectedCount, sizeof(int), 1, f);
        fread(m_buffSelectedSkillIds, sizeof(DWORD), SKILL_SLOT_COUNT, f);
        
        if (m_atkSelectedCount < 0 || m_atkSelectedCount > SKILL_SLOT_COUNT) m_atkSelectedCount = 0;
        if (m_buffSelectedCount < 0 || m_buffSelectedCount > SKILL_SLOT_COUNT) m_buffSelectedCount = 0;
    } else {
        // Unknown version - use defaults
        m_rangeValue = 100;
    }
    fclose(f);

    // Validate range value
    if (m_rangeValue < 0) m_rangeValue = 0;
    if (m_rangeValue > 500) m_rangeValue = 500;

    // Update UI - 9 checkboxes now
    if (m_chkAutoBerserk) m_chkAutoBerserk->FUN_00656d50(checkboxData[0]);
    if (m_chkReturnToTownOnDeath) m_chkReturnToTownOnDeath->FUN_00656d50(checkboxData[1]);
    if (m_chkTownOnLowHP) m_chkTownOnLowHP->FUN_00656d50(checkboxData[2]);
    if (m_chkTownOnLowMP) m_chkTownOnLowMP->FUN_00656d50(checkboxData[3]);
    if (m_chkTownOnLowPetHP) m_chkTownOnLowPetHP->FUN_00656d50(checkboxData[4]);
    if (m_chkTownOnLowArrow) m_chkTownOnLowArrow->FUN_00656d50(checkboxData[5]);
    if (m_chkTownOnLowDurability) m_chkTownOnLowDurability->FUN_00656d50(checkboxData[6]);
    if (m_chkGoBackCenter) m_chkGoBackCenter->FUN_00656d50(checkboxData[7]);
    if (m_chkWalkAround) m_chkWalkAround->FUN_00656d50(checkboxData[8]);

    // Update slider UI - SetHCorrectScrollBar properly syncs thumb position
    if (m_rangeSlider) {
        m_rangeSlider->Set2EC(m_rangeValue);
        m_rangeSlider->SetHCorrectScrollBar(m_rangeValue);
    }
    if (m_rangeValueText) m_rangeValueText->SetTextFormatted("%d", m_rangeValue);

    return true;
}

void CIFAutoHuntSettings::PopulateLearnedSkills() {
    // Get all learned skill IDs from memory
    std::vector<DWORD> skills = LearnedSkillManager::GetAllLearnedSkillIDs();
    
    // Native function to get skill object for chain_id lookup
    typedef DWORD (__thiscall *GetSkillObj_t)(void*, DWORD);
    GetSkillObj_t GetSkillObj = (GetSkillObj_t)0x616790;
    void* pSkillMgr = (void*)0xA01010;


    // ========== EXCLUDED CHAIN IDs ==========
    // Passive skills - should not appear in any panel
    // Force skills - internal skills, should not appear in any panel
    static const DWORD excludedChainIds[] = { 
        // Passive chain IDs
        194, 215, 236, 253, 270, 289, 306,
        // Force chain IDs
        290, 291, 292, 293, 294, 295, 296, 297, 298, 299, 300, 301, 340, 341, 342
    };
    static const int excludedChainCount = sizeof(excludedChainIds) / sizeof(excludedChainIds[0]);
    
    // ========== BUFF CHAIN IDs ==========
    // These skills appear ONLY in Buff panel, NOT in Attack panel
    static const DWORD buffChainIds[] = {
        182, 183, 184, 199, 200, 201, 224, 225, 226, 230, 231, 240, 241, 242,
        246, 247, 248, 257, 258, 259, 260, 261, 262, 266, 267, 274, 275, 276, 
        277, 278, 279, 280, 281, 282, 283, 284, 302, 303, 329, 332, 335, 336, 338, 339
    };

    static const int buffChainCount = sizeof(buffChainIds) / sizeof(buffChainIds[0]);

    // ========== LOAD SKILLS INTO APPROPRIATE PANELS ==========
    m_allAtkSkills.clear();
    m_allBuffSkills.clear();
    
    for (size_t i = 0; i < skills.size(); i++) {
        DWORD skillID = skills[i];
        
        // Get chain_id from SkillData + 0x04
        DWORD chainId = 0;
        DWORD pSkillObj = GetSkillObj(pSkillMgr, skillID);
        if (pSkillObj) {
            chainId = *(DWORD*)(pSkillObj + 0x10C + 0x04);
        }
        
        // Skip excluded skills (passive + force) based on chain_id
        bool isExcluded = false;
        for (int p = 0; p < excludedChainCount; p++) {
            if (chainId == excludedChainIds[p]) { isExcluded = true; break; }
        }
        if (isExcluded) continue;
        
        // Check if buff skill
        bool isBuff = false;
        for (int b = 0; b < buffChainCount; b++) {
            if (chainId == buffChainIds[b]) { isBuff = true; break; }
        }
        
        std::string ddjPath = LearnedSkillManager::GetSkillIconPath(skillID);
        if (!ddjPath.empty()) {
            CachedSkillInfo info;
            info.skillId = skillID;
            strncpy(info.iconPath, ddjPath.c_str(), 259);
            info.iconPath[259] = '\0';
            
            // Buff skills -> only buff panel, others -> only attack panel
            if (isBuff) {
                m_allBuffSkills.push_back(info);
            } else {
                m_allAtkSkills.push_back(info);
            }
        }
    }



    // ========== RENDER VISIBLE SLOTS BASED ON SCROLL OFFSET ==========
    // Attack skills - render 48 visible slots from offset
    int atkStartIndex = m_atkScrollOffset * SKILL_SLOT_COLS;  // offset is in ROWS
    for (int i = 0; i < SKILL_SLOT_COUNT; i++) {
        int skillIndex = atkStartIndex + i;
        if (m_atkAcquiredSlots[i]) {
            if (skillIndex < (int)m_allAtkSkills.size()) {
                m_atkAcquiredSlots[i]->SetSlotItemID(m_allAtkSkills[skillIndex].skillId);
                m_atkAcquiredSlots[i]->TB_Func_12(m_allAtkSkills[skillIndex].iconPath, 0, 0);
                // Don't set visibility here - ShowElements functions handle it
            } else {
                // No skill at this index - clear slot
                m_atkAcquiredSlots[i]->SetSlotItemID(0);
                m_atkAcquiredSlots[i]->ClearIcon();
            }
        }
    }
    
    // Buff skills - render 48 visible slots from offset
    int buffStartIndex = m_buffScrollOffset * SKILL_SLOT_COLS;
    for (int i = 0; i < SKILL_SLOT_COUNT; i++) {
        int skillIndex = buffStartIndex + i;
        if (m_buffAcquiredSlots[i]) {
            if (skillIndex < (int)m_allBuffSkills.size()) {
                m_buffAcquiredSlots[i]->SetSlotItemID(m_allBuffSkills[skillIndex].skillId);
                m_buffAcquiredSlots[i]->TB_Func_12(m_allBuffSkills[skillIndex].iconPath, 0, 0);
                // Don't set visibility here - ShowElements functions handle it
            } else {
                m_buffAcquiredSlots[i]->SetSlotItemID(0);
                m_buffAcquiredSlots[i]->ClearIcon();
            }
        }
    }
    
    // Store current offsets
    m_lastAtkScrollOffset = m_atkScrollOffset;
    m_lastBuffScrollOffset = m_buffScrollOffset;

    // ========== CONFIGURE SCROLLBAR RANGE ==========
    // Calculate total rows for scrollbar range
    int atkTotalRows = ((int)m_allAtkSkills.size() + SKILL_SLOT_COLS - 1) / SKILL_SLOT_COLS;
    int buffTotalRows = ((int)m_allBuffSkills.size() + SKILL_SLOT_COLS - 1) / SKILL_SLOT_COLS;
    int maxAtkOffset = (atkTotalRows > SKILL_SLOT_ROWS) ? (atkTotalRows - SKILL_SLOT_ROWS) : 0;
    int maxBuffOffset = (buffTotalRows > SKILL_SLOT_ROWS) ? (buffTotalRows - SKILL_SLOT_ROWS) : 0;
    
    // Lattice height for scrollbar
    const int latticeHeight = SKILL_SLOT_ROWS * SKILL_SLOT_SIZE;  // 8 * 32 = 256
    
    // Configure attack skills scrollbar (now CIFVerticalScroll directly)
    if (m_atkAcquiredScroll) {
        m_atkAcquiredScroll->SetScrollBarRange(213, 0, maxAtkOffset, 1);
    }
    
    // Configure buff skills scrollbar
    if (m_buffAcquiredScroll) {
        m_buffAcquiredScroll->SetScrollBarRange(213, 0, maxBuffOffset, 1);
    }

    // ========== RESTORE SAVED SKILLS TO "Skills to Use" PANELS ==========
    // Attack Skills to Use
    for (int i = 0; i < SKILL_SLOT_COUNT; i++) {
        if (m_atkSelectedSkillIds[i] != 0 && m_atkToUseSlots[i]) {
            std::string iconPath = LearnedSkillManager::GetSkillIconPath(m_atkSelectedSkillIds[i]);
            if (!iconPath.empty()) {
                m_atkToUseSlots[i]->TB_Func_12(iconPath.c_str(), 0, 0);
                m_atkToUseSlots[i]->SetSlotItemID(m_atkSelectedSkillIds[i]);
            }
        }
    }

    // Buff Skills to Use
    for (int i = 0; i < SKILL_SLOT_COUNT; i++) {
        if (m_buffSelectedSkillIds[i] != 0 && m_buffToUseSlots[i]) {
            std::string iconPath = LearnedSkillManager::GetSkillIconPath(m_buffSelectedSkillIds[i]);
            if (!iconPath.empty()) {
                m_buffToUseSlots[i]->TB_Func_12(iconPath.c_str(), 0, 0);
                m_buffToUseSlots[i]->SetSlotItemID(m_buffSelectedSkillIds[i]);
            }
        }
    }
}

void CIFAutoHuntSettings::RefreshLearnedSkills() {
    // Get current learned skill IDs
    std::vector<DWORD> learnedSkills = LearnedSkillManager::GetAllLearnedSkillIDs();
    
    // Clear all cached skill data (vectors clear automatically in PopulateLearnedSkills)
    m_allAtkSkills.clear();
    m_allBuffSkills.clear();
    
    // Clear icon/data from ALL Acquired Skills slots
    for (int i = 0; i < SKILL_SLOT_COUNT; i++) {
        if (m_atkAcquiredSlots[i]) {
            m_atkAcquiredSlots[i]->ClearIcon();
            m_atkAcquiredSlots[i]->SetSlotItemID(0);
        }
        if (m_buffAcquiredSlots[i]) {
            m_buffAcquiredSlots[i]->ClearIcon();
            m_buffAcquiredSlots[i]->SetSlotItemID(0);
        }
    }
    
    // Validate m_atkSelectedSkillIds - remove any skills that are no longer learned
    int validAtkCount = 0;
    for (int i = 0; i < SKILL_SLOT_COUNT; i++) {
        if (m_atkSelectedSkillIds[i] != 0) {
            bool isLearned = false;
            for (size_t j = 0; j < learnedSkills.size(); j++) {
                if (learnedSkills[j] == m_atkSelectedSkillIds[i]) {
                    isLearned = true;
                    break;
                }
            }
            if (!isLearned) {
                m_atkSelectedSkillIds[i] = 0;
                if (m_atkToUseSlots[i]) {
                    m_atkToUseSlots[i]->ClearIcon();
                    m_atkToUseSlots[i]->SetSlotItemID(0);
                }
            } else {
                validAtkCount++;
            }
        }
    }
    m_atkSelectedCount = validAtkCount;
    
    // Validate m_buffSelectedSkillIds - remove any skills that are no longer learned
    int validBuffCount = 0;
    for (int i = 0; i < SKILL_SLOT_COUNT; i++) {
        if (m_buffSelectedSkillIds[i] != 0) {
            bool isLearned = false;
            for (size_t j = 0; j < learnedSkills.size(); j++) {
                if (learnedSkills[j] == m_buffSelectedSkillIds[i]) {
                    isLearned = true;
                    break;
                }
            }
            if (!isLearned) {
                m_buffSelectedSkillIds[i] = 0;
                if (m_buffToUseSlots[i]) {
                    m_buffToUseSlots[i]->ClearIcon();
                    m_buffToUseSlots[i]->SetSlotItemID(0);
                }
            } else {
                validBuffCount++;
            }
        }
    }
    m_buffSelectedCount = validBuffCount;
    
    // VISUAL SYNC: Clear icons from ALL slots where skillId=0
    for (int i = 0; i < SKILL_SLOT_COUNT; i++) {
        if (m_atkSelectedSkillIds[i] == 0 && m_atkToUseSlots[i]) {
            m_atkToUseSlots[i]->ClearIcon();
            m_atkToUseSlots[i]->SetSlotItemID(0);
        }
        if (m_buffSelectedSkillIds[i] == 0 && m_buffToUseSlots[i]) {
            m_buffToUseSlots[i]->ClearIcon();
            m_buffToUseSlots[i]->SetSlotItemID(0);
        }
    }
    
    // Repopulate learned skills from memory
    PopulateLearnedSkills();
    
    // Refresh visibility based on current tab
    if (m_activeTab == TAB_AUTO_ATTACK_SKILLS) {
        ShowAttackSkillsElements(true);
    } else if (m_activeTab == TAB_AUTO_BUFF_SKILLS) {
        ShowBuffSkillsElements(true);
    }
}

// =========================================================================
// BUTTON HANDLERS
// =========================================================================

void CIFAutoHuntSettings::OnClick_Confirm() {
    SaveConfig();
    if (AutoHuntSettingsMainFrame) {
        AutoHuntSettingsMainFrame->ShowGWnd(false);
    }
}

void CIFAutoHuntSettings::OnClick_Cancel() {
    // Just close window without saving - LoadConfig will be called when reopened
    if (AutoHuntSettingsMainFrame) {
        AutoHuntSettingsMainFrame->ShowGWnd(false);
    }
}