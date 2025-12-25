#pragma once

#include "GFXMainFrame/GWnd.h"
#include "IRMManager.h"
#include "TextBoard.h"
#include <Test/Test.h>
#include <ghidra/undefined.h>
#include <memory/util.h>

class CIFWnd : public CGWnd, public CTextBoard {
    GFX_DECLARE_DYNAMIC_EXISTING(CIFWnd, 0x009FE5C0)//ECSRO

    GFX_DECLARE_MESSAGE_MAP(CIFWnd)
public:
    CIFWnd(void);

    ~CIFWnd(void);

public:
    void OnTimer(int) override;

    bool OnCreate(long ln) override;

    bool OnRelease() override;

    bool Func_18(Event3D *a1) override;

    bool On3DEvent_MAYBE(Event3D *a2) override;

    void BringToFront();

    void BringToUp();

    void SetGWndSize(int width, int height);

    void ChangeGWndSize(int width, int height);

    void Func_22(int x, int y) override;

    virtual void Func_37(int x, int y);

    void TB_Func_7() override;

    char TB_Func_8() override;

    void TB_Func_11() override;

    void OnUpdate();

    void ShowGWnd(bool bVisible);

    void ShowWnd(bool bVisible);

    void OnWndMessage(Event3D *a1) override;

    void RenderMyself() override;

    void MoveGWnd(int x, int y);

    void ChangePos(int x, int y);

    virtual void MoveGWnd2(wnd_pos pos);

    virtual void Func_40();

    /// \address 006529D0
    virtual void OnCIFReady();

    virtual void Func_42();

    /// \address 00652c00
    virtual undefined1 OnCloseWnd();

    virtual bool IsInside(int x, int y);

    virtual bool SetText(const char *src);

    virtual const wchar_t *GetText();

    const wchar_t *ReturnText();

    virtual void Func_47(int);

    virtual void Func_48();

    /// \address 006527D0
    void SetSomeRect(const RECT &rect);

    /// \brief Set the tooltip text (mouse hover)
    /// \address 00653DC0
    void SetTooltipText(const std::n_wstring *str);

    /// \brief Set Style for interface
    /// \See StyleOptions
    /// \address 00652d20
    void SetStyleThingy(StyleOptions option);

    /// \brief Check Style for Window
    /// \See StyleOptions
    /// \address 00652da0
    bool IsStyleThingy(StyleOptions option) const;

    /// \address 006526E0
    void sub_6526E0(char n00009771, unsigned char opacity, float time, float a4, char a5);

    int sub_644820();
	CIRMManager GetWndIRM();
private:
    int OnCreatedInstance(UINT lParam, UINT wParam);

    int OnWindowPosChanged(UINT lParam, UINT wParam);

    void sub_652B00();

    void sub_653060();

    void Set_N00000687(char a2);

public:

	CIRMManager m_IRM;
	public:
	char pad_NULL[6];
    CIRMManager m_IRM1; //0x01BC m_IRM at 1BC
    char pad_0170[220];//0x0170

    BEGIN_FIXTURE()
    ENSURE_SIZE(0x2B4)
    END_FIXTURE()

    RUN_FIXTURE(CIFWnd)
};//Size: 0x02B4
