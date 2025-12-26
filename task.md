# CIFMinimap Reverse Engineering & Custom ImGui Minimap Tasks

## Genel Durum: 🔄 Devam Ediyor (Temel Entegrasyon Tamamlandı)

---

## Phase 1: Analysis (Planning) ✅ TAMAMLANDI
- [x] Find CIFMinimap in functions_index.h
- [x] Analyze constructor (sub_5397E0)
- [x] Analyze OnCreate (sub_539AA0) - texture loading, UI creation
- [x] Analyze UpdateMap (sub_53A5A0) - coordinate calculation
- [x] Document member offsets (692-868 arası)
- [x] Document texture paths (DDJ files)
- [x] Create implementation plan
- [x] Identify native function addresses

---

## Phase 2: Native Class Creation ✅ TAMAMLANDI
- [x] Create CIFMinimap.h with member offsets
  - Texture pointers (+692 to +780)
  - Player position cache (+788 to +800)
  - UI element pointers (+832 to +852)
  - Map coordinates (+856 to +868)
- [x] Create CIFMinimap.cpp with native wrappers
  - GetInstance() - IRM GDR_MINIMAP = 15
  - GetRegionName() - TextStringManager lookup
- [x] Implement coordinate getters
  - GetCurrentRegionX/Y()
  - GetPlayerPosX/Z()
  - CalculateDisplayX/Y() - native formula
  - GetRegionID() - combined XY value
- [ ] Create IDA Python script (apply_CIFMinimap.py)

---

## Phase 3: ImGui Integration ✅ TAMAMLANDI
- [x] Create CustomMinimap.h/cpp
- [x] Fix std::string constructor calling convention
  - Native uses __thiscall, not __cdecl
  - Implemented with inline assembly
- [x] Fix DebugLog crash
  - fopen/fprintf crashes during DLL_PROCESS_ATTACH
  - Changed to OutputDebugStringA
- [x] Implement coordinate reading from native CIFMinimap
  - Uses CIFMinimap::GetInstance()
  - Reads region and position data
- [x] Implement display coordinate calculation
  - Formula: ((3 * region - offset) << 6) - (int)(pos * 10.0f)
- [x] Implement region name display
  - Uses TextStringManager lookup
  - sprintf("%d", regionID) -> TSM->GetString3()
- [x] Draw player marker (green triangle)
- [x] Draw coordinate text (X/Y + region name)
- [x] Draw minimap background (placeholder)

---

## Phase 4: Texture Loading 🔄 DEV'AM EDİYOR
- [x] Identify DDJ loader function (0x409E10)
- [x] Implement LoadDDJTexture helper
- [ ] Load minimap marker textures
  - mm_sign_character.ddj
  - mm_sign_npc.ddj
  - mm_sign_monster.ddj
  - mm_sign_partymember.ddj
- [ ] Load minimap tile textures
  - minimap\\%dx%d.ddj format
- [ ] Render actual map tiles

---

## Phase 5: Entity Markers ❌ BAŞLANMADI
- [ ] Implement entity iteration
  - Get nearby NPCs
  - Get nearby monsters
  - Get nearby players
- [ ] Draw NPC markers
- [ ] Draw monster markers
- [ ] Draw party member markers
- [ ] Draw quest NPC markers

---

## Phase 6: Final Integration ❌ BAŞLANMADI
- [ ] Hide native CIFMinimap
  - CIFMinimap::ShowGWnd(false)
- [ ] Position sync with native
- [ ] Zoom in/out functionality
- [ ] Map panning/dragging
- [ ] Full native replacement

---

## Düzeltilen Buglar

| Bug | Çözüm | Tarih |
|-----|-------|-------|
| std::string crash | __thiscall convention (inline asm) | 2025-12-26 |
| DebugLog crash | fopen -> OutputDebugStringA | 2025-12-26 |
| m_bInitialized false | Lazy init in Render() | 2025-12-26 |
| Wrong coordinates | Player ptr reading + __ftol formula | 2025-12-26 |
| TSM wrong address | 0x00A00F8C (ASM: unk_A00F8C) | 2025-12-26 |
| GetString3 no deref | *result dereference added | 2025-12-26 |
| Wrong region name | TSM GetString3() with deref | 2025-12-26 |

---

## Dosya Listesi

### Native Wrapper (ClientLib)
| Dosya | Durum | Açıklama |
|-------|-------|----------|
| CIFMinimap.h | ✅ | Class definition, member offsets |
| CIFMinimap.cpp | ✅ | GetInstance(), GetRegionName() |

### ImGui Overlay (DevKit_DLL)
| Dosya | Durum | Açıklama |
|-------|-------|----------|
| CustomMinimap.h | ✅ | ImGui minimap class |
| CustomMinimap.cpp | ✅ | Render, coordinate display |

---

## Native Fonksiyon Referansları

```cpp
// CIFMinimap
#define ADDR_CIFMINIMAP_CTOR     0x005397E0
#define ADDR_CIFMINIMAP_ONCREATE 0x00539AA0
#define ADDR_CIFMINIMAP_UPDATE   0x0053A5A0
#define ADDR_CIFMINIMAP_DTOR     0x00539940

// Texture & String
#define ADDR_LOAD_DDJ_TEXTURE    0x00409E10
#define ADDR_STD_STRING_CTOR     0x00406190  // __thiscall!
#define ADDR_TSM_GETSTRING       0x005FCE10

// Global Pointers
#define ADDR_PLAYER_PTR          0x00A0465C  // Player object pointer
#define ADDR_TSM_INSTANCE        0x00A00F8C  // CTextStringManager instance

// Window
#define ADDR_CGWND_MOVEGWND      0x0089F230
```

---

## ASM Analizi (sub_53A5A0)

### Koordinat Hesaplama
```asm
; X Koordinat
fmul    ds:flt_94AE04           ; posX * 10.0f
call    __ftol                  ; simple truncation
lea     ecx, [ebx+ebx*2-195h]   ; regionX*3 - 405 (0x195)
shl     ecx, 6                  ; << 6
sub     ecx, eax                ; - scaled_pos

; Y Koordinat  
lea     ecx, [ecx+ecx*2-114h]   ; regionY*3 - 276 (0x114)
```

### TSM Çağrısı
```asm
mov     ecx, offset unk_A00F8C  ; ECX = TSM instance
push    ebp                     ; Src (string)
call    sub_5FCE10              ; GetString
mov     eax, [eax]              ; DEREFERENCE result!
```

---

## Sonraki Adımlar

1. **Mob/NPC marker'ları** - Entity listesinden okuma ve çizim
2. **Map tile rendering** - DDJ texture yükleme ve composite çizim
3. **Native minimap gizleme** - ShowGWnd(false) çağrısı
4. **Zoom fonksiyonları** - Kullanıcı kontrolü
