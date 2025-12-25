#include "DamageFix.h"
#include <Windows.h>
#include <stdio.h>

// GenDigits function address - draws native damage text
#define ADDR_GEN_DIGITS 0x006B17F0

// DEBUG LOGS DISABLED
static void Log(const char* msg) {
    return; // Disabled for performance
    /*
    FILE* fp = fopen("ClientLog.txt", "a");
    if (fp) {
        fprintf(fp, "%s\n", msg);
        fclose(fp);
    }
    */
}

// Write bytes to memory
static void WriteBytes(DWORD address, BYTE* data, int len) {
    DWORD oldProtect;
    if (VirtualProtect((void*)address, len, PAGE_EXECUTE_READWRITE, &oldProtect)) {
        memcpy((void*)address, data, len);
        VirtualProtect((void*)address, len, oldProtect, &oldProtect);
    }
}

namespace DamageFix {
    void Install() {
        // Patch GenDigits to immediately return
        // This disables the native damage text completely
        // CustomDamageRenderer will handle all damage display via D3D
        
        // C2 0C 00 = RET 12 (return, cleanup 12 bytes from stack)
        // GenDigits signature: int __thiscall sub_6B17F0(this, float, DamageInfo*, int)
        BYTE patchRet[] = { 0xC2, 0x0C, 0x00 };
        WriteBytes(ADDR_GEN_DIGITS, patchRet, sizeof(patchRet));
        
        Log("[DamageFix] GenDigits patched to RET - Native damage text DISABLED");
    }
}