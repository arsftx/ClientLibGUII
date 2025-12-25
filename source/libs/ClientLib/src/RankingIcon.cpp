#include "RankingIcon.h"
#include <BSLib/Debug.h>
#include "GInterface.h"
#include "Game.h"
#include "IFRankingWindow.h"
#include <LegendMainMenu.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <windows.h>

// D3D9 Viewport Resolution (updated every frame from EndScene hook)
extern int g_D3DViewportWidth;
extern int g_D3DViewportHeight;

GFX_IMPLEMENT_DYNCREATE(CRankingIcon, CIFDecoratedStatic)

// Debug log function - DISABLED
static void DebugLog(const char* format, ...)
{
    // Logging disabled for release
    return;
}

// =============================================================================
// CRankingIcon Implementation
// =============================================================================

bool CRankingIcon::OnCreate(long ln)
{
	BS_DEBUG_LOW("> " __FUNCTION__ "(%d)", ln);
	
	CIFDecoratedStatic::OnCreate(ln);
	
	// Initialize member variables
	m_buttonWidth = 40;
	m_buttonHeight = 40;
	m_offsetFromRight = 110;  // 50 pixels to the left of MacroButton (60 + 50 = 110)
	m_offsetFromTop = 200;
	m_isHovering = false;
	
	// Load the icon texture - THIS MAKES THE BUTTON VISIBLE
	TB_Func_13("icon\\etc\\bugle_icon_1.ddj", 0, 0);
	
	// Set animation parameters
	set_N00009BD4(2);
	set_N00009BD3(500);
	
	// Set button size
	SetGWndSize(m_buttonWidth, m_buttonHeight);
	*reinterpret_cast<int*>(reinterpret_cast<char*>(this) + 0x44) = m_buttonWidth;
	*reinterpret_cast<int*>(reinterpret_cast<char*>(this) + 0x48) = m_buttonHeight;
	
	DebugLog("[CRankingIcon] OnCreate completed");

	return true;
}

int CRankingIcon::OnMouseLeftUp(int a1, int x, int y)
{
	BS_DEBUG_LOW("> " __FUNCTION__ "(%d, %d, %d)", a1, x, y);
	
	if (!IsInside(x, y)) {
		return 0;
	}
	
	DebugLog("[CRankingIcon] OnClick at (%d, %d)", x, y);

	// Use IFRankingWindow browser
	CIFRankingWindow* pBrowserWnd = CIFMainMenu::RankingWindow;
	if (pBrowserWnd) {
		DebugLog("[CRankingIcon] Toggling IFRankingWindow browser...");
		pBrowserWnd->ToggleBrowser();
		DebugLog("[CRankingIcon] Browser toggled");
	} else {
		DebugLog("[CRankingIcon] ERROR: RankingWindow is NULL");
	}

	return 1;
}

int CRankingIcon::OnMouseLeftDown(int a1, int x, int y)
{
	BS_DEBUG_LOW("> " __FUNCTION__ "(%d, %d, %d)", a1, x, y);
	
	if (!IsInside(x, y)) {
		return 0;
	}

	return 1;
}

int CRankingIcon::OnMouseMove(int a1, int x, int y)
{
	if (!IsInside(x, y)) {
		if (m_isHovering) {
			m_isHovering = false;
			set_N00009BD0(0);
		}
		return 0;
	}
	
	if (!m_isHovering) {
		m_isHovering = true;
		DebugLog("[CRankingIcon] OnEnter");
		set_N00009BD0(1);
	}
	
	return 1;
}

void CRankingIcon::OnUpdate()
{
	CIFDecoratedStatic::OnUpdate();
	
	// Browser is now managed by IFRankingWindow, no need for separate OnUpdate
	
	// Dynamic resolution-aware repositioning
	static int s_lastWidth = 0;
	static int s_lastHeight = 0;
	
	int currentWidth = g_D3DViewportWidth;
	int currentHeight = g_D3DViewportHeight;
	
	if (currentWidth > 0 && currentHeight > 0 &&
	    (currentWidth != s_lastWidth || currentHeight != s_lastHeight)) {
		
		s_lastWidth = currentWidth;
		s_lastHeight = currentHeight;
		
		int posX = currentWidth - m_offsetFromRight;
		int posY = m_offsetFromTop;
		MoveGWnd(posX, posY);
	}
}

void CRankingIcon::OnCIFReady()
{
	static bool s_positionSet = false;
	if (s_positionSet) {
		CIFDecoratedStatic::OnCIFReady();
		return;
	}
	s_positionSet = true;
	
	BS_DEBUG_LOW("> " __FUNCTION__);

	CIFDecoratedStatic::OnCIFReady();
	sub_633990();

	// Initial positioning
	int screenWidth = g_D3DViewportWidth;
	int screenHeight = g_D3DViewportHeight;
	
	if (screenWidth <= 0 || screenWidth > 7680 || screenHeight <= 0 || screenHeight > 4320) {
		screenWidth = GetSystemMetrics(SM_CXSCREEN);
		screenHeight = GetSystemMetrics(SM_CYSCREEN);
	}
	
	int posX = screenWidth - m_offsetFromRight;
	int posY = m_offsetFromTop;
	MoveGWnd(posX, posY);
}

bool CRankingIcon::IsInside(int x, int y)
{
	int posX = *reinterpret_cast<int*>(reinterpret_cast<char*>(this) + 0x3C);
	int posY = *reinterpret_cast<int*>(reinterpret_cast<char*>(this) + 0x40);
	
	return (x >= posX && x < posX + m_buttonWidth &&
	        y >= posY && y < posY + m_buttonHeight);
}
