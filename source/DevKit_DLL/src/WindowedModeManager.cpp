#include "WindowedModeManager.h"
#include "WindowedMode.h"
#include <stdio.h>
#include <time.h>
#include <stdarg.h>

// Log to ClientLog.txt - DISABLED
static void WMLog(const char* format, ...) {
    // Logging disabled for release
    return;
}

// Get game directory
static void GetGameDir(char* outDir, size_t maxLen) {
    GetModuleFileNameA(NULL, outDir, (DWORD)maxLen);
    char* lastSlash = strrchr(outDir, '\\');
    if (lastSlash) {
        *(lastSlash + 1) = '\0';
    }
}

WindowedModeManager& WindowedModeManager::Instance() {
    static WindowedModeManager instance;
    return instance;
}

WindowedModeManager::WindowedModeManager() 
    : m_pCheckbox(NULL)
    , m_lastKnownState(true)
    , m_lastDebugLog(0) 
{
}

void WindowedModeManager::SetCheckboxPointer(void* checkbox) {
    m_pCheckbox = checkbox;
    WMLog("Checkbox pointer SET: %p", checkbox);
    WMLog("Current m_lastKnownState before LoadSetting: %s", m_lastKnownState ? "CHECKED" : "UNCHECKED");
    
    // Load saved state when checkbox is set
    LoadSetting();
    
    WMLog("After LoadSetting - m_lastKnownState: %s", m_lastKnownState ? "CHECKED" : "UNCHECKED");
}

void WindowedModeManager::ClearCheckboxPointer() {
    WMLog("[WindowedModeManager] Checkbox pointer CLEARED (was: %p)", m_pCheckbox);
    m_pCheckbox = NULL;
}

bool WindowedModeManager::GetCheckboxState() const {
    if (!m_pCheckbox) return false;
    
    // Call the game's GetCheckedState function at 0x004240A0
    typedef bool (__thiscall *GetCheckedStateFn)(const void*);
    GetCheckedStateFn getState = (GetCheckedStateFn)0x004240A0;
    return getState(m_pCheckbox);
}

void WindowedModeManager::LoadSetting() {
    char gameDir[MAX_PATH];
    GetGameDir(gameDir, MAX_PATH);
    
    char buffer[MAX_PATH];
    sprintf_s(buffer, "%sSetting\\SRCSet.dat", gameDir);
    
    FILE* file = fopen(buffer, "rb");
    if (file == NULL) {
        WMLog("[WindowedModeManager] Settings file not found - defaulting to CHECKED");
        m_lastKnownState = true;
        
        // Set checkbox to checked using game function at 0x00424040
        if (m_pCheckbox) {
            // First enable the checkbox (offset 0x2B5)
            *((BYTE*)m_pCheckbox + 0x2B5) = 1;
            
            typedef void (__thiscall *SetCheckedFn)(void*, bool);
            SetCheckedFn setChecked = (SetCheckedFn)0x00424040;
            setChecked(m_pCheckbox, true);
        }
        return;
    }

    bool isChecked = false;
    fread(&isChecked, sizeof(bool), 1, file);
    fclose(file);

    WMLog("[WindowedModeManager] Loaded state: %s from %s", isChecked ? "CHECKED" : "UNCHECKED", buffer);
    m_lastKnownState = isChecked;

    if (m_pCheckbox) {
        // First enable the checkbox (offset 0x2B5)
        *((BYTE*)m_pCheckbox + 0x2B5) = 1;
        
        typedef void (__thiscall *SetCheckedFn)(void*, bool);
        SetCheckedFn setChecked = (SetCheckedFn)0x00424040;
        setChecked(m_pCheckbox, isChecked);
    }
    
    // Sync WindowedMode state with loaded setting
    // Note: Don't call SetEnabled here if window not yet created - hooks handle initial state
}

void WindowedModeManager::SaveSetting() {
    char gameDir[MAX_PATH];
    GetGameDir(gameDir, MAX_PATH);
    
    // Create Setting directory
    char dirBuffer[MAX_PATH];
    sprintf_s(dirBuffer, "%sSetting", gameDir);
    CreateDirectoryA(dirBuffer, NULL);

    char buffer[MAX_PATH];
    sprintf_s(buffer, "%sSetting\\SRCSet.dat", gameDir);

    FILE* file = fopen(buffer, "wb");
    if (file != NULL) {
        bool isChecked = GetCheckboxState();
        WMLog("[WindowedModeManager] Saving state: %s to %s", isChecked ? "CHECKED" : "UNCHECKED", buffer);
        fwrite(&isChecked, sizeof(bool), 1, file);
        fclose(file);
    } else {
        WMLog("[WindowedModeManager] ERROR: Could not save to %s (error: %d)", buffer, GetLastError());
    }
}

void WindowedModeManager::PollAndSave() {
    if (!m_pCheckbox) return;
    
    // Debug: Log every 2 seconds
    DWORD now = GetTickCount();
    if (now - m_lastDebugLog > 2000) {
        m_lastDebugLog = now;
        bool state = GetCheckboxState();
        WMLog("[WindowedModeManager] POLL: checkbox=%p, state=%s, lastKnown=%s",
            m_pCheckbox,
            state ? "CHECKED" : "UNCHECKED",
            m_lastKnownState ? "CHECKED" : "UNCHECKED");
    }
    
    // Check if state changed
    bool currentState = GetCheckboxState();
    if (currentState != m_lastKnownState) {
        WMLog("[WindowedModeManager] State CHANGED: %s -> %s",
            m_lastKnownState ? "CHECKED" : "UNCHECKED",
            currentState ? "CHECKED" : "UNCHECKED");
        m_lastKnownState = currentState;
        SaveSetting();
        
        // Trigger runtime window mode switch
        // CHECKED = windowed mode, UNCHECKED = fullscreen
        WindowedMode::SetEnabled(currentState);
        WMLog("[WindowedModeManager] WindowedMode::SetEnabled(%s) called", currentState ? "true" : "false");
    }
}
