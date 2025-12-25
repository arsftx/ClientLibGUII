#include "PSQuickStart.h"

#include <BSLib/Debug.h>

#include "unsorted.h"
#include "GlobalHelpersThatHaveNoHomeYet.h"
#include "ClientNet/ClientNet.h"
#include "Game.h"

GlobalVar<bool, 0x00EED310> CPSQuickStart::m_ready;

char CPSQuickStart::charname[32];
char CPSQuickStart::ibuv_text[32];

char CPSQuickStart::OnCreate(int a1)
{
	BS_DEBUG_LOW("> " __FUNCTION__ "(%d)", a1);

	if (!StartNetEngine())
	{
		CGFXMainFrame::SetNextProcessSTAT(reinterpret_cast<CGfxRuntimeClass*>(0x0EED894));
		return false;
	}

	g_CGame->LoadTextfiles();
	m_ready = true;

	ShowWindow(g_CGame->GetHWnd(), SW_SHOWNORMAL);
	UpdateWindow(g_CGame->GetHWnd());

	g_CGame->ResizeMainWindow();

	return true;
}

int CPSQuickStart::OnNetMsg(CMsgStreamBuffer* pMsg)
{
	BS_DEBUG_LOW("> " __FUNCTION__ " ~ Got Msg %04x", pMsg->msgid());
	printf("pMsg->msgid() Msg %04x\n", pMsg->msgid());

	return reinterpret_cast<int(__thiscall*)(CPSQuickStart*,CMsgStreamBuffer*)>(0x00594D70)(this, pMsg);
}
