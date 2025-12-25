#pragma once
#include "IFDecoratedStatic.h"

// =============================================================================
// CMenuButton - Abstract base class for custom menu buttons
// Inherits from CIFDecoratedStatic, provides:
// - Dynamic resolution-aware positioning
// - Common mouse event handling structure
// - Invisible by default - derived classes load their own textures
// =============================================================================
class CMenuButton : public CIFDecoratedStatic
{
	GFX_DECLARE_DYNCREATE(CMenuButton)

public:
	bool OnCreate(long ln) override;
	int OnMouseLeftUp(int a1, int x, int y) override;
	int OnMouseLeftDown(int a1, int x, int y) override;
	int OnMouseMove(int a1, int x, int y) override;
	void OnUpdate() override;
	void OnCIFReady() override;
	bool IsInside(int x, int y) override;

protected:
	// Called by derived classes to set button size (default 40x40)
	void SetButtonSize(int width, int height);
	
	// Called by derived classes to update position
	void RepositionButton(int screenWidth, int screenHeight);
	
	// Button dimensions (set by derived classes)
	int m_buttonWidth;
	int m_buttonHeight;
	
	// Position offset from screen edge (set by derived classes)
	int m_offsetFromRight;
	int m_offsetFromTop;
	
	// Hover state tracking
	bool m_isHovering;
};
