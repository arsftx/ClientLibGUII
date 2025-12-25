#include "Util.h"
#include "CustomOpcodeHandler.h"

#include <sys/stat.h>
#include <stdio.h>

#include <memory/hook.h>

#include "hooks/Hooks.h"
#include "hooks/GFXVideo3d_Hook.h"
#include "hooks/CGame_Hook.h"

#include "hooks/WndProc_Hook.h"

#include <GInterface.h>
#include <IFChatViewer.h>
#include <NetProcessIn.h>
#include <NetProcessSecond.h>
#include <NetProcessThird.h>
#include <BSLib/Debug.h>
#include "QuickStart.h"
#include "ICPlayer.h"
#include "ICMonster.h"
#include <PSCharacterSelect.h>
#include <ICUser.h>
#include <GFX3DFunction/RStateMgr.h>
#include <memory/util.h>
#include <LoginScreen.h>
#include <TextStringManager.h>
#include <CustomCharIcons.h>
#include <remodel/MemberFunctionHook.h>
#include <IFTargetWindow.h>
#include <IFStoreForPackage.h>
#include <IFRegionView.h>
#include "IFGGMainSlot.h"
#include <IFGGMenu.h>
#include <IFSystemWnd.h>
#include <IFWorldMap.h>
#include <IFItemMall.h>
#include <CIFOption_Game.h>
#include "WindowedModeManager.h"
#include <IFMessageBox.h>
#include <IFUnderBar.h>
#include <GameSettings.h>
#include <PSQuickStart.h>
#include <ICCos.h>
#include <PetFilter.h>
#include <AutoBuffController.h>
#include <AutoAttackSkillController.h>
#include <AutoTargetController.h>
#include <AutoMoveController.h>
#include "hooks/PlayerMiniInfo_Hook.h"
std::vector<const CGfxRuntimeClass *> register_objects;
std::vector<overrideFnPtr> override_objects;

QuickStart quickstart;

#include <Windows.h>
#include <stdio.h>
#include <time.h>
#include <string>

// File logging system
static FILE* debugLogFile = NULL;
static bool debugLogInitialized = false;

void InitializeDebugLog() {
    // FILE LOGGING DISABLED - No file creation
    debugLogInitialized = true; // Mark as initialized to prevent file creation attempts
}

void WriteToDebugLog(const char* format, ...) {
    // Only output to console - NO FILE WRITING
#ifdef CONFIG_DEBUG_CONSOLE
    time_t now = time(NULL);
    struct tm* timeinfo = localtime(&now);
    
    // Write timestamp to console
    printf("[%02d:%02d:%02d] ", 
            timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
    
    // Write message to console
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    
    printf("\n");
    fflush(stdout);
#endif

    // FILE WRITING DISABLED - Only console output
}

void CloseDebugLog() {
    if (debugLogFile) {
        fprintf(debugLogFile, "=== Debug Log Ended ===\n");
        fclose(debugLogFile);
        debugLogFile = NULL;
        debugLogInitialized = false;
    }
}

// Callback for WindowedModeManager polling
static void WindowedModeManagerCallback() {
    g_WindowedModeManager.PollAndSave();
}

// Callback for CICCos pet tracking (ECSRO offset test)
static void CICCosPetTrackingCallback() {
    CICCos::LogAllPets();
}

// Callback for AutoBuffController update
static void AutoBuffControllerCallback() {
    AutoBuffController::Update();
}

// Callback for AutoAttackSkillController update
static void AutoAttackSkillControllerCallback() {
    AutoAttackSkillController::Update();
}

// Callback for AutoTargetController update
static void AutoTargetControllerCallback() {
    AutoTargetController::Update();
}

// Callback for AutoMoveController update
static void AutoMoveControllerCallback() {
    AutoMoveController::Update();
}

void Setup() {
    // Initialize debug log system
    InitializeDebugLog();
    
    // Install CICCos spawn hook for ECSRO testing
    CICCos::InstallSpawnHook();
    
#ifdef CONFIG_DEBUG_CONSOLE
    AllocConsole();
    freopen("CONOUT$", "w", stdout);
    freopen("CONIN$", "r", stdin);
    printf("=== ECSRO Pet Monitor Console ===\n");
    printf("Console ready for pet tracking output.\n\n");
    
#ifdef CONFIG_DEBUG_REDIRECT_PUTDUMP
    replaceAddr(0x005E25A1 + 1, (int) &DebugPrintCallback);
    printf("PutDump redirection enabled.\n");
#endif
#endif



    vftableHook(0x009679f8, 25, addr_from_this(&CGFXVideo3d::BeginSceneIMPL));

	vftableHook(0x009679f8, 17, addr_from_this(&CGFXVideo3D_Hook::CreateThingsHook));
    vftableHook(0x009679f8, 20, addr_from_this(&CGFXVideo3D_Hook::SetSizeHook));
    vftableHook(0x009679f8, 26, addr_from_this(&CGFXVideo3D_Hook::EndSceneHook));

    vftableHook(0x009464c0, 9, addr_from_this(&CGInterface::OnCreateIMPL));
    replaceAddr(0x005E2D27 + 4, (int)& WndProcHook);

	replaceOffset(0x005E64F8, addr_from_this(&CGame_Hook::LoadGameOption));
	replaceOffset(0x005E25A1, addr_from_this(&CGame_Hook::InitGameAssets_Impl));
	replaceAddr(0x0094161C, addr_from_this(&IFSystemWnd::OnCreateIMPL));
	replaceAddr(0x00949804, addr_from_this(&CIFItemMallMyInfo::OnCreateIMPL));
	replaceAddr(0x00949984, addr_from_this(&CIFItemMallConfirmBuy::OnCreateIMPL));
    
    // Hook CIFOption_Game::OnCreate (VTable Hook via replaceAddr)
    // VTable Slot: 0x009410C4
    replaceAddr(0x009410C4, addr_from_this(&CIFOption_Game::OnCreateIMPL));
    
    // Disable native CIFPlayerMiniInfo render - replace vtable entry with dummy
    InstallPlayerMiniInfoHook();
    
    // Register EndScene callback for checkbox state polling via WindowedModeManager
    OnEndScene(WindowedModeManagerCallback);
    
    // Register EndScene callback for CICCos pet tracking (ECSRO offset testing)
    OnEndScene(CICCosPetTrackingCallback);
    
    // Register EndScene callback for AutoBuffController
    OnEndScene(AutoBuffControllerCallback);
    
    // Register EndScene callback for AutoAttackSkillController
    OnEndScene(AutoAttackSkillControllerCallback);
    
    // Register EndScene callback for AutoTargetController
    OnEndScene(AutoTargetControllerCallback);
    
    // Register EndScene callback for AutoMoveController (patrol)
    OnEndScene(AutoMoveControllerCallback);


    // Install Custom Opcode Handler (0xF001)
    CustomOpcodeHandler::InstallHooks();
    
    // Install Pet Filter Hook for ECSRO testing
    PetFilter::InstallHook();
    g_PetFilter.GetSettings().gold = true;  // Enable gold picking for testing
    g_PetFilter.SetEnabled(true);           // Activate the filter
    printf("[PetFilter] Gold filter enabled for testing!\\n");
    
    // AutoTargetController is now controlled by MacroWindow AutoAttack toggle
    printf("[AutoTarget] Ready - use Macro Window AutoAttack toggle to enable\\n");

	placeHook(0x004E5EF7, addr_from_this(&CIFUnderBar::SaveUnderbarData));
	MEMUTIL_WRITE_VALUE(BYTE, 0x00483C15 + 2, 0x01);
	MEMUTIL_WRITE_VALUE(BYTE, 0x00483C93 + 4, 0x40);
	//ItemMall Disable point silk
	//MEMUTIL_WRITE_VALUE(BYTE, 0x006908E6 + 1, 0x81);
	//MEMUTIL_WRITE_VALUE(BYTE, 0x00690AA8, 0xEB);
}

bool DoesFileExists(const std::string &name) {
    struct stat buffer;
    return (stat(name.c_str(), &buffer) == 0);
}

void RegisterObject(const CGfxRuntimeClass *obj) {
    register_objects.push_back(obj);
}

void OverrideObject(overrideFnPtr fn) {
    override_objects.push_back(fn);
}

void InstallRuntimeClasses(CGame *) {
    // Replace Create & Delete for existing classes
    // Note: We can't just inject existing objects like we would do with new objects.
    //       Joymax uses == on GFX_RUNTIME_CLASS(), so we would end up breaking this comparison

    for (std::vector<const CGfxRuntimeClass *>::const_iterator it = register_objects.begin();
         it != register_objects.end(); ++it) {
        reinterpret_cast<void(__thiscall *)(const CGfxRuntimeClass *, const char *, void *, void *, const CGfxRuntimeClass *, size_t, int)>(0x00898D80)(*it, (*it)->m_lpszClassName, (*it)->m_pfnCreateObject, (*it)->m_pfnDeleteObject, (*it)->m_pBaseClass, (*it)->m_nObjectSize, 0);
    }

    for (std::vector<overrideFnPtr>::const_iterator it = override_objects.begin(); it != override_objects.end(); ++it) {
        (*it)();
    }
}