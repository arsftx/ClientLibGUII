#include <BSLib/Debug.h>
#include "IFMainPopup.h"
#include "TextStringManager.h"
#include "GInterface.h"
#include "Game.h"

#include <remodel/MemberFunctionHook.h>

#define GDR_QUEST 76
#define GDR_BTN_APPRENTICE 16
#define GDR_BTN_QUEST 15
#define GDR_BTN_PARTY 14
#define GDR_BTN_ACT 13
#define GDR_BTN_SKILL 12
#define GDR_BTN_INV 11
#define GDR_BTN_CHAR 10
#define GDR_MAINPOPUP_LEFT_DECO_STATIC 4
#define GDR_MAINPOPUP_BG_TILE 3

GFX_IMPLEMENT_DYNAMIC_EXISTING(CIFMainPopup, 0x9FE9E4)

GFX_IMPLEMENT_DYNCREATE_FN(CIFMainPopup, CIFMainFrame)

GFX_BEGIN_MESSAGE_MAP(CIFMainPopup, CIFMainFrame)
                    ONG_COMMAND(GDR_BTN_CHAR, &CIFMainPopup::OnClick_BtnChar)
                    ONG_COMMAND(GDR_BTN_INV, &CIFMainPopup::OnClick_BtnInv)
                    ONG_COMMAND(GDR_BTN_SKILL, &CIFMainPopup::OnClick_BtnSkill)
                    ONG_COMMAND(GDR_BTN_ACT, &CIFMainPopup::OnClick_BtnAct)
                    ONG_COMMAND(GDR_BTN_PARTY, &CIFMainPopup::OnClick_BtnParty)
                    ONG_COMMAND(GDR_BTN_QUEST, &CIFMainPopup::OnClick_BtnQuest)
                    ONG_COMMAND(GDR_BTN_APPRENTICE, &CIFMainPopup::OnClick_BtnApprentice)
GFX_END_MESSAGE_MAP()

CIFMainPopup::CIFMainPopup() :
        m_pInventoryWindow(NULL),
        m_pEquipmentWindow(NULL),
        m_pSkillWindow(NULL),
        m_pActionWindow(NULL),
        m_pPartyWindow(NULL),
        m_pApprenticeShipWindow(NULL),
        m_pPlayerInfoWindow(NULL),
        m_pQuestWindow(NULL),
        m_nWindowType(DUMMY) {
}

bool CIFMainPopup::OnCreate(long ln) {
    CIFMainFrame::OnCreate(ln);

    /*m_IRM.LoadFromFile("resinfo\\ifmainpopup.txt");

    m_IRM.CreateInterfaceSection("Create", this);

    m_pInventoryWindow = m_IRM.GetResObj<CIFInventory>(GDR_INVENTORY, 1);
    m_pEquipmentWindow = m_IRM.GetResObj<CIFEquipment>(GDR_EQUIPMENT, 1);
    m_pSkillWindow = m_IRM.GetResObj<CIFSkill>(GDR_SKILL, 1);
    m_pActionWindow = m_IRM.GetResObj<CIFAction>(GDR_ACTION, 1);
    m_pPartyWindow = m_IRM.GetResObj<CIFParty>(GDR_PARTY, 1);
    m_pPlayerInfoWindow = m_IRM.GetResObj<CIFPlayerInfo>(GDR_PLAYERINFO, 1);
    m_pQuestWindow = m_IRM.GetResObj<CIFQuest>(GDR_QUEST, 1);
    m_pApprenticeShipWindow = m_IRM.GetResObj<CIFApprenticeShip>(GDR_APPRENTICESHIP, 1);

    m_pBtnInventory = m_IRM.GetResObj<CIFButton>(GDR_BTN_INV, 1);
    m_pBtnSkill = m_IRM.GetResObj<CIFButton>(GDR_BTN_SKILL, 1);
    m_pBtnAction = m_IRM.GetResObj<CIFButton>(GDR_BTN_ACT, 1);
    m_pBtnParty = m_IRM.GetResObj<CIFButton>(GDR_BTN_PARTY, 1);
    m_pBtnCharacter = m_IRM.GetResObj<CIFButton>(GDR_BTN_CHAR, 1);
    m_pBtnQuest = m_IRM.GetResObj<CIFButton>(GDR_BTN_QUEST, 1);
    m_pBtnApprentice = m_IRM.GetResObj<CIFButton>(GDR_BTN_APPRENTICE, 1);

    m_pBackground = m_IRM.GetResObj<CIFStatic>(GDR_MAINPOPUP_BG_TILE, 1);*/

    m_pBtnCharacter->FUN_00655fa0(8);
    m_pBtnInventory->FUN_00655fa0(8);
    m_pBtnSkill->FUN_00655fa0(8);
    m_pBtnAction->FUN_00655fa0(8);
    m_pBtnParty->FUN_00655fa0(8);
    m_pBtnQuest->FUN_00655fa0(8);
    m_pSkillWindow->FUN_006a6e60(1);

    m_pBtnInventory->SetTooltipText(TSM_GETTEXT(L"UIIT_STT_TOGGLE_INVENTORY"));
    m_pBtnInventory->SetStyleThingy(TOOLTIP);

    m_pBtnCharacter->SetTooltipText(TSM_GETTEXT(L"UIIT_STT_TOGGLE_CHARACTER"));
    m_pBtnCharacter->SetStyleThingy(TOOLTIP);

    m_pBtnSkill->SetTooltipText(TSM_GETTEXT(L"UIIT_STT_TOGGLE_SKILL"));
    m_pBtnSkill->SetStyleThingy(TOOLTIP);

    m_pBtnAction->SetTooltipText(TSM_GETTEXT(L"UIIT_STT_TOGGLE_ACTION"));
    m_pBtnAction->SetStyleThingy(TOOLTIP);

    m_pBtnParty->SetTooltipText(TSM_GETTEXT(L"UIIT_STT_TOGGLE_PARTY"));
    m_pBtnParty->SetStyleThingy(TOOLTIP);

    m_pBtnQuest->SetTooltipText(TSM_GETTEXT(L"UIIT_STT_TOGGLE_QUEST"));
    m_pBtnQuest->SetStyleThingy(TOOLTIP);

    m_pBtnApprentice->SetTooltipText(TSM_GETTEXT(L"UIIT_CTL_TC_SHORTKEY_L"));
    m_pBtnApprentice->SetStyleThingy(TOOLTIP);

#ifndef CONFIG_OLD_MAINPOPUP
    m_pBtnInventory->ShowGWnd(false);
    m_pBtnSkill->ShowGWnd(false);
    m_pBtnAction->ShowGWnd(false);
    m_pBtnParty->ShowGWnd(false);
    m_pBtnCharacter->ShowGWnd(false);
    m_pBtnQuest->ShowGWnd(false);
    m_pBtnApprentice->ShowGWnd(false);

    /*m_IRM.GetResObj(GDR_MAINPOPUP_LEFT_DECO_STATIC, 1)->ShowGWnd(false);*/
#endif

    m_pInventoryWindow->BringToFront();

    return true;
}

void CIFMainPopup::OnUpdate() {
    CIFWnd::OnUpdate();
}

void CIFMainPopup::HideAll() {
    m_pInventoryWindow->ShowGWnd(false);
    m_pEquipmentWindow->ShowGWnd(false);
    m_pSkillWindow->ShowGWnd(false);
    m_pActionWindow->ShowGWnd(false);
    m_pPartyWindow->ShowGWnd(false);
    m_pPlayerInfoWindow->ShowGWnd(false);
    m_pQuestWindow->ShowGWnd(false);
    m_pApprenticeShipWindow->ShowGWnd(false);
}

void CIFMainPopup::ShowSubPage(int nPageId) {
    HideAll();

    wnd_rect rect = GetBounds();

    const ClientResolutonData &res = CGame::GetClientDimensionStuff();

    switch (nPageId) {
        case GDR_INVENTORY: {
            m_nWindowType = MAINPOP_INVENTORY;
            //SetText(TSM_GETTEXTPTR(L"UIIT_STT_INVENTORY"));

            m_pInventoryWindow->ShowGWnd(true);
            m_pEquipmentWindow->ShowGWnd(true);

            m_pBackground->ShowGWnd(true);
            m_pBackground->MoveGWnd(rect.pos.x + 189, rect.pos.y + 68);
            m_pBackground->SetGWndSize(9, 292);

            MoveGWnd(((res.width - GetSize().width) - 80), GetPos().y);
            BringToFront();
            break;
        }

        case GDR_PARTY: {
            m_nWindowType = MAINPOP_PARTY;
            //SetText(TSM_GETTEXTPTR(L"UIIT_STT_PARTY"));

            m_pPartyWindow->ShowGWnd(true);
            m_pBackground->ShowGWnd(false);

            MoveGWnd(((res.width - GetSize().width) - 80), GetPos().y);
            BringToFront();
            break;
        }

        case GDR_SKILL: {
            m_nWindowType = MAINPOP_SKILL;
            //SetText(TSM_GETTEXTPTR(L"UIIT_STT_SKILL"));

            m_pSkillWindow->ShowGWnd(true);
            m_pBackground->ShowGWnd(false);

            MoveGWnd(((res.width - GetSize().width) - 80), GetPos().y);
            BringToFront();
            break;
        }

        case GDR_ACTION: {
            m_nWindowType = MAINPOP_ACTION;
            //SetText(TSM_GETTEXTPTR(L"UIIT_STT_ACTION"));

            m_pActionWindow->ShowGWnd(true);

            m_pBackground->ShowGWnd(true);
            m_pBackground->MoveGWnd(rect.pos.x + 40, rect.pos.y + 154);
            m_pBackground->SetGWndSize(308, 124);

            MoveGWnd(((res.width - GetSize().width) - 80), GetPos().y);
            BringToFront();
            break;
        }

        case GDR_PLAYERINFO: {
            m_nWindowType = MAINPOP_PLAYER_INFO;
            //SetText(TSM_GETTEXTPTR(L"UIIT_STT_CHARACTER"));

            m_pPlayerInfoWindow->ShowGWnd(true);
            m_pBackground->ShowGWnd(false);

            MoveGWnd(80, GetPos().y);
            BringToFront();
            break;
        }

        case GDR_QUEST: {
            m_nWindowType = MAINPOP_QUEST;
            //SetText(TSM_GETTEXTPTR(L"UIIT_STT_QUEST"));

            m_pQuestWindow->ShowGWnd(true);
            m_pBackground->ShowGWnd(false);

            MoveGWnd(80, GetPos().y);
            BringToFront();
            break;
        }

        case GDR_APPRENTICESHIP: {
            m_nWindowType = MAINPOP_ACADEMY;
            //SetText(TSM_GETTEXTPTR(L"UIIT_CTL_TC_TRAININGCAMP"));

            m_pApprenticeShipWindow->ShowGWnd(true);
            m_pBackground->ShowGWnd(true);
            m_pBackground->ShowGWnd(false);

            MoveGWnd(80, GetPos().y);
            BringToFront();
            break;
        }
    }
}

void CIFMainPopup::OnClick_BtnChar() {
    ShowSubPage(GDR_PLAYERINFO);
}

void CIFMainPopup::OnClick_BtnInv() {
    ShowSubPage(GDR_INVENTORY);
}

void CIFMainPopup::OnClick_BtnSkill() {
    ShowSubPage(GDR_SKILL);
}

void CIFMainPopup::OnClick_BtnAct() {
    ShowSubPage(GDR_ACTION);
}

void CIFMainPopup::OnClick_BtnParty() {
    ShowSubPage(GDR_PARTY);
}

void CIFMainPopup::OnClick_BtnQuest() {
#ifdef CONFIG_OLD_MAINPOPUP
    g_pCGInterface->ToggleQuestNew();
#else
    ShowSubPage(GDR_QUEST);
#endif
}

void CIFMainPopup::OnClick_BtnApprentice() {
    ShowSubPage(GDR_APPRENTICESHIP);
}

void CIFMainPopup::RenderMyself() {
    CIFFrame::RenderMyself();
}

void CIFMainPopup::ShowGWnd(bool bVisible) {
    if (bVisible) {
        switch (this->m_nWindowType) {
            case MAINPOP_PLAYER_INFO:
                ShowSubPage(GDR_PLAYERINFO);
                break;
            case MAINPOP_INVENTORY:
                ShowSubPage(GDR_INVENTORY);
                break;
            case MAINPOP_SKILL:
                ShowSubPage(GDR_SKILL);
                break;
            case MAINPOP_ACTION:
                ShowSubPage(GDR_ACTION);
                break;
            case MAINPOP_PARTY:
                ShowSubPage(GDR_PARTY);
                break;
            case MAINPOP_QUEST:
                ShowSubPage(GDR_QUEST);
                break;
            case MAINPOP_ACADEMY:
                ShowSubPage(GDR_APPRENTICESHIP);
                break;
        }
    } else {
        m_pInventoryWindow->ShowGWnd(false);
        m_pEquipmentWindow->ShowGWnd(false);
        m_pSkillWindow->ShowGWnd(false);
        m_pActionWindow->ShowGWnd(false);
        m_pPartyWindow->ShowGWnd(false);
        m_pPlayerInfoWindow->ShowGWnd(false);
        m_pQuestWindow->ShowGWnd(false);
        m_pApprenticeShipWindow->ShowGWnd(false);
    }

    CIFMainFrame::ShowGWnd(bVisible);
}


//HOOK_ORIGINAL_MEMBER(0x006a1d20, &CIFMainPopup::GetInventory);
CIFInventory *CIFMainPopup::GetInventory() const {
    BS_ASSERT_MSG(m_pInventoryWindow, "Mainpop에 세팅된 Inventory가 이상하다");
    return m_pInventoryWindow;
}

//HOOK_ORIGINAL_MEMBER(0x006a1d80, &CIFMainPopup::GetSkill);
CIFSkill *CIFMainPopup::GetSkill() const {
    BS_ASSERT_MSG(m_pSkillWindow, "Mainpop에 세팅된 Skill이 이상하다");
    return m_pSkillWindow;
}

//HOOK_ORIGINAL_MEMBER(0x006a1db0, &CIFMainPopup::GetAction);
CIFAction *CIFMainPopup::GetAction() const {
    BS_ASSERT_MSG(m_pActionWindow, "Mainpop에 세팅된 Action이 이상하다");
    return m_pActionWindow;
}

//HOOK_ORIGINAL_MEMBER(0x006a1de0, &CIFMainPopup::GetParty);
CIFParty *CIFMainPopup::GetParty() const {
    BS_ASSERT_MSG(m_pPartyWindow, "Mainpop에 세팅된 Party가 이상하다");
    return m_pPartyWindow;
}

//HOOK_ORIGINAL_MEMBER(0x06a1e10, &CIFMainPopup::GetPlayerInfo);
CIFPlayerInfo *CIFMainPopup::GetPlayerInfo() const {
    BS_ASSERT_MSG(m_pPlayerInfoWindow, "Mainpop에 세팅된 PlayerInfo가 이상하다");
    return m_pPlayerInfoWindow;
}

//HOOK_ORIGINAL_MEMBER(0x006a1e40, &CIFMainPopup::GetQuest);
CIFQuest *CIFMainPopup::GetQuest() const {
    BS_ASSERT_MSG(m_pQuestWindow, "Mainpop에 세팅된 Quest가 이상하다");
    return m_pQuestWindow;
}

//HOOK_ORIGINAL_MEMBER(0x006a1e70, &CIFMainPopup::GetApprenticeShip);
CIFApprenticeShip *CIFMainPopup::GetApprenticeShip() const {
    BS_ASSERT_MSG(m_pApprenticeShipWindow, "Mainpop에 세팅된 ApprenticeShip이 이상하다");
    return m_pApprenticeShipWindow;
}

//HOOK_ORIGINAL_MEMBER(0x006a1d50, &CIFMainPopup::GetEquipment);
CIFEquipment *CIFMainPopup::GetEquipment() const {
    BS_ASSERT_MSG(m_pEquipmentWindow, "Mainpop에 세팅된 Equipment가 이상하다");
    return m_pEquipmentWindow;
}

//HOOK_ORIGINAL_MEMBER(0x006a1cf0, &CIFMainPopup::IsSubPageActive);
bool CIFMainPopup::IsSubPageActive(int nPageId) {
    return 0;
    //return m_IRM.GetResObj<CIFWnd>(nPageId, 1)->IsVisible();
}
