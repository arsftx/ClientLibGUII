#include <Windows.h> // En başa alınması iyi olur
#include <IFChatViewer.h>
#include <IFMainPopup.h>
#include <IFChatOptionBoard.h>
#include <IFWholeChat.h>
#include "IFExtQuickSlot.h"
#include "SRIFLib/NIFUnderMenuBar.h"

#include "hooks/Hooks.h"
#include "Util.h"
#include "WindowedMode.h"
#include "SoundBugFix.h" // <--- Bunu eklediğinden emin ol
#include "DamageFix.h"
#include "CustomDamageRenderer.h"
#include "imgui_windows/ImGui_Windows.h"
#include "imgui_windows/CustomGUISession.h"      // Shared ImGui session manager
#include "imgui_windows/CustomPlayerMiniInfo.h"  // Independent player info overlay
#include "ClientValidation.h"  // Client-Filter validation system

#include "IFflorian0Guide.h"
#include "examples/IFflorian0.h"
#include "examples/NIFKyuubi09.h"
#include "../../ClientLib/src/IFAutoPotion.h"
#include "../../ClientLib/src/CIFOption_Game.h"
#include "memory/hook.h"
#include <LoginScreen.h>

HMODULE g_hModule;
HMODULE sro_client = 0;

// DllMain Artık Temiz
extern "C" _declspec(dllexport) BOOL WINAPI DllMain(HINSTANCE hModule, DWORD fdwReason, LPVOID lpReserved) {
    if (fdwReason == DLL_PROCESS_ATTACH) {
        g_hModule = hModule; // Global module handle'ı kaydetmek iyi pratiktir
        sro_client = GetModuleHandle(NULL);

        // 1. Windowed Mode Hook
        WindowedMode::InstallHooks();

        // 2. Sound Bug Fix Hook (FPS Drop Çözümü)
        // Artık SoundBugFix.cpp içindeki Install fonksiyonuna gidecek
        SoundBugFix::Install();
        
        // 3. Damage Limit Fix Hook
        DamageFix::Install();
        
        // 4. Custom Damage Renderer (D3D EndScene Hook)
        CustomDamageRenderer::Instance().Initialize();
        
        // 4.5 Custom GUI Session (Shared ImGui for all custom overlays)
        CustomGUISession::Instance().Initialize();
        
        // 4.6 Custom Player Mini Info (uses CustomGUISession)
        CustomPlayerMiniInfo::Instance().Initialize();
        
        // 5. Client Validation System (TCP Socket to Filter)
        ClientValidation::Initialize();
        // Note: InstallHooks() removed - TCP socket doesn't need game hooks

        // 6. Diğer Hooklar
        Setup();
        
        // RegisterObject(&GFX_RUNTIME_CLASS(CIFAutoPotion)); // TEMPORARILY DISABLED
        // RegisterObject(&GFX_RUNTIME_CLASS(CIFOption_Game)); // Removed: EXISTING classes are hooked via replaceAddr in Util.cpp, not registered here.

#define ENABLE_IMGUI_MENU 0 // Set to 0 to disable ImGui Menu

#if defined(CONFIG_IMGUI) && ENABLE_IMGUI_MENU
        OnCreate(ImGui_OnCreate);
        OnEndScene(ImGui_OnEndScene);
        OnWndProc(ImGui_WndProc);
#endif // CONFIG_IMGUI && ENABLE_IMGUI_MENU

        // NOTE: SetSize hooks are now registered by CustomGUISession::Initialize()
        // which properly handles its own ImGui context.

        OnPreInitGameAssets(InstallRuntimeClasses);
    }

    return TRUE;
}