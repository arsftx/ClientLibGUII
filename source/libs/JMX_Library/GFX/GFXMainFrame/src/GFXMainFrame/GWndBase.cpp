#include "GWndBase.h"
#include <BSLib/Debug.h>


GFX_IMPLEMENT_DYNAMIC_EXISTING(CGWndBase, 0x0C5DE1C)

void CGWndBase::Func_7(CGWndBase *a2) {
    BS_DEBUG_LOW(__FUNCTION__ " (%p : %s)", a2, a2->GetRuntimeClass()->m_lpszClassName);
    //N00000707.push_back(a2);
    //reinterpret_cast<void(__thiscall *)(CGWndBase *, CGWndBase *)>(0x0089e830)(this, a2);
}

//int CGWndBase::AddControlToList(CGWndBase *a2) {
//    BS_DEBUG_LOW(__FUNCTION__ " (%p : %s)", a2, a2->GetRuntimeClass()->m_lpszClassName);
//	return 0;
//    //return reinterpret_cast<int(__thiscall *)(CGWndBase *, CGWndBase *)>(0x009cefd0)(this, a2);
//}

void CGWndBase::RemoveControlFromList(CGWndBase *a2) {
    BS_DEBUG_LOW(__FUNCTION__ " (%p : %s)", a2, a2->GetRuntimeClass()->m_lpszClassName);
    reinterpret_cast<void(__thiscall *)(CGWndBase *, CGWndBase *)>(0x0089e880)(this, a2);
}

bool CGWndBase::OnCreate(long ln) {
    return 1;
}

bool CGWndBase::OnRelease() {
    return 1;
}

void CGWndBase::OnUpdate() {
    reinterpret_cast<void(__thiscall *)(CGWndBase *)>(0x0089e2f0)(this);
}

void CGWndBase::RenderMyself() {
    //reinterpret_cast<void(__thiscall *)(CGWndBase *)>(0x009cd760)(this);
    /*if (IsVisible()) {
        bool N000006F = MEMUTIL_READ_BY_PTR_OFFSET(this, 0x6f, bool);
        if (N000006F) {
            this->RenderMyself();
            this->RenderMyChildren();
        }
    }*/
}

void CGWndBase::RenderMyChildren() {
    return;
}

void CGWndBase::Func_15() {
    return;
}

void CGWndBase::Render() {
    CGWndBase::RenderMyself();
    //reinterpret_cast<void(__thiscall *)(CGWndBase *)>(0x009cd7b0)(this);
}

bool CGWndBase::On3DEvent_MAYBE(Event3D *a2) {
    return reinterpret_cast<bool(__thiscall *)(CGWndBase *, Event3D *)>(0x0089e9e0)(this, a2);
}

bool CGWndBase::Func_18(Event3D *a1) {
    return false;
}

void CGWndBase::OnWndMessage(Event3D *a1) {
    return;
}

void CGWndBase::BringToFront() {
    reinterpret_cast<void(__thiscall *)(CGWndBase *)>(0x0089F190)(this); //ECSRO
}

void CGWndBase::SetGWndSize(int width, int height) {
    CGWndBase::Func_22(width, height);
}

void CGWndBase::Func_22(int x, int y) {
    reinterpret_cast<void(__thiscall *)(CGWndBase *, int, int)>(0x0089f2e0)(this, x, y);//ECSRO
}

void CGWndBase::ShowGWnd(bool bVisible) {
    reinterpret_cast<void(__thiscall *)(CGWndBase *, bool)>(0x0089f400)(this, bVisible);//ECSRO
}

CGWndBase::CGWndBase() {
	/*this->m_lnListLockWrite = 0;
	this->m_lnListLockRead = 0;
	this->N000006F5 = 0;
	this->N000006F9 = 1;
	this->m_parentControl = 0;
	this->m_UniqueID = 0;
	this->m_hgWnd = 0;
	this->N000006FB = 0;
	this->m_iflist = 0;
	this->N00000703 = 0;
	this->N00000705 = 0;

	this->bounds.pos.x = 0;
	this->bounds.pos.y = 0;
	this->bounds.size.height = 0;
	this->bounds.size.width = 0;

	RECT rect = { 0, 0, 0, 0 };

	this->sub_B8F440(rect);*/
    reinterpret_cast<void(__thiscall *)(CGWndBase *)>(0x0089DFC0)(this); //ECSRO
}

bool CGWndBase::IsVisible() {
    return reinterpret_cast<bool(__thiscall *)(CGWndBase *)>(0x0089F430)(this); //ECSRO
}

void CGWndBase::ApplyGlobalScale(int x) {
    //reinterpret_cast<void(__cdecl *)(CGWndBase *, int)>(0x006f75f0)(this, x);
}

CGWndBase::wnd_rect CGWndBase::GetBounds() const {
	return this->bounds;
	//return MEMUTIL_READ_BY_PTR_OFFSET(this, 0x3C, CGWndBase::wnd_rect);
}

void CGWndBase::sub_9d1e60(bool b) {
    //reinterpret_cast<void(__thiscall *)(CGWndBase *, bool)>(0x009d1e60)(this, b);
}

int CGWndBase::UniqueID() const {
	return this->m_UniqueID;
}

void CGWndBase::sub_B8F440(const RECT &rect) {
	reinterpret_cast<void(__thiscall*)(CGWndBase*, const RECT&)>(0x0089f0e)(this, rect);
}

// CGWndBase::GetParentControl() .text 009cda40 00000004   R . . . . T .
CGWndBase *CGWndBase::GetParentControl() {
	return m_parentControl;
}

// CGWndBase::SetFocus_MAYBE(void) .text 00B9D9F0 00000049 00000004 00000000 R . . . . . .
void CGWndBase::SetFocus_MAYBE() {
	reinterpret_cast<void(__thiscall*)(CGWndBase*)>(0x0089c870)(this);
}



int CGWndBase::GethgWnd() {
    return 0;
}

bool CGWndBase::IsClickable() {
    return m_bClickable;
}

void CGWndBase::SetClickable(bool bState) {
    //reinterpret_cast<void(__thiscall *)(CGWndBase *, bool)>(0x009cda90)(this, bState);
}

bool CGWndBase::IsDragable() {
    return 0;
}

void CGWndBase::SetDragable(bool bState) {
    //reinterpret_cast<void(__thiscall *)(CGWndBase *, bool)>(0x009d1ee0)(this, bState);
}

//jsro dont have new interface
bool CGWndBase::SendMessageToParentDispatcher(DWORD dwMSG, DWORD lParam, DWORD wParam) {
    return 0;
}

void CGWndBase::SetPosition(int nX, int nY) {
    //reinterpret_cast<void(__thiscall *)(CGWndBase *, int, int)>(0x009cf270)(this, nX, nY);
}

std::n_list<CGWndBase*> CGWndBase::GetControlList() const const {
	return N00000707;
}
