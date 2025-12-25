#include "GFX3DFunction/RStateMgr.h"
#include "IFHScroll_Option.h"
#include "Game.h"
#include "ICPlayer.h"
#include "unsorted.h"


int CIFHScroll_Option::Set2E0(int value) {
	return MEMUTIL_WRITE_BY_PTR_OFFSET(this, 0x2E0, int, value);
}
int CIFHScroll_Option::Set2E4(int value) {
	return MEMUTIL_WRITE_BY_PTR_OFFSET(this, 0x2E4, int, value);
}
int CIFHScroll_Option::Set2E8(int value) {
	return MEMUTIL_WRITE_BY_PTR_OFFSET(this, 0x2E8, int, value);
}
int CIFHScroll_Option::Set2EC(int value) {
	return MEMUTIL_WRITE_BY_PTR_OFFSET(this, 0x2Ec, int, value);
}
int CIFHScroll_Option::Get2EC() {
	return MEMUTIL_READ_BY_PTR_OFFSET(this, 0x2Ec, int);
}
int CIFHScroll_Option::Set2F0(int value) {
	return MEMUTIL_WRITE_BY_PTR_OFFSET(this, 0x2f0, int, value);
}

CIFButton* CIFHScroll_Option::Get2F4()
{
	return MEMUTIL_READ_BY_PTR_OFFSET(this, 0x2f4, CIFButton*);
}
CIFButton* CIFHScroll_Option::Get2F8()
{
	return MEMUTIL_READ_BY_PTR_OFFSET(this, 0x2f8, CIFButton*);
}
CIFButton* CIFHScroll_Option::Get2FC()
{
	return MEMUTIL_READ_BY_PTR_OFFSET(this, 0x2fc, CIFButton*);
}
int CIFHScroll_Option::Set350(int value)
{
	
	return MEMUTIL_WRITE_BY_PTR_OFFSET(this, 0x350, int, value);
}

void CIFHScroll_Option::SetHScrollBar(int value1, int value2, int value3, int value4)
{
	reinterpret_cast<void(__thiscall*)(CIFHScroll_Option*,int,int,int,int)>(0x47A3A0)(this, value1, value2, value3, value4);
}

void CIFHScroll_Option::SetHCorrectScrollBar(int value1)
{
	reinterpret_cast<void(__thiscall*)(CIFHScroll_Option*, int)>(0x427240)(this, value1);
}
