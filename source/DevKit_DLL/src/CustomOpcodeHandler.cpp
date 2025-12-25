#include "CustomOpcodeHandler.h"
#include "CustomDamageRenderer.h"
#include <NetProcessIn.h>
#include <BSLib/Debug.h>
#include <memory/hook.h>
#include <Windows.h>
#include <stdio.h>
#include <stdarg.h>

// Global variables to store damage data
unsigned int g_LastExpandedDamage = 0;
unsigned int g_LastDamageTargetID = 0;
unsigned int g_LastAttackType = 0;

// Hack struct to access private members of CMsgStreamBuffer
struct CMsgStreamBuffer_Hack {
    void* vptr; 
    unsigned int m_currentReadBytes;
    unsigned int m_availableBytesForReading;
    unsigned char padding[4]; 
    void *m_node1; // Data Buffer Start (Linked List Node)
    void *m_node2; 
    unsigned short m_msgid;
};

// =============================================================
// 1. HANDLER LOGIC (NEW FORMAT WITH ATTACKER ID)
// =============================================================
void CustomOpcodeHandler::OnF001(CNetProcessIn* pNet, CMsgStreamBuffer& msg) {
    // Packet format: [TargetID][AttackerID][Damage][Flags]
    // Flags: bit0-4 = attackType, bit5 = isBlock, bit6 = isCritical

    __try {
        if (!pNet) return;

        // Cast to Hack Struct
        CMsgStreamBuffer_Hack* pHack = (CMsgStreamBuffer_Hack*)&msg;

        // Validate Node Pointer
        if (!pHack->m_node1 || IsBadReadPtr(pHack->m_node1, 20)) {
            return; 
        }

        // --- MANUAL READ (New Format) ---
        // Offset 4:  TargetID   (pData[1])
        // Offset 8:  AttackerID (pData[2]) - NEW
        // Offset 12: Damage     (pData[3])
        // Offset 16: Flags      (pData[4])
        unsigned int* pData = (unsigned int*)pHack->m_node1;
        
        unsigned int targetID   = pData[1];
        unsigned int attackerID = pData[2];  // NEW: Attacker entity ID
        unsigned int damage     = pData[3];
        unsigned int flags      = pData[4];

        // Decode flags (new format from server):
        // Bits 0-3: Hit Type (0=normal, 2=blocked, 4=KD, 5=KB)
        // Bit 5 (0x20) = BLOCK
        // Bit 6 (0x40) = CRITICAL
        // Bit 7 (0x80) = KILL
        bool isBlock = (flags & 0x20) != 0;
        bool isCritical = (flags & 0x40) != 0;
        bool isKill = (flags & 0x80) != 0;
        
        // Combine flags for attackType used by renderer
        unsigned int attackType = flags & 0x0F;  // Lower 4 bits = hit type
        if (isBlock) attackType |= 0x100;        // Add block flag
        if (isCritical) attackType |= 0x200;     // Add critical flag
        if (isKill) attackType |= 0x400;         // Add kill flag

        // Store Globals (for backward compat)
        g_LastDamageTargetID = targetID;
        g_LastExpandedDamage = damage;
        g_LastAttackType = attackType;

        // Add to D3D CustomDamageRenderer queue with attackerID
        CustomDamageRenderer::Instance().AddDamage(targetID, damage, attackType, attackerID);

        // Manually advance read bytes to simulate consumption
        // 4 (Target) + 4 (Attacker) + 4 (Damage) + 4 (Flags) = 16 bytes
        pHack->m_currentReadBytes += 16;

        // DEBUG LOGS DISABLED
        /*
        static int s_LogCount = 0;
        if (s_LogCount < 10) {
            FILE* fp = fopen("ClientLog.txt", "a");
            if (fp) {
                fprintf(fp, "[DamageFix] Target: %u, Attacker: %u, Dmg: %u, Flags: %X\n", 
                        targetID, attackerID, damage, flags);
                fclose(fp);
                s_LogCount++;
            }
        }
        */
        
        BS_DEBUG("[DamageFix] 0xF001 -> Target: %u, Attacker: %u, Dmg: %u, Flags: %X", 
                 targetID, attackerID, damage, flags);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        BS_DEBUG("[DamageFix] Exception in OnF001");
    }
}

// =============================================================
// 2. STATIC WRAPPER
// =============================================================
void __stdcall CustomOpcodeHandler::OnF001_Wrapper(void* instance, void* msg) {
    if (instance && msg) {
        CNetProcessIn* pNet = (CNetProcessIn*)instance;
        CMsgStreamBuffer* pMsg = (CMsgStreamBuffer*)msg;
        CustomOpcodeHandler::OnF001(pNet, *pMsg);
    }
}

// =============================================================
// 3. ASM STUB - 0xF001 Extended Damage
// =============================================================
void __declspec(naked) F001_Handler_Stub() {
    __asm {
        pushad
        pushfd

        // Stack: [EFLAGS][Regs][RetAddr][MsgPtr] -> Offset 0x28
        mov eax, [esp + 0x28] // MsgPtr
        mov ecx, [esp + 0x1C] // this (from pushad)

        push eax         // Arg2
        push ecx         // Arg1
        call CustomOpcodeHandler::OnF001_Wrapper

        popfd
        popad
        ret 4            
    }
}

// Note: F100 validation now uses Named Pipes instead of game opcodes
// Removed: F100_Validation_Stub, OnF100, OnF100_Wrapper

// =============================================================
// 4. REGISTRATION HOOK (Only F001 - Damage)
// =============================================================

void __declspec(naked) Hook_RegisterPacketHandlers() {
    __asm {
        sub esp, 0x24
        push ebx
        push ebp
        push esi
        
        mov esi, ecx
        add esi, 0x18 
        
        // --- INJECTION: Register 0xF001 (Extended Damage) ---
        pushad 
        pushfd

        sub esp, 32 

        // Pair: [0xF001][StubAddr]
        mov dword ptr [esp + 16], 0xF001          
        lea eax, F001_Handler_Stub                
        mov dword ptr [esp + 20], eax             

        // Call Map::Insert
        lea eax, [esp + 16]
        push eax                                  // Arg2: PairPtr
        lea eax, [esp + 4] 
        push eax                                  // Arg1: ResultPtr
        mov ecx, esi                              // ECX: MapPtr

        mov eax, 0x005ACB20 
        call eax

        add esp, 32 

        popfd
        popad

        // --- EPILOGUE ---
        mov eax, 0x005A9AA6
        jmp eax
    }
}

// =============================================================
// 5. INSTALLATION
// =============================================================

void CustomOpcodeHandler::InstallHooks() {
    // F001 Extended Damage handler hook - no logging needed
    unsigned char* addr = (unsigned char*)0x005A9AA0;
    if (addr[0] == 0x83 && addr[1] == 0xEC) {
        placeHook(0x005A9AA0, Hook_RegisterPacketHandlers);
        FlushInstructionCache(GetCurrentProcess(), (void*)0x005A9AA0, 5);
    }
}