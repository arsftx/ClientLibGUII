#pragma once
#include "ifwnd.h"
class CIFTileWnd : public CIFWnd
{
    GFX_DECLARE_DYNAMIC_EXISTING(CIFTileWnd, 0x009fff54)//ecsro 

public:
	CIFTileWnd(void);
	
	~CIFTileWnd(void);

	void TB_Func_12(const char *str, int a3, int a4) override;
	void TB_Func_13(std::n_string str, int a3, int a4) override;
	
	bool OnCreate(long ln) override;
	void RenderMyself() override;
	void SetGWndSize(int width, int height) override;
	void Func_40() override;
	void OnCIFReady() override;

	virtual void Func_49(std::n_string str);

protected:
	void set_N00009B9D(bool a2);

	void sub_81AE00();

    void ReleaseTextures();

    void SetFrameTexture(std::n_string strTexture);

private:
    bool N00009B9D;    //0x02B4
    char pad_02B5[963];//0x02B5

    BEGIN_FIXTURE()
    ENSURE_SIZE(0x0678)
    END_FIXTURE()

    RUN_FIXTURE(CIFTileWnd)

}; //Size: 0x0678


