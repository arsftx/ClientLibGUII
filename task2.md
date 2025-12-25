# Auto Hunt Settings - Drag-Drop Skill Icons

## 🎯 Hedef
Skills to Use paneline skill icon'larını drag-drop veya click ile atamayı sağlamak.

## ❌ Denenen ve Başarısız Olan Yöntemler

### 1. InitDropSupport (+0x6C VTable, +0x3CC Vector, +0x12C Flag)
- VTable zaten 0x94AEB8 set edilmişti
- Vector düzgün allocate edildi
- Flag 0x02 set edildi
- **Sonuç:** Drop hala çalışmadı

### 2. SetIcon (Native sub_4452F0)
- sub_4452F0 fonksiyonu analiz edildi
- SetIcon, GetIconPath, ClearIcon fonksiyonları eklendi
- AddSkillToUse/RemoveSkillFromUse güncellendi
- **Sonuç:** Icon görünmüyor

---

## 🔍 Analiz Edilenler

| Fonksiyon | Adres | Ne Yapıyor |
|-----------|-------|------------|
| sub_53DAF0 | 0x53DAF0 | CIFSlotWithHelp constructor |
| sub_54F530 | 0x54F530 | CIFUnderBar::OnCreate |
| sub_4452F0 | 0x4452F0 | SetSlotIcon (DDJ path → +0x140) |
| sub_446C70 | 0x446C70 | Load icon texture to +0x6C |
| sub_4465A0 | 0x4465A0 | Flag checker (+0x12C) |
| sub_4456D0 | 0x4456D0 | Drop handler |

---

## 📋 Sonraki Adımlar

- [ ] Debug log'larını kontrol et (DragDropDebug.txt)
- [ ] SetIcon fonksiyonunun döndürdüğü değeri kontrol et
- [ ] +0x140 buffer'ı kontrol et (path doğru set ediliyor mu?)
- [ ] sub_446C70 texture load fonksiyonunu analiz et
- [ ] Alternatif: TB_Func_12 kullanmaya geri dön ve neden çalışmadığını analiz et

---

## 💡 Alternatif Çözümler

1. **Native IRM Kullan:** `resinfo\ifautohunt.txt` ile slot oluştur
2. **Underbar Slot Kopyala:** Çalışan CIFUnderBar slot'undan DDJ path kopyala
3. **Custom Rendering:** Icon'u manuel render et
4. **Click-to-Add Only:** Drag-drop yerine sadece click kullan (mevcut çalışıyor)
