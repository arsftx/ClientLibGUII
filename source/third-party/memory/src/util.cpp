#include "util.h"

void *CMemoryUtility::m_tmpProtectAddr;
DWORD CMemoryUtility::m_tmpProtect;
size_t CMemoryUtility::m_tmpProtectSize;

bool CMemoryUtility::UnProtect(void *addr, size_t count) {
    if (VirtualProtect(addr, count, PAGE_READWRITE, &m_tmpProtect) == FALSE)
        return false;

    m_tmpProtectAddr = addr;
    m_tmpProtectSize = count;

    return true;
}

bool CMemoryUtility::ReProtect() {
    //restore original protection mode
    return VirtualProtect(m_tmpProtectAddr, m_tmpProtectSize, m_tmpProtect, &m_tmpProtect) != FALSE;
}

bool CMemoryUtility::RenderDetour(BYTE instruction, void *source, void *dest) {
    DWORD oldProtect;
    BYTE buffer[5];

    if (!VirtualProtect(source, sizeof(buffer), PAGE_EXECUTE_READWRITE, &oldProtect))
        return false;


    buffer[0] = instruction; //ASM_JMP or ASM_CALL only for now
    *(DWORD * )(buffer + 1) = ((DWORD) dest - ((DWORD) source + 5));

    memcpy(source, buffer, sizeof(buffer));


    VirtualProtect(source, sizeof(buffer), oldProtect, &oldProtect);

    return true;
}

bool CMemoryUtility::RenderNop(void *addr, int count) {
    DWORD oldProtect;
    if (!VirtualProtect(addr, count, PAGE_EXECUTE_READWRITE, &oldProtect))
        return false;

    memset(addr, 0x90, count);

    VirtualProtect(addr, count, oldProtect, &oldProtect);

    return true;


}

BOOL CMemoryUtility::Nop(uintptr_t offset, size_t count) {
    LPVOID lpOffset = reinterpret_cast<LPVOID>(offset);
    DWORD dwOldProtect = 0;

    if (!VirtualProtect(lpOffset, count, PAGE_READWRITE, &dwOldProtect))
        return false;

    memset(lpOffset, MEMUTIL_ASM_OPCODE_NOP, count);

    return VirtualProtect(lpOffset, count, dwOldProtect, &dwOldProtect);
}

BOOL CMemoryUtility::SetupHook(LongENUM type, uintptr_t src, uintptr_t dest) {
    DWORD dwOldProtect;
    LPVOID lpSource = reinterpret_cast<LPVOID>(src);
    LPVOID lpDest = reinterpret_cast<LPVOID>(dest);

    if (type == LongJump || type == LongCall) {
        char instruction[5];
        size_t nInstructionSize = sizeof(instruction);

        instruction[0] = (type == LongJump) ?
                         MEMUTIL_ASM_OPCODE_LONG_JUMP : MEMUTIL_ASM_OPCODE_LONG_CALL;

        *(DWORD * )(instruction + 1) = (dest - (src + 5));

        if (!VirtualProtect(lpSource, nInstructionSize, PAGE_EXECUTE_READWRITE, &dwOldProtect))
            return false;

        memcpy(lpSource, instruction, nInstructionSize);

        return VirtualProtect(lpSource, nInstructionSize, dwOldProtect, &dwOldProtect);
    }

    return false;
}

BOOL CMemoryUtility::Write(uintptr_t offset, const void *data, int length) {
    LPVOID lpOffset = reinterpret_cast<LPVOID>(offset);
    DWORD dwOldProtect = 0;
    if (!VirtualProtect(lpOffset, length, PAGE_READWRITE, &dwOldProtect))
        return false;

    memcpy(lpOffset, data, length);

    return VirtualProtect(lpOffset, length, dwOldProtect, &dwOldProtect);
}
