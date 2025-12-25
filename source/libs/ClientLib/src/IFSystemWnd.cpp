#include "IFSystemWnd.h"
#include <IFStatic.h>



bool IFSystemWnd::OnCreateIMPL(long ln) {


	bool b = reinterpret_cast<bool(__thiscall*)(IFSystemWnd*, long)>(0x00483990)(this, ln);
	m_btnrequest->ShowWnd(false);
	return b;
}