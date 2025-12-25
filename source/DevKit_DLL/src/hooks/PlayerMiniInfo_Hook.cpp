#include "PlayerMiniInfo_Hook.h"
#include <memory/hook.h>
#include <stdio.h>
#include <Windows.h>

// CIFPlayerMiniInfo vtable
// sub_89C620 (RenderMyself) calls vtable slots 12, 13, 14
// Slot 12 = RenderSelf (background)
// Slot 13 = RenderMyChildren
// Replacing both with nullsub disables all rendering
static const DWORD ADDR_VTABLE_SLOT12 = 0x00947910;  // slot 12 - RenderSelf
static const DWORD ADDR_VTABLE_SLOT13 = 0x00947914;  // slot 13 - RenderMyChildren
static const DWORD ADDR_NULLSUB_381 = 0x0089C640;    // nullsub_381

static bool s_patched = false;

static FILE* g_debugFile = NULL;

static void DebugLog(const char* msg) {
    if (!g_debugFile) {
        g_debugFile = fopen("customgui_hook.txt", "a");
    }
    if (g_debugFile) {
        fprintf(g_debugFile, "%s\n", msg);
        fflush(g_debugFile);
    }
}

void HideNativePlayerMiniInfo() {
    // No-op - vtable is patched at startup
}

void InstallPlayerMiniInfoHook() {
    if (s_patched) return;
    
    DebugLog("InstallPlayerMiniInfoHook: Replacing vtable slots 12 and 13");
    
    // Replace slot 12 (RenderSelf) and slot 13 (RenderMyChildren) with nullsub
    replaceAddr(ADDR_VTABLE_SLOT12, ADDR_NULLSUB_381);
    replaceAddr(ADDR_VTABLE_SLOT13, ADDR_NULLSUB_381);
    s_patched = true;
    
    DebugLog("InstallPlayerMiniInfoHook: Slots 12+13 replaced - rendering disabled!");
}
