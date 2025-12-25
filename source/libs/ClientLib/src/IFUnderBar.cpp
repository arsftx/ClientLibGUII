#include "IFUnderBar.h"
#include "GameSettings.h"
#include <set>

// Use the global slotItemIDUnderBarList from GameSettings.cpp
extern std::vector<std::pair<int, int>> slotItemIDUnderBarList;

GFX_IMPLEMENT_DYNAMIC_EXISTING(CIFUnderBar, 0x9FFD30)
bool CIFUnderBar::OnCreateIMPL(long ln) {
	// Call original function first
	bool result = reinterpret_cast<bool(__thiscall*)(CIFUnderBar*, long)>(0x0054F530)(this, ln);
	
	// Then load underbar data
	LoadUnderbarData();
	
	return result;
}
FILE* CIFUnderBar::SaveUnderbarData()
{
	slotItemIDUnderBarList.clear();
	for (int SlotID = 0; SlotID < 41; ++SlotID) {
		if (m_totalslotunderbar[SlotID] != NULL) {
			int itemID = m_totalslotunderbar[SlotID]->GetSlotItemID(); // Get the actual Item ID
			int itemID2 = m_totalslotunderbar[SlotID]->GetSlotItemCheck(); // Get the actual Item ID

			int ItemMallPotionID[12] = { 3817 ,3818 ,3819 ,5912 ,3820 ,3821 ,3822 ,5913 ,235 ,238 ,24 ,25 };

			bool isMallPotion = false;

			// Check if itemID exists in ItemMallPotionID array
			for (int i = 0; i < 12; ++i) {
				if (itemID == ItemMallPotionID[i]) {
					isMallPotion = true;
					break;
				}
			}
			// Also check itemID2 for mall potions (in case the wrong ID is being used)
			if (!isMallPotion) {
				for (int i = 0; i < 12; ++i) {
					if (itemID2 == ItemMallPotionID[i]) {
						isMallPotion = true;
						break;
					}
				}
			}

			if (itemID != 0 && !isMallPotion) {
				slotItemIDUnderBarList.push_back(std::make_pair(SlotID, itemID2));
			}
			if (itemID != 0 && isMallPotion) {
				// Use itemID2 if the mall potion was detected there, otherwise use itemID
				bool mallPotionInItemID2 = false;
				for (int i = 0; i < 12; ++i) {
					if (itemID2 == ItemMallPotionID[i]) {
						mallPotionInItemID2 = true;
						break;
					}
				}
				int finalItemID = mallPotionInItemID2 ? itemID2 : itemID;
				slotItemIDUnderBarList.push_back(std::make_pair(SlotID, finalItemID));
			}
		}
	}
	return reinterpret_cast<FILE* (__thiscall*) (CIFUnderBar*)>(0x00551780)(this);
}
FILE* CIFUnderBar::LoadUnderbar()
{
	slotItemIDUnderBarList.clear();
	for (int SlotID = 0; SlotID < 41; ++SlotID) {
		if (m_totalslotunderbar[SlotID] != NULL) {
			int itemID = m_totalslotunderbar[SlotID]->GetSlotItemID(); // Get the actual Item ID
			int itemID2 = m_totalslotunderbar[SlotID]->GetSlotItemCheck(); // Get the actual Item ID

			int ItemMallPotionID[12] = { 3817 ,3818 ,3819 ,5912 ,3820 ,3821 ,3822 ,5913 ,235 ,238 ,24 ,25 };

			bool isMallPotion = false;

			// Check if itemID exists in ItemMallPotionID array
			for (int i = 0; i < 12; ++i) {
				if (itemID == ItemMallPotionID[i]) {
					isMallPotion = true;
					break;
				}
			}
			// Also check itemID2 for mall potions (in case the wrong ID is being used)
			if (!isMallPotion) {
				for (int i = 0; i < 12; ++i) {
					if (itemID2 == ItemMallPotionID[i]) {
						isMallPotion = true;
						break;
					}
				}
			}

			if (itemID != 0 && !isMallPotion) {
				slotItemIDUnderBarList.push_back(std::make_pair(SlotID, itemID2));
			}
			if (itemID != 0 && isMallPotion) {
				// Use itemID2 if the mall potion was detected there, otherwise use itemID
				bool mallPotionInItemID2 = false;
				for (int i = 0; i < 12; ++i) {
					if (itemID2 == ItemMallPotionID[i]) {
						mallPotionInItemID2 = true;
						break;
					}
				}
				int finalItemID = mallPotionInItemID2 ? itemID2 : itemID;
				slotItemIDUnderBarList.push_back(std::make_pair(SlotID, finalItemID));
			}
		}
	}
	return reinterpret_cast<FILE* (__thiscall*) (CIFUnderBar*)>(0x5512F0)(this);
}
void CIFUnderBar::LoadUnderbarData()
{
	slotItemIDUnderBarList.clear();
	static std::set<int> underbarDebugPrintedSlots; // Throttle debug per slot
	for (int SlotID = 0; SlotID < 41; ++SlotID) {
		if (m_totalslotunderbar[SlotID] != NULL) {
			int itemID = m_totalslotunderbar[SlotID]->GetSlotItemID(); // Get the actual Item ID
			int itemID2 = m_totalslotunderbar[SlotID]->GetSlotItemCheck(); // Get the actual Item ID
			
			int ItemMallPotionID[12] = { 3817 ,3818 ,3819 ,5912 ,3820 ,3821 ,3822 ,5913 ,235 ,238 ,24 ,25 };

			bool isMallPotion = false;

			// Check if itemID exists in ItemMallPotionID array
			for (int i = 0; i < 12; ++i) {
				if (itemID == ItemMallPotionID[i]) {
					isMallPotion = true;
					break;
				}
			}
			// Also check itemID2 for mall potions (in case the wrong ID is being used)
			if (!isMallPotion) {
				for (int i = 0; i < 12; ++i) {
					if (itemID2 == ItemMallPotionID[i]) {
						isMallPotion = true;
						break;
					}
				}
			}

			if (itemID != 0 && !isMallPotion) {
				slotItemIDUnderBarList.push_back(std::make_pair(SlotID, itemID2));
			}
			if (itemID != 0 && isMallPotion) {
				// Use itemID2 if the mall potion was detected there, otherwise use itemID
				bool mallPotionInItemID2 = false;
				for (int i = 0; i < 12; ++i) {
					if (itemID2 == ItemMallPotionID[i]) {
						mallPotionInItemID2 = true;
						break;
					}
				}
				int finalItemID = mallPotionInItemID2 ? itemID2 : itemID;
				slotItemIDUnderBarList.push_back(std::make_pair(SlotID, finalItemID));
			}
		}
	}
}