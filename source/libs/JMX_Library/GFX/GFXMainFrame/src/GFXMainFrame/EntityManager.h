#pragma once

#include <BSLib/BSLib.h>
#include "IObject.h"

#include <map>

// ============================================================
// CEntityManager - ECSRO Complete Class Layout
// IDA Analysis: sub_89FE90 (Constructor)
// Size: 0x40 (64 bytes)
// RuntimeClass: 0xC5DE5C
// VTable: 0x967834
// g_pGfxEttManager: 0xC5DCF0
// ============================================================

class CEntityManager : public CObjChild
{
public:
    // CObjChild doesn't have explicit members in our definition
    // but CEntityManager uses offsets from 0x18+
    // Padding to reach entities at 0x1C
    char pad_0004[0x18];  // 0x04 to 0x1B
    
    // entities map at 0x1C (std::map has _Myhead, _Mysize, etc.)
    std::map<int, CIObject*> entities;  // 0x1C
    
    // Remaining fields (from IDA):
    // 0x28: second map (for different entity type?)
    // 0x34: byte flag (1)
    // 0x35: byte flag (0)
    // 0x38: dword (0x2710 = 10000)
    // 0x3C: buffer pointer
};
