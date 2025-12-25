#pragma once

#include <cstring>
#include <cstdlib>
#include "IFDecoratedStatic.h"

// CIFDecoratedStatic_ECSRO - ECSRO-specific layout wrapper
// Uses existing CIFDecoratedStatic but provides ECSRO-compatible method wrappers
// Size: 0x0358 (856 bytes) - must match game
// VTable: 0x0093CB60

class CIFDecoratedStatic_ECSRO : public CIFDecoratedStatic {
public:
    CIFDecoratedStatic_ECSRO() : CIFDecoratedStatic() {
    }
    
    virtual ~CIFDecoratedStatic_ECSRO() {
    }
    
    // Wrapper for TB_Func_13 with correct 12-byte GameString
    void TB_Func_13_ECSRO(const char* texturePath, int a3, int a4) {
        struct GameString {
            char* data;
            char* end;
            char* capacity;
        };
        
        size_t len = ::strlen(texturePath);
        char* buffer = (char*)::malloc(len + 1);
        ::strcpy(buffer, texturePath);
        
        GameString gs;
        gs.data = buffer;
        gs.end = buffer + len;
        gs.capacity = buffer + len + 1;
        
        // Call native TB_Func_13 at 0x00447210
        reinterpret_cast<void(__thiscall*)(void*, GameString, int, int)>(0x00447210)(this, gs, a3, a4);
    }
    
    // Wrapper for sub_634470 with correct 12-byte GameString
    void sub_634470_ECSRO(const char* texturePath) {
        struct GameString {
            char* data;
            char* end;
            char* capacity;
        };
        
        size_t len = ::strlen(texturePath);
        char* buffer = (char*)::malloc(len + 1);
        ::strcpy(buffer, texturePath);
        
        GameString gs;
        gs.data = buffer;
        gs.end = buffer + len;
        gs.capacity = buffer + len + 1;
        
        // Call native sub_634470 at 0x0042E100
        reinterpret_cast<void(__thiscall*)(void*, GameString)>(0x0042E100)(this, gs);
    }
    
    // Direct native calls for virtual functions
    void OnUpdate_Native() {
        reinterpret_cast<void(__thiscall*)(void*)>(0x004456B0)(this);
    }
    
    void OnCIFReady_Native() {
        reinterpret_cast<void(__thiscall*)(void*)>(0x0042E7E0)(this);
    }
    
    void RenderMyself_Native() {
        reinterpret_cast<void(__thiscall*)(void*)>(0x0042DBD0)(this);
    }
    
    void sub_633990_Native() {
        reinterpret_cast<void(__thiscall*)(void*)>(0x0042E2F0)(this);
    }
};
