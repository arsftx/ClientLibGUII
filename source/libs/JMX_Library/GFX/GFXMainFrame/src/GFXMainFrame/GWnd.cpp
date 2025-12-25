#include "GWnd.h"

GFX_IMPLEMENT_DYNAMIC_EXISTING(CGWnd, 0x0C5DDC8)

void CGWnd::Func_24(int a1, int a2) {
	return;
    //reinterpret_cast<void(__thiscall *)(CGWnd *, int, int)>(0x009d1c00)(this, a1, a2);
}

bool CGWnd::Func_25(int a1) {
    return true;
}

void CGWnd::Func_26(int a1) {
}

bool CGWnd::Func_27() {
    return true;
}

bool CGWnd::Func_28(int a1, int a2, int a3) {
    return false;
}

int CGWnd::OnMouseLeftUp(int a1, int x, int y) {
    return 0;
}

int CGWnd::OnMouseLeftDown(int a1, int x, int y) {
    return 0;
}

int CGWnd::OnMouseDbLeftUp(int a1, int nPos, int nDownVK) {
    return 0;
}

int CGWnd::OnMouseRightDown(int a1, int x, int y) {
    return 0;
}

int CGWnd::OnMouseRightUp(int a1, int x, int y) {
    return 0;
}

int CGWnd::Func_34(int a1, int a2, int a3) {
    return 0;
}

int CGWnd::OnMouseMove(int a1, int x, int y) {
    return 0;
}

int CGWnd::Func_36(int a1, short action, int a3, int a4) {
    return 0;
}

CGWnd::CGWnd() {
    reinterpret_cast<void(__thiscall *)(CGWnd *)>(0x0089C4E0)(this);//ecsro
}

CGWnd::~CGWnd() {

	if (!this)
		return;

    if (g_pOnMouseDownClickCtrl == this)
        g_pOnMouseDownClickCtrl = NULL;

    if (g_pMouseHoldingSlot == this)
        g_pMouseHoldingSlot = NULL;

    if (g_pUnderFocusCtrl == this)
        g_pUnderFocusCtrl = NULL;
}

void CGWnd::OnUpdate() {
    // empty
}

void CGWnd::RenderMyself() {
    if (!IsVisible())
        return;
    // NOTE: This can trigger an endless recursion if RenderMyself is not properly overwritten
    CGWnd::Render();
}

void CGWnd::RenderMyChildren() {
    reinterpret_cast<void(__thiscall *)(CGWnd *)>(0x0089c650)(this);
}

void CGWnd::Render() {
    //reinterpret_cast<void(__thiscall *)(CGWnd *)>(0x009d1be0)(this);
    RenderMyself();
    RenderMyChildren();
    Func_15();
}

void CGWnd::Func_15() {
    // empty
}

bool CGWnd::On3DEvent_MAYBE(Event3D *a2) {
    return false;
}

//009d1f00
void CGWnd::SetHoldingMouseSlot(CGWnd *pSlot) {
    // Do We already holding the same slot?
    if (g_pMouseHoldingSlot == pSlot)
        return;
    // OnClear?
    if (g_pMouseHoldingSlot != (CGWnd *) NULL)
        g_pMouseHoldingSlot->Func_26(0);

    g_pMouseHoldingSlot = pSlot;

    if (pSlot != NULL && !pSlot->Func_25(0))
        g_pMouseHoldingSlot = NULL;
}

//009d2020
int CGWnd::GetCurrentEventMsgCtrlhgWnd() {
    if (g_pCurrentEventCtrl) {
        return g_pCurrentEventCtrl->GethgWnd();
    } else {
        return -1;
    }
}

//009d2050
int CGWnd::GetCurrentEventMsgCtrlId() {
    if (g_pCurrentEventCtrl) {
        return g_pCurrentEventCtrl->UniqueID();
    } else {
        return -1;
    }
}

void CGWnd::EraseWindowObj() {
    reinterpret_cast<void(__thiscall *)(CGWnd *)>(0x009d4230)(this);
}
