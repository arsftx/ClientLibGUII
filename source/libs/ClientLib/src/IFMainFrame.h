#pragma once

#include "IFFrame.h"
#include "IFStatic.h"
#include "IFCloseButton.h"
#include "IFDragableArea.h"

class CIFMainFrame : public CIFFrame {
    GFX_DECLARE_DYNAMIC_EXISTING(CIFMainFrame, 0x009ffed4)//ecsro

GFX_DECLARE_MESSAGE_MAP(CIFMainFrame)
public:
    /// \address 00816F20
    CIFMainFrame();

    // Destructor is trivial
    // \address 00816F60
    // ~CIFMainFrame();

    /// \address 00816F80
    bool OnCreate(long ln) override;

    /// \address 00817210
    void SetGWndSize(int nWidth, int nHeight) override;

    /// \address 00817230
    bool SetText(const char *pSrcText) override;

public:
    /// \address 00817170
    void UpdateControlsSizePos();

    void OnClick_Exit();

public:
    CIFStatic *m_pTitleText;      //0x0678
    CIFDragableArea *m_pHandleBar;//0x067C
    CIFCloseButton *m_pCloseBtn;  //0x680

BEGIN_FIXTURE()
    ENSURE_SIZE(0x0684)
        ENSURE_OFFSET(m_pTitleText, 0x0678)
        ENSURE_OFFSET(m_pHandleBar, 0x067C)
        ENSURE_OFFSET(m_pCloseBtn, 0x680)
    END_FIXTURE()

    RUN_FIXTURE(CIFMainFrame)
};
