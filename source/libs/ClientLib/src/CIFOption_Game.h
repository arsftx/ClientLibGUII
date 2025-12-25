#pragma once
#include "GInterface.h"
#include "IFButton.h"
#include "IFCheckBox.h"
#include "IFNormalTile.h"
#include "IFStatic.h"

class CIFOption_Game : public CIFWnd {
    GFX_DECLARE_DYNAMIC_EXISTING(CIFOption_Game, 0x9FFE94)

public:
    CIFOption_Game();
    virtual ~CIFOption_Game();

    bool OnCreateIMPL(long ln);
    
    // Static function for EndScene polling - called every frame
    static void PollCheckboxState();
    
    // Get windowed mode state
    bool IsWindowedModeChecked() const;

private:
    // Load/Save settings to file
    void LoadWindowedModeSetting();
    void SaveWindowedModeSetting();
    
    // Check and save if state changed
    void CheckAndSaveIfChanged();

private:
    CIFButton *m_pTestButton;
    CIFStatic *m_pTestLabel;
    CIFCheckBox *m_pWindowedModeCheckBox;
    CIFStatic *m_pWindowedModeLabel;
    CIFStatic *m_pWindowedModeAllowLabel;
    CIFStatic *m_pWindowedModeBackground;
    
    // Track last known checkbox state for polling
    bool m_lastKnownState;
    
    // Static instance pointer for polling
    static CIFOption_Game* s_pActiveInstance;
};