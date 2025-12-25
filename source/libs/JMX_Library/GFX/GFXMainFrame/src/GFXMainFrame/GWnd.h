#pragma once

#include "GWndBase.h"

#include "remodel/GlobalPtr.h"

enum StyleOptions : DWORD {
    NONE = 0x00,
    UNKNOWN_0x01 = 0x01,
    PASSWORD_MASKED = 0x02,
    UNKNOWN_0x04 = 0x04,
    UNKNOWN_0x08 = 0x08,
    UNKNOWN_0x10 = 0x10,
    UNKNOWN_0x20 = 0x20,
    INVISIBLE = 0x40,
    TOOLTIP = 0x80
};

/// Adds just a bunch of virtuals to \ref CGWndBase which are pretty much all unknown ...
class CGWnd : public CGWndBase {
    // 132 - 132 = 0 <- no additional bytes :)
    GFX_DECLARE_DYNAMIC_EXISTING(CGWnd, 0x0C5DDC8) //ECSRO ORIG

public:
    /// \address 00B9D810
    virtual void Func_24(int a1, int a2);

    virtual bool Func_25(int a1);

    virtual void Func_26(int a1);

    virtual bool Func_27();

    virtual bool Func_28(int a1, int a2, int a3);

    /// \brief Called when the left mouse button is released
    /// \param x Position X of the mouse
    /// \param y Position Y of the mouse
    /// \address 0046fd70
    virtual int OnMouseLeftUp(int a1, int x, int y);

    /// \brief Called when the left mouse button is pressed
    /// \param x Position X of the mouse
    /// \param y Position Y of the mouse
    /// \address 0046fd60
    virtual int OnMouseLeftDown(int a1, int x, int y);

    /// \brief Called when the left mouse button is released twice
    /// \param nPos Is the x-y of the mouse
    /// \b GET_X_LPARAM Macro to get \b X pos
    /// \b GET_Y_LPARAM Macro to get \b Y pos
    /// \param nDownVK Indicates whether various virtual keys are down.
    /// \address 0046fd50
    virtual int OnMouseDbLeftUp(int a1, int nPos, int nDownVK);

    /// \brief Called when the right mouse button is pressed
    /// \param x Position X of the mouse
    /// \param y Position Y of the mouse
    /// \address 0046fd40
    virtual int OnMouseRightDown(int a1, int x, int y);

    /// \brief Called when the right mouse button is released
    /// \param x Position X of the mouse
    /// \param y Position Y of the mouse
    /// \address 0046fd30
    virtual int OnMouseRightUp(int a1, int x, int y);

    virtual int Func_34(int a1, int a2, int a3);

    /// \brief Called when the mouse is moved over the control
    /// \param x Position X of the mouse
    /// \param y Position Y of the mouse
    /// \address 0046fd10
    virtual int OnMouseMove(int a1, int x, int y);

    virtual int Func_36(int a1, short action, int a3, int a4);

    /// \address 009d23f0
    CGWnd();

    /// \address 00b9d7a0
    ~CGWnd();

    /// \address 00B9D7E0
    void OnUpdate() override;

    /// \address 00B9D850
    void RenderMyself() override;

    /// \address 00BA0300
    void RenderMyChildren() override;

    /// \address 00B9D7F0
    void Render() override;

    /// \address 00B9D880
    void Func_15() override;

    /// \address 00B9D890
    bool On3DEvent_MAYBE(Event3D *a2) override;

public:
    /// \address 00b9db10
    void SetHoldingMouseSlot(CGWnd *pSlot);

    static int GetCurrentEventMsgCtrlhgWnd();

    /// \address 00b9dc60
    static int GetCurrentEventMsgCtrlId();

    /// \address 00ba0e40
    void EraseWindowObj();

    /// Create a new CGWnd-instance based on the given runtime class
    /// \deprecated This function is deprecated. Please use the one that uses wnd_rect instead of RECT!
    /// \param pParent Parent control this control should live in
    /// \param pType Runtime class type identifying the class to be created
    /// \param Rect Initial dimension and position
    /// \param a4 Presumably the Id
    /// \param a5 Unknown
    static CGWnd *CreateInstance(CGWnd *pParent, const CGfxRuntimeClass &pType, RECT &Rect, int a4, int a5) {
        return reinterpret_cast<CGWnd *(__cdecl *) (CGWnd *, const CGfxRuntimeClass *, RECT *, int, int)>(0x089CC40)( //ECSRO
                pParent, &pType, &Rect, a4, a5);
    }

    /// Create a new CGWnd-instance based on the given runtime class
    /// \param pParent Parent control this control should live in
    /// \param pType Runtime class type identifying the class to be created
    /// \param Rect Initial dimension and position
    /// \param a4 Presumably the Id
    /// \param a5 Unknown
    static CGWnd *CreateInstance(CGWnd *pParent, const CGfxRuntimeClass &pType, wnd_rect &Rect, int a4, int a5) {
        return reinterpret_cast<CGWnd *(__cdecl *)(CGWnd *, const CGfxRuntimeClass *, wnd_rect *, int,
                                                    int)>(0x089CC40)(//ECSRO
                pParent, &pType, &Rect, a4, a5);
    }
};

#define g_pOnMouseDownClickCtrl (*((CGWnd **) 0x0C5DE40)) //ECSRO
#define g_pUnderFocusCtrl (*((CGWnd **) 0x0C5DE44)) //ECSRO
#define g_pMouseHoldingSlot (*((CGWnd **) 0x0C5DE48)) //ECSRO
/// \brief Current event msg map element like click etc
#define g_pCurrentEventCtrl (*((CGWnd **) 0x0C5DE50))//ECSRO
