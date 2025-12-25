#include "MacroButton.h"
#include <BSLib/Debug.h>
#include "GInterface.h"
#include "IFMacroWindow.h"
#include "Game.h"
#include <windows.h>

// D3D9 Viewport Resolution (updated every frame from EndScene hook)
extern int g_D3DViewportWidth;
extern int g_D3DViewportHeight;

GFX_IMPLEMENT_DYNCREATE(CMacroButton, CIFDecoratedStatic)

// =============================================================================
// CMacroButton Implementation
// =============================================================================

bool CMacroButton::OnCreate(long ln)
{
	BS_DEBUG_LOW("> " __FUNCTION__ "(%d)", ln);
	
	CIFDecoratedStatic::OnCreate(ln);
	
	// Initialize member variables
	m_buttonWidth = 36;
	m_buttonHeight = 36;
	m_offsetFromRight = 60;
	m_offsetFromTop = 200;
	m_isHovering = false;
	
	// Load the icon texture
	TB_Func_13("icon\\etc\\macro_normal.ddj", 0, 0);
	SetGWndSize(36, 36);
	
	// Set animation parameters
	set_N00009BD4(1);
	set_N00009BD3(500);
	
	// Set button size
	SetGWndSize(m_buttonWidth, m_buttonHeight);
	*reinterpret_cast<int*>(reinterpret_cast<char*>(this) + 0x44) = m_buttonWidth;
	*reinterpret_cast<int*>(reinterpret_cast<char*>(this) + 0x48) = m_buttonHeight;

	return true;
}

int CMacroButton::OnMouseLeftUp(int a1, int x, int y)
{
	BS_DEBUG_LOW("> " __FUNCTION__ "(%d, %d, %d)", a1, x, y);
	
	if (!IsInside(x, y)) {
		return 0;
	}

	// Toggle MacroWindow visibility using global frame
	extern CIFMainFrame* MacroWindowMainFrame;
	
	if (MacroWindowMainFrame) {
		bool isVisible = MacroWindowMainFrame->IsVisible();
		
		if (isVisible) {
			MacroWindowMainFrame->ShowGWnd(false);
		} else {
			// Use hardcoded window size
			const int WINDOW_WIDTH = 360;
			const int WINDOW_HEIGHT = 244;
			
			// Reset position
			int screenWidth = g_D3DViewportWidth > 0 ? g_D3DViewportWidth : GetSystemMetrics(SM_CXSCREEN);
			int screenHeight = g_D3DViewportHeight > 0 ? g_D3DViewportHeight : GetSystemMetrics(SM_CYSCREEN);
			
			int newX = (screenWidth - WINDOW_WIDTH) / 2;
			int newY = (screenHeight - WINDOW_HEIGHT) / 4;
			
			MacroWindowMainFrame->MoveGWnd(newX, newY);
			MacroWindowMainFrame->ShowGWnd(true);
			MacroWindowMainFrame->BringToFront();
		}
	}

	return 1;
}

int CMacroButton::OnMouseLeftDown(int a1, int x, int y)
{
	BS_DEBUG_LOW("> " __FUNCTION__ "(%d, %d, %d)", a1, x, y);
	
	if (!IsInside(x, y)) {
		return 0;
	}

	return 1;
}

int CMacroButton::OnMouseMove(int a1, int x, int y)
{
	return 0;
}

void CMacroButton::OnUpdate()
{
	CIFDecoratedStatic::OnUpdate();
	
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

void CMacroButton::OnCIFReady()
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

bool CMacroButton::IsInside(int x, int y)
{
	int posX = *reinterpret_cast<int*>(reinterpret_cast<char*>(this) + 0x3C);
	int posY = *reinterpret_cast<int*>(reinterpret_cast<char*>(this) + 0x40);
	
	return (x >= posX && x < posX + m_buttonWidth &&
	        y >= posY && y < posY + m_buttonHeight);
}
