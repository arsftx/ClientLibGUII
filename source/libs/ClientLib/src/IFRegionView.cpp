#include "IFRegionView.h"
#include <IFStatic.h>



bool IFRegionView::OnCreateIMPL(long ln) {
	//printf("%s\n", __FUNCTION__);
	bool b = reinterpret_cast<bool(__thiscall*)(const IFRegionView*, long)>(0x005438D0)(this, ln);
	

	return b;
}
