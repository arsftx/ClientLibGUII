#pragma once

#include "IFMainFrame.h"
#include "IFStatic.h"
#include "IFCheckBox.h"
#include "IFButton.h"
#include <IFNormalTile.h>
#include <IFSliderCtrl.h>
#include <IFHScroll_Option.h>
#include <IFComboBox.h>
#include "IRMManager.h"
#include <list>
#include <vector>
#include <set>
#include <map>
#include <ctime>
#include "IFSlotWithHelpPointerBased.h"

class CIFAutoPotion : public CIFMainFrame {
    GFX_DECLARE_DYNCREATE(CIFAutoPotion)
    GFX_DECLARE_MESSAGE_MAP(CIFAutoPotion)
public:
    CIFAutoPotion(void);
    ~CIFAutoPotion(void);

    bool OnCreate(long ln) override;
	void OnUpdate() override;
	int OnMouseMove(int a1, int x, int y) override;

	void CheckAndSelectSlot(CIFComboBox* belt, int fallbackSlot);
	void On_BtnClickConfirm();
	void On_BtnClickCancel();

	void On_BtnClickOpen();
	void On_BtnCheckBoxHP();
	void On_BtnCheckBoxMP();
	bool On_CheckVisiblePotion();
	bool On_SaveAutoPotion();
	bool On_LoadAutoPotion();
	void On_CheckBoxScrollBar();
	int GetItemIDBySlotIDUnderBarList(int slotID);
	void UsePotionItem();
	// Add these declarations for slider event handlers
	void OnHPSliderChanged();
	void OnMPSliderChanged();
	// Pointer-based quickbar monitoring
	void PointerBasedQuickBarStateChanges();
	// Enhanced potion finding methods
	void InitializePotionLists();
	
	// Mall potion ID checking functions
	bool IsMallPotionHP(int itemID);
	bool IsMallPotionMP(int itemID);
	
	// Static potion lists
	static std::list<int> Potion_ObjID_HP_Potion_List;
	static std::list<int> Potion_ObjID_MP_Potion_List;
	static std::list<int> Potion_ObjID_Universal_Pill_List;
	static std::list<int> Potion_ObjID_Return_Scroll_List;
	static std::list<int> Potion_ObjID_Coin_List;
	static std::list<int> Speed_Scroll_ItemID;
	
public:
	CIFStatic* m_hpSlider;
	CIFStatic* m_mpSlider;
	CIFStatic* m_lable1;
    CIFStatic *m_lable2;
	CIFStatic* m_lable3;
	CIFStatic* m_lable4;
	CIFStatic* m_lable5;
	CIFFrame* pFrameWnd1;
	CIFNormalTile* pNormalTile;
	CIFFrame* pFrameWnd2;
	CIFStatic* m_autorecovry;
	CIFStatic* m_autorecovrytext;
	CIFButton* m_confirm;
	CIFButton* m_cancle;
	
	CIFStatic* m_totalhpbelt1;
	CIFStatic* m_totalhpbelt2;
	CIFStatic* m_totalhpbelt3;
	CIFStatic* m_lablehpbelt;
	
	CIFStatic* m_lablequickslot;
	CIFStatic* m_lablehp1;
	CIFStatic* m_lablehp50;
	CIFStatic* m_lablehp100;
	
	CIFStatic* m_totalmpbelt1;
	CIFStatic* m_totalmpbelt2;
	CIFStatic* m_totalmpbelt3;
	CIFStatic* m_lablempbelt;
	
	CIFStatic* m_lablempquickslot;
	CIFStatic* m_lablemp1;
	CIFStatic* m_lablemp50;
	CIFStatic* m_lablemp100;
	
	
	CIFStatic* m_labelabnormal;
	CIFStatic* m_lableabbelt;
	
	CIFStatic* m_lableabquickslot;
	
	
	CIFStatic* m_lablepotiondelay;
	
	// Missing member variables that are used in cpp file
	CIFCheckBox* m_hp;
	CIFCheckBox* m_mp;
	CIFCheckBox* m_abnormal;
	CIFCheckBox* m_potiondelay;
	CIFComboBox* m_hpbelt;
	CIFComboBox* m_hpquickslot;
	CIFComboBox* m_mpbelt;
	CIFComboBox* m_mpquickslot;
	CIFComboBox* m_abbelt;
	CIFComboBox* m_abquickslot;
	CIFComboBox* m_abnormalDelay; // New combo box for abnormal status delay
	CIFHScroll_Option* m_hpSliderCtr;
	CIFHScroll_Option* m_mpSliderCtr;
	
	// IRM for interface loading
	CIRMManager m_IRM;
	
	// Saved values for auto potion (not affected by real-time slider changes)
	int m_savedHPSliderValue;
	int m_savedMPSliderValue;
	
	// Pointer-based quickbar monitoring variables
	static std::map<int, void*> lastKnownQuickbarState;
	static time_t lastQuickbarStateCheck;
	static const int QUICKBAR_STATE_CHECK_INTERVAL = 1; // 1 second
};

// Global frame pointer
extern CIFMainFrame* AutoPotionMainFrame;

// Global pointer to the actual CIFAutoPotion class instance (for accessing settings)
extern CIFAutoPotion* g_pCIFAutoPotion;