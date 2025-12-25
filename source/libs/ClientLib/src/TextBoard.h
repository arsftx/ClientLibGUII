#pragma once
#include "GFX3DFunction/GFontTexture.h"
#include <BSLib/BSLib.h>

class CTextBoard {
public:
    CTextBoard(void);
    virtual ~CTextBoard(void);

    virtual void TB_Func_1(float a2);
    virtual void TB_Func_2();
    virtual void TB_Func_3();
    virtual void TB_Func_4();
    virtual void TB_Func_5(int a2);
    virtual void TB_Func_6(int a2);
    virtual void TB_Func_7();
    virtual char TB_Func_8();
    virtual void TB_Func_9(char a2);
    virtual void TB_Func_10();
    virtual void TB_Func_11();
    virtual void TB_Func_12(const char *str, int a3, int a4);
    virtual void TB_Func_13(std::n_string str, int a3, int a4);
    virtual void TB_Func_14(std::string str, int a3, int a4);
    virtual void TB_Func_15(int a2);
    virtual void TB_Func_16();


    /// \address 005343c0
    void SetFont(void *a2);

    char GetN00009BB9() const;

    CGFontTexture *GetFontTexture();

protected:
    void sub_655B00(const wchar_t *str);

    bool sub_655420(int a2);
    void sub_655770(std::n_string);

    void sub_655CA0(int a2);
    void sub_6553A0(float a2);
    void sub_6554C0();

public:
    char N000096C1;                   //0x0004
    char pad_0005[3];                 //0x0005
    int N000096C2;                    //0x0008
	CGFontTexture m_FontTexture; //0x000C
protected:
	std::n_wstring m_texturestr_font; //0x007C
private:
	int N00009C2D; //0x0098
	int N00009C2E; //0x009C
protected:
	class SGFontChar* m_pInterfaceFont; //0x00A0
private:
	int N00009C30; //0x00A4
	void* m_pTexture;     //0x00A8
	void* m_pTexData1;    //0x00AC
	void* m_pTexData2;    //0x00B0
	char N00009BB9;       //0x00B4
	char pad_00B5[3];     //0x00B5
	float N0000974B;      //0x00B8
	float N0000974C;      //0x00BC
	IDirect3DBaseTexture9* m_texture_un2; //0x00C0
	std::n_string m_str_un2; //0x00C4
	int N00009757; //0x00E0
public:
	std::n_string bg_filename_maybe; //0x00E4
private:
	int N00009768; //0x0100
	char N00009769; //0x0104
	char pad_0105[3]; //0x0105

};//Size: 0x0108


