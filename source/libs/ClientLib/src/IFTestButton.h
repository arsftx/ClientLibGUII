#pragma once

#include "IFDecoratedStatic.h"

#define GUIDE_TEST_BUTTON 9999

// Test button using CIFDecoratedStatic pattern (like IFflorian0Guide for VSRO)
// This is for ECSRO - same pattern, different memory addresses
class CIFTestDecoratedButton : public CIFDecoratedStatic
{
    GFX_DECLARE_DYNCREATE(CIFTestDecoratedButton)

public:
    bool OnCreate(long ln) override;
    int OnMouseLeftUp(int a1, int x, int y) override;
    void OnCIFReady() override;

    // Static method to create and initialize the test button
    static bool CreateTestButton();

    // Debug logging to clientlog.txt
    static void DebugLog(const char* format, ...);
};

extern CIFTestDecoratedButton* g_pTestDecoratedButton;
