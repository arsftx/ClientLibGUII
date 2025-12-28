# CIFMap (WorldMap) Reverse Engineering Task

## Status Legend
- `[ ]` Not started
- `[/]` In progress
- `[x]` Completed

---

## Phase 1: Initial Discovery & Class Structure
- [x] Identify CIFWorldMap registration and class info
- [/] Map complete class memory layout (15620 bytes)
- [ ] Document VTable entries
- [/] Identify all member variables with offsets

## Phase 2: Core Functions Analysis
- [x] Analyze `sub_552D50` - Registration function
- [x] Analyze `sub_552EC0` - Constructor
- [/] Analyze `sub_553090` - Destructor
- [x] Analyze `sub_553490` - OnCreate (resinfo\\ifworldmap.txt)
- [x] Analyze `sub_553500` - UpdateMap / Refresh
- [x] Analyze `sub_555530` - Map Index Change (Jangan/Donhwan/Khotan)
- [x] Analyze `sub_5559E0` - Toggle WorldMap/LocalMap
- [x] Analyze `sub_555AD0` - Location Point Loading
- [x] Analyze `sub_553B90` - Main Render function
- [x] Analyze `sub_553980` - Mouse/Input Handler
- [x] Analyze `sub_553D10` - Render Party Members
- [x] Analyze `sub_554770` - Render Other Players

## Phase 3: Coordinate System
- [ ] World → Map coordinate transformation
- [ ] Player position display
- [ ] Region mapping (RegionID ↔ Map tiles)
- [ ] Local map coordinate system (256x256 tiles)

## Phase 4: Marker/Icon System
- [ ] Player position marker
- [ ] NPC markers
- [ ] Teleport points
- [ ] Quest targets
- [ ] Custom markers (user added)

## Phase 5: Texture & Rendering
- [ ] Map tile loading system
- [ ] DDJ texture references
- [ ] Zoom functionality
- [ ] Pan/scroll functionality

## Phase 6: Integration Points
- [ ] IRM (Interface Runtime Manager) registration
- [ ] Message handlers
- [ ] Event callbacks

## Phase 7: Custom Implementation
- [/] Create CIFWorldMap.h header
- [ ] Create CIFWorldMap.cpp implementation
- [ ] Create IDA Python script
- [ ] Custom map rendering hooks
- [ ] Extended functionality design

---

## Phase 8: CustomWorldMap ImGui Class
**Reference**: CustomMinimap.cpp and CustomPlayerMiniInfo.cpp patterns

### Pattern Analysis (Completed)
- [x] Reviewed `CustomMinimap.cpp` - Uses ImGui for window, native DDJ loader for textures
- [x] Reviewed `CustomPlayerMiniInfo.cpp` - Uses **ID3DXSprite** for native DirectX rendering
- [x] Reviewed `NativeBarRenderer.cpp` - Queue-based D3DX sprite render pattern
- [x] Identified key components:
  - `GameString` + `LoadGameTexture` (sub_409E10) for DDJ loading
  - `ID3DXSprite` for native quality DirectX texture rendering
  - `D3DXCreateSprite()` to initialize sprite renderer
  - `m_pSprite->Begin()` / `m_pSprite->Draw()` / `m_pSprite->End()` pattern
  - ImGui provides window/controls, D3DX provides texture rendering

### Key Rendering Functions (from CustomPlayerMiniInfo.cpp)
```cpp
// Initialize sprite renderer
void InitNativeSprite(IDirect3DDevice9* pDevice) {
    D3DXCreateSprite(pDevice, &m_pSprite);
}

// Render full texture
void RenderNativeSprite(IDirect3DTexture9* pTexture, float x, float y, float w, float h, D3DCOLOR color) {
    D3DXVECTOR2 scaling(w / texWidth, h / texHeight);
    D3DXVECTOR2 position(x, y);
    m_pSprite->Draw(pTexture, NULL, &scaling, NULL, 0.0f, &position, color);
}

// Render partial texture (for progress bars)
void RenderNativeSpriteUV(... uvMaxX, ...) {
    RECT srcRect = {0, 0, texWidth * uvMaxX, texHeight};
    m_pSprite->Draw(pTexture, &srcRect, &scaling, NULL, 0.0f, &position, color);
}
```

### Implementation Tasks
- [x] Create `CustomWorldMap.h` header file
- [x] Create `CustomWorldMap.cpp` implementation
- [x] Implement ID3DXSprite initialization and management
- [x] Implement texture loading (worldmap DDJ tiles + icons)
- [x] Implement render callback registration
- [x] Implement map tile rendering (19x7 grid for world map)
- [x] Implement pan/drag functionality
- [/] Implement zoom toggle (world/local)
- [ ] Implement location marker rendering (NPC/Teleport)
- [x] Implement player position marker
- [ ] Implement party member markers
- [ ] Implement click-to-teleport functionality
- [x] Register with CustomGUISession
- [x] Add RenderDebugWindow() - separate debug panel (like MiniInfo Debug)

### Key Textures to Load
```
interface\worldmap\wmap_window_edge_1.ddj
interface\worldmap\wmap_window_edge_2.ddj
interface\worldmap\wmap_window_edge_3.ddj
interface\worldmap\wmap_window_edge_4.ddj
interface\worldmap\wmap_bg.ddj
interface\worldmap\wmap_zoom.ddj
interface\minimap\mm_sign_character.ddj
interface\worldmap\map\map_world_{X}x{Y}.ddj (X: 102-178 step 4, Y: 81-109 step 4)
```

---

## Notes
- CIFWorldMap class size: **15620 bytes** (sub_552D50 → sub_898D80 call)
- Uses ifworldmap.txt for UI layout
- Local maps: Jangan(1), Donhwan(2), Khotan(3)
- Similar architecture to CIFMinimap but more complex
- CustomWorldMap uses ImGui for window + ID3DXSprite for native DirectX texture rendering

