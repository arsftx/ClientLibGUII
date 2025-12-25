#pragma once
#include "IFFrame.h"
#include <Game.h>
#include <GlobalDataManager.h>
#include "IFWnd.h"
#include <IFScrollManager.h>
#include "IFTileWnd.h"
#include "IFEdit.h"
#include "IFStretchWnd.h"
#include "IFNormalTile.h"
class CIFMessageBox : public CIFFrame {
GFX_DECLARE_DYNAMIC_EXISTING(CIFMessageBox, 0x0CAF3C8);
public:
   void OnCreateIMPL();
   int GetItemID();
};
