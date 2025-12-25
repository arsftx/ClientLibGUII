#include "LegendMainMenu.h"

GFX_IMPLEMENT_DYNCREATE(CIFMainMenu, CIFMainFrame)
GFX_BEGIN_MESSAGE_MAP(CIFMainMenu, CIFMainFrame)
GFX_END_MESSAGE_MAP()


CIFMainMenu* CIFMainMenu::MenuGui;

CIFNotify* CIFMainMenu::PurbleNoitfy;
CIFNotify* CIFMainMenu::OrangeNoitfy;

CGWnd* CIFMainMenu::MenuIcon;
CGWnd* CIFMainMenu::ReloadIcon;
CGWnd* CIFMainMenu::ChestIcon;
CGWnd* CIFMainMenu::LotteryIcon;
CGWnd* CIFMainMenu::LuckNumIcon;
CIFAutoPotion* CIFMainMenu::AutoPotion;
CIFRankingWindow* CIFMainMenu::RankingWindow;
CIFMainMenuButton* CIFMainMenu::MainMenuButton;

CIFMainMenu::CIFMainMenu(void)
{
	//printf("> " __FUNCTION__ "\n");
}


CIFMainMenu::~CIFMainMenu(void)
{
	//printf("> " __FUNCTION__ "\n");
}


bool CIFMainMenu::OnCreate(long ln) {

	ShowGWnd(false);
	return true;
}


int CIFMainMenu::OnChar(UINT nChar, UINT a2, UINT a3) {

	switch (nChar) {
	case 'b':
	case 'B': {
		if (this->IsVisible())
			this->ShowGWnd(false);
		else {
			this->ShowGWnd(true);
		}
		return 1;
	}
			  break;
	}
	return 0;
}