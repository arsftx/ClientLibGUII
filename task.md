# 🕵️‍♂️ Reverse Engineering & Analysis Log

## 🧩 IDA / Assembly Bulguları (Canlı)

### sub_616790 - GetSkillObjectByID
- **Adres:** `0x616790`
- **Tahmini İşlev:** Skill Manager'dan Skill ID ile SkillObject pointer döner
- **Kritik Registerlar:** `ECX` = unk_A01010 (Skill Manager), `arg0` = dwSkillID
- **Pseudocode Özeti:** `SkillManager->GetSkillObject(dwSkillID)` → `SkillObject*`
- **Notlar:** Skill icon yüklemek için ilk adım

### sub_601900 - GetSkillData
- **Adres:** `0x601900`
- **Tahmini İşlev:** SkillObject'ten SkillData adresini döner
- **Assembly:** `lea eax, [ecx+10Ch]; retn`
- **Kritik Registerlar:** `ECX` = SkillObject, `EAX` = SkillData (inline at +0x10C)
- **Pseudocode Özeti:** `return this + 0x10C;` (pointer değil, inline structure)
- **Notlar:** SkillData SkillObject içinde inline olarak saklanıyor

### sub_44DB10 - CIFSkillSlot::SetSkill
- **Adres:** `0x44DB10`
- **Tahmini İşlev:** CIFSkillSlot'a skill ID set eder ve icon yükler
- **Kritik Offsetler:**
  - `+0x2DC` = skill type
  - `+0x2E0` = skill ID
  - `+0x314` = SkillObject pointer
- **DDJ Path:** `SkillData + 0xD4` → std::string (skill icon DDJ)
- **Notlar:** IRM resource sistemi gerektirir, direkt CreateInstance ile çalışmaz

### sub_44DA40 - CIFSkillSlot::OnCreate
- **Adres:** `0x44DA40`
- **Tahmini İşlev:** CIFSkillSlot'u initialize eder
- **IRM Resource:** `resinfo\\ifskill_slot.txt` yükler
- **Child Elements:** `+0x2F4` ve `+0x2F0` offset'lerine child element oluşturur
- **Notlar:** IRM olmadan çağrılırsa crash verir

### sub_5425A0 - SetSlotType
- **Adres:** `0x5425A0`
- **Tahmini İşlev:** Slot tipini ayarlar
- **Assembly:**
  ```asm
  mov eax, [esp+arg_0]   ; arg = slot type
  mov [ecx+2B4h], eax    ; this+0x2B4 = type
  retn 4
  ```
- **Kritik Offset:** `+0x2B4` = Slot Type (DWORD)
- **Pseudocode:** `this->m_slotType = type;`
- **Notlar:** 
  - Validation yok, herhangi bir değer kabul ediliyor
  - Drag davranışı burada DEĞİL - type'ın okunduğu yerde
  - **SONRAKİ ADIM:** `+0x2B4`'ün nerede okunduğunu bul (xref)

### CIFSlot VTable (0x93D9C8) - Analiz
| Offset | Adres | Fonksiyon | Tahmini İşlev |
|--------|-------|-----------|---------------|
| +0x00 | sub_4411E0 | Destructor? |
| +0x04 | sub_4411F0 | ? |
| +0x40 | sub_444FC0 | ? |
| +0x44 | sub_444EB0 | ? |
| +0xA8 | sub_4451B0 | **OnLButtonDown?** |
| +0xAC | sub_4452F0 | **OnMouseMove?** |
| +0xB0 | sub_445650 | **OnLButtonUp?** |
| +0xB8 | sub_4458B0 | ? |

**📌 Araştır:** `sub_4451B0` veya `sub_4452F0`'da `+0x2B4` okunuyor mu?

### Drag String'leri (Bulunan)
| Adres | String | Notlar |
|-------|--------|--------|
| 0x9AEFE4 | `CIFDragableArea` | RuntimeClass name |
| 0x9AA230 | `CIFDraggedStatic` | Dragged icon class? |
| 0x9AD2A8 | `DraggedItemIndex` | Item tracking |
| 0x9AD2C8 | `DraggedPage` | Page tracking |
| 0x9AD2BC | `DraggedTab` | Tab tracking |

**📌 Sonraki:** `CIFDragableArea` (0x9AEFE4) string xref'ine bak → RuntimeClass struct'ı bul

### 🔥 CIFDragableArea RuntimeClass Analizi
- **RuntimeClass Adresi:** `0x9FE9C4`
- **Class Boyutu:** `0x2C4` (708 bytes)
- **Base Class RuntimeClass:** `0x9FE5C0` (muhtemelen CIFStatic)
- **Registration Fonksiyonu:** `sub_477FB0`
- **CreateInstance:** `sub_477FE0`
- **Constructor:** `sub_478050`

### 🔥 CIFDragableArea VTable Adresleri
- **Ana VTable:** `0x940728` ← **Drop handler'lar burada!**
- **İkincil VTable:** `0x9406E0` (offset +0x6C)
- **Base Constructor:** `sub_4449C0`
- **Init Fonksiyonu:** `sub_4465E0`

### CIFDragableArea VTable (0x940728)
| Offset | Fonksiyon | Rolü |
|--------|-----------|------|
| +0x40 | `sub_478200` | **Windows Message Handler** |
| +0x70 | `sub_4781D0` | **OnLButtonUp = DROP!** 🔴 |
| +0x74 | `sub_478150` | **OnLButtonDown = DRAG START** |
| +0xA8 | `sub_4451B0` | Hit-test |
| +0xB0 | `sub_445650` | Mouse handler |

### sub_478200 - Message Dispatcher Analizi
```c
switch (messageType) {
    case 0x200:  // WM_MOUSEMOVE → parent'a ilet
    case 0x201:  // WM_LBUTTONDOWN → sub_478150
    case 0x202:  // WM_LBUTTONUP → sub_4781D0 ← DROP!
}
```
- `this[173]` = offset `+0x2B4` (drag offset X)
- `this[174]` = offset `+0x2B8` (drag offset Y)

### ❌ sub_4781D0 (OnLButtonUp) - Drop Logic YOK!
```c
void OnLButtonUp(this) {
    sub_89F1C0();   // Global
    sub_89C840();   // Reset
    sub_4465E0(0);  // Init
    return 1;       // Sadece cleanup!
}
```
**Sonuç:** CIFDragableArea sadece **pencere sürükleme alanı** - slot drop logic burada DEĞİL!

**📌 Yeni Yön:** CIFSlot veya CIFSlotWithHelp'in drop handler'ına bakmalıyız

### DraggedItemIndex Xref'ler (Drag Logic Fonksiyonları)
| Adres | Fonksiyon | Notlar |
|-------|-----------|--------|
| sub_458C50+B7 | **Öncelikli** - İlk drag handler? |
| sub_458E90+92 | Drag handler 2 |
| sub_459140+CE | Drag handler 3 (iki çağrı) |
| sub_45A200+71 | Drag handler |
| sub_509050+F9 | CIFSlot ile ilgili? (50xxxx range) |

**📌 Sonraki:** `sub_458C50` fonksiyonunu analiz et - slot type kontrolü burada olabilir

### 🔥 sub_458C50 - Drag Handler Analizi (KRİTİK BULGU!)
- **Adres:** `0x458C50`
- **arg_4 (a3):** Slot Type değeri
- **Pseudocode:**
  ```c
  if (a3 == 0x46) {  // Inventory type
      // Özel drag işlemi - DraggedItemIndex set edilir
      sub_435A90("DraggedItemIndex", slotIndex);
      // ... karmaşık inventory drag logic
  } else {
      // QuickSlot (0x0C) ve diğer tipler
      return sub_4573A0(this);  // ← BU FONKSİYON SİLME YAPIYOR OLABİLİR
  }
  ```
- **📌 SONRAKİ ADIM:** `sub_4573A0` analiz et - source slot silme burada olabilir!

### sub_4573A0 - Drag Cleanup (Slot Silmiyor!)
- **Adres:** `0x4573A0`
- **Pseudocode:**
  ```c
  void CleanupDragState(void* this) {
      if (this->field_2CC != NULL) {
          sub_89CEF0(this->field_2CC);  // Destroy drag object
          this->field_2CC = NULL;
          this->field_2D0 = 0;
      }
  }
  ```
- **Offset'ler:** `+0x2CC` = Drag object pointer, `+0x2D0` = Flag
- **Sonuç:** Bu fonksiyon source slot icon'unu SİLMİYOR!

**📌 Yeni Hipotez:** Slot icon silme, drag başlamadan önce veya farklı handler'da yapılıyor. `sub_46C630` (sub_458C50'yi çağıran) analiz edilmeli.

---

## 📊 Struct Yapıları

### LearnedSkillStructure
```
Player + 0x13BC → LearnedSkillStructure*
  +0x0C → Tree Head (std::map root)
    +0x04 → Root Node
      +0x08 → Left Child
      +0x0C → Right Child  
      +0x14 → SkillEntry*
        +0x00 → Skill ID (DWORD)
```

### SkillData Layout
```
SkillObject + 0x10C → SkillData (inline)
  +0x30 → Unknown byte
  +0x31 → Unknown byte
  +0x74 → Mastery type
  +0xC8 → Index
  +0xCC → Index2
  +0xD4 → DDJ path (std::string, ~24 bytes)
```

### MSVC std::string (x86)
```
+0x00 → union { char buffer[16]; char* ptr; }
+0x10 → size_t size
+0x14 → size_t capacity
If capacity < 16: inline in buffer
If capacity >= 16: ptr points to heap
```

---

## ✅ Tamamlanan Eylemler

- [x] `sub_616790` analiz edildi - Skill ID → SkillObject
- [x] `sub_601900` analiz edildi - SkillObject → SkillData (inline +0x10C)
- [x] `sub_44DB10` analiz edildi - CIFSkillSlot::SetSkill
- [x] `sub_44DA40` analiz edildi - CIFSkillSlot::OnCreate (IRM gerekli)
- [x] LearnedSkillStructure tree traversal implement edildi
- [x] GetSkillIconPath fonksiyonu yazıldı (DDJ path at +0xD4)
- [x] Skill iconları başarıyla yüklendi (TB_Func_12)
- [x] Slot type 0x50 crash sorunu tespit edildi, 0x0C ile düzeltildi

---

## 📝 Aktif Analiz Notları

### 🔴 Mevcut Sorun: Drag Davranışı
**Problem:** `CIFSlotWithHelp` + Slot Type `0x0C` (QuickSlot) kullanılıyor.
- QuickSlot tipi drag yapınca source slot'u **temizliyor** (move semantics)
- Bizim istediğimiz: source slot **korunsun** (copy semantics)

**Çözüm Seçenekleri:**
1. **Farklı slot type** - Read-only veya copy-on-drag davranışı olan tip bul
2. **Drag event hook** - OnDragStart veya OnDrop handler'ı override et
3. **Icon restore** - Drag sonrası skill ID'den tekrar icon yükle

### Slot Type Değerleri (Bilinen)
| Type | Kullanım | Davranış |
|------|----------|----------|
| 0x0C | QuickSlot | Move (source temizlenir) |
| 0x46 | Inventory | ? |
| 0x50 | Skill | Hover crash |

### 📌 IDA Araştırılacak
1. `sub_5425A0` (SetSlotType) - Tüm tipleri bul
2. CIFSlot VTable (0x93D9C8) - OnDrag handler offset
3. CIFDragableArea - RuntimeClass doğrula

---

## 📝 Aktif Analiz Notları

### Mevcut 3 Sorun
1. **FPS Drop:** DDJ yüklemesi sync, cache gerekli
2. **Drag-Drop Save:** Skills to Use'a drop edilen skill'ler kaydedilmeli
3. **Skill Silinme:** Acquired Skills'den drag yapınca icon siliniyor

### Slot Type Araştırması Gerekli
- Read-only slot type var mı? (display-only, drag yok)
- Inventory slot type (0x46) drag davranışı nasıl?
- Alternative: Drag event hook ile restore

---

## 📌 Sıradaki Hedefler (Todo)

### 🔴 Aktif: Skill Drag-Drop Sistemi (3 Bileşen)

#### 1️⃣ Acquired Skills → Icon Korunmalı
- [ ] Mevcut davranış: Drag yapınca source slot temizleniyor
- [ ] Çözüm A: Native davranışı değiştir (karmaşık)  
- [ ] Çözüm B: Drag sonrası restore et (pratik) ✅
- [ ] Çözüm C: Click-to-copy yap, drag kullanma

#### 2️⃣ Skills to Use → Slot Type Detayları 🔴 AKTİF ARAŞTIRMA
**Hipotez:** Slot type (0x0C vs 0x46) drop davranışını belirliyor

**Araştırılacaklar:**
- [ ] `sub_458C50` - Slot type 0x46 vs 0x0C nasıl farklı davranıyor?
- [ ] Slot type 0x46 ile AutoHuntSettings içinde drop kabul edebilir mi?
- [ ] Drop target validation nerede yapılıyor?
- [ ] AutoHuntSettings window kontrolü nasıl sağlanır?

**Bilinen Slot Type'lar:**
| Type | Hex | Kullanım | Drop Davranışı |
|------|-----|----------|----------------|
| Inventory | 0x46 | Envanter slotları | ✅ Drop kabul eder |
| QuickSlot | 0x0C | Quickbar slotları | ❓ Araştırılacak |
| Skill | 0x50 | Skill board | ❌ Crash on hover |

#### 3️⃣ Confirm → Skill ID'leri Kaydet ✅ IMPLEMENT EDİLDİ (.dat format)
- [x] `OnClick_Confirm` message handler eklendi
- [x] `OnClick_Cancel` message handler eklendi
- [x] `SaveConfig()` binary .dat format (AutoPotion tarzı)
- [x] `LoadConfig()` binary .dat format
- [x] Dosya: `Setting\AutoHunt_{charname}.dat`
- [x] Version header (future compatibility)
- [ ] Test edilecek: Config dosyası oluşuyor mu?

### ⏳ Bekleyen Görevler
- [ ] FPS drop için DDJ cache implementasyonu
- [ ] CIFDragableArea RuntimeClass doğrulama
- [ ] Slot click event'inin nasıl yakalanacağı (OnCommand vs vtable override)

---

## 🔄 CIFVerticalScroll - Scrollbar Analizi (2025-12-19)

### VTable (0x93C688)
| Offset | Adres | Fonksiyon | İşlev |
|--------|-------|-----------|-------|
| +0x00 | sub_428B80 | Destructor |
| +0x24 | sub_428BE0 | **OnCreate** ⭐ |
| +0x30 | sub_427210 | Thunk → sub_4456D0 |
| +0x50 | - | SetGWndSize |
| +0x90 | - | SetPosition |
| +0xB8 | sub_428CC0 | ShowGWnd for buttons |

### Anahtar Fonksiyonlar
| Adres | İsim | Parametreler | Açıklama |
|-------|------|--------------|----------|
| 0x428BE0 | OnCreate | (arg) | Scrollbar initialize, button ve track oluşturur |
| 0x428DE0 | SetScrollBarRange | (height, min, max, step) | Ana range ayarı + button pozisyonları |
| 0x427390 | SetRange | (min, max, step) | İç range setter |
| 0x4271D0 | SetHeight | (height) | +0x2D4'e height yazar (WORD) |

### 🔥 sub_428DE0 (SetScrollBarRange) Kritik Analiz
```cpp
void SetScrollBarRange(short height, int min, int max, int step) {
    this[0x2F0] = 0;  // Reset flag
    sub_4271D0(height);  // Set height at +0x2D4
    sub_427390(min, max, step);  // Set range
    
    // Position THUMB button
    ThumbButton->SetPosition(thumbRect.x, scrollRect.y);
    
    // ⭐ KRITIK: DOWN button pozisyonu
    // DOWN_Y = height + thumbY + 16
    DownButton->SetPosition(scrollRect.x, height + thumbY + 16);
}
```

### 🔴 Scrollbar Boyut Sorunu ve Çözüm

**Problem:** DOWN button lattice'in altına taşıyor

**Analiz:**
- DOWN button Y pozisyonu = `height + thumbY + 16`
- Eğer height = 256 (latticeHeight) ve thumbY ≈ 0
- DOWN_Y = 256 + 0 + 16 = **272px** (lattice dışına taşar!)

**✅ Çözüm:**
- `SetScrollBarRange(latticeHeight - 32, ...)` kullan
- 256 - 32 = 224
- DOWN_Y = 224 + 0 + 16 = **240px** (lattice içinde!)

### Offset Layout
```
+0x2B4: int  - Unknown (initialized to 0)
+0x2C4: int  - Thumb Y position
+0x2D4: WORD - Height (sub_4271D0 yazar)
+0x2DC: int  - Min value
+0x2E0: int  - Max value
+0x2E4: int  - Range (max - min)
+0x2E8: int  - Step
+0x2EC: int  - Current position
+0x2F0: BYTE - Orientation (0=vertical)
+0x2F4: CIFButton* - Up button
+0x2F8: CIFButton* - Down button
+0x2FC: CIFButton* - Thumb button
```

---

## 🔄 CIFScrollManager - Scroll Container Analizi (2025-12-19)

### ECSRO Adresleri
| Item | Address |
|------|---------|
| RuntimeClass | 0x9FFE54 |
| Size | 0x6A4 (1700 bytes) |
| VTable 1 | 0x94BD30 |
| VTable 2 | 0x94BCE8 (+0x6C) |
| Constructor | sub_561C90 |
| OnCreate | sub_561FB0 |
| SetScrollSize | sub_5626C0 |

### sub_5626C0 (SetScrollSize) Analizi
```cpp
void SetScrollSize(int offsetX, int offsetY) {
    this[0x680] = offsetX;  // Scroll X offset
    this[0x684] = offsetY;  // Scroll Y offset
    
    RECT windowRect;
    GetWindowRect(&windowRect);
    
    int y = windowRect.top + this[0x688] + offsetY + 16;
    int height = windowRect.bottom - (2 * offsetY + 48);  // ⭐ AUTO HEIGHT
    int x = windowRect.right - offsetX + windowRect.left - 16;
    
    CIFVerticalScroll* scroll = this[0x69C];
    scroll->ShowGWnd(1);
    scroll->SetGWndSize(16, height);  // VTable+0x50
    scroll->SetPosition(x, y);        // VTable+0x90
    scroll->SetScrollBarRange(height, 0, 10, 1);
}
```

### Offset Layout (ECSRO)
```
+0x680: int - Scroll X offset
+0x684: int - Scroll Y offset
+0x688: int - Additional Y offset
+0x69C: CIFVerticalScroll* - Scrollbar pointer ⭐
+0x6A0: void* - List node pointer
```

### Denenen Yaklaşımlar

| Yaklaşım | Sonuç | Not |
|----------|-------|-----|
| CIFScrollManager + SetScrollSize | ❌ | Otomatik hesaplama istenilen boyutu vermiyor |
| CIFVerticalScroll + SetGWndSize(256) | ❌ | Track uzun kalıyor |
| CIFVerticalScroll + SetScrollBarRange(256) | ❌ | DOWN button taşıyor |
| CIFVerticalScroll + SetScrollBarRange(224) | ❌ | Hala taşma var |
| **CIFVerticalScroll + Sabit Değerler (220/213)** | ✅ | **ÇÖZÜLDÜ!** |

### ✅ ÇÖZÜM (2025-12-19 - TAMAMLANDI)

**Problem:** Scrollbar DOWN button ve track, lattice frame'in dışına taşıyordu.

**Kök Neden:** `latticeHeight` (256px) kullanmak her zaman taşmaya neden oluyor çünkü native scrollbar formülleri bu değeri kullanırken ekstra padding ekliyor.

**Nihai Çözüm:** Sabit değerler kullanmak:

```cpp
// Scrollbar oluşturma
sz.pos.y = panelY + 44;           // Lattice'in biraz altından başla
sz.size.height = 220;             // Sabit yükseklik

// Scrollbar konfigürasyon
SetGWndSize(16, 220);             // 220px yükseklik
SetScrollBarRange(220, 0, 100, 1); // Range için 220

// PopulateLearnedSkills'da
SetScrollBarRange(213, 0, maxOffset, 1);  // Dynamic range için 213
```

**Değerler:**
- Window height: `220px` (creation time)
- SetScrollBarRange height: `220px` (creation), `213px` (dynamic update)
- Y pozisyonu: `panelY + 44` (lattice ortasına hizalı)

### 📌 Sonraki Adımlar
- [x] Thumb sorunu çözüldü (SetVCorrectScrollBar ile)
- [x] Button pozisyon formülü bulundu: `DOWN_Y = height + thumbY + 16`
- [x] ~~Çözüm: `SetScrollBarRange(latticeHeight - 32, ...)` kullan~~ (YETERSİZ)
- [x] **ÇÖZÜLDÜ: Sabit 220/213 değerleri kullan** ✅
- [x] Track texture düzgün çalışıyor
- [x] Confirm/Cancel butonları ortalandı ve aşağı alındı

---

## 🔧 Anahtar Adresler (ECSRO)

| Item | Address |
|------|---------|
| g_pCICPlayer | 0xA0465C |
| Skill Manager | 0xA01010 |
| sub_616790 (GetSkillObj) | 0x616790 |
| sub_601900 (GetSkillData) | 0x601900 |
| sub_44DB10 (SetSkill) | 0x44DB10 |
| sub_44DA40 (OnCreate) | 0x44DA40 |
| sub_5425A0 (SetSlotType) | 0x5425A0 |
| CIFSkillSlot RuntimeClass | 0x9FE700 |
| CIFSlotWithHelp RuntimeClass | 0x9FFD04 |
| CIFVerticalScroll RuntimeClass | 0x9FE238 |
| CIFVerticalScroll VTable | 0x93C688 |
| CIFScrollBar RuntimeClass | 0x9FE1F8 |
| CIFScrollManager RuntimeClass | 0x9FFE54 |
| CIFScrollManager VTable | 0x94BD30 |
| sub_428BE0 (VScroll OnCreate) | 0x428BE0 |
| sub_428DE0 (SetScrollBarRange) | 0x428DE0 |
| sub_4271D0 (SetHeight) | 0x4271D0 |
| sub_427390 (SetRange) | 0x427390 |
| sub_5626C0 (SetScrollSize) | 0x5626C0 |
| **sub_659D70 (GetBuffManager)** | **0x659D70** |
| sub_59E330 (0xB070 BuffAdd Handler) | 0x59E330 |
| sub_653620 (AddBuffToChar) | 0x653620 |

---

## 🔥 Active Buff System - IDA Analizi (2025-12-20) ✅ ÇALIŞIYOR

### ✅ DOĞRULANAN OFFSET: 0x1C4 (sub_653620)

> **ÖNEMLİ:** İlk başta 0x141C (sub_659D70) kullandık ama bu YANLIŞ!
> Doğru offset **0x1C4** (sub_653620'den alındı: `this[113]` = `this + 0x1C4`)

### Buff List Yapısı (Kesin - Çalışıyor)
```
g_pCICPlayer (0xA0465C)
    └── +0x1C4 → BuffList* (linked list head)
            └── *ListHead → FirstNode
                    ├── +0x00 → NextNode pointer
                    ├── +0x08 → BuffEntry* (v4[2])
                    │           └── +0x54 → Skill ID! (84 decimal)
```

### IDA Analizi: sub_653620
```c
int __thiscall sub_653620(_DWORD *this, int a2) {
  v3 = this[113];           // → Player + 0x1C4 = BuffList head
  v4 = *v3;                 // → Dereference = First node
  while (v4 != v3) {        // → Loop until sentinel
    v5 = v4[2];             // → Node+0x08 = Entry pointer
    if (*(_DWORD *)(v5 + 84) == a2)  // → Entry+0x54 = SkillID!
      return v5;
    v4 = *v4;               // → Node+0x00 = Next node
  }
  return 0;
}
```

### Offset Özeti
| Offset | Konum | Açıklama |
|--------|-------|----------|
| 0x1C4 | Player + 0x1C4 | BuffList head pointer |
| 0x00 | Node + 0x00 | Next node pointer |
| 0x08 | Node + 0x08 | BuffEntry pointer |
| 0x54 | Entry + 0x54 | Skill ID (84 decimal) |

### İlgili Fonksiyonlar
| Adres | İşlev |
|-------|-------|
| **sub_653620** | **FindBuffBySkillID - Ana referans!** |
| sub_659D70 | GetBuffManager (0x141C - FARKLI YAPI!) |
| sub_59E330 | 0xB070 Buff Add packet handler |
| sub_632CB0 | GetBuffEntry by Token |
| sub_6B6760 | UpdateBuffEntry |

### ✅ Implementasyon Durumu
- [x] `ActiveBuffManager.h` - Doğru offset'lerle güncellendi
- [x] `ActiveBuffManager.cpp` - Linked list traversal çalışıyor
- [x] `GetActiveBuffs()` - Aktif buff'ları döndürüyor
- [x] `IsBuffActive(skillId)` - Skill ID kontrolü çalışıyor
- [x] `Update()` - Buff değişikliklerini takip ediyor

---

## 🎮 AutoBuffController - Auto Buff Sistemi (2025-12-20)

### Dosyalar
| Dosya | Açıklama |
|-------|----------|
| `AutoBuffController.h` | Class tanımı |
| `AutoBuffController.cpp` | Implementation |

### Sınıf API'si
```cpp
class AutoBuffController {
public:
    static void Initialize();           // Başlat
    static void Update();               // Frame loop (OnEndScene'den çağrılıyor)
    static void SetEnabled(bool);       // Aç/Kapa
    static bool IsEnabled();            // Durum
    static void CheckAndLogMissingBuffs();  // Log eksik buff'ları
    static std::vector<DWORD> GetMissingBuffs();  // Eksik buff listesi
};
```

### Entegrasyon
- **Macro Window:** `On_AutoAttack_Toggle()` fonksiyonu `AutoBuffController::SetEnabled()` çağırıyor
- **OnEndScene:** `Util.cpp`'de `AutoBuffControllerCallback()` her frame çağırıyor
- **IFAutoHuntSettings:** `GetSelectedBuffSkillIds()` ile seçili buff'ları alıyor
- **ActiveBuffManager:** `GetActiveBuffs()` ile aktif buff'ları alıyor

### Davranış
1. Macro Window'da Auto Attack ikonu tıklanır (ON)
2. Her 2 saniyede bir:
   - Seçili buff'lar alınır (IFAutoHuntSettings)
   - Aktif buff'lar alınır (ActiveBuffManager)
   - Eksik buff'lar bulunur
   - Konsola log yazdırılır
3. Macro Window ikonu tıklanır (OFF) → Durur

### 🚧 TODO: Skill Cast Implementasyonu

**Sorun:** `CastBuffSkill()` fonksiyonu crash veriyor.

**IDA Analizi Gerekli:**
- `sub_56CC70` - Buff skill gönderme fonksiyonu
- `sub_5E4220` - Paket hazırlama
- `sub_41AB90` - Buffer'a yazma
- `sub_5E4340` - Paket gönderme

**Opcode Format (0x7074):**
```
Client → Server: 0x7074
[01] [04] [SkillID - 4 byte]
 │    │    └─ Skill ID (little endian)
 │    └─ Action type (4 = buff?)
 └─ Count/Flag
```

**Packet Capture Örneği (3365 skill):**
```
Client: 0x7074 → 01 04 25 0D 00 00 00
Server: 0xB074 → 01 01 (success)
Server: 0xB070 → Buff add
```

**Crash Sebebi Tahmini:**
- `sub_41AB90` thiscall convention gerektirebilir
- Packet buffer structure tam doğru değil
- `off_93B674` vtable yapısı eksik

### İlgili Fonksiyonlar (IDA)
| Adres | İşlev |
|-------|-------|
| sub_56CC70 | Basit skill usage (0x7074 gönderir) |
| sub_4F5B00 | Karmaşık skill usage |
| sub_4F6DE0 | Skill execution |
| sub_5E4220 | PreparePacket (can send check) |
| sub_41AB90 | WriteBytes to buffer |
| sub_5E4340 | SendPacket |
| 0xA00EB0 | Packet buffer global |
| 0x93B674 | Packet VTable |

