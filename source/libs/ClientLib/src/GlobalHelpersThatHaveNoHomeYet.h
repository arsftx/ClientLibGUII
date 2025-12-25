#pragma once

union uregion {
	struct {
		char y;
		char x;
	} single;
	short r;
};

#define g_CurrentIfUnderCursor (*(CGWndBase **) 0x00C5DE4C) //ECSRO

#define g_Region (*(uregion *)0x00cb3de4)


#define SendMsg(x) reinterpret_cast<void(__cdecl *)(CMsgStreamBuffer &)>(0x5E4340)(x) //ECSRO

