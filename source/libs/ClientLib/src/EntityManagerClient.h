#pragma once
#include "GFXMainFrame/EntityManager.h"

class CEntityManagerClient : public CEntityManager
{
	
};

// ECSRO: 0x00C5DCF0 (found in sub_64F270 -> dword_C5DCF0)
// VSRO:  0x0110F7D8
#define g_pGfxEttManager (*(CEntityManagerClient**)0x00C5DCF0)

