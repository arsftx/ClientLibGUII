#include "WndProc_Hook.h"
#include "Hooks.h"
#include <GInterface.h>

extern std::vector<WNDPROC> hooks_wndproc;

// Browser bounds from SilkRoadWeb - used to filter mouse messages
extern RECT g_BrowserBounds;
extern bool g_BrowserIsActive;

// WM_MOUSEFIRST/LAST may not be defined on older SDK
#ifndef WM_MOUSEFIRST
#define WM_MOUSEFIRST 0x0200
#endif
#ifndef WM_MOUSELAST
#define WM_MOUSELAST 0x020E
#endif

// Helper: Check if a mouse message should be filtered (not sent to game)
static bool ShouldFilterMouseMessage(UINT msg, LPARAM lParam)
{
    // Only filter mouse messages (0x200 - 0x20E range)
    if (msg < 0x0200 || msg > 0x020E)
        return false;
    
    if (!g_BrowserIsActive)
        return false;
    
    // Extract mouse coordinates from lParam
    int x = (int)(short)LOWORD(lParam);
    int y = (int)(short)HIWORD(lParam);
    
    // Check if point is in browser area
    if (x >= g_BrowserBounds.left && x < g_BrowserBounds.right &&
        y >= g_BrowserBounds.top && y < g_BrowserBounds.bottom)
    {
        return true;
    }
    
    return false;
}

LRESULT HandleLeftButtonClick(UINT uniqueID) {
	if (!g_pCGInterface || !g_CurrentIfUnderCursor)
		return 0;

	// Get AutoPotion window and its buttons
	CIFAutoPotion* autoPotionWnd = g_pCGInterface->Get_AutoPotionWnd();
	if (!autoPotionWnd) {
		return 0;
	}

	// Compare clicked button with AutoPotion buttons
	if (g_CurrentIfUnderCursor == autoPotionWnd->m_confirm) {
		autoPotionWnd->On_BtnClickConfirm();
		return 1;
	}
	else if (g_CurrentIfUnderCursor == autoPotionWnd->m_cancle) {
		autoPotionWnd->On_BtnClickCancel();
		return 1;
	}

	return 0;
}

LRESULT CALLBACK WndProcHook(HWND hwnd, UINT msg, LPARAM lParam, WPARAM wParam)
{
    // CRITICAL: Filter mouse messages in browser area
    // This prevents the game from caching the browser area as "occupied"
    // When browser closes, game's internal state will be clean
    if (ShouldFilterMouseMessage(msg, lParam))
    {
        // Don't pass to game's WndProc - just use default handling
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    
    switch (msg) {

        case WM_CLOSE:
            exit(EXIT_SUCCESS);
            break;
		case WM_KEYDOWN:
			if (g_pCGInterface)
			{
			    if (GetAsyncKeyState('T') & 0x8000) { // Check if 'T' is pressed for AutoPotion
					if (g_pCGInterface) {
						CIFAutoPotion* autoPotionWnd = g_pCGInterface->Get_AutoPotionWnd();
						if (autoPotionWnd) {
							if (!autoPotionWnd->On_CheckVisiblePotion())
							{
								autoPotionWnd->On_LoadAutoPotion(); // Load settings when opening with T key
								autoPotionWnd->On_BtnClickOpen();
							}
							else
							{
								// Just close the window without reloading settings
								autoPotionWnd->On_BtnClickCancel();
							}
						}
					}
			    }
			}
			break;
		case WM_LBUTTONUP:
			// OnClick debug mesajları devre dışı bırakıldı
			if (g_pCGInterface && g_CurrentIfUnderCursor) {
				if (g_CurrentIfUnderCursor->IsSame(GFX_RUNTIME_CLASS(CIFButton))) {
					UINT buttonID = g_CurrentIfUnderCursor->UniqueID();
					HandleLeftButtonClick(buttonID);
				}
			}
			break;
    }

	// X key browser handler REMOVED - browser is opened via RankingIcon click only

	for (std::vector<WNDPROC>::iterator it = hooks_wndproc.begin();
		it != hooks_wndproc.end();
		++it) {
		if ((*it)(hwnd, msg, lParam, wParam) == RESULT_DISCARD) {
			// Call default window proc because nothing happens otherwise ...
			return DefWindowProc(hwnd, msg, lParam, wParam);
		}
	}


	return reinterpret_cast<WNDPROC>(0x005E2EE0)(hwnd, msg, lParam, wParam); //ECSRO 
}