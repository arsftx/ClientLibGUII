# PNG Texture-based PlayerMiniInfo UI

## Overview
Redesign CustomPlayerMiniInfo to use PNG textures from `Media.pk2/newui/playerminiinfo/` folder instead of ImGui-drawn elements.

## Textures to Load
| File | Location in pk2 | Purpose |
|------|----------------|---------|
| `mainbackrgound.png` | `newui\playerminiinfo\` | Main frame/background |
| `HpBar_Health.png` | `newui\playerminiinfo\` | HP bar fill |
| `HpBar_Mana.png` | `newui\playerminiinfo\` | MP bar fill |
| `hpbar_hwan.png` | `newui\playerminiinfo\` | Hwan bar fill |

## UI Layout

```
┌──────────────────────────────────────────────────────────────┐
│  ┌─────┐    [LEVEL]  [CHARACTER NAME]                        │
│  │     │    ═════════════════════════════  (HP Bar)          │
│  │ PRT │    ═════════════════════════════  (MP Bar)          │
│  │     │    ═══════════════════════════    (Hwan Bar)        │
│  └─────┘                                                     │
└──────────────────────────────────────────────────────────────┘
```

- Portrait: Existing character portrait in circular frame
- Level: Number-only display in small circle (blue marked area)
- Name: Character name text in top bar (red marked area)
- HP/MP/Hwan: Fill bars clipped based on percentage

---

## Proposed Changes

### Texture Loading System

#### [MODIFY] [CustomPlayerMiniInfo.h](file:///C:/Users/FuatAras/Desktop/Server/ClientLibGUI/source/DevKit_DLL/src/imgui_windows/CustomPlayerMiniInfo.h)
- Add texture pointer members: `m_texBackground`, `m_texHpFill`, `m_texMpFill`, `m_texHwanFill`
- Add texture dimensions for proper sizing
- Add `LoadTextures()` and `ReleaseTextures()` methods

#### [MODIFY] [CustomPlayerMiniInfo.cpp](file:///C:/Users/FuatAras/Desktop/Server/ClientLibGUI/source/DevKit_DLL/src/imgui_windows/CustomPlayerMiniInfo.cpp)
- Implement `LoadTextures()` using native `LoadGameTexture` (0x409E10)
- Implement D3D device lost/reset handling for textures
- Modify `Render()` to use `ImGui::Image()` with texture coordinates
- Calculate HP/MP bar UV coordinates for percentage-based fill
- Position level number and character name correctly on frame

---

## Verification Plan

### Manual Testing
1. Build the project
2. Launch game client
3. Log in with a character
4. Verify:
   - Background frame PNG is visible
   - HP bar fills correctly based on HP percentage
   - MP bar fills correctly based on MP percentage
   - Hwan bar displays correctly
   - Character portrait renders in circular area
   - Level number appears in correct position
   - Character name appears in top bar area
   - Taking damage updates HP bar in real-time
   - Using skills updates MP bar in real-time
5. Test D3D device reset (change resolution) - textures should reload

### Edge Cases
- Character with 0 Hwan - hide Hwan bar?
- Very long character names - truncate/scale?
