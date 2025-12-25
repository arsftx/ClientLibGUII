#include "IFMainFrame.h"

#include "Game.h"
#include <BSLib/Debug.h>

GFX_IMPLEMENT_DYNAMIC_EXISTING(CIFMainFrame, 0x009ffed4)

GFX_BEGIN_MESSAGE_MAP(CIFMainFrame, CIFWnd)
                    ONG_COMMAND(2, &CIFMainFrame::OnClick_Exit)
GFX_END_MESSAGE_MAP()

#define GDR_STA_TITLE 0
#define GDR_STA_DRAG 1
#define GDR_BTN_CLOSE 2

CIFMainFrame::CIFMainFrame()
        : m_pTitleText(NULL), m_pHandleBar(NULL), m_pCloseBtn(NULL) 
{
}

bool CIFMainFrame::OnCreate(long ln) {
    BS_DEBUG_LOW("%s", __FUNCTION__);
    CIFFrame::OnCreate(ln);

    wnd_rect local_10 = GetBounds();
    
    wnd_rect sz;
    sz.pos.x = 10;
    sz.pos.y = 12;
    sz.size.height = 12;
    sz.size.width = local_10.size.width - 21;
    
    m_pTitleText = (CIFStatic *) CreateInstance(this, GFX_RUNTIME_CLASS(CIFStatic), sz, GDR_STA_TITLE, 0);
    BS_DEBUG("Created title at %d|%d [%d|%d]", sz.pos.x, sz.pos.y, sz.size.width, sz.size.height);
    
    sz.pos.y = 0;
    sz.size.height = 34;
    
    m_pHandleBar = (CIFDragableArea *) CreateInstance(this, GFX_RUNTIME_CLASS(CIFDragableArea), sz, GDR_STA_DRAG, 0);
    BS_DEBUG("Created handlebar at %d|%d [%d|%d]", sz.pos.x, sz.pos.y, sz.size.width, sz.size.height);
    
    sz.size.height = 16;
    sz.size.width = 16;
    sz.pos.x = local_10.size.width - 26;
    sz.pos.y = 10;
    
    m_pCloseBtn = (CIFCloseButton *) CreateInstance(this, GFX_RUNTIME_CLASS(CIFCloseButton), sz, GDR_BTN_CLOSE, 0);
    BS_DEBUG("Created close btn at %d|%d [%d|%d]", sz.pos.x, sz.pos.y, sz.size.width, sz.size.height);
    
    RECT rect = { 0 };
    
    m_pTitleText->SetSomeRect(rect);
    m_pTitleText->SetFont(theApp.GetFont(0));
    m_pTitleText->TB_Func_5(1);
    m_pTitleText->TB_Func_6(0);
    
    m_pHandleBar->SetSomeRect(rect);
    m_pHandleBar->SetFont(theApp.GetFont(0));
    m_pHandleBar->TB_Func_5(1);
    m_pHandleBar->TB_Func_6(0);
    
    m_pCloseBtn->SetSomeRect(rect);
    m_pCloseBtn->SetFont(theApp.GetFont(0));
    m_pCloseBtn->TB_Func_5(1);
    m_pCloseBtn->TB_Func_6(0);
	TB_Func_12("interface\\frame\\mframe_wnd_", 0, 0);
    return true;
}

void CIFMainFrame::UpdateControlsSizePos() {
    wnd_rect rcBounds = GetBounds();
    wnd_rect rcNewBounds;
    
    rcNewBounds.pos.y = rcBounds.pos.y + 12;
    rcNewBounds.pos.x = rcBounds.pos.x + 10;
    rcNewBounds.size.width = rcBounds.size.width - 21;
    
    m_pTitleText->MoveGWnd(rcNewBounds.pos.x, rcNewBounds.pos.y);
    m_pTitleText->SetGWndSize(rcNewBounds.size.width, 12);
    
    rcNewBounds.pos.x += 10;
    
    m_pHandleBar->MoveGWnd(rcNewBounds.pos.x, rcNewBounds.pos.y);
    m_pHandleBar->SetGWndSize(rcNewBounds.size.width, 34);
    
    m_pCloseBtn->MoveGWnd(rcNewBounds.pos.x - 26 + rcNewBounds.size.width, rcNewBounds.pos.y + 10);
}

void CIFMainFrame::SetGWndSize(int nWidth, int nHeight) {
    CIFTileWnd::SetGWndSize(nWidth, nHeight);

    UpdateControlsSizePos();
}

bool CIFMainFrame::SetText(const char *pSrcText) {
    return m_pTitleText->SetText(pSrcText);
}

void CIFMainFrame::OnClick_Exit() {
    this->OnCloseWnd();
}