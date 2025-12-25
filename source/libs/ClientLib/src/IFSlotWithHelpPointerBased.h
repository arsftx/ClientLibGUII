#pragma once

// Pointer-based quickbar monitoring için class
class CIFSlotWithHelpPointerBased {
public:
    char unknown_bytes[708]; // 0x2C4 offset'ine kadar olan boşluk
    void* pItemObject;       // +0x2C4: Gerçek item nesnesinin pointer'ı
}; 