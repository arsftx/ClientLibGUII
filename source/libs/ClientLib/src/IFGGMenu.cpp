#include "IFGGMenu.h"
#include <IFStatic.h>
#include <IFSelectableArea.h>



bool CIFGGMenu::OnCreateGGMenuIMPL(long ln) {
	//printf("%s\n", __FUNCTION__);
	bool b = reinterpret_cast<bool(__thiscall*)(const CIFGGMenu*, long)>(0x00644510)(this, ln);

	return b;
}

int CIFGGMenu::OnPressTab(int a1)
{
	return reinterpret_cast<int(__thiscall*)(const CIFGGMenu*,int)>(0x6441C0)(this, a1);
}

CIFScrollManager* CIFGGMenu::GetScrollManager() const
{
	return MEMUTIL_READ_BY_PTR_OFFSET(this, 0x7E0, CIFScrollManager*);
}
int CIFGGMenu::TabGGMenu()
{
	return MEMUTIL_READ_BY_PTR_OFFSET(this, 0x7B0, int);
}

