#pragma once
#include <Game.h>
#include <GlobalDataManager.h>
#include "IFWnd.h"
#include "IFFrame.h"
#include <IFScrollManager.h>
#include "IFTileWnd.h"
#include "IFEdit.h"
#include "IFStretchWnd.h"
#include "IFNormalTile.h"
#include "IFButton.h"
class CIFItemMallMyInfo : public CIFWnd {
GFX_DECLARE_DYNAMIC_EXISTING(CIFItemMallMyInfo, 0x9FFA84)
public:
	bool OnCreateIMPL(long ln);
};
class CSOItemPackage {
public:
	CRefPackageItemData* GetPackageItemData() const;
	CSOItem* GetSOItem() const;

private:
	char pad_0000[0x28]; //0x0000
	std::vector<CSOItem*> m_vPSOItem; //0x0028
	CRefPackageItemData* m_pPackageItemData; //0x0038
};
class CIFItemMallConfirmBuy : public CIFFrame {
	GFX_DECLARE_DYNAMIC_EXISTING(CIFItemMallConfirmBuy, 0x9FFAA4)
public:
	bool OnCreateIMPL(long ln);
public:
	int start_0;
	CIFEdit* EditBox;
	CIFNormalTile* test;
	char pad_00[36];
	BEGIN_FIXTURE()
		ENSURE_SIZE(0x6A8)
		ENSURE_OFFSET(EditBox, 0x067C)
		END_FIXTURE()
		RUN_FIXTURE(CIFItemMallConfirmBuy)
	
};

class CIFItemMallConfirmSlot : public CIFWnd {
	GFX_DECLARE_DYNAMIC_EXISTING(CIFItemMallConfirmSlot, 0x0CB128C)
public:
	bool OnCreateIMPL(long ln);
public:

	char pad_00[0xC];
};