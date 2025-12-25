#include "IFPetFilterSettings.h"
#include "IFNormalTile.h"
#include "GInterface.h"
#include "Game.h"
#include "ICPlayer.h"
#include "PetFilter.h"
#include <cstdio>
#include <direct.h>
#include <string>

// Radio button static visuals
static CIFStatic* g_radioOnStatic = NULL;
static CIFStatic* g_radioOffStatic = NULL;
static bool g_radioOnSelected = false;  // OFF by default - will be loaded from config if exists

extern int g_D3DViewportWidth;
extern int g_D3DViewportHeight;

bool CIFPetFilterSettings::SwitchPetFilter = false;
CIFMainFrame* PetFilterSettingsMainFrame = NULL;
CIFPetFilterSettings* g_pCIFPetFilterSettings = NULL;

GFX_IMPLEMENT_DYNCREATE(CIFPetFilterSettings, CIFMainFrame)
GFX_BEGIN_MESSAGE_MAP(CIFPetFilterSettings, CIFMainFrame)
GFX_END_MESSAGE_MAP()

CIFPetFilterSettings::CIFPetFilterSettings()
    : m_currentTab(0), m_btnSwitch(NULL)
    // Weapon section
    , m_chkSpear(NULL), m_chkGlaive(NULL), m_chkBow(NULL), m_chkBlade(NULL), m_chkSword(NULL)
    // Clothes section
    , m_chkEqArmor(NULL), m_chkEqProtector(NULL), m_chkEqGarment(NULL)
    // Accessory section
    , m_chkRing(NULL), m_chkEarring(NULL), m_chkNecklace(NULL)
    // Others section
    , m_chkGold(NULL), m_chkAlchElixirWeapon(NULL), m_chkPotion(NULL), m_chkAlchTablets(NULL)
    , m_chkAlchMaterials(NULL), m_chkQuests(NULL), m_chkReturnScroll(NULL), m_chkOthers(NULL)
{
    for (int i = 0; i < NUM_TABS; i++) m_pTabs[i] = NULL;
    for (int i = 0; i < 15; i++) { m_chkEqDegree[i] = NULL; m_chkAlchDegree[i] = NULL; }
    g_pCIFPetFilterSettings = this;
}

CIFPetFilterSettings::~CIFPetFilterSettings() { g_pCIFPetFilterSettings = NULL; }

bool CIFPetFilterSettings::OnCreate(long ln) {
    // Main window - 480x480 (square shape)
    RECT rect = {50, 50, 480, 480};
    PetFilterSettingsMainFrame = (CIFMainFrame*)CGWnd::CreateInstance(g_pCGInterface, GFX_RUNTIME_CLASS(CIFMainFrame), rect, 2004, 0);
    if (!PetFilterSettingsMainFrame) return false;
    
    PetFilterSettingsMainFrame->SetText("Pet Filter");
    PetFilterSettingsMainFrame->TB_Func_12("interface\\frame\\mframe_wnd_", 1, 0);
    PetFilterSettingsMainFrame->SetGWndSize(480, 480);

    wnd_rect sz;

    // Main Frame
    sz.pos.x = 15; sz.pos.y = 43; sz.size.width = 450; sz.size.height = 425;
    CIFFrame* pFrame = (CIFFrame*)CGWnd::CreateInstance(PetFilterSettingsMainFrame, GFX_RUNTIME_CLASS(CIFFrame), sz, 1, 0);
    if (pFrame) { pFrame->TB_Func_12("interface\\inventory\\int_window_", 1, 0); pFrame->SetGWndSize(450, 425); }

    // Background tile
    sz.pos.x = 30; sz.pos.y = 54; sz.size.width = 420; sz.size.height = 405;
    CIFNormalTile* pTile = (CIFNormalTile*)CGWnd::CreateInstance(PetFilterSettingsMainFrame, GFX_RUNTIME_CLASS(CIFNormalTile), sz, 2, 0);
    if (pTile) { pTile->TB_Func_12("interface\\ifcommon\\bg_tile\\com_bg_tile_b.ddj", 1, 0); pTile->SetGWndSize(420, 405); }

    // ========== SECTION 1: Grab function (y=57) ==========
    sz.pos.x = 22; sz.pos.y = 57; sz.size.width = 124; sz.size.height = 28;
    CIFStatic* tabSta1 = (CIFStatic*)CGWnd::CreateInstance(PetFilterSettingsMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 10, 0);
    if (tabSta1) { tabSta1->TB_Func_12("interface\\option\\opt_video_tab.ddj", 1, 0); tabSta1->SetGWndSize(124, 28); }
    
    sz.pos.x = 32; sz.pos.y = 66; sz.size.width = 102; sz.size.height = 11;
    CIFStatic* lblFilter = (CIFStatic*)CGWnd::CreateInstance(PetFilterSettingsMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 16, 0);
    if (lblFilter) { lblFilter->SetText("Grab function"); lblFilter->SetTextColor(D3DCOLOR_XRGB(239, 218, 164)); }
    
    sz.pos.x = 22; sz.pos.y = 80; sz.size.width = 416; sz.size.height = 36;
    CIFFrame* sec1Frame = (CIFFrame*)CGWnd::CreateInstance(PetFilterSettingsMainFrame, GFX_RUNTIME_CLASS(CIFFrame), sz, 11, 0);
    if (sec1Frame) { sec1Frame->TB_Func_12("interface\\inventory\\int_window_", 1, 0); sec1Frame->SetGWndSize(416, 36); }
    
    // ON/OFF radio buttons (centered in frame: y=80+10=90)
    // g_radioOnSelected defaults to false (OFF), so show OFF as selected initially
    sz.pos.x = 37; sz.pos.y = 92; sz.size.width = 16; sz.size.height = 16;
    g_radioOnStatic = (CIFStatic*)CGWnd::CreateInstance(PetFilterSettingsMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 20, 0);
    if (g_radioOnStatic) {
        g_radioOnStatic->TB_Func_12(g_radioOnSelected ? 
            "interface\\ifcommon\\com_radiobutton_on.ddj" : 
            "interface\\ifcommon\\com_radiobutton_off.ddj", 1, 0);
        g_radioOnStatic->SetGWndSize(16, 16);
    }
    sz.pos.x = 58; sz.pos.y = 94; sz.size.width = 30; sz.size.height = 11;
    CIFStatic* lblOn = (CIFStatic*)CGWnd::CreateInstance(PetFilterSettingsMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 21, 0);
    if (lblOn) lblOn->SetText("ON");
    
    sz.pos.x = 120; sz.pos.y = 92; sz.size.width = 16; sz.size.height = 16;
    g_radioOffStatic = (CIFStatic*)CGWnd::CreateInstance(PetFilterSettingsMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 22, 0);
    if (g_radioOffStatic) {
        g_radioOffStatic->TB_Func_12(g_radioOnSelected ? 
            "interface\\ifcommon\\com_radiobutton_off.ddj" : 
            "interface\\ifcommon\\com_radiobutton_on.ddj", 1, 0);
        g_radioOffStatic->SetGWndSize(16, 16);
    }
    sz.pos.x = 141; sz.pos.y = 94; sz.size.width = 30; sz.size.height = 11;
    CIFStatic* lblOff = (CIFStatic*)CGWnd::CreateInstance(PetFilterSettingsMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 23, 0);
    if (lblOff) lblOff->SetText("OFF");

    // ========== SECTION 2: Weapon (y=119) - 5 checkboxes in 1 row ==========
    sz.pos.x = 22; sz.pos.y = 119; sz.size.width = 124; sz.size.height = 28;
    CIFStatic* tabSta2 = (CIFStatic*)CGWnd::CreateInstance(PetFilterSettingsMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 12, 0);
    if (tabSta2) { tabSta2->TB_Func_12("interface\\option\\opt_video_tab.ddj", 1, 0); tabSta2->SetGWndSize(124, 28); }
    
    sz.pos.x = 32; sz.pos.y = 128; sz.size.width = 102; sz.size.height = 11;
    CIFStatic* lblWeapon = (CIFStatic*)CGWnd::CreateInstance(PetFilterSettingsMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 17, 0);
    if (lblWeapon) { lblWeapon->SetText("Weapon"); lblWeapon->SetTextColor(D3DCOLOR_XRGB(239, 218, 164)); }
    
    sz.pos.x = 22; sz.pos.y = 142; sz.size.width = 416; sz.size.height = 36;
    CIFFrame* sec2Frame = (CIFFrame*)CGWnd::CreateInstance(PetFilterSettingsMainFrame, GFX_RUNTIME_CLASS(CIFFrame), sz, 13, 0);
    if (sec2Frame) { sec2Frame->TB_Func_12("interface\\inventory\\int_window_", 1, 0); sec2Frame->SetGWndSize(416, 36); }
    
    // 5 checkboxes in 1 row: Spear, Glaive, Bow, Blade, Sword (y=154)
    // X positions: 37, 107, 177, 257, 337 (spacing ~70px per item)
    sz.pos.x = 37; sz.pos.y = 154; sz.size.width = 16; sz.size.height = 16;
    m_chkSpear = (CIFCheckBox*)CGWnd::CreateInstance(PetFilterSettingsMainFrame, GFX_RUNTIME_CLASS(CIFCheckBox), sz, 25, 0);
    sz.pos.x = 56; sz.pos.y = 156; sz.size.width = 40; sz.size.height = 11;
    CIFStatic* s1 = (CIFStatic*)CGWnd::CreateInstance(PetFilterSettingsMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 30, 0);
    if (s1) s1->SetText("Spear");
    
    sz.pos.x = 107; sz.pos.y = 154; sz.size.width = 16; sz.size.height = 16;
    m_chkGlaive = (CIFCheckBox*)CGWnd::CreateInstance(PetFilterSettingsMainFrame, GFX_RUNTIME_CLASS(CIFCheckBox), sz, 26, 0);
    sz.pos.x = 126; sz.pos.y = 156; sz.size.width = 40; sz.size.height = 11;
    CIFStatic* s2 = (CIFStatic*)CGWnd::CreateInstance(PetFilterSettingsMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 31, 0);
    if (s2) s2->SetText("Glaive");
    
    sz.pos.x = 177; sz.pos.y = 154; sz.size.width = 16; sz.size.height = 16;
    m_chkBow = (CIFCheckBox*)CGWnd::CreateInstance(PetFilterSettingsMainFrame, GFX_RUNTIME_CLASS(CIFCheckBox), sz, 27, 0);
    sz.pos.x = 196; sz.pos.y = 156; sz.size.width = 30; sz.size.height = 11;
    CIFStatic* s3 = (CIFStatic*)CGWnd::CreateInstance(PetFilterSettingsMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 32, 0);
    if (s3) s3->SetText("Bow");

    sz.pos.x = 247; sz.pos.y = 154; sz.size.width = 16; sz.size.height = 16;
    m_chkBlade = (CIFCheckBox*)CGWnd::CreateInstance(PetFilterSettingsMainFrame, GFX_RUNTIME_CLASS(CIFCheckBox), sz, 28, 0);
    sz.pos.x = 266; sz.pos.y = 156; sz.size.width = 35; sz.size.height = 11;
    CIFStatic* s4a = (CIFStatic*)CGWnd::CreateInstance(PetFilterSettingsMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 33, 0);
    if (s4a) s4a->SetText("Blade");
    
    sz.pos.x = 317; sz.pos.y = 154; sz.size.width = 16; sz.size.height = 16;
    m_chkSword = (CIFCheckBox*)CGWnd::CreateInstance(PetFilterSettingsMainFrame, GFX_RUNTIME_CLASS(CIFCheckBox), sz, 29, 0);
    sz.pos.x = 336; sz.pos.y = 156; sz.size.width = 40; sz.size.height = 11;
    CIFStatic* s5a = (CIFStatic*)CGWnd::CreateInstance(PetFilterSettingsMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 34, 0);
    if (s5a) s5a->SetText("Sword");

    // ========== SECTION 3: Clothes (y=181) ==========
    sz.pos.x = 22; sz.pos.y = 181; sz.size.width = 124; sz.size.height = 28;
    CIFStatic* tabSta3 = (CIFStatic*)CGWnd::CreateInstance(PetFilterSettingsMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 14, 0);
    if (tabSta3) { tabSta3->TB_Func_12("interface\\option\\opt_video_tab.ddj", 1, 0); tabSta3->SetGWndSize(124, 28); }
    
    sz.pos.x = 32; sz.pos.y = 190; sz.size.width = 102; sz.size.height = 11;
    CIFStatic* lblClothes = (CIFStatic*)CGWnd::CreateInstance(PetFilterSettingsMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 18, 0);
    if (lblClothes) { lblClothes->SetText("Clothes"); lblClothes->SetTextColor(D3DCOLOR_XRGB(239, 218, 164)); }
    
    sz.pos.x = 22; sz.pos.y = 204; sz.size.width = 416; sz.size.height = 36;
    CIFFrame* sec3Frame = (CIFFrame*)CGWnd::CreateInstance(PetFilterSettingsMainFrame, GFX_RUNTIME_CLASS(CIFFrame), sz, 15, 0);
    if (sec3Frame) { sec3Frame->TB_Func_12("interface\\inventory\\int_window_", 1, 0); sec3Frame->SetGWndSize(416, 36); }
    
    // Armor, Protector, Garment (centered: y=204+10=214)
    sz.pos.x = 37; sz.pos.y = 216; sz.size.width = 16; sz.size.height = 16;
    m_chkEqArmor = (CIFCheckBox*)CGWnd::CreateInstance(PetFilterSettingsMainFrame, GFX_RUNTIME_CLASS(CIFCheckBox), sz, 33, 0);
    sz.pos.x = 58; sz.pos.y = 218; sz.size.width = 49; sz.size.height = 11;
    CIFStatic* s4 = (CIFStatic*)CGWnd::CreateInstance(PetFilterSettingsMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 34, 0);
    if (s4) s4->SetText("Armor");
    
    sz.pos.x = 120; sz.pos.y = 216; sz.size.width = 16; sz.size.height = 16;
    m_chkEqProtector = (CIFCheckBox*)CGWnd::CreateInstance(PetFilterSettingsMainFrame, GFX_RUNTIME_CLASS(CIFCheckBox), sz, 35, 0);
    sz.pos.x = 141; sz.pos.y = 218; sz.size.width = 60; sz.size.height = 11;
    CIFStatic* s5 = (CIFStatic*)CGWnd::CreateInstance(PetFilterSettingsMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 36, 0);
    if (s5) s5->SetText("Protector");
    
    sz.pos.x = 210; sz.pos.y = 216; sz.size.width = 16; sz.size.height = 16;
    m_chkEqGarment = (CIFCheckBox*)CGWnd::CreateInstance(PetFilterSettingsMainFrame, GFX_RUNTIME_CLASS(CIFCheckBox), sz, 37, 0);
    sz.pos.x = 231; sz.pos.y = 218; sz.size.width = 55; sz.size.height = 11;
    CIFStatic* s6 = (CIFStatic*)CGWnd::CreateInstance(PetFilterSettingsMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 38, 0);
    if (s6) s6->SetText("Garment");

    // ========== SECTION 4: Accessory (y=243) ==========
    sz.pos.x = 22; sz.pos.y = 243; sz.size.width = 124; sz.size.height = 28;
    CIFStatic* tabSta4 = (CIFStatic*)CGWnd::CreateInstance(PetFilterSettingsMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 70, 0);
    if (tabSta4) { tabSta4->TB_Func_12("interface\\option\\opt_video_tab.ddj", 1, 0); tabSta4->SetGWndSize(124, 28); }
    
    sz.pos.x = 32; sz.pos.y = 252; sz.size.width = 102; sz.size.height = 11;
    CIFStatic* lblAccessory = (CIFStatic*)CGWnd::CreateInstance(PetFilterSettingsMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 71, 0);
    if (lblAccessory) { lblAccessory->SetText("Accessory"); lblAccessory->SetTextColor(D3DCOLOR_XRGB(239, 218, 164)); }
    
    sz.pos.x = 22; sz.pos.y = 266; sz.size.width = 416; sz.size.height = 36;
    CIFFrame* sec4Frame = (CIFFrame*)CGWnd::CreateInstance(PetFilterSettingsMainFrame, GFX_RUNTIME_CLASS(CIFFrame), sz, 72, 0);
    if (sec4Frame) { sec4Frame->TB_Func_12("interface\\inventory\\int_window_", 1, 0); sec4Frame->SetGWndSize(416, 36); }
    
    // Ring, Earring, Necklace (centered: y=266+10=276)
    sz.pos.x = 37; sz.pos.y = 278; sz.size.width = 16; sz.size.height = 16;
    m_chkRing = (CIFCheckBox*)CGWnd::CreateInstance(PetFilterSettingsMainFrame, GFX_RUNTIME_CLASS(CIFCheckBox), sz, 73, 0);
    sz.pos.x = 58; sz.pos.y = 280; sz.size.width = 40; sz.size.height = 11;
    CIFStatic* sR1 = (CIFStatic*)CGWnd::CreateInstance(PetFilterSettingsMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 74, 0);
    if (sR1) sR1->SetText("Ring");
    
    sz.pos.x = 120; sz.pos.y = 278; sz.size.width = 16; sz.size.height = 16;
    m_chkEarring = (CIFCheckBox*)CGWnd::CreateInstance(PetFilterSettingsMainFrame, GFX_RUNTIME_CLASS(CIFCheckBox), sz, 75, 0);
    sz.pos.x = 141; sz.pos.y = 280; sz.size.width = 50; sz.size.height = 11;
    CIFStatic* sR2 = (CIFStatic*)CGWnd::CreateInstance(PetFilterSettingsMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 76, 0);
    if (sR2) sR2->SetText("Earring");
    
    sz.pos.x = 210; sz.pos.y = 278; sz.size.width = 16; sz.size.height = 16;
    m_chkNecklace = (CIFCheckBox*)CGWnd::CreateInstance(PetFilterSettingsMainFrame, GFX_RUNTIME_CLASS(CIFCheckBox), sz, 77, 0);
    sz.pos.x = 231; sz.pos.y = 280; sz.size.width = 60; sz.size.height = 11;
    CIFStatic* sR3 = (CIFStatic*)CGWnd::CreateInstance(PetFilterSettingsMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 78, 0);
    if (sR3) sR3->SetText("Necklace");

    // ========== SECTION 5: Others (y=305) - 2 rows ==========
    sz.pos.x = 22; sz.pos.y = 305; sz.size.width = 124; sz.size.height = 28;
    CIFStatic* tabSta5 = (CIFStatic*)CGWnd::CreateInstance(PetFilterSettingsMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 60, 0);
    if (tabSta5) { tabSta5->TB_Func_12("interface\\option\\opt_video_tab.ddj", 1, 0); tabSta5->SetGWndSize(124, 28); }
    
    sz.pos.x = 32; sz.pos.y = 314; sz.size.width = 102; sz.size.height = 11;
    CIFStatic* lblOthers = (CIFStatic*)CGWnd::CreateInstance(PetFilterSettingsMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 61, 0);
    if (lblOthers) { lblOthers->SetText("Others"); lblOthers->SetTextColor(D3DCOLOR_XRGB(239, 218, 164)); }
    
    sz.pos.x = 22; sz.pos.y = 328; sz.size.width = 416; sz.size.height = 56;
    CIFFrame* sec5Frame = (CIFFrame*)CGWnd::CreateInstance(PetFilterSettingsMainFrame, GFX_RUNTIME_CLASS(CIFFrame), sz, 62, 0);
    if (sec5Frame) { sec5Frame->TB_Func_12("interface\\inventory\\int_window_", 1, 0); sec5Frame->SetGWndSize(416, 56); }
    
    // Row 1: Gold, Elixirs, Potion, Tablets (centered: y=328+8=336)
    sz.pos.x = 37; sz.pos.y = 338; sz.size.width = 16; sz.size.height = 16;
    m_chkGold = (CIFCheckBox*)CGWnd::CreateInstance(PetFilterSettingsMainFrame, GFX_RUNTIME_CLASS(CIFCheckBox), sz, 45, 0);
    sz.pos.x = 58; sz.pos.y = 340; sz.size.width = 40; sz.size.height = 11;
    CIFStatic* s7 = (CIFStatic*)CGWnd::CreateInstance(PetFilterSettingsMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 50, 0);
    if (s7) s7->SetText("Gold");
    
    sz.pos.x = 120; sz.pos.y = 338; sz.size.width = 16; sz.size.height = 16;
    m_chkAlchElixirWeapon = (CIFCheckBox*)CGWnd::CreateInstance(PetFilterSettingsMainFrame, GFX_RUNTIME_CLASS(CIFCheckBox), sz, 46, 0);
    sz.pos.x = 141; sz.pos.y = 340; sz.size.width = 45; sz.size.height = 11;
    CIFStatic* s8 = (CIFStatic*)CGWnd::CreateInstance(PetFilterSettingsMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 51, 0);
    if (s8) s8->SetText("Elixirs");
    
    sz.pos.x = 210; sz.pos.y = 338; sz.size.width = 16; sz.size.height = 16;
    m_chkPotion = (CIFCheckBox*)CGWnd::CreateInstance(PetFilterSettingsMainFrame, GFX_RUNTIME_CLASS(CIFCheckBox), sz, 47, 0);
    sz.pos.x = 231; sz.pos.y = 340; sz.size.width = 45; sz.size.height = 11;
    CIFStatic* s9 = (CIFStatic*)CGWnd::CreateInstance(PetFilterSettingsMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 52, 0);
    if (s9) s9->SetText("Potion");
    
    sz.pos.x = 310; sz.pos.y = 338; sz.size.width = 16; sz.size.height = 16;
    m_chkAlchTablets = (CIFCheckBox*)CGWnd::CreateInstance(PetFilterSettingsMainFrame, GFX_RUNTIME_CLASS(CIFCheckBox), sz, 53, 0);
    sz.pos.x = 331; sz.pos.y = 340; sz.size.width = 55; sz.size.height = 11;
    CIFStatic* s10 = (CIFStatic*)CGWnd::CreateInstance(PetFilterSettingsMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 54, 0);
    if (s10) s10->SetText("Tablets");

    // Row 2: Material, Quest, Return Scroll, Others (y=336+20=356)
    sz.pos.x = 37; sz.pos.y = 358; sz.size.width = 16; sz.size.height = 16;
    m_chkAlchMaterials = (CIFCheckBox*)CGWnd::CreateInstance(PetFilterSettingsMainFrame, GFX_RUNTIME_CLASS(CIFCheckBox), sz, 55, 0);
    sz.pos.x = 58; sz.pos.y = 360; sz.size.width = 55; sz.size.height = 11;
    CIFStatic* s11 = (CIFStatic*)CGWnd::CreateInstance(PetFilterSettingsMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 56, 0);
    if (s11) s11->SetText("Material");
    
    sz.pos.x = 120; sz.pos.y = 358; sz.size.width = 16; sz.size.height = 16;
    m_chkQuests = (CIFCheckBox*)CGWnd::CreateInstance(PetFilterSettingsMainFrame, GFX_RUNTIME_CLASS(CIFCheckBox), sz, 80, 0);
    sz.pos.x = 141; sz.pos.y = 360; sz.size.width = 40; sz.size.height = 11;
    CIFStatic* s12 = (CIFStatic*)CGWnd::CreateInstance(PetFilterSettingsMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 81, 0);
    if (s12) s12->SetText("Quest");
    
    sz.pos.x = 210; sz.pos.y = 358; sz.size.width = 16; sz.size.height = 16;
    m_chkReturnScroll = (CIFCheckBox*)CGWnd::CreateInstance(PetFilterSettingsMainFrame, GFX_RUNTIME_CLASS(CIFCheckBox), sz, 82, 0);
    sz.pos.x = 231; sz.pos.y = 360; sz.size.width = 75; sz.size.height = 11;
    CIFStatic* s13 = (CIFStatic*)CGWnd::CreateInstance(PetFilterSettingsMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 83, 0);
    if (s13) s13->SetText("Return Scroll");
    
    sz.pos.x = 310; sz.pos.y = 358; sz.size.width = 16; sz.size.height = 16;
    m_chkOthers = (CIFCheckBox*)CGWnd::CreateInstance(PetFilterSettingsMainFrame, GFX_RUNTIME_CLASS(CIFCheckBox), sz, 84, 0);
    sz.pos.x = 331; sz.pos.y = 360; sz.size.width = 50; sz.size.height = 11;
    CIFStatic* s14 = (CIFStatic*)CGWnd::CreateInstance(PetFilterSettingsMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 85, 0);
    if (s14) s14->SetText("Others");

    // ========== Buttons: Confirm & Cancel (y=420) ==========
    sz.pos.x = 158; sz.pos.y = 420; sz.size.width = 76; sz.size.height = 24;
    CIFButton* btnConfirm = (CIFButton*)CGWnd::CreateInstance(PetFilterSettingsMainFrame, GFX_RUNTIME_CLASS(CIFButton), sz, 40, 0);
    if (btnConfirm) {
        btnConfirm->TB_Func_12("interface\\ifcommon\\com_button.ddj", 1, 1);
        btnConfirm->SetText("Confirm");
        btnConfirm->SetGWndSize(76, 24);
    }
    
    sz.pos.x = 246; sz.pos.y = 420; sz.size.width = 76; sz.size.height = 24;
    CIFButton* btnCancel = (CIFButton*)CGWnd::CreateInstance(PetFilterSettingsMainFrame, GFX_RUNTIME_CLASS(CIFButton), sz, 41, 0);
    if (btnCancel) {
        btnCancel->TB_Func_12("interface\\ifcommon\\com_button.ddj", 1, 1);
        btnCancel->SetText("Cancel");
        btnCancel->SetGWndSize(76, 24);
    }

    PetFilterSettingsMainFrame->ShowGWnd(false);
    LoadConfig();
    return true;
}

void CIFPetFilterSettings::OnUpdate() {
    if (!PetFilterSettingsMainFrame || !PetFilterSettingsMainFrame->IsVisible()) return;
    
    static bool wasMouseDown = false;
    static DWORD lastClickTime = 0;
    bool isMouseDown = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
    
    DWORD now = GetTickCount();
    
    // Only process click on mouse release (wasMouseDown && !isMouseDown)
    // And add 100ms debounce to prevent double-clicks
    if (wasMouseDown && !isMouseDown && (now - lastClickTime) > 100) {
        lastClickTime = now;
        
        POINT pt; GetCursorPos(&pt);
        extern HWND g_hMainWnd;
        if (g_hMainWnd) {
            ScreenToClient(g_hMainWnd, &pt);
            int fx = *reinterpret_cast<int*>(reinterpret_cast<char*>(PetFilterSettingsMainFrame) + 0x3C);
            int fy = *reinterpret_cast<int*>(reinterpret_cast<char*>(PetFilterSettingsMainFrame) + 0x40);
            int lx = pt.x - fx, ly = pt.y - fy;
            
            // ON radio button click area (x=37-90, y=90-106)
            if (lx >= 37 && lx < 90 && ly >= 90 && ly < 106) {
                if (!g_radioOnSelected) {
                    g_radioOnSelected = true;
                    if (g_radioOnStatic) g_radioOnStatic->TB_Func_12("interface\\ifcommon\\com_radiobutton_on.ddj", 1, 0);
                    if (g_radioOffStatic) g_radioOffStatic->TB_Func_12("interface\\ifcommon\\com_radiobutton_off.ddj", 1, 0);
                    // Immediately apply filter state
                    SwitchPetFilter = true;
                    PetFilter::GetInstance().SetEnabled(true);
                    printf("[PetFilter] Filter ENABLED via radio button\n");
                }
            }
            
            // OFF radio button click area (x=120-175, y=90-106)
            if (lx >= 120 && lx < 175 && ly >= 90 && ly < 106) {
                if (g_radioOnSelected) {
                    g_radioOnSelected = false;
                    if (g_radioOnStatic) g_radioOnStatic->TB_Func_12("interface\\ifcommon\\com_radiobutton_off.ddj", 1, 0);
                    if (g_radioOffStatic) g_radioOffStatic->TB_Func_12("interface\\ifcommon\\com_radiobutton_on.ddj", 1, 0);
                    // Immediately apply filter state
                    SwitchPetFilter = false;
                    PetFilter::GetInstance().SetEnabled(false);
                    printf("[PetFilter] Filter DISABLED via radio button\n");
                }
            }
            
            // Confirm button (y=420) - saves and syncs all settings
            if (lx >= 158 && lx < 234 && ly >= 420 && ly < 444) {
                SwitchFilter();
                SaveConfig();
                // Explicitly sync PetFilter enabled state with radio button
                extern void SetPetFilterEnabled(bool enabled);
                SetPetFilterEnabled(SwitchPetFilter);
                printf("[PetFilter] Confirm: Filter %s\n", SwitchPetFilter ? "ENABLED" : "DISABLED");
                PetFilterSettingsMainFrame->ShowGWnd(false);
            }
            // Cancel button (y=420)
            if (lx >= 246 && lx < 322 && ly >= 420 && ly < 444) {
                PetFilterSettingsMainFrame->ShowGWnd(false);
            }
        }
    }
    wasMouseDown = isMouseDown;
}

void CIFPetFilterSettings::SwitchFilter() {
    // Use radio button state for filter toggle
    SwitchPetFilter = g_radioOnSelected;
    
    // Get PetFilter instance and sync all checkbox states
    PetFilterSettings& settings = PetFilter::GetInstance().GetSettings();
    
    // Master switch
    settings.enabled = g_radioOnSelected;
    
    // ===== SECTION 2: WEAPON (tid1=1, tid2=6) - Using individual weapon checkboxes =====
    settings.eqSpear = IsSpearChecked();      // m_chkSpear → tid3=4
    settings.eqGlaive = IsGlaiveChecked();    // m_chkGlaive → tid3=5
    settings.eqBow = IsBowChecked();          // m_chkBow → tid3=6
    settings.eqBlade = IsBladeChecked();      // m_chkBlade → tid3=3
    settings.eqSword = IsSwordChecked();      // m_chkSword → tid3=2
    
    // Legacy eqWeapon flag - true if any weapon type is enabled
    settings.eqWeapon = settings.eqSpear || settings.eqGlaive || settings.eqBow || 
                        settings.eqBlade || settings.eqSword;
    
    // ===== SECTION 3: CLOTHES (tid1=1, tid2=1-3) =====
    settings.eqHeavy = IsEqArmorChecked();       // m_chkEqArmor → tid2=3 (Armor)
    settings.eqLight = IsEqProtectorChecked();   // m_chkEqProtector → tid2=2 (Protector)
    settings.eqClothes = IsEqGarmentChecked();   // m_chkEqGarment → tid2=1 (Garment)
    
    // Gender filters - disabled for now (no checkboxes)
    settings.eqMale = true;    // Accept male items
    settings.eqFemale = true;  // Accept female items
    
    // Degree filters
    for (int i = 0; i < 15; i++) {
        settings.eqDegree[i] = IsEQDegreeChecked(i + 1);
    }
    
    // ===== SECTION 4: ACCESSORY (tid1=1, tid2=5) =====
    settings.eqRing = IsRingChecked();          // m_chkRing → tid3=3
    settings.eqEarring = IsEarringChecked();    // m_chkEarring → tid3=1
    settings.eqNecklace = IsNecklaceChecked();  // m_chkNecklace → tid3=2
    
    // Legacy: combined accessory flag
    settings.eqAccessory = settings.eqRing || settings.eqEarring || settings.eqNecklace;
    
    // ===== SECTION 5: OTHERS =====
    settings.gold = IsGoldChecked();
    settings.alchemyElixirWeapon = IsElixirsChecked();
    settings.potionHP = IsPotionChecked();
    settings.alchemyTabletBlue = IsTabletsChecked();
    settings.alchemyMaterial = IsMaterialsChecked();
    settings.universalPill = IsQuestsChecked();
    settings.returnScroll = IsReturnScrollChecked();
    // m_chkOthers - for other items not categorized
    
    printf("[PetFilter] Settings synced from GUI:\n");
    printf("  Enabled: %s\n", settings.enabled ? "ON" : "OFF");
    printf("  Spear: %s, Glaive: %s, Bow: %s, Blade: %s, Sword: %s\n",
           settings.eqSpear ? "ON" : "OFF",
           settings.eqGlaive ? "ON" : "OFF",
           settings.eqBow ? "ON" : "OFF",
           settings.eqBlade ? "ON" : "OFF",
           settings.eqSword ? "ON" : "OFF");
    printf("  Armor: %s, Protector: %s, Garment: %s\n",
           settings.eqHeavy ? "ON" : "OFF",
           settings.eqLight ? "ON" : "OFF",
           settings.eqClothes ? "ON" : "OFF");
    printf("  Ring: %s, Earring: %s, Necklace: %s\n",
           settings.eqRing ? "ON" : "OFF",
           settings.eqEarring ? "ON" : "OFF",
           settings.eqNecklace ? "ON" : "OFF");
    printf("  Gold: %s, Elixirs: %s, Tablets: %s, Materials: %s\n",
           settings.gold ? "ON" : "OFF",
           settings.alchemyElixirWeapon ? "ON" : "OFF",
           settings.alchemyTabletBlue ? "ON" : "OFF",
           settings.alchemyMaterial ? "ON" : "OFF");
}

int CIFPetFilterSettings::OnMouseMove(int a1, int x, int y) { return 0; }

void CIFPetFilterSettings::ResetPosition() {
    if (!PetFilterSettingsMainFrame) return;
    int sw = g_D3DViewportWidth > 0 ? g_D3DViewportWidth : GetSystemMetrics(SM_CXSCREEN);
    int sh = g_D3DViewportHeight > 0 ? g_D3DViewportHeight : GetSystemMetrics(SM_CYSCREEN);
    PetFilterSettingsMainFrame->MoveGWnd((sw - 480) / 2, (sh - 480) / 4);
}

void CIFPetFilterSettings::ActivateTabPage(int page) { }
void CIFPetFilterSettings::ShowTab(int tabIndex) { }
void CIFPetFilterSettings::ShowTab0(bool show) { }
void CIFPetFilterSettings::ShowTab1(bool show) { }
void CIFPetFilterSettings::ShowTab2(bool show) { }
void CIFPetFilterSettings::ShowTab3(bool show) { }

// Checkbox getters - Individual Weapon Types
bool CIFPetFilterSettings::IsSpearChecked() const { return m_chkSpear && m_chkSpear->GetCheckedState_MAYBE(); }
bool CIFPetFilterSettings::IsGlaiveChecked() const { return m_chkGlaive && m_chkGlaive->GetCheckedState_MAYBE(); }
bool CIFPetFilterSettings::IsBowChecked() const { return m_chkBow && m_chkBow->GetCheckedState_MAYBE(); }
bool CIFPetFilterSettings::IsBladeChecked() const { return m_chkBlade && m_chkBlade->GetCheckedState_MAYBE(); }
bool CIFPetFilterSettings::IsSwordChecked() const { return m_chkSword && m_chkSword->GetCheckedState_MAYBE(); }

// Checkbox getters - Clothes Section
bool CIFPetFilterSettings::IsEqArmorChecked() const { return m_chkEqArmor && m_chkEqArmor->GetCheckedState_MAYBE(); }
bool CIFPetFilterSettings::IsEqProtectorChecked() const { return m_chkEqProtector && m_chkEqProtector->GetCheckedState_MAYBE(); }
bool CIFPetFilterSettings::IsEqGarmentChecked() const { return m_chkEqGarment && m_chkEqGarment->GetCheckedState_MAYBE(); }

// Checkbox getters - Accessory Section
bool CIFPetFilterSettings::IsRingChecked() const { return m_chkRing && m_chkRing->GetCheckedState_MAYBE(); }
bool CIFPetFilterSettings::IsEarringChecked() const { return m_chkEarring && m_chkEarring->GetCheckedState_MAYBE(); }
bool CIFPetFilterSettings::IsNecklaceChecked() const { return m_chkNecklace && m_chkNecklace->GetCheckedState_MAYBE(); }

// Checkbox getters - Degree (keeping for future use)
bool CIFPetFilterSettings::IsEQDegreeChecked(int d) const { 
    if (d < 1 || d > 15) return false;
    return m_chkEqDegree[d-1] && m_chkEqDegree[d-1]->GetCheckedState_MAYBE(); 
}

// Checkbox getters - Others Section
bool CIFPetFilterSettings::IsGoldChecked() const { return m_chkGold && m_chkGold->GetCheckedState_MAYBE(); }
bool CIFPetFilterSettings::IsElixirsChecked() const { return m_chkAlchElixirWeapon && m_chkAlchElixirWeapon->GetCheckedState_MAYBE(); }
bool CIFPetFilterSettings::IsPotionChecked() const { return m_chkPotion && m_chkPotion->GetCheckedState_MAYBE(); }
bool CIFPetFilterSettings::IsTabletsChecked() const { return m_chkAlchTablets && m_chkAlchTablets->GetCheckedState_MAYBE(); }
bool CIFPetFilterSettings::IsMaterialsChecked() const { return m_chkAlchMaterials && m_chkAlchMaterials->GetCheckedState_MAYBE(); }
bool CIFPetFilterSettings::IsQuestsChecked() const { return m_chkQuests && m_chkQuests->GetCheckedState_MAYBE(); }
bool CIFPetFilterSettings::IsReturnScrollChecked() const { return m_chkReturnScroll && m_chkReturnScroll->GetCheckedState_MAYBE(); }
bool CIFPetFilterSettings::IsOthersChecked() const { return m_chkOthers && m_chkOthers->GetCheckedState_MAYBE(); }

// Helper function to get executable directory
static std::string GetPetFilterExeDir() {
    char buffer[MAX_PATH];
    GetModuleFileNameA(NULL, buffer, MAX_PATH);
    std::string exePath(buffer);
    size_t lastSlash = exePath.find_last_of("\\/");
    return exePath.substr(0, lastSlash + 1);
}

bool CIFPetFilterSettings::SaveConfig() {
    std::string executableDir = GetPetFilterExeDir();
    
    // Get character name for per-character settings
    const char* charName = "Default";
    if (g_pCICPlayerEcsro && g_pCICPlayerEcsro->charname[0] != '\0') {
        charName = g_pCICPlayerEcsro->charname;
    }
    
    char buffer[MAX_PATH];
    sprintf_s(buffer, sizeof(buffer), "%s\\Setting\\PetFilter_%s.dat",
        executableDir.c_str(), charName);
    
    // Create Setting directory if it doesn't exist
    char settingDir[MAX_PATH];
    sprintf_s(settingDir, sizeof(settingDir), "%s\\Setting", executableDir.c_str());
    _mkdir(settingDir);
    
    FILE* f = fopen(buffer, "wb+");
    if (!f) {
        printf("[PetFilter] ERROR: Failed to save to %s\n", buffer);
        return false;
    }
    
    // Write 22 checkbox states with correct names
    bool data[22] = {
        g_radioOnSelected,                                                  // [0] ON/OFF radio
        // Weapon section (5)
        m_chkSpear && m_chkSpear->GetCheckedState_MAYBE(),                  // [1] Spear
        m_chkGlaive && m_chkGlaive->GetCheckedState_MAYBE(),                // [2] Glaive
        m_chkBow && m_chkBow->GetCheckedState_MAYBE(),                      // [3] Bow
        m_chkBlade && m_chkBlade->GetCheckedState_MAYBE(),                  // [4] Blade
        m_chkSword && m_chkSword->GetCheckedState_MAYBE(),                  // [5] Sword
        // Clothes section (3)
        m_chkEqArmor && m_chkEqArmor->GetCheckedState_MAYBE(),              // [6] Armor
        m_chkEqProtector && m_chkEqProtector->GetCheckedState_MAYBE(),      // [7] Protector
        m_chkEqGarment && m_chkEqGarment->GetCheckedState_MAYBE(),          // [8] Garment
        // Accessory section (3)
        m_chkRing && m_chkRing->GetCheckedState_MAYBE(),                    // [9] Ring
        m_chkEarring && m_chkEarring->GetCheckedState_MAYBE(),              // [10] Earring
        m_chkNecklace && m_chkNecklace->GetCheckedState_MAYBE(),            // [11] Necklace
        // Others section (8)
        m_chkGold && m_chkGold->GetCheckedState_MAYBE(),                    // [12] Gold
        m_chkAlchElixirWeapon && m_chkAlchElixirWeapon->GetCheckedState_MAYBE(), // [13] Elixirs
        m_chkPotion && m_chkPotion->GetCheckedState_MAYBE(),                // [14] Potion
        m_chkAlchTablets && m_chkAlchTablets->GetCheckedState_MAYBE(),      // [15] Tablets
        m_chkAlchMaterials && m_chkAlchMaterials->GetCheckedState_MAYBE(),  // [16] Material
        m_chkQuests && m_chkQuests->GetCheckedState_MAYBE(),                // [17] Quest
        m_chkReturnScroll && m_chkReturnScroll->GetCheckedState_MAYBE(),    // [18] Return Scroll
        m_chkOthers && m_chkOthers->GetCheckedState_MAYBE(),                // [19] Others
        false,  // [20] Reserved
        false   // [21] Reserved
    };
    fwrite(data, sizeof(bool), 22, f);
    fclose(f);
    
    printf("[PetFilter] Config saved to: %s\n", buffer);
    return true;
}

bool CIFPetFilterSettings::LoadConfig() {
    std::string executableDir = GetPetFilterExeDir();
    
    // Get character name for per-character settings
    const char* charName = "Default";
    if (g_pCICPlayerEcsro && g_pCICPlayerEcsro->charname[0] != '\0') {
        charName = g_pCICPlayerEcsro->charname;
    }
    
    char buffer[MAX_PATH];
    sprintf_s(buffer, sizeof(buffer), "%s\\Setting\\PetFilter_%s.dat",
        executableDir.c_str(), charName);
    
    FILE* f = fopen(buffer, "rb");
    if (!f) {
        printf("[PetFilter] Config not found, using defaults: %s\n", buffer);
        // Set defaults - all checkboxes off, filter disabled
        g_radioOnSelected = false;
        SwitchPetFilter = false;
        
        // Update radio button visuals
        if (g_radioOnStatic && g_radioOffStatic) {
            g_radioOnStatic->TB_Func_12("interface\\ifcommon\\com_radiobutton_off.ddj", 1, 0);
            g_radioOffStatic->TB_Func_12("interface\\ifcommon\\com_radiobutton_on.ddj", 1, 0);
        }
        
        // Explicitly uncheck all checkboxes
        if (m_chkSpear) m_chkSpear->FUN_00656d50(false);
        if (m_chkGlaive) m_chkGlaive->FUN_00656d50(false);
        if (m_chkBow) m_chkBow->FUN_00656d50(false);
        if (m_chkBlade) m_chkBlade->FUN_00656d50(false);
        if (m_chkSword) m_chkSword->FUN_00656d50(false);
        if (m_chkEqArmor) m_chkEqArmor->FUN_00656d50(false);
        if (m_chkEqProtector) m_chkEqProtector->FUN_00656d50(false);
        if (m_chkEqGarment) m_chkEqGarment->FUN_00656d50(false);
        if (m_chkRing) m_chkRing->FUN_00656d50(false);
        if (m_chkEarring) m_chkEarring->FUN_00656d50(false);
        if (m_chkNecklace) m_chkNecklace->FUN_00656d50(false);
        if (m_chkGold) m_chkGold->FUN_00656d50(false);
        if (m_chkAlchElixirWeapon) m_chkAlchElixirWeapon->FUN_00656d50(false);
        if (m_chkPotion) m_chkPotion->FUN_00656d50(false);
        if (m_chkAlchTablets) m_chkAlchTablets->FUN_00656d50(false);
        if (m_chkAlchMaterials) m_chkAlchMaterials->FUN_00656d50(false);
        if (m_chkQuests) m_chkQuests->FUN_00656d50(false);
        if (m_chkReturnScroll) m_chkReturnScroll->FUN_00656d50(false);
        if (m_chkOthers) m_chkOthers->FUN_00656d50(false);
        
        // Sync to PetFilter - disabled
        extern void SetPetFilterEnabled(bool enabled);
        SetPetFilterEnabled(false);
        
        return true;  // Not an error - just use defaults
    }
    
    // Read 22 checkbox states
    bool data[22];
    memset(data, 0, sizeof(data));  // Initialize to false
    size_t read = fread(data, sizeof(bool), 22, f);
    fclose(f);
    
    if (read < 1) {
        printf("[PetFilter] ERROR: Failed to read config\n");
        return false;
    }
    
    // Apply to UI using FUN_00656d50
    g_radioOnSelected = data[0];
    SwitchPetFilter = data[0];
    
    // Update radio button visuals
    if (g_radioOnStatic && g_radioOffStatic) {
        if (g_radioOnSelected) {
            g_radioOnStatic->TB_Func_12("interface\\ifcommon\\com_radiobutton_on.ddj", 1, 0);
            g_radioOffStatic->TB_Func_12("interface\\ifcommon\\com_radiobutton_off.ddj", 1, 0);
        } else {
            g_radioOnStatic->TB_Func_12("interface\\ifcommon\\com_radiobutton_off.ddj", 1, 0);
            g_radioOffStatic->TB_Func_12("interface\\ifcommon\\com_radiobutton_on.ddj", 1, 0);
        }
    }
    
    // Apply checkbox states - Weapon section
    if (m_chkSpear) m_chkSpear->FUN_00656d50(data[1]);
    if (m_chkGlaive) m_chkGlaive->FUN_00656d50(data[2]);
    if (m_chkBow) m_chkBow->FUN_00656d50(data[3]);
    if (m_chkBlade) m_chkBlade->FUN_00656d50(data[4]);
    if (m_chkSword) m_chkSword->FUN_00656d50(data[5]);
    
    // Clothes section
    if (m_chkEqArmor) m_chkEqArmor->FUN_00656d50(data[6]);
    if (m_chkEqProtector) m_chkEqProtector->FUN_00656d50(data[7]);
    if (m_chkEqGarment) m_chkEqGarment->FUN_00656d50(data[8]);
    
    // Accessory section
    if (m_chkRing) m_chkRing->FUN_00656d50(data[9]);
    if (m_chkEarring) m_chkEarring->FUN_00656d50(data[10]);
    if (m_chkNecklace) m_chkNecklace->FUN_00656d50(data[11]);
    
    // Others section
    if (m_chkGold) m_chkGold->FUN_00656d50(data[12]);
    if (m_chkAlchElixirWeapon) m_chkAlchElixirWeapon->FUN_00656d50(data[13]);
    if (m_chkPotion) m_chkPotion->FUN_00656d50(data[14]);
    if (m_chkAlchTablets) m_chkAlchTablets->FUN_00656d50(data[15]);
    if (m_chkAlchMaterials) m_chkAlchMaterials->FUN_00656d50(data[16]);
    if (m_chkQuests) m_chkQuests->FUN_00656d50(data[17]);
    if (m_chkReturnScroll) m_chkReturnScroll->FUN_00656d50(data[18]);
    if (m_chkOthers) m_chkOthers->FUN_00656d50(data[19]);
    
    // Sync to PetFilter
    extern void SetPetFilterEnabled(bool enabled);
    SetPetFilterEnabled(SwitchPetFilter);
    
    // Call SwitchFilter to sync all checkbox states to PetFilter settings
    SwitchFilter();
    
    printf("[PetFilter] Config loaded from: %s\n", buffer);
    return true;
}

void CIFPetFilterSettings::UpdateRadioButtons(bool onState) {
    /* Update both the visual AND the internal state */
    g_radioOnSelected = onState;
    
    // Update radio button visuals based on state
    if (g_radioOnStatic && g_radioOffStatic) {
        if (onState) {
            g_radioOnStatic->TB_Func_12("interface\\ifcommon\\com_radiobutton_on.ddj", 1, 0);
            g_radioOffStatic->TB_Func_12("interface\\ifcommon\\com_radiobutton_off.ddj", 1, 0);
        } else {
            g_radioOnStatic->TB_Func_12("interface\\ifcommon\\com_radiobutton_off.ddj", 1, 0);
            g_radioOffStatic->TB_Func_12("interface\\ifcommon\\com_radiobutton_on.ddj", 1, 0);
        }
    }
}
