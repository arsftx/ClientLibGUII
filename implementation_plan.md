# CIFMinimap Reverse Engineering & Custom ImGui Minimap

## Proje Durumu: ✅ Temel Entegrasyon Tamamlandı

---

## Amaç
Orijinal CIFMinimap sınıfını reverse edip, harita verilerini ImGui minimap'e aktarmak.

---

## CIFMinimap Sınıf Yapısı

| Özellik | Değer |
|---------|-------|
| **Size** | 880 bytes (0x370) |
| **VTable** | 0x94AD48 |
| **Inner VTable** | 0x94AD00 (offset +108) |
| **Constructor** | sub_5397E0 (0x5397E0) |
| **OnCreate** | sub_539AA0 (0x539AA0) |
| **Destructor** | sub_539940 (0x539940) |
| **UpdateMap** | sub_53A5A0 (0x53A5A0) |
| **Parent** | CIFWnd |
| **IRM ID** | GDR_MINIMAP = 15 |

---

## Önemli Member Offset'ler

```cpp
// === Texture Pointers (DDJ Loaded) ===
+692  (0x02B4) = m_pTexSignPartyArrow     // mm_sign_partyarrow.ddj
+700  (0x02BC) = m_pTexSignCharacter      // mm_sign_character.ddj
+740  (0x02E4) = m_pTexNPCSign            // mm_sign_npc.ddj
+748  (0x02EC) = m_pTexMonsterSign        // mm_sign_monster.ddj
+752  (0x02F0) = m_pTexPlayerSign         // mm_sign_player.ddj
+756  (0x02F4) = m_pTexPickedItemSign     // mm_sign_picked_item.ddj
+760  (0x02F8) = m_pTexTargetSign         // mm_sign_target.ddj
+768  (0x0300) = m_pTexPartyMemberSign    // mm_sign_partymember.ddj
+772  (0x0304) = m_pTexGuildSign          // mm_sign_guild.ddj
+776  (0x0308) = m_pTexQuestNPCSign       // mm_sign_quest_npc.ddj
+780  (0x030C) = m_pTexWorldMapSign       // wmap_marker.ddj

// === Linked List ===
+784  (0x0310) = m_pMinimapList           // Linked list head for markers

// === Player Position Cache ===
+788  (0x0314) = m_fPlayerPosX            // Player X position within region (float)
+792  (0x0318) = m_fPlayerPosY            // Player Y/Height (float)
+796  (0x031C) = m_fPlayerPosZ            // Player Z position within region (float)
+800  (0x0320) = m_fPlayerRotation        // Player rotation/heading (float)

// === UI Element Pointers ===
+832  (0x0340) = m_pBtnZoomIn             // Button ID 5
+836  (0x0344) = m_pBtnZoomOut            // Button ID 6
+840  (0x0348) = m_pBtnOptions            // Button ID 8
+844  (0x034C) = m_pBtnMoveUp             // Button ID 2 - X coord text
+848  (0x0350) = m_pBtnMoveDown           // Button ID 3 - Y coord text
+852  (0x0354) = m_pBtnMoveLeft           // Button ID 4 - Region text

// === Map Coordinates ===
+856  (0x0358) = m_nCurrentRegionX        // Current region X (1-255)
+860  (0x035C) = m_nCurrentRegionY        // Current region Y (1-255)
+864  (0x0360) = m_nPrevRegionX           // Previous region X (change detection)
+868  (0x0364) = m_nPrevRegionY           // Previous region Y (change detection)
```

---

## Koordinat Hesaplama (ASM Analizi - sub_53A5A0)

```cpp
// C++ Formülü (ASM'den türetilmiş)
DisplayX = ((regionX * 3 - 405) << 6) - (int)(playerPosX * 10.0f)
DisplayY = ((regionY * 3 - 276) << 6) - (int)(playerPosZ * 10.0f)
```

### IDA ASM (0x53A7B8-0x53A82A)
```asm
; X Koordinat
fmul    ds:flt_94AE04           ; posX * 10.0f (flt_94AE04 = 10.0f)
call    __ftol                  ; truncate to int (simple, not complex cast)
lea     ecx, [ebx+ebx*2-195h]   ; regionX*3 - 0x195 (405)
shl     ecx, 6                  ; << 6
sub     ecx, eax                ; - scaled_posX

; Y Koordinat
fld     dword ptr [esi+31Ch]    ; load posZ from CIFMinimap+0x31C
fmul    ds:flt_94AE04           ; posZ * 10.0f
call    __ftol                  ; truncate to int
lea     ecx, [ecx+ecx*2-114h]   ; regionY*3 - 0x114 (276)
shl     ecx, 6                  ; << 6
sub     ecx, eax                ; - scaled_posZ
```

---

## Region Adı Çevirme (ASM Analizi)

### C++ Kullanımı
```cpp
// g_CTextStringManager adresi: 0x00A00F8C
// sub_5FCE10 pointer-to-pointer döner, dereference gerekli!

char buffer[16];
sprintf(buffer, "%d", regionID);  // Örn: "24680"
const char** pResult = g_CTextStringManager->GetString3Raw(buffer);
const char* regionName = *pResult;  // DEREFERENCE!
```

### IDA ASM (0x53A866-0x53A870)
```asm
push    ecx                     ; Src (regionID string "%d")
mov     ecx, offset unk_A00F8C  ; ECX = CTextStringManager instance
call    sub_5FCE10              ; GetString
mov     eax, [eax]              ; DEREFERENCE - eax now points to actual string
```

> [!IMPORTANT]
> `sub_5FCE10` fonksiyonu `_DWORD*` (pointer-to-pointer) döner!
> Sonuç `[eax]` ile dereference edilmeli.

---

## Texture Paths (Media.pk2)

```
interface\minimap\mm_sign_partyarrow.ddj
interface\minimap\mm_sign_character.ddj
interface\minimap\mm_sign_npc.ddj
interface\minimap\mm_sign_monster.ddj
interface\minimap\mm_sign_player.ddj
interface\minimap\mm_sign_picked_item.ddj
interface\minimap\mm_sign_target.ddj
interface\minimap\mm_sign_guild.ddj
interface\minimap\mm_sign_partymember.ddj
interface\minimap\mm_sign_quest_npc.ddj
interface\worldmap\wmap_marker.ddj
```

---

## Minimap Tile Loading (sub_53A5A0)

```cpp
// Dungeon/özel harita için:
sprintf(Src, "minimap_d\\%s\\%s_%dx%d.ddj", regionName, mapName, regionX + offset, regionY - offset);

// Normal harita için:
sprintf(Src, "minimap\\%dx%d.ddj", regionX + offset, regionY - offset);
```

---

## Oluşturulan Dosyalar

### CIFMinimap (Native Wrapper)
- ✅ `libs/ClientLib/src/CIFMinimap.h` - Class tanımı ve member offset'ler
- ✅ `libs/ClientLib/src/CIFMinimap.cpp` - GetInstance() ve GetRegionName() implementasyonu

### CustomMinimap (ImGui Overlay)
- ✅ `DevKit_DLL/src/imgui_windows/CustomMinimap.h` - ImGui minimap class
- ✅ `DevKit_DLL/src/imgui_windows/CustomMinimap.cpp` - Render implementasyonu

---

## Native Fonksiyon Adresleri

| Fonksiyon | Adres | Açıklama |
|-----------|-------|----------|
| CIFMinimap Constructor | 0x5397E0 | Sınıf constructor'ı |
| CIFMinimap OnCreate | 0x539AA0 | Texture yükleme, UI oluşturma |
| CIFMinimap UpdateMap | 0x53A5A0 | Harita güncelleme, koordinat hesaplama |
| LoadDDJTexture | 0x409E10 | DDJ texture yükleyici |
| std::string Constructor | 0x406190 | __thiscall calling convention |
| CGWnd::MoveGWnd | 0x89F230 | Pencere taşıma |
| TSM GetString | 0x5FCE10 | TextStringManager lookup |

---

## Kalan İşler

### Öncelikli
- [ ] Mob/NPC/Monster marker'ları çizme
- [ ] Harita tile texture'larını yükleme ve çizme
- [ ] Native CIFMinimap'i gizleme

### İkincil
- [ ] Party member marker'ları
- [ ] Quest NPC özel marker'ları
- [ ] Zoom in/out fonksiyonları
- [ ] Harita sürükleme (pan)

### Uzun Vadeli
- [ ] IDA Python script oluşturma
- [ ] Tam minimap replika (native yerine)

---

## Notlar

- Minimap 192x192 pixel boyutunda
- Tile'lar `minimap_d` veya `minimap` klasöründe
- Zoom: 6x6 veya 8x8 tile grid (ButtonID 5/6 ile kontrol)
- Region ID: 16-bit değer (Y << 8 | X)
- std::string constructor __thiscall kullanır, __cdecl değil!
- DLL_PROCESS_ATTACH sırasında fopen/fprintf crash yapabilir - OutputDebugStringA kullan
