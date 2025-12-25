#pragma once

#include "IFButton.h"
#include "IFCheckBox.h"
#include "IFFrame.h"
#include "IFMainFrame.h"
#include "IFNormalTile.h"
#include "IFStatic.h"
#include "IFLattice.h"
#include "IFTileWnd.h"
#include "IFStretchWnd.h"
#include "IFVerticalScroll.h"
#include "IFScrollManager.h"
#include "IFSlotWithHelp.h"
#include "IFSkillSlot.h"
#include <IFHScroll_Option.h>
#include <vector>

#define GDR_AUTO_HUNT_SETTINGS 19703

// Tab indices
#define TAB_AUTO_HUNT         0
#define TAB_AUTO_ATTACK_SKILLS 1
#define TAB_AUTO_BUFF_SKILLS   2

// Skill slot grid configuration
#define SKILL_SLOT_COLS        6     // 6 columns
#define SKILL_SLOT_ROWS        8     // 8 rows
#define SKILL_SLOT_SIZE        32    // 32x32 pixels
#define SKILL_SLOT_COUNT       (SKILL_SLOT_COLS * SKILL_SLOT_ROWS)  // 48 slots per panel

// =====================================================================
// CIFAutoHuntSettings - Auto Hunt Settings Window
// Contains:
//   Tab 1: Hunt options (checkboxes + range)
//   Tab 2: Attack Skills configuration
//   Tab 3: Buff Skills configuration
// =====================================================================

class CIFAutoHuntSettings : public CIFMainFrame {
    GFX_DECLARE_DYNCREATE(CIFAutoHuntSettings)
    GFX_DECLARE_MESSAGE_MAP(CIFAutoHuntSettings)

public:
    CIFAutoHuntSettings();
    ~CIFAutoHuntSettings();

    bool OnCreate(long ln) override;
    void OnUpdate() override;
    int OnMouseMove(int a1, int x, int y) override;
    void ResetPosition();

    // Button handlers (registered in MESSAGE_MAP)
    void OnClick_Confirm();
    void OnClick_Cancel();

    // Configuration
    bool SaveConfig();
    bool LoadConfig();

    // Tab management
    void SwitchToTab(int tabIndex);
    int GetActiveTab() const { return m_activeTab; }

    // Checkbox state getters
    bool IsAutoBerserkChecked() const;
    bool IsReturnToTownOnDeathChecked() const;
    bool IsTownOnLowHPChecked() const;
    bool IsTownOnLowMPChecked() const;
    bool IsTownOnLowPetHPChecked() const;
    bool IsTownOnLowArrowChecked() const;
    bool IsTownOnLowDurabilityChecked() const;
    bool IsGoBackCenterChecked() const;
    bool IsWalkAroundChecked() const;

    // Range getter - returns value in game units (slider is 0-500, multiplied by 10)
    float GetRangeValue() const { return (float)m_rangeValue * 10.0f; }

private:
    void UpdateTabButtons();
    
    void CreateAutoHuntPanel();
    void CreateAttackSkillsPanel();
    void CreateBuffSkillsPanel();
    
    void ShowAutoHuntElements(bool show);
    void ShowAttackSkillsElements(bool show);
    void ShowBuffSkillsElements(bool show);
    
    // Skill population from memory
    void PopulateLearnedSkills();

public:
    // Called when window is shown to refresh all learned skills
    void RefreshLearnedSkills();

private:
    // Tab system (3 tabs)
    int m_activeTab;
    CIFStatic *m_tabAutoHunt;
    CIFStatic *m_tabAttackSkills;
    CIFStatic *m_tabBuffSkills;

    // ========== AUTO HUNT PANEL ELEMENTS ==========
    CIFStatic *m_huntTabHeader;
    CIFStatic *m_huntTabLabel;
    CIFFrame *m_huntSection1Frame;
    CIFStatic *m_rangeTabHeader;
    CIFStatic *m_rangeTabLabel;
    CIFFrame *m_rangeSectionFrame;
    CIFFrame *m_rangeSection2Frame;  // Second subframe (Go Back Center, Walk Around)
    
    // Checkboxes + labels (only Auto Berserk remains in Hunt Settings frame)
    CIFCheckBox *m_chkAutoBerserk;      CIFStatic *m_lblAutoBerserk;
    CIFCheckBox *m_chkReturnToTownOnDeath;  CIFStatic *m_lblReturnToTownOnDeath;
    CIFCheckBox *m_chkTownOnLowHP;      CIFStatic *m_lblTownOnLowHP;
    CIFCheckBox *m_chkTownOnLowMP;      CIFStatic *m_lblTownOnLowMP;
    CIFCheckBox *m_chkTownOnLowPetHP;   CIFStatic *m_lblTownOnLowPetHP;
    CIFCheckBox *m_chkTownOnLowArrow;   CIFStatic *m_lblTownOnLowArrow;
    CIFCheckBox *m_chkTownOnLowDurability; CIFStatic *m_lblTownOnLowDurability;
    // Go Back Center and Walk Around are in second subframe
    CIFCheckBox *m_chkGoBackCenter;     CIFStatic *m_lblGoBackCenter;
    CIFCheckBox *m_chkWalkAround;       CIFStatic *m_lblWalkAround;

    // Range slider + value display
    CIFStatic *m_rangeSliderBg;          // Slider background
    CIFHScroll_Option *m_rangeSlider;    // The slider control
    CIFStatic *m_rangeValueText;         // Text showing current value
    int m_rangeValue;                    // Actual range value (0-500)

    // ========== ATTACK SKILLS PANEL ELEMENTS ==========
    CIFStatic *m_atkInfoLabel;           // Info text
    CIFFrame *m_atkAcquiredFrame;        // Left panel frame
    CIFStatic *m_atkAcquiredLabel;       // "Acquired Skills" label
    CIFStretchWnd *m_atkAcquiredOutline;    // Lattice outline (left)
    CIFLattice *m_atkAcquiredLattice;    // Background grid (left)
    CIFSlotWithHelp *m_atkAcquiredSlots[SKILL_SLOT_COUNT]; // Skill slots - DRAG SOURCE (slot type 0x49)
    CIFVerticalScroll *m_atkAcquiredScroll;
    CIFFrame *m_atkToUseFrame;           // Right panel frame
    CIFStatic *m_atkToUseLabel;          // "Skills to Use" label
    CIFStretchWnd *m_atkToUseOutline;       // Lattice outline (right)
    CIFLattice *m_atkToUseLattice;       // Background grid (right)
    CIFSlotWithHelp *m_atkToUseSlots[SKILL_SLOT_COUNT]; // Skill slots - DROP TARGET (like Underbar)
    CIFVerticalScroll *m_atkToUseScroll; // Scrollbar

    // ========== BUFF SKILLS PANEL ELEMENTS ==========
    CIFStatic *m_buffInfoLabel;          // Info text
    CIFFrame *m_buffAcquiredFrame;       // Left panel
    CIFStatic *m_buffAcquiredLabel;
    CIFStretchWnd *m_buffAcquiredOutline;   // Lattice outline (left)
    CIFLattice *m_buffAcquiredLattice;   // Background grid (left)
    CIFSlotWithHelp *m_buffAcquiredSlots[SKILL_SLOT_COUNT]; // Skill slots - DRAG SOURCE (slot type 0x49)
    CIFVerticalScroll *m_buffAcquiredScroll;
    CIFFrame *m_buffToUseFrame;          // Right panel
    CIFStatic *m_buffToUseLabel;
    CIFStretchWnd *m_buffToUseOutline;      // Lattice outline (right)
    CIFLattice *m_buffToUseLattice;      // Background grid (right)
    CIFSlotWithHelp *m_buffToUseSlots[SKILL_SLOT_COUNT]; // Skill slots on top of grid
    CIFVerticalScroll *m_buffToUseScroll; // Scrollbar

    // Buttons
    CIFButton *m_btnConfirm;
    CIFButton *m_btnCancel;

    // Window position tracking for vertex recalc
    int m_lastWindowX;
    int m_lastWindowY;

    // ========== SKILL ID TRACKING (Skills to Use) ==========
    DWORD m_atkSelectedSkillIds[SKILL_SLOT_COUNT];   // Attack skill IDs in "Skills to Use"
    DWORD m_buffSelectedSkillIds[SKILL_SLOT_COUNT];  // Buff skill IDs in "Buffs to Use"
    int m_atkSelectedCount;                           // Number of selected attack skills
    int m_buffSelectedCount;                          // Number of selected buff skills

    // ========== CACHED SKILL DATA (ALL learned skills, not just visible) ==========
    struct CachedSkillInfo {
        DWORD skillId;
        char iconPath[260];  // DDJ path for icon
    };
    std::vector<CachedSkillInfo> m_allAtkSkills;   // ALL attack skills (can be 200+)
    std::vector<CachedSkillInfo> m_allBuffSkills;  // ALL buff skills (can be 200+)
    
    // Scroll offset for Acquired Skills panels (which row to start from)
    int m_atkScrollOffset;    // Attack skills scroll offset (in rows)
    int m_buffScrollOffset;   // Buff skills scroll offset (in rows)
    int m_lastAtkScrollOffset;  // Track changes for re-render
    int m_lastBuffScrollOffset;

    // ========== MANUAL DRAG STATE (for custom drop detection) ==========
    bool m_isDragging;                    // True if we're tracking a drag
    int m_dragSourceSlotIndex;            // Index of source slot (0-47)
    DWORD m_dragSkillId;                  // Skill ID being dragged
    char m_dragIconPath[260];             // DDJ path of dragged icon
    bool m_dragIsAttackSkill;             // True if from attack panel, false if buff
    bool m_dragFromToUsePanel;            // True if drag from Skills to Use (for removal)

public:
    // Getters for AutoHunt system
    int GetSelectedAttackSkillCount() const { return m_atkSelectedCount; }
    int GetSelectedBuffSkillCount() const { return m_buffSelectedCount; }
    const DWORD* GetSelectedAttackSkillIds() const { return m_atkSelectedSkillIds; }
    const DWORD* GetSelectedBuffSkillIds() const { return m_buffSelectedSkillIds; }
};

extern CIFMainFrame *AutoHuntSettingsMainFrame;
extern CIFAutoHuntSettings *g_pCIFAutoHuntSettings;