#pragma once
#include "IFDecoratedStatic.h"

#define GUIDE_RANKINGICON 13379

// =============================================================================
// CRankingIcon - Ranking button derived from CIFDecoratedStatic
// Visible button with icon texture and hover/click functionality
// Features: Dynamic resolution-aware positioning
// =============================================================================
class CRankingIcon : public CIFDecoratedStatic
{
	GFX_DECLARE_DYNCREATE(CRankingIcon)

public:
	bool OnCreate(long ln) override;
	int OnMouseLeftUp(int a1, int x, int y) override;
	int OnMouseLeftDown(int a1, int x, int y) override;
	int OnMouseMove(int a1, int x, int y) override;
	void OnUpdate() override;
	void OnCIFReady() override;
	bool IsInside(int x, int y) override;

protected:
	// Hover state tracking
	bool m_isHovering;
	
	// Button dimensions
	int m_buttonWidth;
	int m_buttonHeight;
	
	// Position offset from screen edge
	int m_offsetFromRight;
	int m_offsetFromTop;
};
