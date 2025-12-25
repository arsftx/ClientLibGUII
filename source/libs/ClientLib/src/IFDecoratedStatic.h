#pragma once

#include "IFStatic.h"

class CIFDecoratedStatic : public CIFStatic
{
	GFX_DECLARE_DYNAMIC_EXISTING(CIFDecoratedStatic, 0x009FE4E0)

public:
	bool OnCreate(long ln) override;
	void OnUpdate() override;
	void OnTimer(int) override;
	
	// Mouse event handlers - ALL must be overridden to maintain vtable order
	int OnMouseLeftUp(int a1, int x, int y) override;
	int OnMouseLeftDown(int a1, int x, int y) override;
	int OnMouseMove(int a1, int x, int y) override;

	void RenderMyself() override;

	void OnCIFReady() override;

	virtual bool Func_52();

public:
	CIFDecoratedStatic();
	virtual ~CIFDecoratedStatic();

private:
	void set_N00009BDD(char a1);
	void set_N00009C18(char a1);
protected:
	void set_N00009BD4(char a1);
	void set_N00009BD3(int a1);
	void set_N00009BD0(char a1);  // ECSRO: 0x0042DBB0 - Hover state setter
	void sub_634470(std::n_string a1);  // ECSRO: 0x0042E100
	void sub_633990();                   // ECSRO: 0x0042E2F0

private:
	char N00009BD2; //0x02C8
	char N00009C1D; //0x02C9
	char N00009C20; //0x02CA
	char pad_02CB[1]; //0x02CB
	int N00009BD4; //0x02CC
	char N00009BD0; //0x02D0
	char pad_02D1[3]; //0x02D1
	int N00009BD5; //0x02D4
	int N00009BD8; //0x02D8
	int N00009BD3; //0x02DC
	int N00009BDC; //0x02E0
	char N00009BDD; //0x02E4
	char N00009C18; //0x02E5
	char pad_02E6[2]; //0x02E6
	char RenderData[96]; //0x02E8 - render coordinate data (6 x 4 floats)
	int N00009BFC; //0x0348
	// CRITICAL: Game uses 12-byte string, not 28-byte MSVC std::n_string!
	// Using char array placeholder to match exact game size
	char N00009BFD[12]; //0x034C - 12-byte game string (ptr, end, capacity)

	// Class size must be 0x358 (856 bytes) to match game
	BEGIN_FIXTURE()
	ENSURE_SIZE(0x358)
	END_FIXTURE()

	RUN_FIXTURE(CIFDecoratedStatic)
};

