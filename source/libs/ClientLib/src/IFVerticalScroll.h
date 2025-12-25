#pragma once

#include <windows.h>
#include "IFScrollBar.h"
#include "IFButton.h"
#include <Test/Test.h>

// =====================================================================
// CIFVerticalScroll - Vertical Scrollbar Control
// IDA Analysis (ECSRO):
//   Runtime Class: 0x9FE238
//   Size: 0x300 (768 bytes)
//   Parent: CIFScrollBar (0x9FE1F8)
//
//   Registration: sub_4289A0
//   Factory:      sub_4289D0
//   Constructor:  sub_428B60
//   CreateObject: sub_428A30
//
//   VTable 1: 0x93C688 (at +0x00)
//   VTable 2: 0x93C640 (at +0x6C, secondary/inner class)
//
// Offset Layout (from sub_428B60):
//   +0x000 - 0x2F3: CIFScrollBar base
//   +0x2F0: BYTE - orientation/type flag (initialized to 0)
//   +0x2F4 - 0x2FF: CIFVerticalScroll data (0x0C bytes = 12 bytes)
// =====================================================================

class CIFVerticalScroll : public CIFScrollBar {
    GFX_DECLARE_DYNAMIC_EXISTING(CIFVerticalScroll, 0x9FE238)

public:
    CIFVerticalScroll();
    ~CIFVerticalScroll();

    // =====================================================================
    // Offset-based accessors
    // =====================================================================
    
    // +0x2F0: BYTE - orientation flag (0 = vertical?)
    void SetOrientationFlag(BYTE value) {
        MEMUTIL_WRITE_BY_PTR_OFFSET(this, 0x2F0, BYTE, value);
    }
    BYTE GetOrientationFlag() const {
        return MEMUTIL_READ_BY_PTR_OFFSET(this, 0x2F0, BYTE);
    }

    // =====================================================================
    // Button accessors (based on CIFHScroll_Option pattern)
    // +0x2F4: Up button
    // +0x2F8: Down button  
    // +0x2FC: Thumb button
    // =====================================================================
    CIFButton* GetUpButton() {
        return MEMUTIL_READ_BY_PTR_OFFSET(this, 0x2F4, CIFButton*);
    }
    CIFButton* GetDownButton() {
        return MEMUTIL_READ_BY_PTR_OFFSET(this, 0x2F8, CIFButton*);
    }
    CIFButton* GetThumbButton() {
        return MEMUTIL_READ_BY_PTR_OFFSET(this, 0x2FC, CIFButton*);
    }
    
    // =====================================================================
    // Scrollbar configuration (IDA verified: sub_428DE0, sub_427390)
    // sub_428DE0(height, min, max, step) - Main setup function
    // sub_427390(min, max, step) - Internal range setter
    // sub_4271D0(height) - Height setter
    // =====================================================================
    
    // sub_428DE0: SetScrollBarRange(height, min, max, step)
    // This is the main function to configure scrollbar properly
    void SetScrollBarRange(int height, int minVal, int maxVal, int step) {
        typedef void(__thiscall* SetScrollBarRange_t)(void*, int, int, int, int);
        static SetScrollBarRange_t func = (SetScrollBarRange_t)0x428DE0;
        func(this, height, minVal, maxVal, step);
    }
    
    // sub_427390: SetRange(min, max, step) - internal range setter
    void SetRange(int minVal, int maxVal, int step) {
        typedef void(__thiscall* SetRange_t)(void*, int, int, int);
        static SetRange_t func = (SetRange_t)0x427390;
        func(this, minVal, maxVal, step);
    }
    
    // sub_4271D0: SetScrollHeight(height)
    void SetScrollHeight(int height) {
        typedef void(__thiscall* SetHeight_t)(void*, int);
        static SetHeight_t func = (SetHeight_t)0x4271D0;
        func(this, height);
    }
    
    // =====================================================================
    // Track child window access (IDA verified: sub_428BE0 uses +0x6C)
    // OnCreate calls: lea ecx,[esi+6Ch]; mov edx,[esi+6Ch]; call [edx+34h]
    // LEA means +0x6C is an EMBEDDED object, not a pointer!
    // =====================================================================
    
    // Get track embedded object address at +0x6C (NOT a pointer dereference!)
    void* GetTrackWindow() {
        // ASM uses LEA not MOV - return address directly, don't dereference
        return reinterpret_cast<void*>(reinterpret_cast<char*>(this) + 0x6C);
    }
    
    // Resize track window directly using vtable+0x50 (SetGWndSize)
    // NOTE: VTable+0x34 is TB_Func_12, NOT SetGWndSize!
    void ResizeTrack(int width, int height) {
        void* track = GetTrackWindow();  // Address of embedded object at +0x6C
        if (track) {
            // vtable is at the start of the embedded object
            void* vtable = *reinterpret_cast<void**>(track);
            typedef void(__thiscall* SetGWndSize_t)(void*, int, int);
            SetGWndSize_t setSize = reinterpret_cast<SetGWndSize_t>(
                *reinterpret_cast<void**>(reinterpret_cast<char*>(vtable) + 0x50)  // SetGWndSize
            );
            setSize(track, width, height);
        }
    }
    
    // =====================================================================
    // Direct offset accessors (IDA verified)
    // =====================================================================
    
    // +0x2C4: Thumb Y position (int)
    int GetThumbPosition() const {
        return MEMUTIL_READ_BY_PTR_OFFSET(this, 0x2C4, int);
    }
    void SetThumbPosition(int y) {
        MEMUTIL_WRITE_BY_PTR_OFFSET(this, 0x2C4, int, y);
    }
    
    // +0x2D4: Scrollbar height (WORD - 16 bit!)
    // IDA: sub_4271D0 writes to this offset
    short GetHeight() const {
        return MEMUTIL_READ_BY_PTR_OFFSET(this, 0x2D4, short);
    }
    void SetHeight(short height) {
        MEMUTIL_WRITE_BY_PTR_OFFSET(this, 0x2D4, short, height);
    }
    
    // IDA sub_427390 verified offsets:
    // +0x2DC: Min value
    int GetMinValue() const {
        return MEMUTIL_READ_BY_PTR_OFFSET(this, 0x2DC, int);
    }
    void SetMinValue(int val) {
        MEMUTIL_WRITE_BY_PTR_OFFSET(this, 0x2DC, int, val);
    }
    
    // +0x2E0: Max value
    int GetMaxValue() const {
        return MEMUTIL_READ_BY_PTR_OFFSET(this, 0x2E0, int);
    }
    void SetMaxValue(int val) {
        MEMUTIL_WRITE_BY_PTR_OFFSET(this, 0x2E0, int, val);
    }
    
    // +0x2E4: Range (max - min) - sub_427240 reads this for thumb calculation!
    int GetRange() const {
        return MEMUTIL_READ_BY_PTR_OFFSET(this, 0x2E4, int);
    }
    void SetRange_Direct(int val) {
        MEMUTIL_WRITE_BY_PTR_OFFSET(this, 0x2E4, int, val);
    }
    
    // +0x2E8: Step
    int GetStep() const {
        return MEMUTIL_READ_BY_PTR_OFFSET(this, 0x2E8, int);
    }
    void SetStep(int val) {
        MEMUTIL_WRITE_BY_PTR_OFFSET(this, 0x2E8, int, val);
    }
    
    // +0x2EC: Current position
    int GetCurrentPosition() const {
        return MEMUTIL_READ_BY_PTR_OFFSET(this, 0x2EC, int);
    }
    void SetCurrentPosition(int pos) {
        MEMUTIL_WRITE_BY_PTR_OFFSET(this, 0x2EC, int, pos);
    }
    
    // =====================================================================
    // Thumb sync - discovered from CIFHScroll_Option pattern!
    // sub_427240 properly updates thumb position based on current value
    // =====================================================================
    void SetVCorrectScrollBar(int position) {
        typedef void(__thiscall* SetCorrect_t)(void*, int);
        static SetCorrect_t func = (SetCorrect_t)0x427240;
        func(this, position);
    }

private:
    // CIFScrollBar ends at 0x2F4, CIFVerticalScroll adds 0x0C bytes (0x300 - 0x2F4 = 0x0C)
    char m_vscrollData[0x0C];  // 0x2F4 to 0x300

    BEGIN_FIXTURE()
    ENSURE_SIZE(0x0300)
    END_FIXTURE()

    RUN_FIXTURE(CIFVerticalScroll)
};
