#pragma once
#include "IFMainFrame.h"
#include "IFWnd.h"
#include "IFTileWnd.h"
class IFSystemWnd : public CIFMainFrame {
GFX_DECLARE_DYNAMIC_EXISTING(IFSystemWnd, 0x9FEB64)
public:
	bool OnCreateIMPL(long ln);
public:
	
	CIFButton* m_btnoption;
	CIFButton* m_btnhelp;
	CIFButton* m_btnrequest;
	CIFButton* m_btnrestart;
	CIFButton* m_btnexit;
	CIFFrame* m_frame;
	char pad_Unk[8];
BEGIN_FIXTURE()
        ENSURE_SIZE(0x6A4)
	
	ENSURE_OFFSET(m_btnoption, 0x684)
	ENSURE_OFFSET(m_btnhelp, 0x688)
	ENSURE_OFFSET(m_btnrequest, 0x68c)
	ENSURE_OFFSET(m_btnrestart, 0x690)
	ENSURE_OFFSET(m_btnexit, 0x694)
	ENSURE_OFFSET(m_frame, 0x698)
    END_FIXTURE()

    RUN_FIXTURE(IFSystemWnd)
};

