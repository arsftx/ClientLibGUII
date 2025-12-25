#pragma once

#include <cstring>
#include <cstdlib>

// CTextBoard_ECSRO - ECSRO-specific layout
// Based on IDA analysis of constructor at 0x00446760
// Size: 0x00C0 (192 bytes)
// VTable: 0x0093E170

class CTextBoard_ECSRO {
public:
    // VTable at offset 0x00 (4 bytes)
    
    // Members from IDA constructor analysis (0x00446760)
    char pad_0004[0x0C - 0x04];       // 0x0004 - padding
    
    // CGFontTexture equivalent starts at 0x0C
    char m_FontTextureData[0x58];      // 0x000C - 88 bytes (0x64 - 0x0C = 0x58)
    
    // 12-byte game strings at IDA-confirmed offsets
    char GameString1[12];              // 0x0064
    
    int N00009770;                     // 0x0070
    int N00009774;                     // 0x0074
    void* m_pFontData;                 // 0x0078
    int N0000977C;                     // 0x007C
    
    char pad_0080[9];                  // 0x0080
    char N00009789;                    // 0x0089
    char pad_008A[2];                  // 0x008A
    
    float N0000978C;                   // 0x008C
    float N00009790;                   // 0x0090
    int N00009794;                     // 0x0094
    int N00009798;                     // 0x0098
    
    char GameString2[12];              // 0x009C
    int N000097A8;                     // 0x00A8
    char GameString3[12];              // 0x00AC
    int N000097B8;                     // 0x00B8
    char N000097BC;                    // 0x00BC
    char pad_00BD[3];                  // 0x00BD

public:
    CTextBoard_ECSRO() {
        reinterpret_cast<void(__thiscall*)(CTextBoard_ECSRO*)>(0x00446760)(this);
    }
    
    virtual ~CTextBoard_ECSRO() {
        reinterpret_cast<void(__thiscall*)(CTextBoard_ECSRO*)>(0x00446920)(this);
    }
    
    virtual void TB_Func_13(const char* texturePath, int a3, int a4) {
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
        
        reinterpret_cast<void(__thiscall*)(CTextBoard_ECSRO*, GameString, int, int)>(0x00447210)(this, gs, a3, a4);
    }
    
}; // Size: 0x00C0 (192 bytes)
