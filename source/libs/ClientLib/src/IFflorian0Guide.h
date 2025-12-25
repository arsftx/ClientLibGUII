#pragma once
#include "IFDecoratedStatic.h"

#define GUIDE_FLORIAN0 13378

class CIFflorian0Guide : public CIFDecoratedStatic
{
	GFX_DECLARE_DYNCREATE(CIFflorian0Guide)

public:
	bool OnCreate(long ln) override;
	int OnMouseLeftUp(int a1, int x, int y) override;
	int OnMouseLeftDown(int a1, int x, int y) override;  // Must consume to prevent map click
	int OnMouseMove(int a1, int x, int y) override;  // For hover effect
	void OnUpdate() override;  // For hover exit detection
	void OnCIFReady() override;
	
	// Override IsInside to restrict click area to actual 40x40 button size
	bool IsInside(int x, int y) override;
};
