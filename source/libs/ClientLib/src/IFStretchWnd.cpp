#include "IFStretchWnd.h"
#include <cstring>

// Link to native runtime class at 0x9FFF14
GFX_IMPLEMENT_DYNAMIC_EXISTING(CIFStretchWnd, 0x9FFF14)

// ECSRO: sub_56A1B0 - Takes DDJ prefix and loads all 8 textures (4 corners + 4 edges)
// The prefix is something like "interface\\ifcommon\\lattice_window\\com_lattice_outline_"
// It appends: left_down.ddj, right_down.ddj, right_up.ddj, left_up.ddj (corners)
//             left_side.ddj, right_side.ddj (edges)
void CIFStretchWnd::LoadTexturesFromPrefix(const char* ddjPrefix) {
    if (!ddjPrefix || ddjPrefix[0] == '\0') return;
    
    // Build GameString struct for ECSRO (12 bytes: data, end, capacity)
    struct GameString {
        char* data;
        char* end;
        char* capacity;
    };
    
    size_t len = strlen(ddjPrefix);
    char* buffer = (char*)malloc(len + 1);
    strcpy(buffer, ddjPrefix);
    
    GameString gs;
    gs.data = buffer;
    gs.end = buffer + len;
    gs.capacity = buffer + len + 1;
    
    // Call native ECSRO function sub_56A1B0
    // This function takes the prefix and loads all 8 textures
    // GameString struct is passed by value on stack (no extra parameter!)
    reinterpret_cast<void (__thiscall *)(CIFStretchWnd *, GameString)>(0x0056A1B0)(this, gs);
}
