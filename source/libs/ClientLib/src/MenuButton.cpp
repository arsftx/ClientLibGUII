#include "MenuButton.h"
#include <BSLib/Debug.h>
#include "GInterface.h"
#include "Game.h"
#include <windows.h>

// D3D9 Viewport Resolution (updated every frame from EndScene hook)
extern int g_D3DViewportWidth;
extern int g_D3DViewportHeight;

GFX_IMPLEMENT_DYNCREATE(CMenuButton, CIFDecoratedStatic)

// =============================================================================
// CMenuButton - Base class implementation (invisible by default)
// =============================================================================

bool CMenuButton::OnCreate(long ln)
{
	BS_DEBUG_LOW("> " __FUNCTION__ "(%d)", ln);
	
	CIFDecoratedStatic::OnCreate(ln);
	
	// Initialize default values
	m_buttonWidth = 40;
	m_buttonHeight = 40;
	m_offsetFromRight = 60;
	m_offsetFromTop = 200;
	m_isHovering = false;
	
	// Set default size
	SetGWndSize(m_buttonWidth, m_buttonHeight);
	
	// Base class is INVISIBLE - no texture loading, no show
	// Derived classes should load their own textures
	
	return true;
}

int CMenuButton::OnMouseLeftUp(int a1, int x, int y)
{
	// Base class: pass-through (don't consume event)
	return 0;
}

int CMenuButton::OnMouseLeftDown(int a1, int x, int y)
{
	// Base class: pass-through (don't consume event)
	return 0;
}

int CMenuButton::OnMouseMove(int a1, int x, int y)
{
	// Base class: pass-through (don't consume event)
	return 0;
}

void CMenuButton::OnUpdate()
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
		
		RepositionButton(currentWidth, currentHeight);
	}
}

void CMenuButton::OnCIFReady()
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
	
	RepositionButton(screenWidth, screenHeight);
}

bool CMenuButton::IsInside(int x, int y)
{
	int posX = *reinterpret_cast<int*>(reinterpret_cast<char*>(this) + 0x3C);
	int posY = *reinterpret_cast<int*>(reinterpret_cast<char*>(this) + 0x40);
	
	return (x >= posX && x < posX + m_buttonWidth &&
	        y >= posY && y < posY + m_buttonHeight);
}

void CMenuButton::SetButtonSize(int width, int height)
{
	m_buttonWidth = width;
	m_buttonHeight = height;
	SetGWndSize(width, height);
	*reinterpret_cast<int*>(reinterpret_cast<char*>(this) + 0x44) = width;
	*reinterpret_cast<int*>(reinterpret_cast<char*>(this) + 0x48) = height;
}

void CMenuButton::RepositionButton(int screenWidth, int screenHeight)
{
	int posX = screenWidth - m_offsetFromRight;
	int posY = m_offsetFromTop;
	MoveGWnd(posX, posY);
}
