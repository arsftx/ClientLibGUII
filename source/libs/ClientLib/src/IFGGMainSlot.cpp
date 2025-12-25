#include "IFGGMainSlot.h"
#include <IFStatic.h>
#include <GInterface.h>



bool CIFGGMainSlot::OnCreateGuideSlotIMPL(long ln) {
	//printf("%s\n", __FUNCTION__);
	bool b = reinterpret_cast<bool(__thiscall*)(const CIFGGMainSlot*, long)>(0x00640810)(this, ln);

	switch (UniqueID()) {
	case 112000:
	case 120000:
	case 121000:
	case 124000:
	case 127000:
	case 140000:
	{
		TB_Func_13("", 0, 0);
		/*CIFStatic* m_GDR_BTN = m_IRM.GetResObj<CIFStatic>(1, 1);
		CIFStatic* m_GDR_TEXT_NAME = m_IRM.GetResObj<CIFStatic>(2, 1);*/
		/*m_GDR_TEXT_NAME->ShowWnd(false);
		m_GDR_BTN->ShowWnd(false);*/
		SetClickable(false);
		ShowWnd(false);
	}
	break;
	}

	return b;
}


int CIFGGMainSlot::OnPressGGMainSlot()
{
	
	//printf("g_pCGInterface->Get_CIFGGMenu()->IsVisible() 0x%x\n", g_pCGInterface->Get_CIFGGMenu()->TabGGMenu());
	return reinterpret_cast<int(__thiscall*)(const CIFGGMainSlot*)>(0x00640690)(this);
}