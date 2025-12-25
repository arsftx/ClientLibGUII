#include "IFItemMall.h"
#include <TextStringManager.h>
#include <GInterface.h>



bool CIFItemMallMyInfo::OnCreateIMPL(long ln) {

	wnd_rect sz;
	
	sz.pos.x = 2;
	sz.pos.y = 127;
	sz.size.width = 60;
	sz.size.height = 24;
	CIFStatic* m_fakebutton = (CIFStatic*)CGWnd::CreateInstance(this, GFX_RUNTIME_CLASS(CIFStatic), sz, 256, 0);
	m_fakebutton->TB_Func_12("interface\\ifcommon\\com_long_tab_on.ddj", 0, 0);
	m_fakebutton->SetText("Equipment");
	m_fakebutton->m_FontTexture.sub_8B4750(14);

	sz.pos.x = 80;
	sz.pos.y = 59;
	sz.size.width = 20;
	sz.size.height = 20;
	CIFStatic* m_moneysilk = (CIFStatic*)CGWnd::CreateInstance(this, GFX_RUNTIME_CLASS(CIFStatic), sz, 257, 0);
	m_moneysilk->TB_Func_12("interface\\mall\\mall_moneybutton.ddj", 1, 0);
	m_moneysilk->BringToUp();
	return reinterpret_cast<bool(__thiscall*)(CIFItemMallMyInfo*, long)>(0x005299A0)(this, ln);
}

CRefPackageItemData* CSOItemPackage::GetPackageItemData() const {
	return m_pPackageItemData;
}
CSOItem* CSOItemPackage::GetSOItem() const {
	if (m_vPSOItem.size() > 0) return m_vPSOItem[0];

	return NULL;
}
bool CIFItemMallConfirmBuy::OnCreateIMPL(long ln) {


	bool b = reinterpret_cast<bool(__thiscall*)(CIFItemMallConfirmBuy*, long)>(0x0052A500)(this, ln);
	this->ChangeGWndSize(313, 150);

	return b;
}


bool CIFItemMallConfirmSlot::OnCreateIMPL(long ln)
{
	bool b = reinterpret_cast<bool(__thiscall*)(CIFItemMallConfirmSlot*, long)>(0x68E640)(this, ln);
	
	return b;
}