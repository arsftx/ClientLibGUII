#pragma once

#include "ifwnd.h"

class CIFStatic : public CIFWnd {
    GFX_DECLARE_DYNAMIC_EXISTING(CIFStatic, 0x009fe4e0)

public:
    CIFStatic(void);

    ~CIFStatic(void);

public:
    bool OnCreate(long ln) override;

    void SetTextColor(D3DCOLOR color);

    bool SetText(const char *src);
	bool SetTextOriginal(const char* src);
    void OnWndMessage(Event3D *a1) override;

    void RenderMyself() override;

    void Func_42() override;

    virtual bool SetTextFormatted(const char *format, ...);

    virtual bool Func_50(const wchar_t *format, ...);

    virtual bool Func_51(const wchar_t *src);

private:
    int N000096E4;//0x02B4
    char pad_02B8[16];//0x02B8
};
