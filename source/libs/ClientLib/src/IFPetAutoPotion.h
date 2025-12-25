#pragma once

#include "IFMainFrame.h"
#include "IFStatic.h"
#include "IFCheckBox.h"
#include "IFButton.h"
#include "IFFrame.h"
#include "IFNormalTile.h"
#include "IFHScroll_Option.h"

#define GDR_PET_AUTO_POTION 19701

// =====================================================================
// CIFPetAutoPotion - Pet Auto Potion Settings Window
// Two sections:
//   1. Attack Pet: HP, HGP, Cure
//   2. Transport Pet (Horse/Job): HP, Cure (no HGP)
// =====================================================================
class CIFPetAutoPotion : public CIFMainFrame
{
    GFX_DECLARE_DYNCREATE(CIFPetAutoPotion)
    GFX_DECLARE_MESSAGE_MAP(CIFPetAutoPotion)

public:
    CIFPetAutoPotion();
    ~CIFPetAutoPotion();

    bool OnCreate(long ln) override;
    void OnUpdate() override;
    int OnMouseMove(int a1, int x, int y) override;
    void ResetPosition();

    // Configuration getters
    bool IsAttackPetHPEnabled() const { return m_attackPetHP; }
    bool IsAttackPetHGPEnabled() const { return m_attackPetHGP; }
    bool IsAttackPetCureEnabled() const { return m_attackPetCure; }
    bool IsTransportPetHPEnabled() const { return m_transportPetHP; }
    bool IsTransportPetCureEnabled() const { return m_transportPetCure; }
    
    int GetAttackPetHPThreshold() const { return m_attackHPThreshold; }
    int GetAttackPetHGPThreshold() const { return m_attackHGPThreshold; }
    int GetTransportPetHPThreshold() const { return m_transportHPThreshold; }
    
    // Save config (called by macro window after toggling)
    bool SaveConfig();
    
    // Checkbox access for macro window icon sync
    CIFCheckBox* m_chkAttackHP;
    CIFCheckBox* m_chkAttackHGP;
    CIFCheckBox* m_chkAttackCure;
    CIFCheckBox* m_chkTransportHP;
    CIFCheckBox* m_chkTransportCure;
    
    // State variables - public for macro window access
    bool m_attackPetHP;
    bool m_attackPetHGP;
    bool m_attackPetCure;
    bool m_transportPetHP;
    bool m_transportPetCure;

private:
    // Button handlers
    void On_BtnClickConfirm();
    void On_BtnClickCancel();
    
    // Checkbox handlers
    void On_CheckAttackHP();
    void On_CheckAttackHGP();
    void On_CheckAttackCure();
    void On_CheckTransportHP();
    void On_CheckTransportCure();
    
    // Slider handlers
    void OnAttackHPSliderChanged();
    void OnAttackHGPSliderChanged();
    void OnTransportHPSliderChanged();

    // Load config
    bool LoadConfig();
    
    // Checkbox-Slider synchronization
    void On_CheckBoxScrollBar();

private:
    // === Threshold Settings ===
    int m_attackHPThreshold;      // 1-100%
    int m_attackHGPThreshold;     // 1-100%
    int m_transportHPThreshold;   // 1-100%
    
    // UI Controls - Attack Pet
    CIFStatic* m_lblAttackPet;
    // Checkboxes moved to public section
    CIFStatic* m_lblAttackHP;
    CIFStatic* m_lblAttackHGP;
    CIFStatic* m_lblAttackCure;
    CIFHScroll_Option* m_sliderAttackHP;
    CIFHScroll_Option* m_sliderAttackHGP;
    CIFStatic* m_lblAttackHPValue;
    CIFStatic* m_lblAttackHGPValue;
    
    // UI Controls - Transport Pet
    CIFStatic* m_lblTransportPet;
    // Checkboxes moved to public section
    CIFStatic* m_lblTransportHP;
    CIFStatic* m_lblTransportCure;
    CIFHScroll_Option* m_sliderTransportHP;
    CIFStatic* m_lblTransportHPValue;
    
    CIFButton* m_btnConfirm;
    CIFButton* m_btnCancel;
    
    // Slider background statics (for enable/disable visual)
    CIFStatic* m_sliderBgAttackHP;
    CIFStatic* m_sliderBgAttackHGP;
    CIFStatic* m_sliderBgTransportHP;
    
    // Slider state tracking (true = disabled)
    bool m_attackHPSliderDisabled;
    bool m_attackHGPSliderDisabled;
    bool m_transportHPSliderDisabled;
    
    // Last known slider values for change detection
    int m_lastAttackHPSlider;
    int m_lastAttackHGPSlider;
    int m_lastTransportHPSlider;
};

// Global enable flag (controlled by macro window toggle)
extern bool g_PetAutoPotionEnabled;

// Global frame pointer
extern CIFMainFrame* PetAutoPotionMainFrame;

// Global pointer to the actual CIFPetAutoPotion class instance (for accessing settings)
extern CIFPetAutoPotion* g_pCIFPetAutoPotion;
