#pragma once

#include <cstring>
#include <cstdlib>
#include <windows.h>

// Pure ECSRO-based CIFStatic implementation
// NO virtual functions - avoids vtable conflict with game
// Size: 0x02C8 (from IDA analysis)

class CIFStatic_ECSRO {
public:
    // Full class memory - matches game exactly
    // Note: NO virtual functions in this class!
    char data[0x2C8];

public:
    // Non-virtual constructor
    CIFStatic_ECSRO() {
        memset(data, 0, sizeof(data));
        // Call native CIFStatic constructor - it will set up everything including vtable
        reinterpret_cast<void(__thiscall*)(void*)>(0x00441340)(this);
    }
    
    // Non-virtual destructor
    ~CIFStatic_ECSRO() {
        // Native destructor would be called if needed
    }
    
    // --- Native function wrappers (non-virtual) ---
    
    void ShowGWnd(bool bVisible) {
        reinterpret_cast<void(__thiscall*)(void*, bool)>(0x00446330)(this, bVisible);
    }
    
    void BringToFront() {
        reinterpret_cast<void(__thiscall*)(void*)>(0x0089F190)(this);
    }
    
    void SetGWndSize(int width, int height) {
        reinterpret_cast<void(__thiscall*)(void*, int, int)>(0x00531af0)(this, width, height);
    }
    
    void TB_Func_12(const char* texturePath, int a3, int a4) {
        // Create 12-byte GameString for native call
        struct GameString {
            char* data_ptr;
            char* end_ptr;
            char* capacity_ptr;
        };
        
        GameString gs;
        if (texturePath == NULL || texturePath[0] == '\0') {
            gs.data_ptr = NULL;
            gs.end_ptr = NULL;
            gs.capacity_ptr = NULL;
        } else {
            size_t len = strlen(texturePath);
            char* buffer = (char*)malloc(len + 1);
            strcpy(buffer, texturePath);
            gs.data_ptr = buffer;
            gs.end_ptr = buffer + len;
            gs.capacity_ptr = buffer + len + 1;
        }
        
        // Call native TB_Func_13 at 0x00447210
        reinterpret_cast<void(__thiscall*)(void*, GameString, int, int)>(0x00447210)(this, gs, a3, a4);
    }
    
    // Native OnCreate
    bool OnCreate_Native(long ln) {
        // CIFStatic::OnCreate just returns true
        return true;
    }
    
    // Native RenderMyself (CIFWnd's - ECSRO)
    void RenderMyself_Native() {
        reinterpret_cast<void(__thiscall*)(void*)>(0x004456D0)(this);
    }
    
    // Native OnUpdate (CIFWnd's - ECSRO)
    void OnUpdate_Native() {
        reinterpret_cast<void(__thiscall*)(void*)>(0x004456B0)(this);
    }
    
    // Native OnCIFReady (CIFWnd's - ECSRO)
    void OnCIFReady_Native() {
        reinterpret_cast<void(__thiscall*)(void*)>(0x00445A70)(this);
    }

}; // Size: 0x02C8
