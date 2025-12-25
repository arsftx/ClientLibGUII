#include "CIFOption_Game.h"
#include "Game.h"
#include <Windows.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include "../../DevKit_DLL/src/WindowedModeManager.h"

// Log to C:\ClientLog.txt - DISABLED for performance
static void ClientLog(const char* format, ...) {
    return; // Logging disabled
    /*
    FILE* logFile = fopen("C:\\Users\\FuatAras\\Desktop\\Silkroad\\ClientLog.txt", "a");
    if (logFile) {
        time_t now = time(NULL);
        struct tm timeinfo;
        localtime_s(&timeinfo, &now);
        fprintf(logFile, "[%02d:%02d:%02d] ", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
        
        va_list args;
        va_start(args, format);
        vfprintf(logFile, format, args);
        va_end(args);
        
        fprintf(logFile, "\n");
        fflush(logFile);
        fclose(logFile);
    }
    */
}

// Get game directory using Windows API (theApp.GetWorkingDir() returns corrupted path)
static void GetGameDirectory(char* outDir, size_t maxLen) {
    GetModuleFileNameA(NULL, outDir, (DWORD)maxLen);
    // Remove executable name to get directory
    char* lastSlash = strrchr(outDir, '\\');
    if (lastSlash) {
        *(lastSlash + 1) = '\0';  // Keep trailing backslash
    }
}

GFX_IMPLEMENT_DYNAMIC_EXISTING(CIFOption_Game, 0x9FFE94)

CIFOption_Game::CIFOption_Game() : m_pTestButton(NULL), m_pTestLabel(NULL), m_pWindowedModeCheckBox(NULL), m_pWindowedModeLabel(NULL), m_pWindowedModeAllowLabel(NULL), m_pWindowedModeBackground(NULL), m_lastKnownState(true) {
    ClientLog("[CIFOption_Game] Constructor called - this=%p", this);
    s_pActiveInstance = this;  // Register for polling
}

CIFOption_Game::~CIFOption_Game() {
    ClientLog("[CIFOption_Game] Destructor called - this=%p, checkbox=%p", this, m_pWindowedModeCheckBox);
    
    // Save checkbox state when window is destroyed
    if (m_pWindowedModeCheckBox) {
        ClientLog("[CIFOption_Game] Destructor: Calling SaveWindowedModeSetting...");
        SaveWindowedModeSetting();
    } else {
        ClientLog("[CIFOption_Game] Destructor: Checkbox is NULL, not saving!");
    }
}

bool CIFOption_Game::OnCreateIMPL(long ln) {
    ClientLog("[CIFOption_Game] OnCreateIMPL called - this=%p", this);
    
    // 1. Call Original Function first to setup the window
    bool bResult = reinterpret_cast<bool(__thiscall *)(CIFOption_Game *, long)>(0x0047CCD0)(this, ln);
    ClientLog("[CIFOption_Game] Original OnCreate returned: %s", bResult ? "TRUE" : "FALSE");

    // 2. Add Custom UI Elements
    if (bResult) {
        wnd_rect sz;

        // Background Position
        sz.pos.x = 174;
        sz.pos.y = 112;
        sz.size.width = 156;
        sz.size.height = 28;

        m_pWindowedModeBackground = (CIFStatic *) CGWnd::CreateInstance(this, GFX_RUNTIME_CLASS(CIFStatic), sz, 1004, 0);
        if (m_pWindowedModeBackground) {
            m_pWindowedModeBackground->TB_Func_12("interface\\option\\opt_key02.ddj", 1, 0);
            m_pWindowedModeBackground->BringToUp();
        }

        // Label Position "Window"
        sz.pos.x = 178;
        sz.pos.y = 120;
        sz.size.width = 100;
        sz.size.height = 20;

        m_pWindowedModeLabel = (CIFStatic *) CGWnd::CreateInstance(this, GFX_RUNTIME_CLASS(CIFStatic), sz, 1002, 0);
        if (m_pWindowedModeLabel) {
            m_pWindowedModeLabel->SetText("Window Mode");
            m_pWindowedModeLabel->SetTextColor(0xFFFFFFFF);
            m_pWindowedModeLabel->BringToUp();
        }

        

        // Checkbox Position
        sz.pos.x = 305;
        sz.pos.y = 119;
        sz.size.width = 16;
        sz.size.height = 16;

        m_pWindowedModeCheckBox = (CIFCheckBox *) CGWnd::CreateInstance(this, GFX_RUNTIME_CLASS(CIFCheckBox), sz, 1003, 0);
        ClientLog("[CIFOption_Game] Checkbox created: %p", m_pWindowedModeCheckBox);
        
        if (m_pWindowedModeCheckBox) {
            m_pWindowedModeCheckBox->BringToUp();
            m_pWindowedModeCheckBox->SetEnabled(true);
            
            // Register checkbox with global manager (lives in DevKit_DLL)
            g_WindowedModeManager.SetCheckboxPointer(m_pWindowedModeCheckBox);
        }
    }

    return bResult;
}

bool CIFOption_Game::IsWindowedModeChecked() const {
    if (m_pWindowedModeCheckBox) {
        return m_pWindowedModeCheckBox->GetCheckedState_MAYBE();
    }
    return false;
}

void CIFOption_Game::LoadWindowedModeSetting() {
    char gameDir[MAX_PATH];
    GetGameDirectory(gameDir, MAX_PATH);
    
    char buffer[MAX_PATH];
    sprintf_s(buffer, "%sSetting\\SRCSet.dat", gameDir);
    ClientLog("[CIFOption_Game] LoadWindowedModeSetting - path: %s", buffer);

    FILE* file = fopen(buffer, "rb");
    if (file == NULL) {
        ClientLog("[CIFOption_Game] Settings file not found - defaulting to CHECKED");
        if (m_pWindowedModeCheckBox) {
            m_pWindowedModeCheckBox->SetChecked(true);
            m_lastKnownState = true;  // Track initial state
        }
        return;
    }

    bool isChecked = false;
    fread(&isChecked, sizeof(bool), 1, file);
    fclose(file);

    ClientLog("[CIFOption_Game] Loaded state: %s", isChecked ? "CHECKED" : "UNCHECKED");
    m_lastKnownState = isChecked;  // Track loaded state

    if (m_pWindowedModeCheckBox) {
        m_pWindowedModeCheckBox->SetChecked(isChecked);
    }
}

void CIFOption_Game::SaveWindowedModeSetting() {
    char gameDir[MAX_PATH];
    GetGameDirectory(gameDir, MAX_PATH);
    
    // Create Setting directory if it doesn't exist
    char dirBuffer[MAX_PATH];
    sprintf_s(dirBuffer, "%sSetting", gameDir);
    CreateDirectoryA(dirBuffer, NULL);
    ClientLog("[CIFOption_Game] SaveWindowedModeSetting - dir: %s", dirBuffer);

    char buffer[MAX_PATH];
    sprintf_s(buffer, "%sSetting\\SRCSet.dat", gameDir);

    FILE* file = fopen(buffer, "wb");
    if (file != NULL) {
        bool isChecked = false;
        if (m_pWindowedModeCheckBox) {
            isChecked = m_pWindowedModeCheckBox->GetCheckedState_MAYBE();
        }
        ClientLog("[CIFOption_Game] Saving state: %s to %s", isChecked ? "CHECKED" : "UNCHECKED", buffer);
        fwrite(&isChecked, sizeof(bool), 1, file);
        fclose(file);
        ClientLog("[CIFOption_Game] Save complete!");
    } else {
        ClientLog("[CIFOption_Game] ERROR: Could not open file for writing: %s (error: %d)", buffer, GetLastError());
    }
}

// Static instance pointer
CIFOption_Game* CIFOption_Game::s_pActiveInstance = NULL;

// Debug timer for periodic logging
static DWORD s_lastDebugLog = 0;

// Static polling function - called from EndScene hook
void CIFOption_Game::PollCheckboxState() {
    // Track when instance becomes valid
    static bool wasInstanceNull = true;
    
    if (s_pActiveInstance && wasInstanceNull) {
        ClientLog("[DEBUG] PollCheckboxState - Instance NOW VALID! instance=%p, checkbox=%p", 
            s_pActiveInstance, s_pActiveInstance->m_pWindowedModeCheckBox);
        wasInstanceNull = false;
    } else if (!s_pActiveInstance && !wasInstanceNull) {
        ClientLog("[DEBUG] PollCheckboxState - Instance became NULL!");
        wasInstanceNull = true;
    }
    
    if (s_pActiveInstance) {
        // Debug: Log every 2 seconds to confirm polling is working
        DWORD now = GetTickCount();
        if (now - s_lastDebugLog > 2000) {
            s_lastDebugLog = now;
            if (s_pActiveInstance->m_pWindowedModeCheckBox) {
                bool state = s_pActiveInstance->m_pWindowedModeCheckBox->GetCheckedState_MAYBE();
                ClientLog("[DEBUG] POLL: checkbox=%p, currentState=%s, lastKnown=%s",
                    s_pActiveInstance->m_pWindowedModeCheckBox,
                    state ? "CHECKED" : "UNCHECKED",
                    s_pActiveInstance->m_lastKnownState ? "CHECKED" : "UNCHECKED");
            } else {
                ClientLog("[DEBUG] POLL: checkbox is NULL!");
            }
        }
        
        s_pActiveInstance->CheckAndSaveIfChanged();
    }
}

void CIFOption_Game::CheckAndSaveIfChanged() {
    if (m_pWindowedModeCheckBox) {
        bool currentState = m_pWindowedModeCheckBox->GetCheckedState_MAYBE();
        if (currentState != m_lastKnownState) {
            ClientLog("[CIFOption_Game] Checkbox state CHANGED: %s -> %s", 
                m_lastKnownState ? "CHECKED" : "UNCHECKED",
                currentState ? "CHECKED" : "UNCHECKED");
            m_lastKnownState = currentState;
            SaveWindowedModeSetting();
        }
    }
}