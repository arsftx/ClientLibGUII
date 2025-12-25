#include "TextBoard.h"
#include "GFX3DFunction/GFXVideo3d.h"

#include "Client.h"


CTextBoard::CTextBoard(void)
{
    reinterpret_cast<void(__thiscall *)(CTextBoard *)>(0x00446760)(this); //ECSRO
}


// CTextBoard::~CTextBoard(void) .text 00655900 000001C5 00000060 00000000 R . . . . . .
CTextBoard::~CTextBoard(void)
{
    reinterpret_cast<void(__thiscall *)(CTextBoard *)>(0x00446920)(this); //ECSRO
}

// CTextBoard::Func_1(float) .text 00655470 0000004F 00000008 00000004 R . . . . . .
void CTextBoard::TB_Func_1(float a2)
{
    reinterpret_cast<void(__thiscall *)(CTextBoard *)>(0x005345c0)(this);
}

// CTextBoard::Func_2(void) .text 00655D50 0000010B 00000004 00000000 R . . . . . .
void CTextBoard::TB_Func_2()
{
    reinterpret_cast<void(__thiscall *)(CTextBoard *)>(0x00534ea0)(this);
}

// CTextBoard::Func_3(void) .text 004F4CC0 00000001   R . . . . . .
void CTextBoard::TB_Func_3()
{
	return; // empty
}

// CTextBoard::Func_4(void) .text 004F4CD0 00000001   R . . . . . .
void CTextBoard::TB_Func_4()
{
	return; // empty
}

// CTextBoard::Func_5(void) .text 004F4CE0 00000011 00000000 00000004 R . . . . . .
void CTextBoard::TB_Func_5(int a2)
{
    //reinterpret_cast<void(__thiscall *)(CTextBoard *, int)>(0x004b5730)(this, a2);

	this->N000096C1 = a2;
	TB_Func_3();
}

// CTextBoard::Func_6(void) .text 004F4D00 00000011 00000000 00000004 R . . . . . .
void CTextBoard::TB_Func_6(int a2)
{
    //reinterpret_cast<void(__thiscall *)(CTextBoard *, int)>(0x004b5750)(this, a2);

	this->N000096C2 = a2;
	TB_Func_3();
}

// CTextBoard::Func_7(void) .text 00655AD0 00000001   R . . . . . .
void CTextBoard::TB_Func_7()
{
	return; // emtpy
}

// CTextBoard::AddControlToList(void) .text 00655AE0 00000007   R . . . . . .
char CTextBoard::TB_Func_8() {
    return reinterpret_cast<char(__thiscall *)(CTextBoard *)>(0x00534c30)(this);
}

// CTextBoard::RemoveControlFromList(void) .text 00655460 0000000D 00000000 00000001 R . . . . . .
void CTextBoard::TB_Func_9(char a2) {
    reinterpret_cast<void(__thiscall *)(CTextBoard *, char)>(0x005389e0)(this, a2);
    //N00009BB9 = a2;
}

// CTextBoard::Func_10(void) .text 004F4D20 00000001   R . . . . . .
void CTextBoard::TB_Func_10()
{
	return; // empty
}

// CTextBoard::Func_11(void) .text 00655AF0 00000001   R . . . . . .
void CTextBoard::TB_Func_11()
{
	return; // empty
}

// CTextBoard::Func_12(void) .text 00655590 000000E5 0000004C 0000000C R . . . . . .
// ECSRO: Uses 12-byte GameString struct, NOT 28-byte std::n_string!
void CTextBoard::TB_Func_12(const char *str, int a3, int a4) {
    // Game uses 12-byte string struct: { char* data, char* end, char* capacity }
    struct GameString {
        char* data;
        char* end;
        char* capacity;
    };
    
    GameString gs;
    if (str == NULL || str[0] == '\0') {
        gs.data = NULL;
        gs.end = NULL;
        gs.capacity = NULL;
    } else {
        size_t len = strlen(str);
        char* buffer = (char*)malloc(len + 1);
        strcpy(buffer, str);
        gs.data = buffer;
        gs.end = buffer + len;
        gs.capacity = buffer + len + 1;
    }
    
    // Call native ECSRO TB_Func_13 with correct 12-byte GameString
    reinterpret_cast<void(__thiscall *)(CTextBoard *, GameString, int, int)>(0x00447210)(this, gs, a3, a4);
}

// CTextBoard::Func_13(std::string const *,int,int) .text 00655680 000000E6 00000014 00000024 R . . . . T .
// ECSRO FIX: Convert std::n_string (28 bytes) to 12-byte GameString before calling native
void CTextBoard::TB_Func_13(const std::n_string str, int a3, int a4)
{
    // Game uses 12-byte string struct: { char* data, char* end, char* capacity }
    struct GameString {
        char* data;
        char* end;
        char* capacity;
    };
    
    GameString gs;
    if (str.empty()) {
        gs.data = NULL;
        gs.end = NULL;
        gs.capacity = NULL;
    } else {
        size_t len = str.length();
        char* buffer = (char*)malloc(len + 1);
        strcpy(buffer, str.c_str());
        gs.data = buffer;
        gs.end = buffer + len;
        gs.capacity = buffer + len + 1;
    }
    
    // Call native ECSRO function with correct 12-byte GameString
    reinterpret_cast<void(__thiscall *)(CTextBoard *, GameString, int, int)>(0x00447210)(this, gs, a3, a4);
}

// CTextBoard::Func_14(void) .text 00655820 000000E0 00000014 00000024 R . . . . T .
void CTextBoard::TB_Func_14(const std::string str, int a3, int a4)
{
    reinterpret_cast<void(__thiscall *)(CTextBoard *, const std::string, int, int)>(0x00447210)(this, str, a3, a4);
}

// CTextBoard::Func_15(void) .text 00655510 00000058 00000004 00000004 R . . . . . .
void CTextBoard::TB_Func_15(int a2)
{
	assert(FALSE);
}

// CTextBoard::Func_16(void) .text 00655570 0000001E 00000004 00000000 R . . . . . .
void CTextBoard::TB_Func_16() {
    reinterpret_cast<void(__thiscall *)(CTextBoard *)>(0x005346c0)(this);
    //bg_filename_maybe.clear();
    //N00009757 = 0;
    //N00009769 = 0;
}

// CTextBoard::sub_655B00(wchar_t const *) .text 00655B00 0000004E 00000004 00000004 R . . . . . .
void CTextBoard::sub_655B00(const wchar_t* str)
{
    reinterpret_cast<void(__thiscall *)(CTextBoard *, const wchar_t *)>(0x00534c50)(this, str);
	//if (str)
	//{
	//	m_texturestr_font = str;
	//	m_FontTexture.sub_8B3B60(&m_texturestr_font);
	//}
	//else
	//{
	//	m_texturestr_font.clear();
	//	m_FontTexture.sub_8B37A0();
	//}
}

// CTextBoard::sub_655420(int) .text 00655420 00000010 00000000 00000004 R . . . . T .
bool CTextBoard::sub_655420(int a2) {
   return reinterpret_cast<bool(__thiscall *)(CTextBoard *, int)>(0x00534570)(this, a2);
}

// CTextBoard::sub_655770(std::string) .text 00655770 000000AA 00000014 0000001C R . . . . . .
void CTextBoard::sub_655770(std::n_string a2) {
    reinterpret_cast<void(__thiscall *)(CTextBoard *, std::n_string)>(0x005348c0)(this, a2);

    //if (m_texture_un2)
    //	Fun_CacheTexture_Release(&m_str_un2);

    //if (a2.length())
    //{
    //	m_texture_un2 = Fun_CacheTexture_Create(a2);
    //	m_str_un2 = a2;
    //}
    //else
    //{
    //	m_texture_un2 = 0;
    //}
}

// CTextBoard__sub_655CA0 .text 00655CA0 000000A4 0000002C 00000004 R . . . . . .
void CTextBoard::sub_655CA0(int a2) {
    reinterpret_cast<void(__thiscall *)(CTextBoard *, int)>(0x00534df0)(this, a2);
}

// CTextBoard__sub_6553A0 .text 006553A0 0000001E 00000004 00000004 R . . . . T .
void CTextBoard::sub_6553A0(float a2) {
    reinterpret_cast<void(__thiscall *)(CTextBoard *)>(0x005344f0)(this);
}

// CTextBoard__sub_6554C0 .text 006554C0 0000004E 00000004 00000000 R . . . . . .
void CTextBoard::sub_6554C0() {
    reinterpret_cast<void(__thiscall *)(CTextBoard *)>(0x00534610)(this);
}

void CTextBoard::SetFont(void *a2) {
    reinterpret_cast<void(__thiscall *)(CTextBoard *, void *)>(0x00446C50)(this, a2); //ecsro
}

char CTextBoard::GetN00009BB9() const {
    return N00009BB9;
}

CGFontTexture* CTextBoard::GetFontTexture() {
   return &MEMUTIL_READ_BY_PTR_OFFSET(this, 0x0024, CGFontTexture);
}