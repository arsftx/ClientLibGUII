#include "IFTargetWindowPlayer.h"
#include "IFStatic.h"
#include "unsorted.h"
#include "TextStringManager.h"
#include "ICPlayer.h"

#include <remodel/MemberFunctionHook.h>
#include <IFStoreForPackage.h>
#include <IFSelectableArea.h>

GFX_IMPLEMENT_DYNAMIC_EXISTING(CIFTargetWindowPlayer, 0x00eea5dc)

GFX_IMPLEMENT_DYNCREATE_FN(CIFTargetWindowPlayer, CIFWnd)

enum {
    GDR_TWP_TEXT_NAME = 1,      // CIFStatic
    GDR_TWJP_JOB_GRADENAME = 11,// CIFStatic
    GDR_TW_KINDRED_MARK = 20,   // CIFStatic
    GDR_TWJP_NEW_BACK = 25,     // CIFStatic
    GDR_TWJP_HUNTER_LEV = 14,   // CIFStatic
    GDR_TWJP_MERCHANT_LEV = 13, // CIFStatic
    GDR_TWJP_THIEF_LEV = 15,    // CIFStatic
    GDR_TWJP_KINDRED_MARK = 20, // CIFStatic

    GDR_TWJP_HUNTER_ICON = 11,  // CIFStatic
    GDR_TWJP_MERCHANT_ICON = 10,// CIFStatic
    GDR_TWJP_THIEF_ICON = 12,   // CIFStatic
    GDR_TW_BUFF = 100,           // CIFStatic
    GDR_TW_CLOSE = 11,         // CIFStatic

};

GFX_BEGIN_MESSAGE_MAP(CIFTargetWindowPlayer, CIFWnd)

GFX_END_MESSAGE_MAP()

//0056bac0
bool CIFTargetWindowPlayer::OnCreate(long ln) {
    //printf("%s\n", __FUNCTION__);
    //return reinterpret_cast<bool (__thiscall *)(const CIFTargetWindowPlayer *, long)>(0x0069b180)(this, ln);

    /*m_IRM.LoadFromFile("resinfo\\iftw_player.txt");
    m_IRM.CreateInterfaceSection("Create", this);*/

    return true;
}

bool CIFTargetWindowPlayer::OnCreateIMPL(long ln) {
    //printf("%s\n", __FUNCTION__);
    bool b = reinterpret_cast<bool(__thiscall *)(const CIFTargetWindowPlayer *, long)>(0x0056bac0)(this, ln);
    //SetddjPath("", 0, 0);

    wnd_rect bounds = GetBounds();
   /* CIFStatic *m_GDR_TWJP_NEW_BACK = m_IRM.GetResObj<CIFStatic>(GDR_TWJP_NEW_BACK, 1);
    m_GDR_TWJP_NEW_BACK->TB_Func_13("interface\\targetwindow\\tw_player_wind.ddj", 0, 0);
    m_GDR_TWJP_NEW_BACK->ChangePos(bounds.pos.x - 4, bounds.pos.y);
    m_GDR_TWJP_NEW_BACK->ChangeGWndSize(202, 59);*/

    //CIFStatic *m_GDR_TW_KINDRED_MARK = m_IRM.GetResObj<CIFStatic>(GDR_TW_KINDRED_MARK, 1);
    //wnd_rect mbounds = m_GDR_TW_KINDRED_MARK->GetBounds();
    //m_GDR_TW_KINDRED_MARK->ChangePos(mbounds.pos.x + 5, mbounds.pos.y + 3);
    //m_GDR_TWJP_NEW_BACK->BringToUp();
    return b;
}

bool CIFStoreForPackageWnd::OnCreateIMPL(long ln) {
    //printf("%s\n", __FUNCTION__);
    bool b = reinterpret_cast<bool(__thiscall *)(const CIFStoreForPackageWnd *, long)>(0x0059c010)(this, ln);

   /* for (size_t i = 50; i < 55; i++) {
        m_IRM.GetResObj<CIFStatic>(i, 1)->ShowWnd(false);
    }

    m_IRM.GetResObj<CIFStatic>(60, 1)->ShowWnd(false);
    m_IRM.GetResObj<CIFStatic>(61, 1)->ShowWnd(false);

    CIFStatic *m_btnrpear = m_IRM.GetResObj<CIFStatic>(11, 1);
    CIFStatic *m_btnrpearall = m_IRM.GetResObj<CIFStatic>(12, 1);
    CIFStatic *m_frame = m_IRM.GetResObj<CIFStatic>(6, 1);
    CIFStatic *m_normal7 = m_IRM.GetResObj<CIFStatic>(40, 1);
    m_normal7->ShowWnd(false);*/

    wnd_rect bounds = GetBounds();
   /* wnd_rect fbounds = m_frame->GetBounds();
    wnd_rect brbounds = m_btnrpear->GetBounds();
    wnd_rect ballbounds = m_btnrpearall->GetBounds();
    m_btnrpear->ChangePos(brbounds.pos.x-40, fbounds.pos.y + fbounds.size.height + 5);
    m_btnrpearall->ChangePos(ballbounds.pos.x-40, fbounds.pos.y + fbounds.size.height + 5);*/
    ChangeGWndSize(bounds.size.width, bounds.size.height - 52);

    return b;
}



void CIFTargetWindowPlayer::OnUpdateIMPL() {
    reinterpret_cast<void(__thiscall *)(CIFTargetWindowPlayer *)>(0x00531a20)(this);
}

int CIFTargetWindowPlayer::sub_9D3B60(int a1) {
    int res = reinterpret_cast<int(__cdecl *)(CIFTargetWindowPlayer *, int)>(0x00531a20)(this, a1);
    return res;
}

void CIFTargetWindowPlayer::FUN_0056ba10(int objectId) {
    reinterpret_cast<void(__thiscall *)(const CIFTargetWindowPlayer *, int)>(0x0056ba10)(this, objectId);

    m_objectId = objectId;
    CICUser *pObject = static_cast<CICUser *>(GetCharacterObjectByID_MAYBE(m_objectId));
    if (pObject != NULL) {

        //printf("%s - pObject %p\n", __FUNCTION__, pObject);

        /*CIFStatic *m_GDR_TWP_TEXT_NAME = m_IRM.GetResObj<CIFStatic>(GDR_TWP_TEXT_NAME, 1);
        CIFStatic *m_GDR_TW_KINDRED_MARK = m_IRM.GetResObj<CIFStatic>(GDR_TW_KINDRED_MARK, 1);
        CIFStatic *m_GDR_TWJP_THIEF_LEV = m_IRM.GetResObj<CIFStatic>(GDR_TWJP_THIEF_LEV, 1);
        CIFStatic *m_GDR_TWJP_HUNTER_LEV = m_IRM.GetResObj<CIFStatic>(GDR_TWJP_HUNTER_LEV, 1);
        CIFStatic *m_GDR_TWJP_MERCHANT_LEV = m_IRM.GetResObj<CIFStatic>(GDR_TWJP_MERCHANT_LEV, 1);
        CIFStatic *m_GDR_TWJP_HUNTER_ICON = m_IRM.GetResObj<CIFStatic>(GDR_TWJP_HUNTER_ICON, 1);
        CIFStatic *m_GDR_TWJP_MERCHANT_ICON = m_IRM.GetResObj<CIFStatic>(GDR_TWJP_MERCHANT_ICON, 1);
        CIFStatic *m_GDR_TWJP_THIEF_ICON = m_IRM.GetResObj<CIFStatic>(GDR_TWJP_THIEF_ICON, 1);

        m_GDR_TWJP_MERCHANT_ICON->TB_Func_13("interface\\targetwindow\\tw_job_merchant.ddj", 0, 0);
        m_GDR_TW_KINDRED_MARK->TB_Func_13(GetKindredTextureFilePath(pObject->GetCommonData()->Country, 0), 0, 0);
        m_GDR_TWJP_THIEF_LEV->SetTextFormatted(L"%dlevel", pObject->GetJobLevel());
        m_GDR_TWJP_HUNTER_LEV->SetTextFormatted(L"%dlevel", pObject->GetJobLevel());
        m_GDR_TWJP_MERCHANT_LEV->SetTextFormatted(L"%dlevel", pObject->GetJobLevel());

        m_GDR_TWJP_THIEF_LEV->BringToUp();
        m_GDR_TWJP_HUNTER_LEV->BringToUp();
        m_GDR_TWJP_MERCHANT_LEV->BringToUp();
        m_GDR_TW_KINDRED_MARK->BringToUp();
        m_GDR_TWJP_HUNTER_ICON->BringToUp();
        m_GDR_TWJP_MERCHANT_ICON->BringToUp();
        m_GDR_TWJP_THIEF_ICON->BringToUp();
        m_GDR_TWP_TEXT_NAME->BringToUp();*/

    }
}

int CIFTargetWindowPlayer::sub_6B9320(char a2, char a3, char a4, char a5, char a6) {
    printf("%s - MySlef %p a2 %d a3 %d a4 %d a5 %d a6 %d\n", __FUNCTION__, this, a2, a3, a4, a5, a6);
    return reinterpret_cast<int(__thiscall *)(void *, char, char, char, char, char)>(0x006B9320)(this, a2, a3, a4, a5, a6);
}

void CIFTargetWindowPlayer::OnSpeicaltyRenderMySelf() {
    reinterpret_cast<void(__thiscall *)(CIFTargetWindowPlayer *)>(0x009d3b60)(this);

    if (g_pCICPlayer->GetJobState() == TRIJOB_THIEF) {
        /*CIFStatic *m_lable1 = m_IRM.GetResObj<CIFStatic>(32, 1);
        CIFStatic *m_lable2 = m_IRM.GetResObj<CIFStatic>(33, 1);
        m_lable1->ChangeText(L"");
        m_lable2->ChangeText(L"");*/
    }
}

void CIFTargetWindowPlayer::OnMyCIFCosInventoryRenderMySelf() {
    reinterpret_cast<void(__thiscall *)(CIFTargetWindowPlayer *)>(0x009d3b60)(this);

    //printf("My Pet invt %p\n", this);

    if (g_pCICPlayer->GetJobState() == TRIJOB_THIEF) {

        CCOSDataMgr *pCosMgr = g_pCICPlayer->GetCosMgr();
        SCOSInfo *pCosInfo = pCosMgr->GetSCOSInfo(pCosMgr->GetSelectedPetGameId());

        //printf("GetInventoryType %d\n", pCosInfo->GetInventoryType());

        if (pCosInfo->GetInventoryType() == 1) {
            //CIFButton *m_sellall = m_IRM.GetResObj<CIFButton>(35, 1);//

            //if (m_sellall) {
            //    m_sellall->ShowWnd(false);
            //}
        }
    }
}
