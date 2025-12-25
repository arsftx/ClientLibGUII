#include "SoundBugFix.h"
#include <Windows.h>
#include <stdio.h>

// --- STATİK ADRESLER (sro_client.exe Offsetleri) ---
// Bellekte yan yana duruyorlar: 88, 89, 8A
#define STATIC_OFFSET_BG 0x600C88 // Background
#define STATIC_OFFSET_FX 0x600C89 // FX Sound
#define STATIC_OFFSET_ENV 0x600C8A// Environmental

// Hedef Fonksiyon: PrepareSound
#define ADDR_PREPARE_SOUND 0x007A84F0

typedef int(__thiscall *tPrepareSound)(void *pSoundObj, void *a2, void *a3, int a4);
tPrepareSound Original_PrepareSound = NULL;

void CreateDebugConsole() {
    if (AllocConsole()) {
        freopen("CONOUT$", "w", stdout);
        freopen("CONOUT$", "w", stderr);
        SetConsoleTitle("SRO SoundFix: COMPLETE STATIC");
        printf("[SRO_FIX] Tam Koruma Aktif (Static Memory).\n");
        printf("[SRO_FIX] BG(88) - FX(89) - ENV(8A) izleniyor.\n");
    }
}

// --- DÜZELTİLEN DETOUR FONKSİYONU ---
void *Detour_Simple(void *target, void *detour) {
    if (!target || !detour) return NULL;

    DWORD old;
    if (!VirtualProtect(target, 5, PAGE_EXECUTE_READWRITE, &old)) return NULL;

    // HATA BURADAYDI: Değişken adını 'trampoline' olarak düzelttim
    void *trampoline = VirtualAlloc(NULL, 32, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

    if (!trampoline) {
        VirtualProtect(target, 5, old, &old);
        return NULL;
    }

    // 1. Orijinal 5 byte'ı kopyala
    memcpy(trampoline, target, 5);

    // 2. Trampoline'den Orijinal koda dönüş JMP'i
    *((BYTE *) trampoline + 5) = 0xE9;
    // Hesaplama artık doğru (trampoline değişkenini kullanıyor)
    *(DWORD *) ((BYTE *) trampoline + 6) = (DWORD) target + 5 - ((DWORD) trampoline + 5 + 5);

    // 3. Hedef fonksiyona bizim Detour'a giden JMP'i at
    *((BYTE *) target) = 0xE9;
    *(DWORD *) ((BYTE *) target + 1) = (DWORD) detour - ((DWORD) target + 5);

    VirtualProtect(target, 5, old, &old);

    return trampoline;
}

// --- HOOK FONKSİYONU ---
int __fastcall Hooked_PrepareSound(void *pSoundObj, void *edx, void *a2, void *a3, int a4) {
    static DWORD baseAddr = (DWORD) GetModuleHandle(NULL);
    bool shouldBlock = false;

    __try {
        // 1. BACKGROUND MUSIC (0x600C88)
        BYTE *pFlagBG = (BYTE *) (baseAddr + STATIC_OFFSET_BG);
        if (*pFlagBG == 1) shouldBlock = true;

        // 2. FX SOUND (0x600C89)
        if (!shouldBlock) {
            BYTE *pFlagFX = (BYTE *) (baseAddr + STATIC_OFFSET_FX);
            if (*pFlagFX == 1) shouldBlock = true;
        }

        // 3. ENVIRONMENTAL (0x600C8A)
        if (!shouldBlock) {
            BYTE *pFlagEnv = (BYTE *) (baseAddr + STATIC_OFFSET_ENV);
            if (*pFlagEnv == 1) shouldBlock = true;
        }
    } __except (1) {}

    // EĞER HERHANGİ BİRİ KAPALIYSA
    if (shouldBlock) {
        Sleep(1);// CPU Freni
        return 0;// İptal
    }

    return Original_PrepareSound(pSoundObj, a2, a3, a4);
}

namespace SoundBugFix {
    void Install() {
        //CreateDebugConsole();
        void *tramp = Detour_Simple((void *) ADDR_PREPARE_SOUND, (void *) Hooked_PrepareSound);
        if (tramp) {
            Original_PrepareSound = (tPrepareSound) tramp;
            printf("[SRO_FIX] Hook Basarili! Tum ses kanallari kontrol altinda.\n");
        }
    }
}// namespace SoundBugFix