#pragma once
#include <windows.h>
#include <d3d9.h>

// CBufRender - Renders 3D content to a texture buffer
// From IDA analysis: Size 0x28 (40 bytes), VTable at 0x94F268
// VS2005 compatible version
class CBufRender
{
public:
    // Native function addresses (using enum for VS2005 compatibility)
    enum {
        ADDR_CONSTRUCTOR = 0x66A400,
        ADDR_RESIZE = 0x66A460,
        ADDR_BEGIN_RENDER = 0x66A570,
        ADDR_END_RENDER = 0x66A690,
        ADDR_RELEASE = 0x66A450
    };

    // Native function typedefs
    typedef void* (__thiscall *ConstructorFn)(void* thisPtr);
    typedef int (__thiscall *ResizeFn)(void* thisPtr, int width, int height);
    typedef int (__thiscall *BeginRenderFn)(void* thisPtr, int param);
    typedef int (__thiscall *EndRenderFn)(void* thisPtr);
    typedef int (__thiscall *ReleaseFn)(void* thisPtr);

    // Create a new BufRender instance using native constructor
    static CBufRender* Create() {
        void* memory = operator new(0x28);  // Native size
        if (!memory) return NULL;
        
        ConstructorFn pConstructor = (ConstructorFn)ADDR_CONSTRUCTOR;
        pConstructor(memory);
        return (CBufRender*)memory;
    }

    // Resize/create the render target
    void Resize(int width, int height) {
        ResizeFn pResize = (ResizeFn)ADDR_RESIZE;
        pResize(this, width, height);
    }

    // Begin rendering to this buffer
    bool BeginRender(int param) {
        BeginRenderFn pBegin = (BeginRenderFn)ADDR_BEGIN_RENDER;
        return pBegin(this, param) != 0;
    }

    // End rendering
    void EndRender() {
        EndRenderFn pEnd = (EndRenderFn)ADDR_END_RENDER;
        pEnd(this);
    }

    // Release resources
    void Release() {
        ReleaseFn pRelease = (ReleaseFn)ADDR_RELEASE;
        pRelease(this);
    }

    // Get the D3D texture for ImGui
    IDirect3DTexture9* GetTexture() const {
        // Texture is at offset +4 (this[1])
        return (IDirect3DTexture9*)(*(DWORD*)((DWORD)this + 4));
    }

    // Check if currently rendering
    bool IsRendering() const {
        return (*(DWORD*)((DWORD)this + 0x18)) != 0;  // this[6]
    }

private:
    // Structure layout from IDA:
    // +0x00: vtable
    // +0x04: D3D texture (IDirect3DTexture9*)
    // +0x08: D3D surface
    // +0x0C: Depth stencil
    // +0x10: unknown
    // +0x14: unknown  
    // +0x18: rendering flag
    // +0x1C: old rendering flag
    // +0x20: saved state 1
    // +0x24: saved state 2
    
    char _data[0x28];  // 40 bytes total
};