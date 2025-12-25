#pragma once

#include <Game.h>

typedef void(* endscene_handler_t)();
typedef void(* create_handler_t)(HWND, void*, int);
typedef void(* setsize_handler_t)(int,int);
typedef void(* void_cgame_handler_t)(CGame*);

// =============================================================================
// Global D3D9 Viewport Resolution
// Updated every frame from EndScene hook via D3D9 GetViewport()
// Use these for dynamic UI positioning (e.g. IFflorian0Guide button)
// =============================================================================
extern int g_D3DViewportWidth;
extern int g_D3DViewportHeight;

#define RESULT_PASS 0
#define RESULT_DISCARD 1

void OnEndScene(endscene_handler_t handler);
void OnCreate(create_handler_t handler);
void OnWndProc(WNDPROC handler);
void OnPreSetSize(setsize_handler_t handler);
void OnPostSetSize(setsize_handler_t handler);

// Manual trigger functions for runtime mode switching
void TriggerPreSetSize(int width, int height);
void TriggerPostSetSize(int width, int height);

void OnPreLoadGameOption(void_cgame_handler_t handler);
void OnPostLoadGameOption(void_cgame_handler_t handler);

void OnPreInitGameAssets(void_cgame_handler_t handler);
void OnPostInitGameAssets(void_cgame_handler_t handler);

void DebugPrintCallback(const char *buffer);
