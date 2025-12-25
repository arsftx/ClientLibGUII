#pragma once

#include <Windows.h>
#include <iostream>
#include <vector>

//#include "detours.h"

//Assembly mnemonics
#define ASM_NOP 0x90
#define ASM_JMP 0xE9 //LONG
#define ASM_CALL 0xE8 //LONG

enum LongENUM {
    LongJump,
    LongCall
};


#define MEMUTIL_ASM_OPCODE_LONG_JUMP             0xE9
#define MEMUTIL_ASM_OPCODE_SHORT_JUMP            0xEB
#define MEMUTIL_ASM_OPCODE_LONG_CALL             0xE8
#define MEMUTIL_ASM_OPCODE_NOP                   0x90

#define MEMUTIL_WRITE_VALUE(type, offset, value) \
    CMemoryUtility::Write<type>(offset, value)

#define MEMUTIL_WRITE_POINTER(offset, dataPtr, dataLen) \
    CMemoryUtility::Write(offset, dataPtr, dataLen)

#define MEMUTIL_NOP(offset, count) \
    CMemoryUtility::Nop(offset, count)

#define MEMUTIL_READ_BY_PTR_OFFSET(ptr, offset, type) \
    *(type*)(((uintptr_t)ptr) + offset)

#define MEMUTIL_WRITE_BY_PTR_OFFSET(ptr, offset, type, value) \
    *(type*)(((uintptr_t)ptr) + offset) = value;

#define MEMUTIL_ADD_PTR(ptr, offset) \
	(((uintptr_t)(ptr)) + offset)

#define MEMUTIL_SETUP_HOOK(type, src, dest) \
    CMemoryUtility::SetupHook(type, src, dest);

#define MEMUTIL_HOOK_FN(src, target) \
    DetourTransactionBegin(); \
    DetourAttach(&(PVOID&)src, target); \
    DetourTransactionCommit();


class CMemoryUtility {
public:
    template<typename T>
    static bool Write(uintptr_t offset, const T& value)
    {
        LPVOID lpOffset = reinterpret_cast<LPVOID>(offset);

        DWORD dwOldProtect = 0;
        if (!VirtualProtect(lpOffset, sizeof(T), PAGE_READWRITE, &dwOldProtect))
            return false;

        *(T*)(offset) = value;

        return VirtualProtect(lpOffset, sizeof(T), dwOldProtect, &dwOldProtect);
    }

    static bool UnProtect(void *addr, size_t count);
    static bool ReProtect();
    static BOOL Write(uintptr_t offset, const void *data, int length);
    static bool RenderDetour(BYTE instruction, void *source, void *dest);
    static bool RenderNop(void *addr, int count);
    static BOOL Nop(uintptr_t offset, size_t count);
    static BOOL SetupHook(LongENUM type, uintptr_t src, uintptr_t dest);

    template<typename T>
    static T call_virtual(void *base_ptr, unsigned index) {
        return (*static_cast<T **>(base_ptr))[index];
    }

    template<typename T>
    static T read_offset_as_ref(void *base_ptr, unsigned offset = 0) {
        return *reinterpret_cast<T *>(reinterpret_cast<unsigned>(base_ptr) + offset);
    }

    template<typename T>
    static T read_offset_as_ptr(void *base_ptr, unsigned offset = 0) {
        return reinterpret_cast<T>(reinterpret_cast<unsigned>(base_ptr) + offset);
    }

private:
    static void *m_tmpProtectAddr;
    static DWORD m_tmpProtect;
    static size_t m_tmpProtectSize;
};