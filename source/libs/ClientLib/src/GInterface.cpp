#include "GInterface.h"
#include "CIFMainMenuBar.h"
#include "CIFMainMenuButton.h"
#include "GlobalDataManager.h"
#include "IFAutoHuntSettings.h"
#include "IFAutoPotion.h"
#include "IFChatViewer.h"
#include "IFNotify.h"
#include "IFPetFilterSettings.h"
#include "IFRankingWindow.h"
#include "IFflorian0Guide.h"
#include "MacroButton.h"
#include "RankingIcon.h"
#include <Game.h>
#include <LegendMainMenu.h>

#include <BSLib/multibyte.h>

#include <BSLib/Debug.h>
#include <direct.h>
#include <remodel/MemberFunctionHook.h>
#include <stdarg.h>
#include <stdio.h>

GFX_IMPLEMENT_DYNAMIC_EXISTING(CGInterface, 0x009ff590)//ECSRO

//HOOK_ORIGINAL_MEMBER(0x0079D5B0, &CGInterface::ToggleActionWnd);
void CGInterface::ToggleActionWnd() {
}

//HOOK_ORIGINAL_MEMBER(0x0079B0B0, &CGInterface::ToggleApprenticeshipWnd);
void CGInterface::ToggleApprenticeshipWnd() {
}

//HOOK_ORIGINAL_MEMBER(0x0079ACE0, &CGInterface::TogglePlayerInfoWnd);
void CGInterface::TogglePlayerInfoWnd() {
}

void CGInterface::RenderToggle_GDR_GAMEGUIDE() {
    return reinterpret_cast<void(__thiscall *)(void *)>(0x0079F690)(this);
}

//HOOK_ORIGINAL_MEMBER(0x0079B020, &CGInterface::ToggleInventoryWnd);
void CGInterface::ToggleInventoryWnd() {
}

//HOOK_ORIGINAL_MEMBER(0x0079ad70, &CGInterface::ToggleEquipmentWnd);
void CGInterface::ToggleEquipmentWnd() {
}

//HOOK_ORIGINAL_MEMBER(0x0079AE90, &CGInterface::TogglePartyWnd);
void CGInterface::TogglePartyWnd() {
}

//HOOK_ORIGINAL_MEMBER(0x0079AE00, &CGInterface::ToggleSkillWnd);
void CGInterface::ToggleSkillWnd() {
}

//HOOK_ORIGINAL_MEMBER(0x0079af20, &CGInterface::ShowInventoryWnd);
void CGInterface::ShowInventoryWnd() {
}

//HOOK_ORIGINAL_MEMBER(0x0079af70, &CGInterface::ShowApprenticeshipWnd);
void CGInterface::ShowApprenticeshipWnd() {
}

void CGInterface::ToggleMainPopup() {
}

void CGInterface::RenderToggle_WORLDMAP_GUIDE() {
    return reinterpret_cast<void(__thiscall *)(void *)>(0x0079ABE0)(this);
}

void CGInterface::Render_GDR_AUTO_POTION(bool visible) {
    return reinterpret_cast<void(__thiscall *)(void *, bool)>(0x0079C750)(this, visible);
}

void CGInterface::CreateFlorian0Event() {
    CGWnd *guide = GetAlarmManager()->GetGuide(GUIDE_FLORIAN0);
}

CAlramGuideMgrWnd *CGInterface::GetAlarmManager() {
    return 0;
    //return m_IRM.GetResObj<CAlramGuideMgrWnd>(GDR_ALRAM_GUIDE_MGR_WND, 1);
}

bool CGInterface::OnCreateIMPL(long ln) {

    PutDump("%s - Entered\n", __FUNCTION__);

    BeforeOnCreate();

    bool b = reinterpret_cast<bool(__thiscall *)(CGInterface *, long)>(0x004E1EE0)(this, ln);//ECSRO

    AfterOnCreate();

    return b;
}

void CGInterface::BeforeOnCreate() {
}

void CGInterface::AfterOnCreate() {
    RECT AutoPotionRect = {0, 0, 0, 0};
    if (CIFMainMenu::AutoPotion == NULL) {
        CIFMainMenu::AutoPotion = (CIFAutoPotion *) CreateInstance(this, GFX_RUNTIME_CLASS(CIFAutoPotion), AutoPotionRect, 2000, 0);
    }

    // Create RankingWindow interface
    if (CIFMainMenu::RankingWindow == NULL) {
        CIFMainMenu::RankingWindow = (CIFRankingWindow *) CreateInstance(this, GFX_RUNTIME_CLASS(CIFRankingWindow), AutoPotionRect, 2001, 0);
    }

    // Create MacroButton (derived from CIFDecoratedStatic)
    RECT guideRect = {960, 200, 1000, 240};// 40x40 button at (960, 200)
    CGWnd *pGuide = CreateInstance(this, GFX_RUNTIME_CLASS(CMacroButton), guideRect, GUIDE_MACROBUTTON, 0);
    if (pGuide) {
        // CRITICAL: Fix CTextBoard vtable at offset 0x6C
        *reinterpret_cast<DWORD *>(reinterpret_cast<char *>(pGuide) + 0x6C) = 0x0093DA88;

        pGuide->ShowGWnd(true);
        pGuide->BringToFront();
    }

    // Create RankingIcon (next to MacroButton, 50 pixels to the left)
    RECT rankingRect = {910, 200, 950, 240};// 40x40 button at (910, 200)
    CGWnd *pRanking = CreateInstance(this, GFX_RUNTIME_CLASS(CRankingIcon), rankingRect, GUIDE_RANKINGICON, 0);
    if (pRanking) {
        // CRITICAL: Fix CTextBoard vtable at offset 0x6C
        *reinterpret_cast<DWORD *>(reinterpret_cast<char *>(pRanking) + 0x6C) = 0x0093DA88;

        pRanking->ShowGWnd(true);
        pRanking->BringToFront();
    }

    // Create MacroWindow - CRITICAL: Create OFF-SCREEN to prevent blocking clicks!
    RECT macroWindowRect = {-500, -500, 360, 244};
    CGWnd::CreateInstance(this, GFX_RUNTIME_CLASS(CIFMacroWindow), macroWindowRect, GDR_MACRO_WINDOW, 0);

    // Create PetAutoPotion settings window - CRITICAL: Create OFF-SCREEN!
    RECT petPotionRect = {-500, -500, 406, 340};
    CGWnd::CreateInstance(this, GFX_RUNTIME_CLASS(CIFPetAutoPotion), petPotionRect, GDR_PET_AUTO_POTION, 0);

    // Create PetFilterSettings window - CRITICAL: Create OFF-SCREEN!
    RECT petFilterRect = {-500, -500, 405, 415};
    CGWnd::CreateInstance(this, GFX_RUNTIME_CLASS(CIFPetFilterSettings), petFilterRect, GDR_PET_FILTER_SETTINGS, 0);

    // Create AutoHuntSettings window - CRITICAL: Create OFF-SCREEN!
    RECT autoHuntRect = {-500, -500, 400, 420};
    CGWnd::CreateInstance(this, GFX_RUNTIME_CLASS(CIFAutoHuntSettings), autoHuntRect, GDR_AUTO_HUNT_SETTINGS, 0);
}
void CGInterface::ResetPosition() {
    const ClientRes &client = theApp.GetRes();
    wnd_size v109 = this->GetSize();
    this->MoveGWnd((client.res[0].width - v109.width) / 2, (client.res[0].height - v109.height) / 4);
}
void CGInterface::ShowMessage_Quest(const std::n_wstring &msg) {
    /* CIFNotify *notify = m_IRM.GetResObj<CIFNotify>(GDR_UPDATE_QUEST_INFO, 1);
    notify->ShowMessage(msg);*/
}

void CGInterface::ShowMessage_Notice(const std::n_wstring &msg) {
    /*CIFNotify *notify = m_IRM.GetResObj<CIFNotify>(GDR_NOTICE, 1);
    notify->ShowMessage(msg);*/
}

//006567e0
void CGInterface::ShowMessage_Warning(const std::n_wstring &msg) {
    /*CIFNotify *notify = m_IRM.GetResObj<CIFNotify>(GDR_WARNING_WND, 1);
    notify->ShowMessage(msg);*/
}


int CGInterface::Get_SelectedObjectId() {
    return reinterpret_cast<int(__thiscall *)(CGInterface *)>(0x00654a30)(this);
}

CIFTimerWnd *CGInterface::Get_TimerWindow() {
    return 0;
    //return this->m_timerWindow;
}
CIFAutoPotion *CGInterface::Get_AutoPotionWnd() {
    return CIFMainMenu::AutoPotion;
}
CIFAutoPotion *CGInterface::Get_AutoPotionFrame() {
    return CIFMainMenu::AutoPotion;
}
CIFRankingWindow *CGInterface::Get_RankingWindowWnd() {
    return CIFMainMenu::RankingWindow;
}
CIFRankingWindow *CGInterface::Get_RankingWindowFrame() {
    return CIFMainMenu::RankingWindow;
}

CIFQuickStateHalfWnd *CGInterface::Get_QuickStateHalfWnd() {
    return 0;
    //return this->N00002637;
}
CIFItemMallConfirmBuy *CGInterface::Get_GDR_ItemMallConfirmBuy() {
    return m_IRM.GetResObj<CIFItemMallConfirmBuy>(400, 1);
}
CIF_NPCWindow *CGInterface::Get_GDR_NPCWINDOW() {
    return 0;
    //return (CIF_NPCWindow *) this->m_IRM.GetResObj(GDR_NPCWINDOW, 1);
}
CIFItemMallConfirmBuy *CGInterface::GetItemMallConfirmBuy() {
    return (CIFItemMallConfirmBuy *) this->GetControlFromList(400);
}
//HOOK_ORIGINAL_MEMBER(0x00798D00, &CGInterface::GetMainPopup);
CIFMainPopup *CGInterface::GetMainPopup() {

    return (CIFMainPopup *) this->m_IRM.GetResObj(GDR_MAINPOPUP, 1);
}

CIFStorageRoom *CGInterface::Get_GDR_STORAGEROOM() {
    return 0;
    //return (CIFStorageRoom *) this->m_IRM.GetResObj(GDR_STORAGEROOM, 1);
}

CIFStorageRoom *CGInterface::Get_GDR_GUILDSTORAGEROOM() {
    return 0;
    //return (CIFStorageRoom *) this->m_IRM.GetResObj(GDR_GUILDSTORAGEROOM, 1);
}
CIFGGMenu *CGInterface::Get_CIFGGMenu() {
    return 0;
    //return this->m_IRM.GetResObj<CIFGGMenu>(13, 1);
}
void CGInterface::WriteErrorMessage(byte errorType, unsigned __int16 errorCode, int colorARGB, int a5, int a6) {
    return reinterpret_cast<
        void(__thiscall *)(void *, byte, unsigned __int16, int, int, int)>(0x00778190)(this, errorType, errorCode, colorARGB, a5, a6);
}

void CGInterface::WriteSystemMessage(eLogType level, const wchar_t *str) {
    reinterpret_cast<void(__thiscall *)(CGInterface *, eLogType, const wchar_t *)>(0x00654020)(this, level, str);
}

//HOOK_ORIGINAL_MEMBER(0x007901c0, &CGInterface::WriteGlobalMessage)
void CGInterface::WriteGlobalMessage(unsigned char nSlot, std::n_wstring message) {
    CIFMainPopup *popup = GetMainPopup();
    CIFInventory *inventory = popup->GetInventory();

    CSOItem *item = inventory->GetItemBySlot(nSlot);

    if (item->field_28 == 0) {
        return;
    }

    const SItemData *pItemData = item->GetItemData();

    if (pItemData->IsGlobalMessageScroll()) {
        NEWMSG(0x704c)

        pReq << static_cast<unsigned char>(nSlot + 13u) << pItemData->m_typeId << acp_encode(message);

        SENDMSG()
    }
}
CIFUnderBar *CGInterface::GetUnderBar() {
    return reinterpret_cast<CIFUnderBar *(__thiscall *) (CGInterface *)>(0x00502F70)(this);
}

void CGInterface::sub_787C10(SChatMetaData &meta) {
    reinterpret_cast<void(__thiscall *)(CGInterface *, SChatMetaData *)>(0x787C10)(this, &meta);
}

int CGInterface::TryParseChatCommands(const wchar_t *text, RECT &r, std::vector<void *> &v) {
    return reinterpret_cast<
        int(__thiscall *)(CGInterface *, const wchar_t *, RECT *, std::vector<void *> *)>(0x0078BEA0)(this, text, &r, &v);
}

void CGInterface::ToggleQuestNew() {
    reinterpret_cast<void(__thiscall *)(CGInterface *)>(0x007990e0)(this);
}

CNIFCommunityWnd *CGInterface::GetCommunityWnd() {
    return reinterpret_cast<CNIFCommunityWnd *(__thiscall *) (CGInterface *)>(0x007994f0)(this);
}

void CGInterface::FUN_00777c30(ChatType type, const wchar_t *message, D3DCOLOR color, int a5) {
    /*CIFChatViewer *chatViewer = m_IRM.GetResObj<CIFChatViewer>(GDR_CHATVIEWER, 1);
    chatViewer->FUN_007aca30(type, color, message, 0, a5);*/
}

void CGInterface::FUN_00778a10(int a2, const wchar_t *message, D3DCOLOR color) {
    reinterpret_cast<
        void(__thiscall *)(CGInterface *, int, const wchar_t *, D3DCOLOR)>(0x00778a10)(this, a2, message, color);
}

void CGInterface::FUN_00777cf0(const std::n_wstring &recipient) {
    reinterpret_cast<void(__thiscall *)(CGInterface *, const std::n_wstring *)>(0x00777cf0)(this, &recipient);
}

CIFSystemMessage *CGInterface::GetSystemMessageView() {
    return 0;
    //return m_IRM.GetResObj<CIFSystemMessage>(GDR_SYSTEM_MESSAGE_VIEW, 1);
}

void CGInterface::FUN_0079a7e0(CGWndBase *pGWnd) const {
    reinterpret_cast<void(__thiscall *)(const CGInterface *, CGWndBase *)>(0x0079a7e0)(this, pGWnd);
}

void CGInterface::FUN_0079b8a0(undefined1 a1, undefined4 a2) {
    reinterpret_cast<void(__thiscall *)(CGInterface *, undefined1, undefined4)>(0x0079b8a0)(this, a1, a2);
}

void CGInterface::RequestStatIncrement(undefined4 a1, undefined4 a2, undefined4 a3, undefined4 a4) {
    if (a1 == 0x4b) {
        if (a2 == 1) {
            NEWMSG(0x7050)
            SENDMSG()
        } else if (a2 == 2) {
            NEWMSG(0x7051)
            SENDMSG()
        }
    }
}

CNIFUnderMenuBar *CGInterface::GetUnderMenuBar() {
    return reinterpret_cast<CNIFUnderMenuBar *(__thiscall *) (CGInterface *)>(0x007994e0)(this);
}

CIFExtQuickSlot *CGInterface::GetExtQuickSlot() {
    return 0;
    //return m_IRM.GetResObj<CIFExtQuickSlot>(GDR_EXT_QUICK_SLOT, 1);
}

int CGInterface::EnableExtQuickSlot(bool value) {
    g_pCGInterface->GetExtQuickSlot()->ShowGWnd(false);
    return reinterpret_cast<int(__thiscall *)(CGInterface *, bool)>(0x00674B90)(this, 1);
}
int CGInterface::DisableRegionView() {
    return 0;
}
void CGInterface::FUN_00777a70(undefined4 param_1, undefined4 param_2) {
    reinterpret_cast<void(__thiscall *)(CGInterface *, undefined4, undefined4)>(0x00777a70)(this, param_1, param_2);
}

void CGInterface::UseItem(int UsedItemSlot, int EffectedItemSlot, int param_3) {
    reinterpret_cast<void(__thiscall *)(CGInterface *, int, int, int)>(0x004F4360)(this, UsedItemSlot, EffectedItemSlot, param_3);
}


CIFMainMenuBar *CGInterface::GetMainMenuBar() {
    return m_IRM.GetResObj<CIFMainMenuBar>(19577, 1);
}

void CGInterface::WriteDebugLog(const char *format, ...) {
    char buffer[1024];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    _mkdir("Log");
    FILE *fp = fopen("Log/debug.txt", "a");
    if (fp) {
        time_t now = time(0);
        struct tm tstruct;
        char timeBuf[80];
        localtime_s(&tstruct, &now);
        strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M:%S", &tstruct);

        fprintf(fp, "[%s] %s\n", timeBuf, buffer);
        fclose(fp);
    }
}

CIFMacroWindow *CGInterface::Get_MacroWindow() {
    return GetControlFromList<CIFMacroWindow>(GDR_MACRO_WINDOW);
}

CIFPetAutoPotion *CGInterface::Get_PetAutoPotionWnd() {
    return GetControlFromList<CIFPetAutoPotion>(GDR_PET_AUTO_POTION);
}