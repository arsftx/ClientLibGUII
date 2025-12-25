#include "IFMessageBox.h"
#include <TextStringManager.h>

int CIFMessageBox::GetItemID()
{
	if (!this)
		return 0;

	int ItemID = MEMUTIL_READ_BY_PTR_OFFSET(this, 0x870, int);
	
	return ItemID;
}
void CIFMessageBox::OnCreateIMPL()
{
	reinterpret_cast<void(__thiscall*)(CIFMessageBox*)>(0x0051CBD0)(this);
}