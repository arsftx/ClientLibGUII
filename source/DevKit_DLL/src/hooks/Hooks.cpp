#include "Hooks.h"

// Global D3D9 viewport resolution - updated from EndScene hook
int g_D3DViewportWidth = 0;
int g_D3DViewportHeight = 0;

std::vector<endscene_handler_t> hooks_endscene;
std::vector<create_handler_t> hooks_create;
std::vector<WNDPROC> hooks_wndproc;
std::vector<setsize_handler_t> hooks_setsize_pre;
std::vector<setsize_handler_t> hooks_setsize_post;
std::vector<void_cgame_handler_t> hooks_lgo_pre;
std::vector<void_cgame_handler_t> hooks_lgo_post;
std::vector<void_cgame_handler_t> hooks_iga_pre;
std::vector<void_cgame_handler_t> hooks_iga_post;

void OnEndScene(endscene_handler_t handler) {
    hooks_endscene.push_back(handler);
}

void OnCreate(create_handler_t handler) {
    hooks_create.push_back(handler);
}

void OnWndProc(WNDPROC handler) {
    hooks_wndproc.push_back(handler);
}

void OnPreSetSize(setsize_handler_t handler) {
    hooks_setsize_pre.push_back(handler);
}

void OnPostSetSize(setsize_handler_t handler) {
    hooks_setsize_post.push_back(handler);
}

// Manual trigger for PreSetSize callbacks (for runtime mode switching)
void TriggerPreSetSize(int width, int height) {
    for (std::vector<setsize_handler_t>::iterator it = hooks_setsize_pre.begin();
        it != hooks_setsize_pre.end(); ++it) {
        (*it)(width, height);
    }
}

// Manual trigger for PostSetSize callbacks (for runtime mode switching)
void TriggerPostSetSize(int width, int height) {
    for (std::vector<setsize_handler_t>::iterator it = hooks_setsize_post.begin();
        it != hooks_setsize_post.end(); ++it) {
        (*it)(width, height);
    }
}

void OnPreLoadGameOption(void_cgame_handler_t handler) {
    hooks_lgo_pre.push_back(handler);
}

void OnPostLoadGameOption(void_cgame_handler_t handler) {
    hooks_lgo_post.push_back(handler);
}

void OnPreInitGameAssets(void_cgame_handler_t handler) {
    hooks_iga_pre.push_back(handler);
}

void OnPostInitGameAssets(void_cgame_handler_t handler) {
    hooks_iga_post.push_back(handler);
}

void DebugPrintCallback(const char *buffer) {
    printf(buffer);
}
