#pragma once

#include "IFMainFrame.h"
#include "IFStatic.h"
#include "IFCheckBox.h"
#include "IFButton.h"
#include "IFFrame.h"
#include "IFNormalTile.h"
#include "IFSelectableArea.h"

#define GDR_PET_FILTER_SETTINGS 19702

// =====================================================================
// CIFPetFilterSettings - Pet Filter Settings Window
// 4 Tabs:
//   Tab 0: Equipment - Weapon, Accessory, Heavy, Light, Clothes, Gender, Degree
//   Tab 1: Alchemy - Elixirs, Tablets, Stones, Materials, Degree
//   Tab 2: Others - Potions, Grains, Pills, Scrolls, Arrows, Bolts, Coins
//   Tab 3: Gold - Gold pickup
// =====================================================================

class CIFPetFilterSettings : public CIFMainFrame
{
    GFX_DECLARE_DYNCREATE(CIFPetFilterSettings)
    GFX_DECLARE_MESSAGE_MAP(CIFPetFilterSettings)

public:
    CIFPetFilterSettings();
    ~CIFPetFilterSettings();

    bool OnCreate(long ln) override;
    void OnUpdate() override;
    int OnMouseMove(int a1, int x, int y) override;
    void ResetPosition();
    
    // Tab management
    void ShowTab(int tabIndex);
    void ActivateTabPage(int page);
    
    // Filter toggle
    void SwitchFilter();
    static bool SwitchPetFilter;
    
    // Configuration
    bool SaveConfig();
    bool LoadConfig();
    
    // Update radio button visuals (called from MacroWindow)
    void UpdateRadioButtons(bool onState);
    
    // Checkbox state getters - Individual Weapon Types
    bool IsSpearChecked() const;
    bool IsGlaiveChecked() const;
    bool IsBowChecked() const;
    bool IsBladeChecked() const;
    bool IsSwordChecked() const;
    
    // Checkbox state getters - Clothes Section
    bool IsEqArmorChecked() const;
    bool IsEqProtectorChecked() const;
    bool IsEqGarmentChecked() const;
    
    // Checkbox state getters - Accessory Section
    bool IsRingChecked() const;
    bool IsEarringChecked() const;
    bool IsNecklaceChecked() const;
    
    // Checkbox state getters - Degree
    bool IsEQDegreeChecked(int degree) const;  // 1-15
    
    // Checkbox state getters - Others Section
    bool IsGoldChecked() const;
    bool IsElixirsChecked() const;
    bool IsPotionChecked() const;
    bool IsTabletsChecked() const;
    bool IsMaterialsChecked() const;
    bool IsQuestsChecked() const;
    bool IsReturnScrollChecked() const;
    bool IsOthersChecked() const;

private:
    // Create UI helpers
    void CreateTab0_Equipment();
    void CreateTab1_Alchemy();
    void CreateTab2_Others();
    void CreateTab3_Gold();
    
    void ShowTab0(bool show);
    void ShowTab1(bool show);
    void ShowTab2(bool show);
    void ShowTab3(bool show);

private:
    // Current active tab
    int m_currentTab;
    
    // Tabs
    static const int NUM_TABS = 4;
    CIFSelectableArea* m_pTabs[NUM_TABS];
    
    // Switch button
    CIFButton* m_btnSwitch;
    
    // === TAB 0: Equipment ===
    // Weapon Section (5 checkboxes)
    CIFCheckBox* m_chkSpear;    // tid3=4 - Spear
    CIFCheckBox* m_chkGlaive;   // tid3=5 - Glaive
    CIFCheckBox* m_chkBow;      // tid3=6 - Bow
    CIFCheckBox* m_chkBlade;    // tid3=3 - Blade
    CIFCheckBox* m_chkSword;    // tid3=2 - Sword
    
    // Clothes Section (3 checkboxes)
    CIFCheckBox* m_chkEqArmor;      // Armor
    CIFCheckBox* m_chkEqProtector;  // Protector
    CIFCheckBox* m_chkEqGarment;    // Garment
    
    // Accessory Section (3 checkboxes)
    CIFCheckBox* m_chkRing;         // Ring
    CIFCheckBox* m_chkEarring;      // Earring
    CIFCheckBox* m_chkNecklace;     // Necklace
    
    // Degree (15 checkboxes - kept for future use)
    CIFCheckBox* m_chkEqDegree[15];
    CIFCheckBox* m_chkAlchDegree[15];
    
    // === Others Section ===
    // Row 1
    CIFCheckBox* m_chkGold;             // Gold
    CIFCheckBox* m_chkAlchElixirWeapon; // Elixirs
    CIFCheckBox* m_chkPotion;           // Potion
    CIFCheckBox* m_chkAlchTablets;      // Tablets
    
    // Row 2
    CIFCheckBox* m_chkAlchMaterials;    // Material
    CIFCheckBox* m_chkQuests;           // Quest
    CIFCheckBox* m_chkReturnScroll;     // Return Scroll
    CIFCheckBox* m_chkOthers;           // Others
};

// Global frame pointer
extern CIFMainFrame* PetFilterSettingsMainFrame;

// Global class pointer
extern CIFPetFilterSettings* g_pCIFPetFilterSettings;
