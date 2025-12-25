#include "IFAutoPotion.h"
#include "IFUnderBar.h"
#include "SOItem.h"
#include "IFInventory.h"
#include "IFMainPopup.h"
#include <ctime>
#include <set>
#include <map>
#include <LegendMainMenu.h>
#include "GFX3DFunction/RStateMgr.h"
#include "GInterface.h"
#include "Game.h"
#include "ICPlayer.h"
#include "unsorted.h"
#include <TextStringManager.h>
#include "../../JMX_Library/BSLib/src/BSLib/Debug.h"
#include <GameSettings.h>
#include <sys/stat.h>
#include <direct.h>
#include <algorithm>
#define GDR_CONFIRM_BUTTON 5006
#define GDR_CANCEL_BUTTON 5007
#define GDR_TEST_BUTTON 5099

GFX_IMPLEMENT_DYNCREATE(CIFAutoPotion, CIFMainFrame)
GFX_BEGIN_MESSAGE_MAP(CIFAutoPotion, CIFMainFrame)
ONG_COMMAND(GDR_CONFIRM_BUTTON, &On_BtnClickConfirm)
ONG_COMMAND(GDR_CANCEL_BUTTON, &On_BtnClickCancel)
GFX_END_MESSAGE_MAP()

// Global potion lists
std::list<int> Potion_ObjID_HP_Potion_List;
std::list<int> Potion_ObjID_MP_Potion_List;
std::list<int> Potion_ObjID_Vigor_Potion_List;
std::list<int> Potion_ObjID_Universal_Pill_List;
std::list<int> Potion_ObjID_Return_Scroll_List;
std::list<int> Potion_ObjID_Coin_List;
std::list<int> Speed_Scroll_ItemID;

// Cooldown tracking variables (if needed for future use)
time_t lastHPUseTime = 0;
time_t lastMPUseTime = 0;
time_t lastAbnormalUseTime = 0;
time_t lastVigorUseTime = 0;

// Debug message throttling variables
time_t lastUnderbarDebugTime = 0;
time_t lastSlotDebugTime = 0;
time_t lastHPDebugTime = 0;
time_t lastMPDebugTime = 0;
bool underbarDebugShown = false;
bool slotDebugShown = false;
bool hpDebugShown = false;
bool mpDebugShown = false;

// Better debug throttling - track specific slot/item combinations
static std::set<std::pair<int, int>> hpDebugPrinted; // slot, itemID
static std::set<std::pair<int, int>> mpDebugPrinted; // slot, itemID

// Global variables
CIFMainFrame* AutoPotionMainFrame = NULL;
CIFAutoPotion* g_pCIFAutoPotion = NULL;

// Add variables for periodic underbar refresh
static time_t lastUnderbarRefreshTime = 0;
static const int UNDERBAR_REFRESH_INTERVAL = 2; // Refresh every 2 seconds for more responsiveness

// Cache state tracking
static bool cacheWasEmpty = true;
static time_t lastCacheCheckTime = 0;

// Debug spam prevention variables
static std::map<int, std::pair<int, time_t>> slotDebugHistory; // slotID -> (itemID, lastDebugTime)

// Static member definitions
std::list<int> CIFAutoPotion::Potion_ObjID_HP_Potion_List;
std::list<int> CIFAutoPotion::Potion_ObjID_MP_Potion_List;
std::list<int> CIFAutoPotion::Potion_ObjID_Universal_Pill_List;
std::list<int> CIFAutoPotion::Potion_ObjID_Return_Scroll_List;
std::list<int> CIFAutoPotion::Potion_ObjID_Coin_List;
std::list<int> CIFAutoPotion::Speed_Scroll_ItemID;

// Pointer-based quickbar monitoring static variables
std::map<int, void*> CIFAutoPotion::lastKnownQuickbarState;
time_t CIFAutoPotion::lastQuickbarStateCheck = 0;

// Use the global slotItemIDUnderBarList from GameSettings.cpp
extern std::vector<std::pair<int, int>> slotItemIDUnderBarList;

std::string Get_Str(unsigned int addr) // Safe string reader
{
	if (!addr) {
		return "";
	}

	unsigned int realAddr = 0;
	if (!memcpy(&realAddr, (void*)addr, sizeof(realAddr))) {
		return "";
	}

	if (!realAddr) {
		return "";
	}

	// Safely copy from realAddr, limiting to 255 characters
	char buffer[256] = { 0 };
	strncpy(buffer, (char*)realAddr, sizeof(buffer) - 1);
	buffer[sizeof(buffer) - 1] = '\0'; // Ensure null termination

	return std::string(buffer);
}
std::string Get_PathFolderStr(unsigned int addr) //READING POINTER by DevLegendary
{
	unsigned int realAddr = *(unsigned int*)addr; // Read pointer to actual string
	char buffer[260] = { 0 };
	memcpy(buffer, (void*)realAddr, sizeof(buffer) - 1); // Read string from heap
	buffer[sizeof(buffer) - 1] = '\0'; // Ensure null termination
	return std::string(buffer);
}
std::string ConvertToString(const std::n_wstring& n_wstr) {
	if (n_wstr.empty()) return std::string();

	size_t len = std::wcstombs(NULL, n_wstr.c_str(), 0);
	if (len == static_cast<size_t>(-1)) return std::string(); // Conversion failed

	std::string result(len, '\0');
	std::wcstombs(&result[0], n_wstr.c_str(), len);
	return result;
}
bool DoesFileDatExists(const std::string& name) {
	struct stat buffer;
	return (stat(name.c_str(), &buffer) == 0);
}
std::string WideToAnsi(const std::wstring& wstr) {
	if (wstr.empty()) return "";

	int size_needed = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
	if (size_needed <= 0) return "";

	std::string result(size_needed - 1, 0); // Exclude null terminator
	WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, &result[0], size_needed, NULL, NULL);
	return result;
}
CIFAutoPotion::CIFAutoPotion(void) {
    //printf("> " __FUNCTION__ "\n");
    InitializePotionLists();
    
    // Initialize saved values with defaults
    m_savedHPSliderValue = 70;
    m_savedMPSliderValue = 30;
    
    // Store global pointer for macro window access
    g_pCIFAutoPotion = this;
}

CIFAutoPotion::~CIFAutoPotion(void) {
    //printf("> " __FUNCTION__ "\n");
}

void CIFAutoPotion::InitializePotionLists() {
    // HP Potions - SR_DevKit potion IDs (corrected)
    Potion_ObjID_HP_Potion_List.push_back(4);   // ITEM_ETC_HP_POTION_01
    Potion_ObjID_HP_Potion_List.push_back(5);   // ITEM_ETC_HP_POTION_02
    Potion_ObjID_HP_Potion_List.push_back(6);   // ITEM_ETC_HP_POTION_03
    Potion_ObjID_HP_Potion_List.push_back(7);   // ITEM_ETC_HP_POTION_04
    Potion_ObjID_HP_Potion_List.push_back(8);   // ITEM_ETC_HP_POTION_05
    Potion_ObjID_HP_Potion_List.push_back(3817); // ITEM_MALL_HP_SUPERSET_2_BAG
    Potion_ObjID_HP_Potion_List.push_back(3818); // ITEM_MALL_HP_SUPERSET_3_BAG
    Potion_ObjID_HP_Potion_List.push_back(3819); // ITEM_MALL_HP_SUPERSET_4_BAG
    Potion_ObjID_HP_Potion_List.push_back(5912); // ITEM_MALL_HP_SUPERSET_5_BAG
    Potion_ObjID_HP_Potion_List.push_back(235);  // Actual game HP mall potion ID
    Potion_ObjID_HP_Potion_List.push_back(24);   // HP Mall Potion 5 Quick Slot ID
    
    // MP Potions - SR_DevKit potion IDs (corrected)
    Potion_ObjID_MP_Potion_List.push_back(11);  // ITEM_ETC_MP_POTION_01
    Potion_ObjID_MP_Potion_List.push_back(12);  // ITEM_ETC_MP_POTION_02
    Potion_ObjID_MP_Potion_List.push_back(13);  // ITEM_ETC_MP_POTION_03
    Potion_ObjID_MP_Potion_List.push_back(14);  // ITEM_ETC_MP_POTION_04
    Potion_ObjID_MP_Potion_List.push_back(15);  // ITEM_ETC_MP_POTION_05
    Potion_ObjID_MP_Potion_List.push_back(3820); // ITEM_MALL_MP_SUPERSET_2_BAG
    Potion_ObjID_MP_Potion_List.push_back(3821); // ITEM_MALL_MP_SUPERSET_3_BAG
    Potion_ObjID_MP_Potion_List.push_back(3822); // ITEM_MALL_MP_SUPERSET_4_BAG
    Potion_ObjID_MP_Potion_List.push_back(5913); // ITEM_MALL_MP_SUPERSET_5_BAG
    Potion_ObjID_MP_Potion_List.push_back(238);  // Actual game MP mall potion ID
    Potion_ObjID_MP_Potion_List.push_back(25);   // MP Mall Potion 5 Quick Slot ID
    
    // Universal Pills (Abnormal Cure) - Status effect removal
    Potion_ObjID_Universal_Pill_List.push_back(12652);
    Potion_ObjID_Universal_Pill_List.push_back(12653);
    Potion_ObjID_Universal_Pill_List.push_back(12654);
    
    // Return Scrolls - Town portal items
    Potion_ObjID_Return_Scroll_List.push_back(61);
    Potion_ObjID_Return_Scroll_List.push_back(2198);
    Potion_ObjID_Return_Scroll_List.push_back(2199);
    Potion_ObjID_Return_Scroll_List.push_back(10382);
    Potion_ObjID_Return_Scroll_List.push_back(2128);
    Potion_ObjID_Return_Scroll_List.push_back(3769);
    Potion_ObjID_Return_Scroll_List.push_back(3795);
    
    // Coins/Tokens - Currency items
    Potion_ObjID_Coin_List.push_back(24667);
    Potion_ObjID_Coin_List.push_back(24668);
    Potion_ObjID_Coin_List.push_back(24669);
    Potion_ObjID_Coin_List.push_back(24670);
    Potion_ObjID_Coin_List.push_back(25834);
    
    // Speed Scrolls - Movement speed boost items
    Speed_Scroll_ItemID.push_back(10382);
    Speed_Scroll_ItemID.push_back(2128);
    Speed_Scroll_ItemID.push_back(3769);
    Speed_Scroll_ItemID.push_back(3795);
}

bool CIFAutoPotion::IsMallPotionHP(int itemID) {
    // Check both inventory IDs (38xx, 59xx) and quick slot IDs (23x, 24, 25)
    return (itemID == 3817 || itemID == 3818 || itemID == 3819 || itemID == 5912 || 
            itemID == 233 || itemID == 234 || itemID == 235 || itemID == 24);
}

bool CIFAutoPotion::IsMallPotionMP(int itemID) {
    // Check both inventory IDs (38xx, 59xx) and quick slot IDs (23x, 24, 25)
    return (itemID == 3820 || itemID == 3821 || itemID == 3822 || itemID == 5913 || 
            itemID == 236 || itemID == 237 || itemID == 238 || itemID == 25);
}

// These functions are no longer needed with the new implementation
// but kept for potential future use

int CIFAutoPotion::OnMouseMove(int a1, int x, int y) {
	//this->BringToFront();
	return 0;
}

// OnMouseDown removed - using message map approach instead
bool CIFAutoPotion::OnCreate(long ln) {

	RECT AutoPotionRect = { 50, 50, 394, 520 };
	AutoPotionMainFrame = (CIFMainFrame*)CGWnd::CreateInstance(g_pCGInterface, GFX_RUNTIME_CLASS(CIFMainFrame), AutoPotionRect, 2000, 0);
	AutoPotionMainFrame->OnCreate(ln);
	AutoPotionMainFrame->SetText("Auto Potion");
	AutoPotionMainFrame->TB_Func_12("interface\\frame\\mframe_wnd_", 1, 0);
	AutoPotionMainFrame->SetGWndSize(394, 580);
	wnd_rect sz;


	sz.pos.x = 15;
	sz.pos.y = 43;
	sz.size.width = 380;
	sz.size.height = 485;
	pFrameWnd1 = (CIFFrame*)CGWnd::CreateInstance(AutoPotionMainFrame, GFX_RUNTIME_CLASS(CIFFrame), sz, 5001, 0);
	pFrameWnd1->TB_Func_12("interface\\inventory\\int_window_", 1, 0);
	pFrameWnd1->SetGWndSize(364, 520);


	sz.pos.x = 30;
	sz.pos.y = 54;
	sz.size.width = 335;
	sz.size.height = 440;
	pNormalTile = static_cast<CIFNormalTile*>(CGWnd::CreateInstance(AutoPotionMainFrame, GFX_RUNTIME_CLASS(CIFNormalTile), sz, 5002, 0));
	pNormalTile->TB_Func_12("interface\\ifcommon\\bg_tile\\com_bg_tile_b.ddj", 1, 0);
	pNormalTile->SetGWndSize(335, 500);

	sz.pos.x = 28;
	sz.pos.y = 56;
	sz.size.width = 340;
	sz.size.height = 135;
	pFrameWnd2 = (CIFFrame*)CGWnd::CreateInstance(AutoPotionMainFrame, GFX_RUNTIME_CLASS(CIFFrame), sz, 5003, 0);
	pFrameWnd2->TB_Func_12("interface\\frame\\frameg_wnd_", 1, 0);
	pFrameWnd2->SetGWndSize(338, 135);


	sz.pos.x = -55;
	sz.pos.y = 70;
	sz.size.width = 100;
	sz.size.height = 24;
	m_lable1 = (CIFStatic*)CGWnd::CreateInstance(AutoPotionMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 5004, 0);

	m_lable1->SetText("Auto recovery settings");
	m_lable1->SetTextColor(D3DCOLOR_XRGB(233, 208, 148));
	m_lable1->SetGWndSize(328, 24);
	m_lable1->BringToFront();

	sz.pos.x = 36;
	sz.pos.y = 90;
	sz.size.width = 300;
	sz.size.height = 24;
	m_lable2 = (CIFStatic*)CGWnd::CreateInstance(AutoPotionMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 5005, 0);

	m_lable2->SetText("Set the % of the HP/MP gage to the preferred value, then");
	m_lable2->SetGWndSize(328, 24);
	m_lable2->BringToFront();

	sz.pos.x = 22;
	sz.pos.y = 108;
	sz.size.width = 300;
	sz.size.height = 24;
	m_lable3 = (CIFStatic*)CGWnd::CreateInstance(AutoPotionMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 5006, 0);

	m_lable3->SetText("auto potion recovery will be in use. To not use auto");
	m_lable3->SetGWndSize(328, 24);
	m_lable3->BringToFront();

	sz.pos.x = 31;
	sz.pos.y = 128;
	sz.size.width = 300;
	sz.size.height = 24;
	m_lable4 = (CIFStatic*)CGWnd::CreateInstance(AutoPotionMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 5007, 0);

	m_lable4->SetText("recovery for a certain category, remove the check from");
	m_lable4->SetGWndSize(328, 24);
	m_lable4->BringToFront();

	sz.pos.x = -77;
	sz.pos.y = 147;
	sz.size.width = 300;
	sz.size.height = 24;
	m_lable5 = (CIFStatic*)CGWnd::CreateInstance(AutoPotionMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 5008, 0);

	m_lable5->SetText("the check box");
	m_lable5->SetGWndSize(328, 24);
	m_lable5->BringToFront();


	sz.pos.x = 32;
	sz.pos.y = 219;
	sz.size.width = 16;
	sz.size.height = 16;
	m_hp = (CIFCheckBox*)CGWnd::CreateInstance(AutoPotionMainFrame, GFX_RUNTIME_CLASS(CIFCheckBox), sz, 5009, 0);
	m_hp->SetGWndSize(16, 16);
	m_hp->BringToFront();

	sz.pos.x = 58;
	sz.pos.y = 216;
	sz.size.width = 24;
	sz.size.height = 24;
	m_totalhpbelt1 = (CIFStatic*)CGWnd::CreateInstance(AutoPotionMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 5010, 0);
	m_totalhpbelt1->SetText("HP");
	m_totalhpbelt1->SetGWndSize(24, 24);


	sz.pos.x = 84;
	sz.pos.y = 218;
	sz.size.width = 28;
	sz.size.height = 20;
	m_totalhpbelt2 = (CIFStatic*)CGWnd::CreateInstance(AutoPotionMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 5011, 0);
	m_totalhpbelt2->SetText("0");
	m_totalhpbelt2->TB_Func_12("interface\\guild\\gil_bar04.ddj", 1, 0);
	m_totalhpbelt2->SetGWndSize(28, 20);



	sz.pos.x = 110;
	sz.pos.y = 216;
	sz.size.width = 24;
	sz.size.height = 24;
	m_totalhpbelt3 = (CIFStatic*)CGWnd::CreateInstance(AutoPotionMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 5012, 0);

	m_totalhpbelt3->SetText("%");
	m_totalhpbelt3->SetGWndSize(24, 24);


	sz.pos.x = 12;
	sz.pos.y = 217;
	sz.size.width = 100;
	sz.size.height = 24;
	m_lablehpbelt = (CIFStatic*)CGWnd::CreateInstance(AutoPotionMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 5013, 0);

	m_lablehpbelt->SetText("Belt");
	m_lablehpbelt->SetTextColor(D3DCOLOR_XRGB(233, 208, 148));
	m_lablehpbelt->SetGWndSize(328, 24);


	sz.pos.x = 192;
	sz.pos.y = 218;
	sz.size.width = 54;
	sz.size.height = 20;
	m_hpbelt = (CIFComboBox*)CGWnd::CreateInstance(AutoPotionMainFrame, GFX_RUNTIME_CLASS(CIFComboBox), sz, 5014, 0);
	m_hpbelt->InsertSlotToList(0, "F1");
	m_hpbelt->InsertSlotToList(1, "F2");
	m_hpbelt->InsertSlotToList(2, "F3");
	m_hpbelt->InsertSlotToList(3, "F4");
	m_hpbelt->SetGWndSize(54, 20);
	m_hpbelt->BringToFront();



	sz.pos.x = 110;
	sz.pos.y = 217;
	sz.size.width = 100;
	sz.size.height = 24;
	m_lablequickslot = (CIFStatic*)CGWnd::CreateInstance(AutoPotionMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 5015, 0);

	m_lablequickslot->SetText("Quick slot");
	m_lablequickslot->SetTextColor(D3DCOLOR_XRGB(233, 208, 148));
	m_lablequickslot->SetGWndSize(328, 24);


	sz.pos.x = 307;
	sz.pos.y = 218;
	sz.size.width = 54;
	sz.size.height = 20;
	m_hpquickslot = (CIFComboBox*)CGWnd::CreateInstance(AutoPotionMainFrame, GFX_RUNTIME_CLASS(CIFComboBox), sz, 5016, 0);
	m_hpquickslot->InsertSlotToList(0, "1");
	m_hpquickslot->InsertSlotToList(1, "2");
	m_hpquickslot->InsertSlotToList(2, "3");
	m_hpquickslot->InsertSlotToList(3, "4");
	m_hpquickslot->InsertSlotToList(4, "5");
	m_hpquickslot->InsertSlotToList(5, "6");
	m_hpquickslot->InsertSlotToList(6, "7");
	m_hpquickslot->InsertSlotToList(7, "8");
	m_hpquickslot->InsertSlotToList(8, "9");
	m_hpquickslot->InsertSlotToList(9, "0");
	m_hpquickslot->SetGWndSize(54, 20);
	m_hpquickslot->BringToFront();

	sz.pos.x = 33;
	sz.pos.y = 247;
	sz.size.width = 328;
	sz.size.height = 24;
	m_hpSlider = (CIFStatic*)CGWnd::CreateInstance(AutoPotionMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 5017, 0);
	m_hpSlider->TB_Func_12("interface\\recovery\\re_selectbar.ddj", 1, 0);
	m_hpSlider->SetGWndSize(328, 22);


	sz.pos.x = 55;
	sz.pos.y = 250;
	sz.size.width = 267;
	sz.size.height = 16;
	m_hpSliderCtr = (CIFHScroll_Option*)CGWnd::CreateInstance(AutoPotionMainFrame, GFX_RUNTIME_CLASS(CIFHScroll_Option), sz, 5018, 0);
	m_hpSliderCtr->Set2E0(100);
	m_hpSliderCtr->Set2E4(100);
	m_hpSliderCtr->Set2E8(1);
	m_hpSliderCtr->Set2F0(1);
	m_hpSliderCtr->Get2F4()->SetGWndSize(24, 20);
	m_hpSliderCtr->Get2F8()->SetGWndSize(24, 20);
	m_hpSliderCtr->SetHScrollBar(267,0,100,1);
	m_hpSliderCtr->SetGWndSize(267, 20);
	m_hpSliderCtr->BringToFront();
	m_hpSliderCtr->Get2F4()->BringToFront();
	m_hpSliderCtr->Get2F8()->BringToFront();
	m_hpSliderCtr->Get2FC()->BringToFront();

	sz.pos.x = 32;
	sz.pos.y = 270;
	sz.size.width = 24;
	sz.size.height = 24;
	m_lablehp1 = (CIFStatic*)CGWnd::CreateInstance(AutoPotionMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 5019, 0);
	m_lablehp1->SetText("1");
	m_lablehp1->SetGWndSize(16, 16);


	sz.pos.x = 185;
	sz.pos.y = 270;
	sz.size.width = 24;
	sz.size.height = 24;
	m_lablehp50 = (CIFStatic*)CGWnd::CreateInstance(AutoPotionMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 5020, 0);
	m_lablehp50->SetText("50");
	m_lablehp50->SetGWndSize(16, 16);


	sz.pos.x = 340;
	sz.pos.y = 270;
	sz.size.width = 24;
	sz.size.height = 24;
	m_lablehp100 = (CIFStatic*)CGWnd::CreateInstance(AutoPotionMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 5021, 0);
	m_lablehp100->SetText("100");
	m_lablehp100->SetGWndSize(16, 16);



	///**MP**///

	sz.pos.x = 32;
	sz.pos.y = 319;
	sz.size.width = 16;
	sz.size.height = 16;
	m_mp = (CIFCheckBox*)CGWnd::CreateInstance(AutoPotionMainFrame, GFX_RUNTIME_CLASS(CIFCheckBox), sz, 5022, 0);
	m_mp->SetGWndSize(16, 16);
	m_mp->BringToFront();

	sz.pos.x = 58;
	sz.pos.y = 316;
	sz.size.width = 24;
	sz.size.height = 24;
	m_totalmpbelt1 = (CIFStatic*)CGWnd::CreateInstance(AutoPotionMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 5023, 0);
	m_totalmpbelt1->SetText("MP");
	m_totalmpbelt1->SetGWndSize(24, 24);


	sz.pos.x = 84;
	sz.pos.y = 318;
	sz.size.width = 28;
	sz.size.height = 20;
	m_totalmpbelt2 = (CIFStatic*)CGWnd::CreateInstance(AutoPotionMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 5024, 0);
	m_totalmpbelt2->SetText("0");
	m_totalmpbelt2->TB_Func_12("interface\\guild\\gil_bar04.ddj", 1, 0);
	m_totalmpbelt2->SetGWndSize(28, 20);



	sz.pos.x = 110;
	sz.pos.y = 316;
	sz.size.width = 24;
	sz.size.height = 24;
	m_totalmpbelt3 = (CIFStatic*)CGWnd::CreateInstance(AutoPotionMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 5025, 0);

	m_totalmpbelt3->SetText("%");
	m_totalmpbelt3->SetGWndSize(24, 24);



	sz.pos.x = 12;
	sz.pos.y = 317;
	sz.size.width = 100;
	sz.size.height = 24;
	m_lablempbelt = (CIFStatic*)CGWnd::CreateInstance(AutoPotionMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 5026, 0);

	m_lablempbelt->SetText("Belt");
	m_lablempbelt->SetTextColor(D3DCOLOR_XRGB(233, 208, 148));
	m_lablempbelt->SetGWndSize(328, 24);


	sz.pos.x = 192;
	sz.pos.y = 318;
	sz.size.width = 54;
	sz.size.height = 20;
	m_mpbelt = (CIFComboBox*)CGWnd::CreateInstance(AutoPotionMainFrame, GFX_RUNTIME_CLASS(CIFComboBox), sz, 5027, 0);
	m_mpbelt->InsertSlotToList(0, "F1");
	m_mpbelt->InsertSlotToList(1, "F2");
	m_mpbelt->InsertSlotToList(2, "F3");
	m_mpbelt->InsertSlotToList(3, "F4");
	m_mpbelt->SetGWndSize(54, 20);
	m_mpbelt->BringToFront();



	sz.pos.x = 110;
	sz.pos.y = 317;
	sz.size.width = 100;
	sz.size.height = 24;
	m_lablempquickslot = (CIFStatic*)CGWnd::CreateInstance(AutoPotionMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 5028, 0);

	m_lablempquickslot->SetText("Quick slot");
	m_lablempquickslot->SetTextColor(D3DCOLOR_XRGB(233, 208, 148));
	m_lablempquickslot->SetGWndSize(328, 24);


	sz.pos.x = 307;
	sz.pos.y = 318;
	sz.size.width = 54;
	sz.size.height = 20;
	m_mpquickslot = (CIFComboBox*)CGWnd::CreateInstance(AutoPotionMainFrame, GFX_RUNTIME_CLASS(CIFComboBox), sz, 5029, 0);
	m_mpquickslot->InsertSlotToList(0, "1");
	m_mpquickslot->InsertSlotToList(1, "2");
	m_mpquickslot->InsertSlotToList(2, "3");
	m_mpquickslot->InsertSlotToList(3, "4");
	m_mpquickslot->InsertSlotToList(4, "5");
	m_mpquickslot->InsertSlotToList(5, "6");
	m_mpquickslot->InsertSlotToList(6, "7");
	m_mpquickslot->InsertSlotToList(7, "8");
	m_mpquickslot->InsertSlotToList(8, "9");
	m_mpquickslot->InsertSlotToList(9, "0");
	m_mpquickslot->SetGWndSize(54, 20);
	m_mpquickslot->BringToFront();

	sz.pos.x = 33;
	sz.pos.y = 345;
	sz.size.width = 328;
	sz.size.height = 24;
	m_mpSlider = (CIFStatic*)CGWnd::CreateInstance(AutoPotionMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 5030, 0);
	m_mpSlider->TB_Func_12("interface\\recovery\\re_selectbar.ddj", 1, 0);
	m_mpSlider->SetGWndSize(328, 22);


	sz.pos.x = 55;
	sz.pos.y = 349;
	sz.size.width = 267;
	sz.size.height = 20;
	m_mpSliderCtr = (CIFHScroll_Option*)CGWnd::CreateInstance(AutoPotionMainFrame, GFX_RUNTIME_CLASS(CIFHScroll_Option), sz, 5031, 0);
	m_mpSliderCtr->Set2E0(100);
	m_mpSliderCtr->Set2E4(100);
	m_mpSliderCtr->Set2E8(1);
	m_mpSliderCtr->Set2F0(1);
	m_mpSliderCtr->Get2F4()->SetGWndSize(24, 20);
	m_mpSliderCtr->Get2F8()->SetGWndSize(24, 20);
	m_mpSliderCtr->SetHScrollBar(267, 0, 100, 1);
	m_mpSliderCtr->SetGWndSize(267, 20);
	m_mpSliderCtr->BringToFront();
	m_mpSliderCtr->Get2F4()->BringToFront();
	m_mpSliderCtr->Get2F8()->BringToFront();
	m_mpSliderCtr->Get2FC()->BringToFront();

	sz.pos.x = 32;
	sz.pos.y = 370;
	sz.size.width = 24;
	sz.size.height = 24;
	m_lablemp1 = (CIFStatic*)CGWnd::CreateInstance(AutoPotionMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 5032, 0);
	m_lablemp1->SetText("1");
	m_lablemp1->SetGWndSize(16, 16);


	sz.pos.x = 185;
	sz.pos.y = 370;
	sz.size.width = 24;
	sz.size.height = 24;
	m_lablemp50 = (CIFStatic*)CGWnd::CreateInstance(AutoPotionMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 5033, 0);
	m_lablemp50->SetText("50");
	m_lablemp50->SetGWndSize(16, 16);


	sz.pos.x = 340;
	sz.pos.y = 370;
	sz.size.width = 24;
	sz.size.height = 24;
	m_lablemp100 = (CIFStatic*)CGWnd::CreateInstance(AutoPotionMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 5034, 0);
	m_lablemp100->SetText("100");
	m_lablemp100->SetGWndSize(16, 16);


	///**abnormal**///

	sz.pos.x = 32;
	sz.pos.y = 419;
	sz.size.width = 16;
	sz.size.height = 16;
	m_abnormal = (CIFCheckBox*)CGWnd::CreateInstance(AutoPotionMainFrame, GFX_RUNTIME_CLASS(CIFCheckBox), sz, 5035, 0);
	m_abnormal->SetGWndSize(16, 16);
	m_abnormal->BringToFront();

	sz.pos.x = 90;
	sz.pos.y = 416;
	sz.size.width = 100;
	sz.size.height = 24;
	m_labelabnormal = (CIFStatic*)CGWnd::CreateInstance(AutoPotionMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 5036, 0);
	m_labelabnormal->SetText("Abnormal status");
	m_labelabnormal->SetGWndSize(24, 24);


	sz.pos.x = 12;
	sz.pos.y = 417;
	sz.size.width = 100;
	sz.size.height = 24;
	m_lableabbelt = (CIFStatic*)CGWnd::CreateInstance(AutoPotionMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 5037, 0);

	m_lableabbelt->SetText("Belt");
	m_lableabbelt->SetTextColor(D3DCOLOR_XRGB(233, 208, 148));
	m_lableabbelt->SetGWndSize(328, 24);


	sz.pos.x = 192;
	sz.pos.y = 418;
	sz.size.width = 54;
	sz.size.height = 20;
	m_abbelt = (CIFComboBox*)CGWnd::CreateInstance(AutoPotionMainFrame, GFX_RUNTIME_CLASS(CIFComboBox), sz, 5038, 0);
	m_abbelt->InsertSlotToList(0, "F1");
	m_abbelt->InsertSlotToList(1, "F2");
	m_abbelt->InsertSlotToList(2, "F3");
	m_abbelt->InsertSlotToList(3, "F4");
	m_abbelt->SetGWndSize(54, 20);
	m_abbelt->BringToFront();



	sz.pos.x = 110;
	sz.pos.y = 417;
	sz.size.width = 100;
	sz.size.height = 24;
	m_lableabquickslot = (CIFStatic*)CGWnd::CreateInstance(AutoPotionMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 5039, 0);

	m_lableabquickslot->SetText("Quick slot");
	m_lableabquickslot->SetTextColor(D3DCOLOR_XRGB(233, 208, 148));
	m_lableabquickslot->SetGWndSize(328, 24);


	sz.pos.x = 307;
	sz.pos.y = 418;
	sz.size.width = 54;
	sz.size.height = 20;
	m_abquickslot = (CIFComboBox*)CGWnd::CreateInstance(AutoPotionMainFrame, GFX_RUNTIME_CLASS(CIFComboBox), sz, 5040, 0);
	m_abquickslot->InsertSlotToList(0, "1");
	m_abquickslot->InsertSlotToList(1, "2");
	m_abquickslot->InsertSlotToList(2, "3");
	m_abquickslot->InsertSlotToList(3, "4");
	m_abquickslot->InsertSlotToList(4, "5");
	m_abquickslot->InsertSlotToList(5, "6");
	m_abquickslot->InsertSlotToList(6, "7");
	m_abquickslot->InsertSlotToList(7, "8");
	m_abquickslot->InsertSlotToList(8, "9");
	m_abquickslot->InsertSlotToList(9, "0");
	m_abquickslot->SetGWndSize(54, 20);
	m_abquickslot->BringToFront();

	// Abnormal status delay combo box - positioned at same horizontal level as potion use delay text
	sz.pos.x = 307;
	sz.pos.y = 466;
	sz.size.width = 54;
	sz.size.height = 20;
	m_abnormalDelay = (CIFComboBox*)CGWnd::CreateInstance(AutoPotionMainFrame, GFX_RUNTIME_CLASS(CIFComboBox), sz, 5043, 0);
	m_abnormalDelay->InsertSlotToList(0, "1.0");
	m_abnormalDelay->SetGWndSize(54, 20);
	m_abnormalDelay->BringToFront();
	m_abnormalDelay->SelectSlotList(0); // Set default selection to "1.0"


	sz.pos.x = 32;
	sz.pos.y = 469;
	sz.size.width = 16;
	sz.size.height = 16;
	m_potiondelay = (CIFCheckBox*)CGWnd::CreateInstance(AutoPotionMainFrame, GFX_RUNTIME_CLASS(CIFCheckBox), sz, 5044, 0);
	m_potiondelay->SetGWndSize(16, 16);
	m_potiondelay->BringToFront();

	sz.pos.x = 90;
	sz.pos.y = 466;
	sz.size.width = 24;
	sz.size.height = 24;
	m_lablepotiondelay = (CIFStatic*)CGWnd::CreateInstance(AutoPotionMainFrame, GFX_RUNTIME_CLASS(CIFStatic), sz, 5045, 0);
	m_lablepotiondelay->SetText("Potion use delay");
	m_lablepotiondelay->SetGWndSize(24, 24);




	sz.pos.x = 115;
	sz.pos.y = 520;
	sz.size.width = 76;
	sz.size.height = 24;
	m_confirm = static_cast<CIFButton*>(CGWnd::CreateInstance(AutoPotionMainFrame, GFX_RUNTIME_CLASS(CIFButton), sz, 5006, 0));
	m_confirm->SetText("Confirm");
	m_confirm->TB_Func_12("interface\\ifcommon\\com_button.ddj", 1, 0);
	m_confirm->BringToFront();
	m_confirm->SetGWndSize(76, 24);


	sz.pos.x = 204;
	sz.pos.y = 520;
	sz.size.width = 76;
	sz.size.height = 24;
	m_cancle = static_cast<CIFButton*>(CGWnd::CreateInstance(AutoPotionMainFrame, GFX_RUNTIME_CLASS(CIFButton), sz, 5007, 0));
	m_cancle->SetText("Cancel");
	m_cancle->TB_Func_12("interface\\ifcommon\\com_button.ddj", 1, 0);
	m_cancle->BringToFront();
	m_cancle->SetGWndSize(76, 24);
	HPSliderAutoPotion = false;
	MPSliderAutoPotion = false;
	if (m_hp) m_hp->FUN_00656d50(false);
	if (m_mp) m_mp->FUN_00656d50(false);
	if (m_abnormal) m_abnormal->FUN_00656d50(false);
	if (m_potiondelay) m_potiondelay->FUN_00656d50(false);
	if (AutoPotionMainFrame) {
		AutoPotionMainFrame->ShowGWnd(false);
	}
	On_LoadAutoPotion();
	On_CheckBoxScrollBar(); // Initialize slider states once
	
	// Initialize saved values with current slider values
	m_savedHPSliderValue = m_hpSliderCtr ? m_hpSliderCtr->Get2EC() : 70;
	m_savedMPSliderValue = m_mpSliderCtr ? m_mpSliderCtr->Get2EC() : 30;
	
	// Force cache refresh like when T screen is opened
	// printf("[AutoPotion-Init] Forcing cache refresh for immediate potion usage...\n");
	if (g_pCGInterface && g_pCGInterface->GetUnderBar()) {
		// Clear existing cache first
		slotItemIDUnderBarList.clear();
		// Then reload underbar data to populate cache properly
		g_pCGInterface->GetUnderBar()->LoadUnderbarData();
		// printf("[AutoPotion-Init] Cache refresh completed! Found %d items\n", slotItemIDUnderBarList.size());
	}
	
	// DEBUG mesajları devre dışı - PetAutoPotion için gerek yok
	// printf("[DEBUG] OnCreate completed successfully\n");
	// printf("[DEBUG] All buttons created: Confirm=%p, Cancel=%p\n", m_confirm, m_cancle);
	return true;
}


void CIFAutoPotion::OnUpdate() {
    if (!g_pCGInterface || !g_pCICPlayerEcsro) return;

    // Update checkbox-slider synchronization
    On_CheckBoxScrollBar();

    // Pointer-based quickbar state monitoring
    PointerBasedQuickBarStateChanges();

    // Slider değerleri değiştikçe üstteki yüzde metnini güncelle
    static int lastHPSliderValue = -1;
    static int lastMPSliderValue = -1;
    if (m_hpSliderCtr && m_totalhpbelt2) {
        int currentHP = m_hpSliderCtr->Get2EC();
        if (currentHP != lastHPSliderValue) {
            m_totalhpbelt2->SetTextFormatted("%d", currentHP);
            lastHPSliderValue = currentHP;
            // printf("[DEBUG] HP Slider value updated to: %d\n", currentHP);
        }
    }
    if (m_mpSliderCtr && m_totalmpbelt2) {
        int currentMP = m_mpSliderCtr->Get2EC();
        if (currentMP != lastMPSliderValue) {
            m_totalmpbelt2->SetTextFormatted("%d", currentMP);
            lastMPSliderValue = currentMP;
            // printf("[DEBUG] MP Slider value updated to: %d\n", currentMP);
        }
    }

    // Periodic underbar data refresh for responsiveness
    time_t currentTime = time(NULL);
    if (currentTime - lastUnderbarRefreshTime >= UNDERBAR_REFRESH_INTERVAL) {
        // Clear debug flags periodically to allow fresh detection
        lastUnderbarRefreshTime = currentTime;
    }

    // HP Potion Logic
    int CurrentHP = g_pCICPlayerEcsro->RemaingHP;
    if (m_hp && m_hp->GetCheckedState_MAYBE() && CurrentHP > 0) {
        CIFInventory* inventory = g_pCGInterface->GetMainPopup()->GetInventory();
        if (inventory) {
            int invCount = *(unsigned char*)((DWORD_PTR)g_pCICPlayer + 0x13B4);
            int MaxHP = g_pCICPlayerEcsro->MaxHP;
            int ConsumeHP = (MaxHP * m_savedHPSliderValue) / 100;
            
            int GetFKey = m_hpbelt ? m_hpbelt->GetSelectedSlotId() : 0;
            int GetSlotKey = m_hpquickslot ? m_hpquickslot->GetSelectedSlotId() : 0;
            int CorrectSlotUnderBar = 0;

            if (GetFKey == 0) CorrectSlotUnderBar = GetSlotKey;
            else if (GetFKey == 1) CorrectSlotUnderBar = GetSlotKey + 10;
            else if (GetFKey == 2) CorrectSlotUnderBar = GetSlotKey + 20;
            else if (GetFKey == 3) CorrectSlotUnderBar = GetSlotKey + 30;

            // SADECE POINTER-BASED CHECK: Slot'ta item var mı?
            bool slotHasItem = false;
            if (lastKnownQuickbarState.find(CorrectSlotUnderBar) != lastKnownQuickbarState.end()) {
                slotHasItem = (lastKnownQuickbarState[CorrectSlotUnderBar] != NULL);
            }
            
            if (!slotHasItem) {
                // Slot boş, HP potion kullanma ama MP ve Abnormal için devam et
                goto mp_potion_logic; // HP potion logic'ini atla, MP ve Abnormal için devam et
            }

            // Get the proper item ID with mall potion mapping (sadece mapping için)
            int GetItemIDCorrectSlotHP = GetItemIDBySlotIDUnderBarList(CorrectSlotUnderBar);
            
            // Debug throttling for mapping results
            static std::map<int, std::pair<int, time_t>> hpMappingDebugHistory;
            time_t currentTime = time(NULL);
            bool shouldDebugHP = false;
            
            std::map<int, std::pair<int, time_t>>::iterator itHP = hpMappingDebugHistory.find(CorrectSlotUnderBar);
            if (itHP == hpMappingDebugHistory.end()) {
                shouldDebugHP = true;
                hpMappingDebugHistory[CorrectSlotUnderBar] = std::make_pair(GetItemIDCorrectSlotHP, currentTime);
            } else if (itHP->second.first != GetItemIDCorrectSlotHP || (currentTime - itHP->second.second) > 30) {
                shouldDebugHP = true;
                itHP->second = std::make_pair(GetItemIDCorrectSlotHP, currentTime);
            }
            
            if (shouldDebugHP) {
                			// printf("[AutoPotion-HP] Slot %d mapping result: ItemID = %d\n", CorrectSlotUnderBar, GetItemIDCorrectSlotHP);
            }

            if (CurrentHP <= ConsumeHP && CurrentHP > 0) {
                int lowestQuantity = INT_MAX;
                int lowestSlotID = -1;
                int lastSlotWithSameItemID = -1;
                
                // Loop through inventory to find the lowest quantity item
                for (int i = 0; i < invCount; i++) {
                    CSOItem* pItem = inventory->GetItemBySlot(i);
                    if (pItem) {
                        int ItemIDInventory = pItem->m_refObjItemId;
                        int ItemIDQuantity = pItem->m_itemQuantity;

                        // Ignore empty slots (Item ID == 0)
                        if (ItemIDInventory == 0) continue;
                        
                        // Check if the item matches the underbar item (including mall potion ID variations)
                        bool isMatchingHP = false;
                        int targetHPID = GetItemIDCorrectSlotHP;
                        
                        // Check exact match first
                        if (targetHPID == ItemIDInventory) {
                            isMatchingHP = true;
                        }
                        // Removed broad mall potion matching to ensure the specific potion type (e.g. Large vs Small) is used
                        // Check mall potion ID variations (both 38xx and 23x IDs)
                        // else if (IsMallPotionHP(targetHPID) && IsMallPotionHP(ItemIDInventory)) {
                        //    isMatchingHP = true;
                        // }
                        
                        if (isMatchingHP) {
                            // Track the last slot where this item was found
                            lastSlotWithSameItemID = i;

                            // Check for the lowest quantity item
                            if (ItemIDQuantity < lowestQuantity) {
                                lowestQuantity = ItemIDQuantity;
                                lowestSlotID = i;
                            }
                        }
                    }
                }

                // Use the lowest quantity item first, if found
                int finalSlotID = (lowestSlotID != -1) ? lowestSlotID : lastSlotWithSameItemID;
                if (finalSlotID != -1) {
                    g_pCGInterface->UseItem(finalSlotID, -1, -1);
                }
            }
        }
    }
    
    // MP Potion Logic
mp_potion_logic:
    int CurrentMP = g_pCICPlayerEcsro->RemaingMP;
    if (m_mp && m_mp->GetCheckedState_MAYBE() && CurrentMP > 0) {
        CIFInventory* inventory = g_pCGInterface->GetMainPopup()->GetInventory();
        if (inventory) {
            int invCount = *(unsigned char*)((DWORD_PTR)g_pCICPlayer + 0x13B4);
            int MaxMP = g_pCICPlayerEcsro->MaxMP;
            int ConsumeMP = (MaxMP * m_savedMPSliderValue) / 100;
            
            int GetFKey = m_mpbelt ? m_mpbelt->GetSelectedSlotId() : 0;
            int GetSlotKey = m_mpquickslot ? m_mpquickslot->GetSelectedSlotId() : 0;
            int CorrectSlotUnderBar = 0;

            if (GetFKey == 0) CorrectSlotUnderBar = GetSlotKey;
            else if (GetFKey == 1) CorrectSlotUnderBar = GetSlotKey + 10;
            else if (GetFKey == 2) CorrectSlotUnderBar = GetSlotKey + 20;
            else if (GetFKey == 3) CorrectSlotUnderBar = GetSlotKey + 30;

            // SADECE POINTER-BASED CHECK: Slot'ta item var mı?
            bool slotHasItem = false;
            if (lastKnownQuickbarState.find(CorrectSlotUnderBar) != lastKnownQuickbarState.end()) {
                slotHasItem = (lastKnownQuickbarState[CorrectSlotUnderBar] != NULL);
            }
            
            if (!slotHasItem) {
                // Slot boş, MP potion kullanma ama Abnormal için devam et
                goto abnormal_logic; // MP potion logic'ini atla, Abnormal için devam et
            }

            // Get the proper item ID with mall potion mapping (sadece mapping için)
            int GetItemIDCorrectSlotMP = GetItemIDBySlotIDUnderBarList(CorrectSlotUnderBar);
            
            // Debug throttling for mapping results
            static std::map<int, std::pair<int, time_t>> mpMappingDebugHistory;
            time_t currentTime = time(NULL);
            bool shouldDebugMP = false;
            
            std::map<int, std::pair<int, time_t>>::iterator itMP = mpMappingDebugHistory.find(CorrectSlotUnderBar);
            if (itMP == mpMappingDebugHistory.end()) {
                shouldDebugMP = true;
                mpMappingDebugHistory[CorrectSlotUnderBar] = std::make_pair(GetItemIDCorrectSlotMP, currentTime);
            } else if (itMP->second.first != GetItemIDCorrectSlotMP || (currentTime - itMP->second.second) > 30) {
                shouldDebugMP = true;
                itMP->second = std::make_pair(GetItemIDCorrectSlotMP, currentTime);
            }
            
            if (shouldDebugMP) {
                			// printf("[AutoPotion-MP] Slot %d mapping result: ItemID = %d\n", CorrectSlotUnderBar, GetItemIDCorrectSlotMP);
            }

            if (CurrentMP <= ConsumeMP && CurrentMP > 0) {
                int lowestQuantity = INT_MAX;
                int lowestSlotID = -1;
                int lastSlotWithSameItemID = -1;
                
                // Loop through inventory to find the lowest quantity item
                for (int i = 0; i < invCount; i++) {
                    CSOItem* pItem = inventory->GetItemBySlot(i);
                    if (pItem) {
                        int ItemIDInventory = pItem->m_refObjItemId;
                        int ItemIDQuantity = pItem->m_itemQuantity;

                        // Ignore empty slots (Item ID == 0)
                        if (ItemIDInventory == 0) continue;

                        // Check if the item matches the underbar item (including mall potion ID variations)
                        bool isMatchingMP = false;
                        int targetMPID = GetItemIDCorrectSlotMP;
                        
                        // Check exact match first
                        if (targetMPID == ItemIDInventory) {
                            isMatchingMP = true;
                        }
                        // Removed broad mall potion matching to ensure the specific potion type (e.g. Large vs Small) is used
                        // Check mall potion ID variations (both 38xx and 23x IDs)
                        // else if (IsMallPotionMP(targetMPID) && IsMallPotionMP(ItemIDInventory)) {
                        //    isMatchingMP = true;
                        // }
                        
                        if (isMatchingMP) {
                            // Track the last slot where this item was found
                            lastSlotWithSameItemID = i;

                            // Check for the lowest quantity item
                            if (ItemIDQuantity < lowestQuantity) {
                                lowestQuantity = ItemIDQuantity;
                                lowestSlotID = i;
                            }
                        }
                    }
                }

                // Use the lowest quantity item first, if found
                int finalSlotID = (lowestSlotID != -1) ? lowestSlotID : lastSlotWithSameItemID;
                if (finalSlotID != -1) {
                    g_pCGInterface->UseItem(finalSlotID, -1, -1);
                }
            }
        }
    }
    
    // Abnormal State Logic
abnormal_logic:
    int AbnormalState = g_pCICPlayerEcsro->AbnormalState;
    if (m_abnormal && m_abnormal->GetCheckedState_MAYBE() && AbnormalState > 0 && CurrentHP > 0) {
        CIFInventory* inventory = g_pCGInterface->GetMainPopup()->GetInventory();
        if (inventory) {
            int invCount = *(unsigned char*)((DWORD_PTR)g_pCICPlayer + 0x13B4);

            int GetFKey = m_abbelt ? m_abbelt->GetSelectedSlotId() : 0;
            int GetSlotKey = m_abquickslot ? m_abquickslot->GetSelectedSlotId() : 0;
            int CorrectSlotUnderBar = 0;

            if (GetFKey == 0) CorrectSlotUnderBar = GetSlotKey;
            else if (GetFKey == 1) CorrectSlotUnderBar = GetSlotKey + 10;
            else if (GetFKey == 2) CorrectSlotUnderBar = GetSlotKey + 20;
            else if (GetFKey == 3) CorrectSlotUnderBar = GetSlotKey + 30;

            // SADECE POINTER-BASED CHECK: Slot'ta item var mı?
            bool slotHasItem = false;
            if (lastKnownQuickbarState.find(CorrectSlotUnderBar) != lastKnownQuickbarState.end()) {
                slotHasItem = (lastKnownQuickbarState[CorrectSlotUnderBar] != NULL);
            }
            
            if (!slotHasItem) {
                // Slot boş, abnormal potion kullanma
                return; // Abnormal potion logic'ini atla
            }

            int GetItemIDCorrectSlotAB = GetItemIDBySlotIDUnderBarList(CorrectSlotUnderBar);
            
            // Debug throttling for mapping results
            static std::map<int, std::pair<int, time_t>> abMappingDebugHistory;
            time_t currentTime = time(NULL);
            bool shouldDebugAB = false;
            
            std::map<int, std::pair<int, time_t>>::iterator itAB = abMappingDebugHistory.find(CorrectSlotUnderBar);
            if (itAB == abMappingDebugHistory.end()) {
                shouldDebugAB = true;
                abMappingDebugHistory[CorrectSlotUnderBar] = std::make_pair(GetItemIDCorrectSlotAB, currentTime);
            } else if (itAB->second.first != GetItemIDCorrectSlotAB || (currentTime - itAB->second.second) > 30) {
                shouldDebugAB = true;
                itAB->second = std::make_pair(GetItemIDCorrectSlotAB, currentTime);
            }
            
            if (shouldDebugAB) {
                			// printf("[AutoPotion-AB] Slot %d mapping result: ItemID = %d\n", CorrectSlotUnderBar, GetItemIDCorrectSlotAB);
            }

            int lowestQuantity = INT_MAX;
            int lowestSlotID = -1;
            int lastSlotWithSameItemID = -1;

            // Loop through inventory to find the lowest quantity item
            for (int i = 0; i < invCount; i++) {
                CSOItem* pItem = inventory->GetItemBySlot(i);
                if (pItem) {
                    int ItemIDInventory = pItem->m_refObjItemId;
                    int ItemIDQuantity = pItem->m_itemQuantity;

                    // Ignore empty slots (Item ID == 0)
                    if (ItemIDInventory == 0) continue;

                    // Check if the item matches the underbar item
                    if (GetItemIDCorrectSlotAB == ItemIDInventory) {
                        // Track the last slot where this item was found
                        lastSlotWithSameItemID = i;

                        // Check for the lowest quantity item
                        if (ItemIDQuantity < lowestQuantity) {
                            lowestQuantity = ItemIDQuantity;
                            lowestSlotID = i;
                        }
                    }
                }
            }

            // Use the lowest quantity item first, if found
            int finalSlotID = (lowestSlotID != -1) ? lowestSlotID : lastSlotWithSameItemID;
            if (finalSlotID != -1) {
                g_pCGInterface->UseItem(finalSlotID, -1, -1);
            }
        }
    }
}

void CIFAutoPotion::CheckAndSelectSlot(CIFComboBox* belt, int fallbackSlot)
{
	//handler by devlegendary
	if (belt != NULL)
	{
		int selectedSlot = belt->GetSelectedSlotId();

		if (selectedSlot == -1)
		{
			belt->SelectSlotList(fallbackSlot); 
		}
	}
}
void CIFAutoPotion::On_BtnClickConfirm()
{
	// printf("[DEBUG] Confirm button clicked!\n");
	
	// Update saved values with current slider values
	if (m_hpSliderCtr) {
		m_savedHPSliderValue = m_hpSliderCtr->Get2EC();
		// printf("[DEBUG] HP Slider value saved: %d\n", m_savedHPSliderValue);
	}
	if (m_mpSliderCtr) {
		m_savedMPSliderValue = m_mpSliderCtr->Get2EC();
		// printf("[DEBUG] MP Slider value saved: %d\n", m_savedMPSliderValue);
	}
	
	g_pCGInterface->GetUnderBar()->LoadUnderbarData();
	On_SaveAutoPotion();
	if (AutoPotionMainFrame) {
		AutoPotionMainFrame->OnCloseWnd();
	}
}

void CIFAutoPotion::On_BtnClickCancel()
{
	// printf("[DEBUG] Cancel button clicked!\n");
	// printf("[DEBUG] Cancel button handler called successfully!\n");
	g_pCGInterface->GetUnderBar()->LoadUnderbarData();
	// Don't reload settings on cancel, just close the window
	if (AutoPotionMainFrame) {
		AutoPotionMainFrame->OnClick_Exit();
	}
}



void CIFAutoPotion::On_BtnClickOpen()
{
	if (AutoPotionMainFrame) {
		On_LoadAutoPotion(); // Load settings when opening GUI
		AutoPotionMainFrame->ShowGWnd(true);
	} else {
		// printf("AutoPotion: AutoPotionMainFrame is null, cannot open GUI\n");
	}
}

void CIFAutoPotion::On_BtnCheckBoxHP()
{
	m_hpSliderCtr->Set2EC(70);
}

void CIFAutoPotion::On_BtnCheckBoxMP()
{
	m_hpSliderCtr->Set2EC(10);
}

bool CIFAutoPotion::On_CheckVisiblePotion()
{
	if (AutoPotionMainFrame) {
		return AutoPotionMainFrame->IsVisible();
	}
	return false;
}
std::string GetExecutableDirectory() {
	char buffer[MAX_PATH];
	GetModuleFileNameA(NULL, buffer, MAX_PATH);
	std::string exePath(buffer);
	size_t lastSlash = exePath.find_last_of("\\/");
	return exePath.substr(0, lastSlash + 1);
}
bool CIFAutoPotion::On_SaveAutoPotion()
{
	std::string executableDir = GetExecutableDirectory();
	
	// Server name check removed as it relied on a hardcoded address (0x0A00D48) 
    // and the result was not used in the filename anyway.
	/*
	std::string ServerName = Get_Str(0x0A00D48);

	size_t pos = ServerName.find(':');
	std::string serverPart = ServerName.substr(0, pos);

	if (serverPart.empty()) {
		printf("Error: Invalid server name format! %s\n", serverPart.c_str());
		return false;
	}
	*/

	char buffer[MAX_PATH];
	sprintf_s(buffer, sizeof(buffer), "%s\\Setting\\AutoPtion_%s_QS.dat",
		executableDir.c_str(), g_pCICPlayerEcsro->charname);
		

	if (!DoesFileDatExists(buffer)) {
		printf("Creating new AutoPotion settings file: %s\n", buffer);
	}


	FILE* ioFile = fopen(buffer, "wb+");
	if (!ioFile) {
		perror("Error opening file");
		return false;
	}

	bool dataBool[4] = {
		static_cast<bool>(m_hp ? m_hp->GetCheckedState_MAYBE() : false),
		static_cast<bool>(m_mp ? m_mp->GetCheckedState_MAYBE() : false),
		static_cast<bool>(m_abnormal ? m_abnormal->GetCheckedState_MAYBE() : false),
		static_cast<bool>(m_potiondelay ? m_potiondelay->GetCheckedState_MAYBE() : false)
	};
	size_t writtenBool = fwrite(dataBool, 1, sizeof(dataBool), ioFile);


	int dataInt[8] = {
		m_hpbelt ? m_hpbelt->GetSelectedSlotId() : 0, 
		m_hpquickslot ? m_hpquickslot->GetSelectedSlotId() : 0,
		m_mpbelt ? m_mpbelt->GetSelectedSlotId() : 0, 
		m_mpquickslot ? m_mpquickslot->GetSelectedSlotId() : 0,
		m_abbelt ? m_abbelt->GetSelectedSlotId() : 0, 
		m_abquickslot ? m_abquickslot->GetSelectedSlotId() : 0,
		m_hpSliderCtr ? m_hpSliderCtr->Get2EC() : 70, 
		m_mpSliderCtr ? m_mpSliderCtr->Get2EC() : 30
	};
	size_t writtenInt = fwrite(dataInt, 1, sizeof(dataInt), ioFile);

	fclose(ioFile); 


	if (writtenBool != sizeof(dataBool) || writtenInt != sizeof(dataInt)) {
		printf("Failed to write all data to %s\n", buffer);
		return false;
	}

	HPSliderAutoPotion = dataBool[0];
	MPSliderAutoPotion = dataBool[1];

	printf("AutoPotion settings saved successfully: %s\n", buffer);
	return true;
}
bool CIFAutoPotion::On_LoadAutoPotion()
{
	std::string executableDir = GetExecutableDirectory();
	// std::string ServerName = Get_Str(0x0A00D48);
	// size_t pos = ServerName.find(':');

	char buffer[MAX_PATH];
	sprintf_s(buffer, MAX_PATH, "%s\\Setting\\AutoPtion_%s_QS.dat", executableDir.c_str(), g_pCICPlayerEcsro->charname);

	FILE* ioFile = fopen(buffer, "rb+");
	if (ioFile != NULL) {

		bool isCheckedHP, isCheckedMP, isCheckedAbnormal, isCheckedDelay;
		int IsHPComboBox, IsHPQuickComboBox, IsMPComboBox, IsMPQuickComboBox;
		int IsAbnormalComboBox, IsAbnormalQuickComboBox, IsHPOptionScrollBar, IsMPOptionScrollBar;

		// Read data and validate success
		if (fread(&isCheckedHP, sizeof(bool), 1, ioFile) != 1 ||
			fread(&isCheckedMP, sizeof(bool), 1, ioFile) != 1 ||
			fread(&isCheckedAbnormal, sizeof(bool), 1, ioFile) != 1 ||
			fread(&isCheckedDelay, sizeof(bool), 1, ioFile) != 1 ||
			fread(&IsHPComboBox, sizeof(int), 1, ioFile) != 1 ||
			fread(&IsHPQuickComboBox, sizeof(int), 1, ioFile) != 1 ||
			fread(&IsMPComboBox, sizeof(int), 1, ioFile) != 1 ||
			fread(&IsMPQuickComboBox, sizeof(int), 1, ioFile) != 1 ||
			fread(&IsAbnormalComboBox, sizeof(int), 1, ioFile) != 1 ||
			fread(&IsAbnormalQuickComboBox, sizeof(int), 1, ioFile) != 1 ||
			fread(&IsHPOptionScrollBar, sizeof(int), 1, ioFile) != 1 ||
			fread(&IsMPOptionScrollBar, sizeof(int), 1, ioFile) != 1)
		{
			std::cerr << "Error reading file: " << buffer << std::endl;
			fclose(ioFile);
			return false;
		}
		HPSliderAutoPotion = isCheckedHP;
		MPSliderAutoPotion = isCheckedMP;
		// Apply values to UI elements (with null checks)
		if (m_hp) m_hp->FUN_00656d50(isCheckedHP);
		if (m_mp) m_mp->FUN_00656d50(isCheckedMP);
		if (m_abnormal) m_abnormal->FUN_00656d50(isCheckedAbnormal);
		if (m_potiondelay) m_potiondelay->FUN_00656d50(isCheckedDelay);

		if (m_hpbelt) m_hpbelt->SelectSlotList(IsHPComboBox);
		if (m_hpquickslot) m_hpquickslot->SelectSlotList(IsHPQuickComboBox);
		if (m_mpbelt) m_mpbelt->SelectSlotList(IsMPComboBox);
		if (m_mpquickslot) m_mpquickslot->SelectSlotList(IsMPQuickComboBox);
		if (m_abbelt) m_abbelt->SelectSlotList(IsAbnormalComboBox);
		if (m_abquickslot) m_abquickslot->SelectSlotList(IsAbnormalQuickComboBox);
		if (m_hpSliderCtr) {
			m_hpSliderCtr->Set2EC(IsHPOptionScrollBar);
			m_hpSliderCtr->SetHCorrectScrollBar(IsHPOptionScrollBar);
		}
		if (m_mpSliderCtr) {
			m_mpSliderCtr->Set2EC(IsMPOptionScrollBar);
			m_mpSliderCtr->SetHCorrectScrollBar(IsMPOptionScrollBar);
		}

		fclose(ioFile);
		
		// printf("AutoPotion settings loaded successfully: %s\n", buffer);
		
		// Auto-populate underbar cache after loading user settings
		if (g_pCGInterface && g_pCGInterface->GetUnderBar()) {
			// printf("[AutoPotion-Cache] Auto-populating underbar cache for user settings...\n");
			g_pCGInterface->GetUnderBar()->LoadUnderbarData();
			// printf("[AutoPotion-Cache] Underbar cache populated successfully!\n");
		}
	}
	else {
		// printf("AutoPotion settings file not found, using defaults\n");
		
		// Set default values as requested:
		// HP: F1 (Belt 0), Slot 9 (Index 8)
		// MP: F1 (Belt 0), Slot 9 (Index 8)
		// Abnormal: F1 (Belt 0), Slot 1 (Index 0)
		
		if (m_hpbelt) m_hpbelt->SelectSlotList(0); // F1
		if (m_hpquickslot) m_hpquickslot->SelectSlotList(0); // Slot 9
		if (m_mpbelt) m_mpbelt->SelectSlotList(0); // F1
		if (m_mpquickslot) m_mpquickslot->SelectSlotList(0); // Slot 9
		if (m_abbelt) m_abbelt->SelectSlotList(0); // F1
		if (m_abquickslot) m_abquickslot->SelectSlotList(0); // Slot 1
		
		// Disable checkboxes by default
		if (m_hp) m_hp->FUN_00656d50(false);
		if (m_mp) m_mp->FUN_00656d50(false);
		if (m_abnormal) m_abnormal->FUN_00656d50(false);
		
		// Save these defaults to create the file
		On_SaveAutoPotion();
	}
	return true;
}


void CIFAutoPotion::On_CheckBoxScrollBar()
{
	// Update UI state and text display
	if (m_totalhpbelt2 != NULL && m_hpSliderCtr != NULL && m_hp != NULL)
	{
		if (!m_hp->GetCheckedState_MAYBE())
		{
			if (!HPSliderAutoPotion)
			{
				HPSliderAutoPotion = true;
				m_hpSlider->TB_Func_12("interface\\recovery\\re_selectbar_disable.ddj", 0, 0);
				m_hpSliderCtr->Get2F4()->ShowGWnd(false);
				m_hpSliderCtr->Get2F8()->ShowGWnd(false);
				m_hpSliderCtr->Get2FC()->ShowGWnd(false);
			}
		}
		else
		{
			if (HPSliderAutoPotion)
			{
				HPSliderAutoPotion = false;
				m_hpSlider->TB_Func_12("interface\\recovery\\re_selectbar.ddj", 0, 0);
				m_hpSliderCtr->Get2F4()->ShowGWnd(true);
				m_hpSliderCtr->Get2F8()->ShowGWnd(true);
				m_hpSliderCtr->Get2FC()->ShowGWnd(true);
				m_hpSliderCtr->Get2F4()->BringToFront();
				m_hpSliderCtr->Get2F8()->BringToFront();
				m_hpSliderCtr->Get2FC()->BringToFront();
			}
			// Update text display with current slider value
			m_totalhpbelt2->SetTextFormatted("%d", m_hpSliderCtr->Get2EC());
		}
	}
	if (m_totalmpbelt2 != NULL && m_mpSliderCtr != NULL && m_mp != NULL)
	{
		if (!m_mp->GetCheckedState_MAYBE())
		{
			if (!MPSliderAutoPotion)
			{
				MPSliderAutoPotion = true;
				m_mpSlider->TB_Func_12("interface\\recovery\\re_selectbar_disable.ddj", 0, 0);
				m_mpSliderCtr->Get2F4()->ShowGWnd(false);
				m_mpSliderCtr->Get2F8()->ShowGWnd(false);
				m_mpSliderCtr->Get2FC()->ShowGWnd(false);
			}
		}
		else
		{
			if (MPSliderAutoPotion)
			{
				MPSliderAutoPotion = false;
				m_mpSlider->TB_Func_12("interface\\recovery\\re_selectbar.ddj", 0, 0);
				m_mpSliderCtr->Get2F4()->ShowGWnd(true);
				m_mpSliderCtr->Get2F8()->ShowGWnd(true);
				m_mpSliderCtr->Get2FC()->ShowGWnd(true);
			}
			// Update text display with current slider value
			m_totalmpbelt2->SetTextFormatted("%d", m_mpSliderCtr->Get2EC());
		}
	}
}

void CIFAutoPotion::UsePotionItem()
{
    if (!g_pCICPlayer || !g_pCGInterface) {
        return;
    }
    
    // HP Potion Logic - Using pointer-based slot checking
    if (m_hp && m_hp->GetCheckedState_MAYBE()) 
    {
        // 1. Get the quickbar slot selected in your UI (e.g., F1, Slot 3 -> total slot 13)
        int GetFKey = m_hpbelt ? m_hpbelt->GetSelectedSlotId() : 0;
        int GetSlotKey = m_hpquickslot ? m_hpquickslot->GetSelectedSlotId() : 0;
        int targetSlot = 0; // Calculate the real slot index (0-40)

        if (GetFKey == 0) targetSlot = GetSlotKey;
        else if (GetFKey == 1) targetSlot = GetSlotKey + 10;
        else if (GetFKey == 2) targetSlot = GetSlotKey + 20;
        else if (GetFKey == 3) targetSlot = GetSlotKey + 30;

        // 2. SADECE POINTER-BASED CHECK: Slot'ta item var mı?
        bool slotHasItem = false;
        if (lastKnownQuickbarState.find(targetSlot) != lastKnownQuickbarState.end()) {
            slotHasItem = (lastKnownQuickbarState[targetSlot] != NULL);
            printf("[AutoPotion-HP] Slot %d pointer check: %s\n", targetSlot, slotHasItem ? "HAS ITEM" : "EMPTY");
        } else {
            printf("[AutoPotion-HP] Slot %d not found in lastKnownQuickbarState map\n", targetSlot);
        }
        
        if (!slotHasItem) {
            printf("[AutoPotion-HP] *** Slot %d is EMPTY (pointer-based) - skipping HP potion ***\n", targetSlot);
            return; // Exit early to prevent using potions from empty slots
        }

        // 3. Get the Item ID with mall potion mapping (sadece mapping için)
        int itemIDInSlot = GetItemIDBySlotIDUnderBarList(targetSlot);
        
        // Debug throttling for mapping results
        static std::map<int, std::pair<int, time_t>> usePotionHPMappingDebugHistory;
        time_t currentTime = time(NULL);
        bool shouldDebugUsePotionHP = false;
        
        std::map<int, std::pair<int, time_t>>::iterator itUsePotionHP = usePotionHPMappingDebugHistory.find(targetSlot);
        if (itUsePotionHP == usePotionHPMappingDebugHistory.end()) {
            shouldDebugUsePotionHP = true;
            usePotionHPMappingDebugHistory[targetSlot] = std::make_pair(itemIDInSlot, currentTime);
        } else if (itUsePotionHP->second.first != itemIDInSlot || (currentTime - itUsePotionHP->second.second) > 30) {
            shouldDebugUsePotionHP = true;
            itUsePotionHP->second = std::make_pair(itemIDInSlot, currentTime);
        }
        
        if (shouldDebugUsePotionHP) {
            printf("[AutoPotion-HP] Slot %d mapping result: ItemID = %d\n", targetSlot, itemIDInSlot);
        }

        // 4. Check if the item in the slot is an HP potion
        bool isHPPotion = false;
        for (std::list<int>::iterator it = Potion_ObjID_HP_Potion_List.begin(); it != Potion_ObjID_HP_Potion_List.end(); ++it) {
            if (itemIDInSlot == *it) {
                isHPPotion = true;
                break;
            }
        }
        
        // 5. Check HP and Use the Potion ONLY if all conditions are met
        int CurrentHP = g_pCICPlayerEcsro->RemaingHP;
        int MaxHP = g_pCICPlayerEcsro->MaxHP;
        int ConsumeHP = (MaxHP * m_savedHPSliderValue) / 100;

        if (isHPPotion && CurrentHP <= ConsumeHP && CurrentHP > 0)
        {
            printf("[AutoPotion-HP] *** HP POTION WILL BE USED (Slot: %d, ItemID: %d) ***\n", targetSlot, itemIDInSlot);
            
            // The slot contains a valid HP potion and the player needs health.
            // Find this item in the main inventory to use it.
            CIFInventory* inventory = g_pCGInterface->GetMainPopup()->GetInventory();
            if(inventory)
            {
                int invCount = *(unsigned char*)((DWORD_PTR)g_pCICPlayer + 0x13B4);
                int lowestQuantity = INT_MAX;
                int lowestSlotID = -1;
                int lastSlotWithSameItemID = -1;
                
                printf("[AutoPotion-HP] Scanning inventory (%d slots) for ItemID %d...\n", invCount, itemIDInSlot);
                
                // Loop through inventory to find the lowest quantity item
                for (int i = 0; i < invCount; i++) {
                    CSOItem* pItem = inventory->GetItemBySlot(i);
                    if (pItem) {
                        int ItemIDInventory = pItem->m_refObjItemId;
                        int ItemIDQuantity = pItem->m_itemQuantity;

                        // Ignore empty slots (Item ID == 0)
                        if (ItemIDInventory == 0) continue;
                        
                        // Check if the item matches the underbar item (including mall potion ID variations)
                        bool isMatchingHP = false;
                        int targetHPID = itemIDInSlot;
                        
                        // Check exact match first
                        if (targetHPID == ItemIDInventory) {
                            isMatchingHP = true;
                        }
                        // Removed broad mall potion matching to ensure the specific potion type (e.g. Large vs Small) is used
                        // Check mall potion ID variations (both 38xx and 23x IDs)
                        // else if (IsMallPotionHP(targetHPID) && IsMallPotionHP(ItemIDInventory)) {
                        //    isMatchingHP = true;
                        // }
                        
                        if (isMatchingHP) {
                            printf("[AutoPotion-HP] Found matching item in inventory slot %d: ItemID=%d, Quantity=%d\n", i, ItemIDInventory, ItemIDQuantity);
                            
                            // Track the last slot where this item was found
                            lastSlotWithSameItemID = i;

                            // Check for the lowest quantity item
                            if (ItemIDQuantity < lowestQuantity) {
                                lowestQuantity = ItemIDQuantity;
                                lowestSlotID = i;
                                printf("[AutoPotion-HP] New lowest quantity: Slot %d with %d items\n", i, ItemIDQuantity);
                            }
                        }
                    }
                }

                // Use the lowest quantity item first, if found
                int finalSlotID = (lowestSlotID != -1) ? lowestSlotID : lastSlotWithSameItemID;
                if (finalSlotID != -1) {
                    printf("[AutoPotion-HP] *** USING HP POTION from inventory slot %d ***\n", finalSlotID);
                    g_pCGInterface->UseItem(finalSlotID, -1, -1);
                } else {
                    printf("[AutoPotion-HP] *** ERROR: No matching HP potion found in inventory! ***\n");
                }
            } else {
                printf("[AutoPotion-HP] *** ERROR: Inventory is NULL! ***\n");
            }
        } else {
            printf("[AutoPotion-HP] HP Potion not used: isHPPotion=%s, needHP=%s\n", 
                   isHPPotion ? "YES" : "NO", 
                   (CurrentHP <= ConsumeHP && CurrentHP > 0) ? "YES" : "NO");
        }
    }
    
    // MP Potion Logic - Using pointer-based slot checking
    if (m_mp && m_mp->GetCheckedState_MAYBE()) 
    {
        // 1. Get the quickbar slot selected in your UI
        int GetFKey = m_mpbelt ? m_mpbelt->GetSelectedSlotId() : 0;
        int GetSlotKey = m_mpquickslot ? m_mpquickslot->GetSelectedSlotId() : 0;
        int targetSlot = 0;

        if (GetFKey == 0) targetSlot = GetSlotKey;
        else if (GetFKey == 1) targetSlot = GetSlotKey + 10;
        else if (GetFKey == 2) targetSlot = GetSlotKey + 20;
        else if (GetFKey == 3) targetSlot = GetSlotKey + 30;

        // 2. SADECE POINTER-BASED CHECK: Slot'ta item var mı?
        bool slotHasItem = false;
        if (lastKnownQuickbarState.find(targetSlot) != lastKnownQuickbarState.end()) {
            slotHasItem = (lastKnownQuickbarState[targetSlot] != NULL);
            printf("[AutoPotion-MP] Slot %d pointer check: %s\n", targetSlot, slotHasItem ? "HAS ITEM" : "EMPTY");
        } else {
            printf("[AutoPotion-MP] Slot %d not found in lastKnownQuickbarState map\n", targetSlot);
        }
        
        if (!slotHasItem) {
            printf("[AutoPotion-MP] *** Slot %d is EMPTY (pointer-based) - skipping MP potion ***\n", targetSlot);
            return; // Exit early to prevent using potions from empty slots
        }

        // 3. Get the Item ID with mall potion mapping (sadece mapping için)
        int itemIDInSlot = GetItemIDBySlotIDUnderBarList(targetSlot);
        
        // Debug throttling for mapping results
        static std::map<int, std::pair<int, time_t>> usePotionMPMappingDebugHistory;
        time_t currentTime = time(NULL);
        bool shouldDebugUsePotionMP = false;
        
        std::map<int, std::pair<int, time_t>>::iterator itUsePotionMP = usePotionMPMappingDebugHistory.find(targetSlot);
        if (itUsePotionMP == usePotionMPMappingDebugHistory.end()) {
            shouldDebugUsePotionMP = true;
            usePotionMPMappingDebugHistory[targetSlot] = std::make_pair(itemIDInSlot, currentTime);
        } else if (itUsePotionMP->second.first != itemIDInSlot || (currentTime - itUsePotionMP->second.second) > 30) {
            shouldDebugUsePotionMP = true;
            itUsePotionMP->second = std::make_pair(itemIDInSlot, currentTime);
        }
        
        if (shouldDebugUsePotionMP) {
            printf("[AutoPotion-MP] Slot %d mapping result: ItemID = %d\n", targetSlot, itemIDInSlot);
        }

        // 4. Check if the item in the slot is an MP potion
        bool isMPPotion = false;
        for (std::list<int>::iterator it = Potion_ObjID_MP_Potion_List.begin(); it != Potion_ObjID_MP_Potion_List.end(); ++it) {
            if (itemIDInSlot == *it) {
                isMPPotion = true;
                break;
            }
        }
        
        // 5. Check MP and Use the Potion ONLY if all conditions are met
        int CurrentMP = g_pCICPlayerEcsro->RemaingMP;
        int MaxMP = g_pCICPlayerEcsro->MaxMP;
        int ConsumeMP = (MaxMP * m_savedMPSliderValue) / 100;

        if (isMPPotion && CurrentMP <= ConsumeMP && CurrentMP > 0)
        {
            printf("[AutoPotion-MP] *** MP POTION WILL BE USED (Slot: %d, ItemID: %d) ***\n", targetSlot, itemIDInSlot);
            
            // The slot contains a valid MP potion and the player needs mana.
            CIFInventory* inventory = g_pCGInterface->GetMainPopup()->GetInventory();
            if(inventory)
            {
                int invCount = *(unsigned char*)((DWORD_PTR)g_pCICPlayer + 0x13B4);
                int lowestQuantity = INT_MAX;
                int lowestSlotID = -1;
                int lastSlotWithSameItemID = -1;
                
                printf("[AutoPotion-MP] Scanning inventory (%d slots) for ItemID %d...\n", invCount, itemIDInSlot);
                
                // Loop through inventory to find the lowest quantity item
                for (int i = 0; i < invCount; i++) {
                    CSOItem* pItem = inventory->GetItemBySlot(i);
                    if (pItem) {
                        int ItemIDInventory = pItem->m_refObjItemId;
                        int ItemIDQuantity = pItem->m_itemQuantity;

                        // Ignore empty slots (Item ID == 0)
                        if (ItemIDInventory == 0) continue;

                        // Check if the item matches the underbar item (including mall potion ID variations)
                        bool isMatchingMP = false;
                        int targetMPID = itemIDInSlot;
                        
                        // Check exact match first
                        if (targetMPID == ItemIDInventory) {
                            isMatchingMP = true;
                        }
                        // Removed broad mall potion matching to ensure the specific potion type (e.g. Large vs Small) is used
                        // Check mall potion ID variations (both 38xx and 23x IDs)
                        // else if (IsMallPotionMP(targetMPID) && IsMallPotionMP(ItemIDInventory)) {
                        //    isMatchingMP = true;
                        // }
                        
                        if (isMatchingMP) {
                            printf("[AutoPotion-MP] Found matching item in inventory slot %d: ItemID=%d, Quantity=%d\n", i, ItemIDInventory, ItemIDQuantity);
                            
                            // Track the last slot where this item was found
                            lastSlotWithSameItemID = i;

                            // Check for the lowest quantity item
                            if (ItemIDQuantity < lowestQuantity) {
                                lowestQuantity = ItemIDQuantity;
                                lowestSlotID = i;
                                printf("[AutoPotion-MP] New lowest quantity: Slot %d with %d items\n", i, ItemIDQuantity);
                            }
                        }
                    }
                }

                // Use the lowest quantity item first, if found
                int finalSlotID = (lowestSlotID != -1) ? lowestSlotID : lastSlotWithSameItemID;
                if (finalSlotID != -1) {
                    printf("[AutoPotion-MP] *** USING MP POTION from inventory slot %d ***\n", finalSlotID);
                    g_pCGInterface->UseItem(finalSlotID, -1, -1);
                } else {
                    printf("[AutoPotion-MP] *** ERROR: No matching MP potion found in inventory! ***\n");
                }
            } else {
                printf("[AutoPotion-MP] *** ERROR: Inventory is NULL! ***\n");
            }
        } else {
            printf("[AutoPotion-MP] MP Potion not used: isMPPotion=%s, needMP=%s\n", 
                   isMPPotion ? "YES" : "NO", 
                   (CurrentMP <= ConsumeMP && CurrentMP > 0) ? "YES" : "NO");
        }
    }
    
    // Abnormal State Logic - Using pointer-based slot checking
    int AbnormalState = g_pCICPlayerEcsro->AbnormalState;
    if (m_abnormal && m_abnormal->GetCheckedState_MAYBE() && AbnormalState > 0 && g_pCICPlayerEcsro->RemaingHP > 0) {
        // 1. Get the quickbar slot selected in your UI
        int GetFKey = m_abbelt ? m_abbelt->GetSelectedSlotId() : 0;
        int GetSlotKey = m_abquickslot ? m_abquickslot->GetSelectedSlotId() : 0;
        int targetSlot = 0;

        if (GetFKey == 0) targetSlot = GetSlotKey;
        else if (GetFKey == 1) targetSlot = GetSlotKey + 10;
        else if (GetFKey == 2) targetSlot = GetSlotKey + 20;
        else if (GetFKey == 3) targetSlot = GetSlotKey + 30;

        // 2. SADECE POINTER-BASED CHECK: Slot'ta item var mı?
        bool slotHasItem = false;
        if (lastKnownQuickbarState.find(targetSlot) != lastKnownQuickbarState.end()) {
            slotHasItem = (lastKnownQuickbarState[targetSlot] != NULL);
            printf("[AutoPotion-AB] Slot %d pointer check: %s\n", targetSlot, slotHasItem ? "HAS ITEM" : "EMPTY");
        }
        
        if (!slotHasItem) {
            printf("[AutoPotion-AB] *** Slot %d is EMPTY (pointer-based) - skipping abnormal potion ***\n", targetSlot);
            return; // Exit early to prevent using potions from empty slots
        }

        // 3. Get the Item ID with mall potion mapping (sadece mapping için)
        int itemIDInSlot = GetItemIDBySlotIDUnderBarList(targetSlot);
        
        // Debug throttling for mapping results
        static std::map<int, std::pair<int, time_t>> usePotionABMappingDebugHistory;
        time_t currentTime = time(NULL);
        bool shouldDebugUsePotionAB = false;
        
        std::map<int, std::pair<int, time_t>>::iterator itUsePotionAB = usePotionABMappingDebugHistory.find(targetSlot);
        if (itUsePotionAB == usePotionABMappingDebugHistory.end()) {
            shouldDebugUsePotionAB = true;
            usePotionABMappingDebugHistory[targetSlot] = std::make_pair(itemIDInSlot, currentTime);
        } else if (itUsePotionAB->second.first != itemIDInSlot || (currentTime - itUsePotionAB->second.second) > 30) {
            shouldDebugUsePotionAB = true;
            itUsePotionAB->second = std::make_pair(itemIDInSlot, currentTime);
        }
        
        if (shouldDebugUsePotionAB) {
            printf("[AutoPotion-AB] Slot %d mapping result: ItemID = %d\n", targetSlot, itemIDInSlot);
        }

        // 4. Check if the item in the slot is an abnormal cure item
        bool isAbnormalCure = false;
        for (std::list<int>::iterator it = Potion_ObjID_Universal_Pill_List.begin(); it != Potion_ObjID_Universal_Pill_List.end(); ++it) {
            if (itemIDInSlot == *it) {
                isAbnormalCure = true;
                break;
            }
        }
        
        // 5. Use the item if it's a valid abnormal cure
        if (isAbnormalCure)
        {
            printf("[AutoPotion-AB] *** ABNORMAL CURE WILL BE USED (Slot: %d, ItemID: %d) ***\n", targetSlot, itemIDInSlot);
            
            CIFInventory* inventory = g_pCGInterface->GetMainPopup()->GetInventory();
            if(inventory)
            {
                int invCount = *(unsigned char*)((DWORD_PTR)g_pCICPlayer + 0x13B4);
                for (int i = 0; i < invCount; i++) {
                    CSOItem* pItem = inventory->GetItemBySlot(i);
                    if (pItem && pItem->m_refObjItemId == itemIDInSlot) {
                        printf("[AutoPotion-AB] *** USING ABNORMAL CURE from inventory slot %d ***\n", i);
                        g_pCGInterface->UseItem(i, -1, -1);
                        break;
                    }
                }
            }
        } else {
            printf("[AutoPotion-AB] Abnormal cure not used: isAbnormalCure=%s\n", isAbnormalCure ? "YES" : "NO");
        }
    }
}


int CIFAutoPotion::GetItemIDBySlotIDUnderBarList(int slotID) {
    time_t currentTime = time(NULL);
    
    // Always check the current underbar state first - this is the most reliable method
    if (g_pCGInterface && g_pCGInterface->GetUnderBar()) {
        CIFUnderBar* underBar = g_pCGInterface->GetUnderBar();
        
        if (slotID >= 0 && slotID < 41 && underBar->m_totalslotunderbar[slotID] != NULL) {
            int currentItemID = underBar->m_totalslotunderbar[slotID]->GetSlotItemID();
            
            // Check if we should debug this slot/item combination
            bool shouldDebug = false;
            std::map<int, std::pair<int, time_t> >::iterator it = slotDebugHistory.find(slotID);
            if (it == slotDebugHistory.end()) {
                // First time seeing this slot
                shouldDebug = true;
                slotDebugHistory[slotID] = std::make_pair(currentItemID, currentTime);
            } else {
                // Check if item changed or enough time has passed (30 seconds)
                if (it->second.first != currentItemID || (currentTime - it->second.second) > 30) {
                    shouldDebug = true;
                    it->second = std::make_pair(currentItemID, currentTime);
                }
            }
            
            // Track if cache is empty on first run
            if (cacheWasEmpty && slotItemIDUnderBarList.size() == 0) {
                // Cache is empty, using real-time data
            }
            
            if (currentItemID == 0) {
                // Slot is empty, clean up cache if this slot was previously mapped
                for (size_t i = 0; i < slotItemIDUnderBarList.size(); ++i) {
                    if (slotItemIDUnderBarList[i].first == slotID) {
                        slotItemIDUnderBarList.erase(slotItemIDUnderBarList.begin() + i);
                        break;
                    }
                }
                return -1;
            }
            
            // Check if cache state has changed (empty -> populated)
            time_t currentTime = time(NULL);
            if (cacheWasEmpty && slotItemIDUnderBarList.size() > 0 && (currentTime - lastCacheCheckTime) > 1) {
                cacheWasEmpty = false;
                lastCacheCheckTime = currentTime;
                slotDebugHistory.clear();
            }
            
            // First, try to get the item from our cached list for proper mall potion mapping
            int cachedItemID = -1;
            for (size_t i = 0; i < slotItemIDUnderBarList.size(); ++i) {
                if (slotItemIDUnderBarList[i].first == slotID) { 
                    int quickbarID = slotItemIDUnderBarList[i].second;
                    
                    // Complete mall potion ID mapping: Quick Slot ID -> Inventory ID
                    int mappedID = quickbarID; // Default to original ID
                    switch (quickbarID) {
                        case 233: mappedID = 3817; break; // HP Mall Potion 2
                        case 234: mappedID = 3818; break; // HP Mall Potion 3
                        case 235: mappedID = 3819; break; // HP Mall Potion 4
                        case 24:  mappedID = 5912; break; // HP Mall Potion 5
                        case 236: mappedID = 3820; break; // MP Mall Potion 2
                        case 237: mappedID = 3821; break; // MP Mall Potion 3
                        case 238: mappedID = 3822; break; // MP Mall Potion 4
                        case 25:  mappedID = 5913; break; // MP Mall Potion 5
                        default: break; // Normal potion (no mapping)
                    }
                    
                    cachedItemID = mappedID;
                    break;
                }
            }
            
            // If we found the item in cache, use it
            if (cachedItemID != -1) {
                return cachedItemID;
            }
            
            // If not in cache, use real-time data but apply mall potion mapping
            int finalItemID = currentItemID;
            
            // Apply mall potion mapping to real-time data if needed
            switch (currentItemID) {
                case 233: finalItemID = 3817; break; // HP Mall Potion 2
                case 234: finalItemID = 3818; break; // HP Mall Potion 3
                case 235: finalItemID = 3819; break; // HP Mall Potion 4
                case 24:  finalItemID = 5912; break; // HP Mall Potion 5
                case 236: finalItemID = 3820; break; // MP Mall Potion 2
                case 237: finalItemID = 3821; break; // MP Mall Potion 3
                case 238: finalItemID = 3822; break; // MP Mall Potion 4
                case 25:  finalItemID = 5913; break; // MP Mall Potion 5
                default: break; // Normal potion (no mapping)
            }
            
            return finalItemID;
        }
    }
    
    // If we get here, either the slot is empty or not found in cache
    return -1; 
}

void CIFAutoPotion::OnHPSliderChanged() {
    if (m_totalhpbelt2 && m_hpSliderCtr) {
        m_totalhpbelt2->SetTextFormatted("%d", m_hpSliderCtr->Get2EC());
    }
}
void CIFAutoPotion::OnMPSliderChanged() {
    if (m_totalmpbelt2 && m_mpSliderCtr) {
        m_totalmpbelt2->SetTextFormatted("%d", m_mpSliderCtr->Get2EC());
    }
}

void CIFAutoPotion::PointerBasedQuickBarStateChanges()
{
	if (!g_pCGInterface || !g_pCGInterface->GetUnderBar()) {
		return;
	}

	time_t currentTime = time(NULL);
	if (currentTime - lastQuickbarStateCheck < QUICKBAR_STATE_CHECK_INTERVAL) {
		return;
	}
	lastQuickbarStateCheck = currentTime;

	CIFUnderBar* underBar = g_pCGInterface->GetUnderBar();
	bool stateChanged = false;

	for (int slotID = 0; slotID < 40; slotID++) {
		void* currentItemPointer = NULL;

		if (underBar->m_totalslotunderbar[slotID] != NULL) {
			CIFSlotWithHelpPointerBased* pSlot = reinterpret_cast<CIFSlotWithHelpPointerBased*>(underBar->m_totalslotunderbar[slotID]);
			currentItemPointer = pSlot->pItemObject;
		}

		if (lastKnownQuickbarState.find(slotID) == lastKnownQuickbarState.end() || lastKnownQuickbarState[slotID] != currentItemPointer) {
			stateChanged = true;
			lastKnownQuickbarState[slotID] = currentItemPointer;
		}
	}

	if (stateChanged) {
		if (g_pCGInterface && g_pCGInterface->GetUnderBar()) {
			g_pCGInterface->GetUnderBar()->LoadUnderbarData();
		}
	}
}