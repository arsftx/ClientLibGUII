#include "IFStatic.h"

GFX_IMPLEMENT_DYNAMIC_EXISTING(CIFStatic, 0x009fe4e0)

CIFStatic::CIFStatic(void) {
    // Just call native constructor - let it initialize everything
    // NOTE: This will overwrite our DLL vtable, but that's OK for now
    // because the class size mismatch is the real problem we need to fix
    reinterpret_cast<void(__thiscall *)(CIFStatic *)>(0x00441340)(this);
}

CIFStatic::~CIFStatic(void) {
    // empty
}

bool CIFStatic::OnCreate(long ln) {
    return true;
}

void CIFStatic::SetTextColor(D3DCOLOR color) {
	m_texturestr_font.clear();
	m_FontTexture.SetColorFont(color);
	m_FontTexture.sub_8B4400(10, 10);
}

// CIFStatic::SetText(void *) .text 0052c960 00000029 00000004 00000004 R . . . . . .
bool CIFStatic::SetText(const char *src) {
    //return reinterpret_cast<bool(__thiscall *)(CIFStatic *, const wchar_t *)>(0x00441570)(this, src);
    if (src) {
        CIFWnd::SetText(src);
        Func_42();
        return true;
    }

    return false;
}
bool CIFStatic::SetTextOriginal(const char* src) {
	return reinterpret_cast<bool(__thiscall*)(CIFStatic*, const char*)>(0x441570)(this, src);
}

// CIFStatic::OnWndMessage - use CIFWnd's ECSRO address
void CIFStatic::OnWndMessage(Event3D *a1) {
    reinterpret_cast<void(__thiscall *)(CIFStatic *, Event3D *)>(0x00445930)(this, a1); // ECSRO
}

// CIFStatic::RenderMyself - use CIFWnd's ECSRO address
void CIFStatic::RenderMyself() {
    reinterpret_cast<void(__thiscall *)(CIFStatic *)>(0x004456D0)(this); // ECSRO
}

// CIFStatic::Func_42(void) .text 0064D9B0 0000019B 00000018 00000000 R . . . . . .
void CIFStatic::Func_42() {
    reinterpret_cast<void(__thiscall *)(CIFStatic *)>(0x00445bf0)(this);
}

// CIFStatic::Func_49(void) .text 0064D780 0000008E 00002008 00000009 R . . . . T .
bool CIFStatic::SetTextFormatted(const char *format, ...) {
	if (format) {
		CIFWnd::SetText(format);
		Func_42();
		return true;
	}
	return false;
}

// CIFStatic::Func_50(void) .text 0064D6C0 0000008E 00002008 00000009 R . . . . T .
bool CIFStatic::Func_50(const wchar_t *format, ...) {
    assert(FALSE);
    return false;
}

// CIFStatic::Func_51(void) .text 0064D750 00000029 00000004 00000004 R . . . . T .
bool CIFStatic::Func_51(const wchar_t *src) {
    assert(FALSE);
    return false;
}
