#include "IFTestButton.h"
#include "GInterface.h"
#include "GEffSoundBody.h"
#include <BSLib/Debug.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <windows.h>

// Global instance
CIFTestDecoratedButton* g_pTestDecoratedButton = NULL;

GFX_IMPLEMENT_DYNCREATE(CIFTestDecoratedButton, CIFDecoratedStatic)

// Debug log function
void CIFTestDecoratedButton::DebugLog(const char* format, ...)
{
    const char* logPath = "clientlog.txt";
    
    char buffer[2048];
    va_list args;
    va_start(args, format);
    _vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    buffer[sizeof(buffer) - 1] = '\0';
    
    time_t now = time(0);
    struct tm tstruct;
    char timeBuf[80];
    localtime_s(&tstruct, &now);
    strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M:%S", &tstruct);
    
    FILE* fp = fopen(logPath, "a");
    if (fp) {
        fprintf(fp, "[%s] %s\n", timeBuf, buffer);
        fclose(fp);
    }
    
    OutputDebugStringA(buffer);
    OutputDebugStringA("\n");
}

// Pattern from IFflorian0Guide.cpp (VSRO) adapted for ECSRO
bool CIFTestDecoratedButton::OnCreate(long ln)
{
    BS_DEBUG_LOW("> " __FUNCTION__ "(%d)", ln);
    DebugLog("[CIFTestDecoratedButton] OnCreate called with ln=%d", ln);
    
    CIFDecoratedStatic::OnCreate(ln);
    
    // Use TB_Func_12 (const char*) instead of TB_Func_13 (std::n_string)
    // TB_Func_12 in TextBoard.cpp is already fixed to use 12-byte GameString
    TB_Func_12("interface\\ifcommon\\com_button.ddj", 1, 0);
    DebugLog("[CIFTestDecoratedButton] TB_Func_12 completed");
    
    // Hover texture using sub_634470 (already fixed to use 12-byte GameString)
    // sub_634470("interface\\ifcommon\\com_button_over.ddj");
    
    // Animation settings like IFflorian0Guide
    set_N00009BD4(2);  // Animation type
    set_N00009BD3(500); // Animation timing
    
    DebugLog("[CIFTestDecoratedButton] Initialization complete");
    
    return true;
}

int CIFTestDecoratedButton::OnMouseLeftUp(int a1, int x, int y)
{
    BS_DEBUG_LOW("> " __FUNCTION__ "(%d, %d, %d)", a1, x, y);
    DebugLog("[CIFTestDecoratedButton] OnMouseLeftUp at (%d, %d)", x, y);
    
    // Play sound like IFflorian0Guide does
    // CGEffSoundBody::get()->PlaySound(L"snd_quest");
    
    return 0;
}

void CIFTestDecoratedButton::OnCIFReady()
{
    BS_DEBUG_LOW("> " __FUNCTION__);
    DebugLog("[CIFTestDecoratedButton] OnCIFReady called");
    
    CIFDecoratedStatic::OnCIFReady();
    sub_633990();
}

// Static method to create the test button using CGWnd::CreateInstance
bool CIFTestDecoratedButton::CreateTestButton()
{
    DebugLog("[CreateTestButton] Starting button creation...");
    
    if (!g_pCGInterface) {
        DebugLog("[CreateTestButton] ERROR: g_pCGInterface is NULL!");
        return false;
    }
    DebugLog("[CreateTestButton] g_pCGInterface is valid: 0x%p", g_pCGInterface);
    
    if (g_pTestDecoratedButton != NULL) {
        DebugLog("[CreateTestButton] Button already exists, skipping creation");
        return true;
    }
    
    // Position for the button (top-right, below minimap area)
    RECT buttonRect = { 780, 200, 812, 232 }; // 32x32 button
    DebugLog("[CreateTestButton] Button rect: left=%d, top=%d, right=%d, bottom=%d", 
             buttonRect.left, buttonRect.top, buttonRect.right, buttonRect.bottom);
    
    // Create using CGWnd::CreateInstance
    DebugLog("[CreateTestButton] Calling CGWnd::CreateInstance...");
    g_pTestDecoratedButton = static_cast<CIFTestDecoratedButton*>(
        CGWnd::CreateInstance(
            g_pCGInterface,                          // Parent window
            GFX_RUNTIME_CLASS(CIFTestDecoratedButton), // Runtime class
            buttonRect,                               // Position
            GUIDE_TEST_BUTTON,                        // Control ID
            0                                         // Flags
        )
    );
    
    if (!g_pTestDecoratedButton) {
        DebugLog("[CreateTestButton] ERROR: CreateInstance returned NULL!");
        return false;
    }
    DebugLog("[CreateTestButton] CreateInstance successful: 0x%p", g_pTestDecoratedButton);
    
    // Show the button
    g_pTestDecoratedButton->ShowGWnd(true);
    DebugLog("[CreateTestButton] ShowGWnd(true) called");
    
    // Bring to front
    g_pTestDecoratedButton->BringToFront();
    DebugLog("[CreateTestButton] BringToFront() called");
    
    DebugLog("[CreateTestButton] Button creation completed successfully!");
    return true;
}
