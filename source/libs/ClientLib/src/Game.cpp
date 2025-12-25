#include "Game.h"

// CGame::GetFont(int) .text 00BA5460 00000022 00000000 00000004 R . . . . . .
void* CGame::GetFont(int a2)
{
	// assert(FALSE);
	return reinterpret_cast<void*(__thiscall*)(CGame*,int)>(0x008A1FC0)(this, a2); //ecsro
}

void CGame::InitGameAssets() {

	reinterpret_cast<void(__thiscall *)(CGame * pthis)>(0x005E5EB0)(this); //ECSRO ORI
}

void CGame::LoadTextfiles()
{
	// TODO:: Implement me
	reinterpret_cast<void(__thiscall*)(CGame*)>(0x00844CD0)(this);
}

void CGame::ResizeMainWindow()
{
	reinterpret_cast<void(__thiscall*)(CGame*)>(0x00840E90)(this);
}

const ClientRes &CGame::GetRes() const {
    return N00000888;
}

const ClientResolutonData &CGame::GetClientDimensionStuff() {
    const ClientRes &res = theApp.GetRes();
    return res.res[res.index];
}

CStringCheck *CGame::GetBadwordFilter() {
    return theApp.m_badWordFilter;
}

WhatIsThis &CGame::STA_FUN_004f9d00() {
    WhatIsThis* ptr = reinterpret_cast<WhatIsThis*(*)()>(0x004f9d00)();
    return *ptr;
}

void CGame::sub_8371D0() {
    reinterpret_cast<void(__thiscall*)(CGame*)>(0x008371D0)(this);
}


std::string CGame::GetServerName()
{
	return MEMUTIL_READ_BY_PTR_OFFSET(this, 0x598, std::string);
}

char* CGame::GetPathFolder() {
	char* path = MEMUTIL_READ_BY_PTR_OFFSET(this, 0x10C, char*);
	if (!path) return _strdup(""); // Return empty string safely

	return _strdup(path); // Duplicate and return dynamically allocated string
}

