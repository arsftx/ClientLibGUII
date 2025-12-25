#pragma once

// Forward declarations
class CNetProcessIn;
class CMsgStreamBuffer;

// Global variables for Damage Injection
extern unsigned int g_LastExpandedDamage;
extern unsigned int g_LastDamageTargetID;
extern unsigned int g_LastAttackType;

class CustomOpcodeHandler {
public:
    // Installs the hook on RegisterPacketHandlers
    static void InstallHooks();

    // The actual handler logic for 0xF001 (Extended Damage)
    static void OnF001(CNetProcessIn* pNet, CMsgStreamBuffer& msg);

    // Static wrapper to bridge ASM -> C++ for 0xF001
    static void __stdcall OnF001_Wrapper(void* instance, void* msg);
    
    // Note: F100 validation now uses Named Pipes - see ClientValidation.cpp
};
