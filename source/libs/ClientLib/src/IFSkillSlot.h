#pragma once

#include "IFWnd.h"

// =====================================================================
// CIFSkillSlot - Native skill slot class for displaying skill icons
// Used by SkillBoard as drag SOURCE - icons can be dragged FROM this
// =====================================================================
// IDA Analysis (ECSRO):
//   RuntimeClass: 0x9FE700
//   Size: 0x318 bytes (792)
//   Parent: CIFWnd (0x9FE5C0)
//   VTable Main: 0x93EB68 (+0x00)
//   VTable Sub:  0x93EB20 (+0x6C)
//   Constructor: sub_44D740
//   OnCreate: sub_44DA40
//   SetSkill: sub_44DB10
// =====================================================================

class CIFSkillSlot : public CIFWnd {
    GFX_DECLARE_DYNAMIC_EXISTING(CIFSkillSlot, 0x009FE700)

public:
    CIFSkillSlot() {}
    virtual ~CIFSkillSlot() {}

    // Call native OnCreate to initialize child elements
    // MUST be called after CreateInstance before using SetSkill
    bool InitSkillSlot() {
        typedef BOOL (__thiscall *OnCreate_t)(void*, DWORD);
        static OnCreate_t fnOnCreate = (OnCreate_t)0x44DA40;
        return fnOnCreate(this, 0) != 0;
    }

    // Set skill by ID - calls native sub_44DB10
    // type: 0=normal, 5=passive, 6-8=other types
    void SetSkill(BYTE type, DWORD dwSkillID) {
        typedef void (__thiscall *SetSkill_t)(void*, DWORD, DWORD);
        static SetSkill_t fnSetSkill = (SetSkill_t)0x44DB10;
        fnSetSkill(this, type, dwSkillID);
    }

    // Get skill ID from +0x2E0
    DWORD GetSkillID() const {
        return *(DWORD*)((char*)this + 0x2E0);
    }

    // Get skill object pointer from +0x314
    DWORD GetSkillObject() const {
        return *(DWORD*)((char*)this + 0x314);
    }

    // Set slot index (used by SkillBoard for slot identification)
    void SetSlotIndex(int index) {
        *(int*)((char*)this + 0x304) = index;
    }

    int GetSlotIndex() const {
        return *(int*)((char*)this + 0x304);
    }

    // Hide the "ADD" button child element (offset +0x2F0)
    // Call this after InitSkillSlot() to hide the level-up button
    void HideAddButton() {
        void* addButton = *(void**)((char*)this + 0x2F0);
        if (addButton) {
            // Call ShowGWnd(false) on the child element
            typedef void (__thiscall *ShowGWnd_t)(void*, bool);
            // VTable entry for ShowGWnd is typically at offset 0x58 (88)
            // But to be safe, use the known function directly
            static ShowGWnd_t fnShowGWnd = NULL;
            if (!fnShowGWnd) {
                // Get from VTable - ShowGWnd is at +0x58 in vtable
                DWORD* vtable = *(DWORD**)addButton;
                fnShowGWnd = (ShowGWnd_t)vtable[0x58 / 4];
            }
            if (fnShowGWnd) {
                fnShowGWnd(addButton, false);
            }
        }
    }

    // Hide the icon child element (offset +0x2F4) - for empty slot display
    void HideIconChild() {
        void* iconChild = *(void**)((char*)this + 0x2F4);
        if (iconChild) {
            typedef void (__thiscall *ShowGWnd_t)(void*, bool);
            DWORD* vtable = *(DWORD**)iconChild;
            ShowGWnd_t fnShowGWnd = (ShowGWnd_t)vtable[0x58 / 4];
            if (fnShowGWnd) {
                fnShowGWnd(iconChild, false);
            }
        }
    }

private:
    // CIFWnd base: 0x000 - 0x2B4
    // Memory layout from constructor sub_44D740:
    char pad_02B4[0x2C];          // 0x2B4 - 0x2E0 (padding)
    DWORD m_skillID;              // 0x2E0 (init 0)
    char pad_02E4[0x04];          // 0x2E4 - 0x2E8
    DWORD m_flag;                 // 0x2E8 (init 0)
    char pad_02EC[0x0C];          // 0x2EC - 0x2F8
    // Vector at 0x2F8 (begin, end, capacity) - 12 bytes
    DWORD m_vectorBegin;          // 0x2F8
    DWORD m_vectorEnd;            // 0x2FC
    DWORD m_vectorCapacity;       // 0x300
    int m_index1;                 // 0x304 (init -1) - slot index
    int m_index2;                 // 0x308 (init -1)
    int m_index3;                 // 0x30C (init -1)
    int m_index4;                 // 0x310 (init -1)
    DWORD m_pSkillObject;         // 0x314 (init 0)
    // Total Size: 0x318

public:
    BEGIN_FIXTURE()
        ENSURE_SIZE(0x318)
        ENSURE_OFFSET(m_skillID, 0x2E0)
        ENSURE_OFFSET(m_index1, 0x304)
        ENSURE_OFFSET(m_pSkillObject, 0x314)
    END_FIXTURE()

    RUN_FIXTURE(CIFSkillSlot)
};
