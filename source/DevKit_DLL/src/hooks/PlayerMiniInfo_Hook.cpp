#include "PlayerMiniInfo_Hook.h"
#include <memory/hook.h>
#include <stdio.h>
#include <Windows.h>

// CIFPlayerMiniInfo vtable render disable
// sub_89C620 (RenderMyself) calls vtable slots 12, 13, 14
// Slot 12 = RenderSelf (background/frame)
// Slot 13 = RenderMyChildren (HP/MP bars, portrait, etc.)
// Replacing both with nullsub disables all rendering
static const DWORD ADDR_VTABLE_SLOT12 = 0x00947910;
static const DWORD ADDR_VTABLE_SLOT13 = 0x00947914;
static const DWORD ADDR_NULLSUB_381 = 0x0089C640;

static bool s_patched = false;

void HideNativePlayerMiniInfo() {
    // No-op - vtable is patched at startup
    // BuffViewer position is left unchanged - ImGui window moved below it instead
}

void InstallPlayerMiniInfoHook() {
    if (s_patched) return;
    
    // Replace vtable slots 12 and 13 with nullsub to disable rendering
    replaceAddr(ADDR_VTABLE_SLOT12, ADDR_NULLSUB_381);
    replaceAddr(ADDR_VTABLE_SLOT13, ADDR_NULLSUB_381);
    s_patched = true;
}

