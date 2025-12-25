#include "IRMManager.h"
#include <BSLib/Debug.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <windows.h>

// =============================================================================
// Debug logging to clientlog.txt (same as IFflorian0Guide)
// =============================================================================
static void DebugLog(const char* format, ...)
{
    const char* logPath = "clientlog.txt";
    
    char buffer[2048];
    va_list args;
    va_start(args, format);
    _vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    buffer[sizeof(buffer) - 1] = '\0';
    
    time_t now = time(0);
    struct tm tstruct;
    char timeBuf[80];
    localtime_s(&tstruct, &now);
    strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M:%S", &tstruct);
    
    FILE* fp = fopen(logPath, "a");
    if (fp) {
        fprintf(fp, "[%s] %s\n", timeBuf, buffer);
        fclose(fp);
    }
    
    OutputDebugStringA(buffer);
    OutputDebugStringA("\n");
}

// =============================================================================
// ECSRO IRM ADDRESSES - Verified from ASM Analysis
// =============================================================================
#define ADDR_IRM_LOADFROMFILE           0x0064DF30  // retn 4
#define ADDR_IRM_CREATEINTERFACESECTION 0x0064D870  // retn 10h (16 bytes)
#define ADDR_IRM_GETRESOBJ              0x0064D790  // retn 8

// Helper addresses
#define ADDR_STRING_CTOR                0x00406A70  // std::string constructor

// =============================================================================
// Constructor
// =============================================================================
CIRMManager::CIRMManager() {
    N00009CBC = 0;
    // DebugLog("[IRM] CIRMManager constructed at 0x%08X", (DWORD)this);
}

// =============================================================================
// LoadFromFile
// ASM: sub_64DF30, __thiscall, retn 4 (cleans 4 bytes = 1 param)
// Signature: void LoadFromFile(const char* filename)
// =============================================================================
void CIRMManager::LoadFromFile(const char *filename) {
    // DebugLog("[IRM] LoadFromFile ENTER - File: %s, this: 0x%08X", filename, (DWORD)this);
    
    typedef void (__thiscall *LoadFromFileFn)(CIRMManager*, const char*);
    LoadFromFileFn fn = (LoadFromFileFn)ADDR_IRM_LOADFROMFILE;
    fn(this, filename);
    
    // DebugLog("[IRM] LoadFromFile EXIT - Success");
}

// =============================================================================
// CreateInterfaceSection
// ASM: sub_64D870, __thiscall, retn 10h (cleans 16 bytes = 4 params)
// 
// Stack layout from ASM (CIFCheckBox::OnCreate pattern):
//   push    esi            ; CObj* base (4 bytes)
//   sub     esp, 0Ch       ; Allocate 12 bytes for std::string
//   [construct std::string on stack]
//   call    sub_64D870
//   ; Function does retn 10h - cleans 16 bytes total
//
// CRITICAL: std::string is passed BY VALUE (12 bytes on stack), not by pointer!
// =============================================================================
void CIRMManager::CreateInterfaceSection(const char* section, CObj *base) {
    // DebugLog("[IRM] CreateInterfaceSection ENTER - Section: '%s', Base: 0x%08X, this: 0x%08X", 
    //         section, (DWORD)base, (DWORD)this);
    
    struct GameString {
        char* data;
        char* end;
        char* capacity;
    };
    
    GameString gs;
    memset(&gs, 0, sizeof(gs));
    
    const char* sectionStr = section;
    const char* endPtr = section + strlen(section);
    
    // DebugLog("[IRM] Before sub_406A70: gs at 0x%08X, section='%s', end=0x%08X", 
    //          (DWORD)&gs, sectionStr, (DWORD)endPtr);
    
    // Call sub_406A70 to construct the string
    // sub_406A70(this=GameString*, Src=const char*, a3=endPtr)
    void* gsConstructPtr = &gs;
    __asm {
        mov     ecx, gsConstructPtr   // ECX = this = &GameString
        mov     eax, endPtr
        push    eax                   // a3 = end pointer
        mov     eax, sectionStr
        push    eax                   // Src = section string
        mov     eax, 0x00406A70
        call    eax
        // retn 8 cleans params
    }
    
    // DebugLog("[IRM] After sub_406A70: data=0x%08X, end=0x%08X, cap=0x%08X", 
    //          (DWORD)gs.data, (DWORD)gs.end, (DWORD)gs.capacity);
    
    void* irmThis = this;
    void* basePtr = base;
    void* gsPtr = &gs;
    
    // DEBUG: Check base+0x64 (disabled)
    // DWORD* baseOffset64 = (DWORD*)((BYTE*)base + 0x64);
    // DebugLog("[IRM] Base+0x64 value: 0x%08X", *baseOffset64);
    
    // DebugLog("[IRM] Calling CreateInterfaceSection with 12-byte GameString...");
    
    __asm {
        mov     eax, basePtr
        push    eax
        
        mov     eax, gsPtr
        push    dword ptr [eax+8]
        push    dword ptr [eax+4]
        push    dword ptr [eax]
        
        mov     ecx, irmThis
        mov     eax, 0x0064D870
        call    eax
    }
    
    // DebugLog("[IRM] CreateInterfaceSection EXIT - Success");
}

// Overload that takes std::string - converts to const char* and calls main version
void CIRMManager::CreateInterfaceSection(std::string section, CObj *base) {
    CreateInterfaceSection(section.c_str(), base);
}

// Overload with size parameter - calls main version (size ignored for now)
void CIRMManager::CreateInterfaceSection(std::string section, CObj *base, size_t siz) {
    CreateInterfaceSection(section.c_str(), base);
}

// =============================================================================
// GetResObj
// ASM: sub_64D790, __thiscall, retn 8 (cleans 8 bytes = 2 params)
// Signature: CIFWnd* GetResObj(int id, int a2)
//
// a2 usage: if (a2 != 0) id += this[7] (offset adjustment)
// =============================================================================
CIFWnd *CIRMManager::GetResObj(int id, int a2) {
    // Note: Debug logs removed - this function is called very frequently and spams logs
    
    // Direct call - matches exactly: __thiscall with 2 params, retn 8
    typedef CIFWnd* (__thiscall *GetResObjFn)(CIRMManager*, int, int);
    GetResObjFn fn = (GetResObjFn)ADDR_IRM_GETRESOBJ;
    CIFWnd* result = fn(this, id, a2);
    
    return result;
}

// =============================================================================
// DeleteCreatedSection
// TODO: Find correct ECSRO address from ASM
// VSRO address was: 0x008b59f0
// =============================================================================
void CIRMManager::DeleteCreatedSection(std::string section) {
    DebugLog("[IRM] DeleteCreatedSection - Section: '%s' (NOT IMPLEMENTED)", section.c_str());
    // TODO: Need to find ECSRO address
    // reinterpret_cast<void(__thiscall *)(CIRMManager *, std::string)>(0x????????)(this, section);
}
