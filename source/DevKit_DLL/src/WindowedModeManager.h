#pragma once
#include <Windows.h>

// Global manager for windowed mode checkbox state
// This lives in DevKit_DLL to avoid cross-module static variable issues

class WindowedModeManager {
public:
    static WindowedModeManager& Instance();
    
    // Called by CIFOption_Game when checkbox is created
    void SetCheckboxPointer(void* checkbox);
    
    // Called by CIFOption_Game when window is destroyed
    void ClearCheckboxPointer();
    
    // Get current checkbox state using game function at 0x004240A0
    bool GetCheckboxState() const;
    
    // Load/Save functions
    void LoadSetting();
    void SaveSetting();
    
    // Polling function - called every frame from EndScene
    void PollAndSave();
    
    // Check if we have a valid checkbox
    bool HasCheckbox() const { return m_pCheckbox != NULL; }

private:
    WindowedModeManager();
    
    void* m_pCheckbox;
    bool m_lastKnownState;
    DWORD m_lastDebugLog;
};

// Get the global instance
#define g_WindowedModeManager WindowedModeManager::Instance()
