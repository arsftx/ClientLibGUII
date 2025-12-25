#include "IFTargetWindowSpecialMob.h"
#include "IFStatic.h"
#include "IFGauge.h"
#include <BSLib/Debug.h>
#include <memory/RemodelUtility.h>
#include <TextStringManager.h>

GFX_IMPLEMENT_DYNAMIC_EXISTING(CIFTargetWindowSpecialMob, 0xcafa44)

GFX_IMPLEMENT_DYNCREATE_FN(CIFTargetWindowSpecialMob, CIFWnd)

enum {
    GDR_TWSM_GEM = 10, // CIFStatic
    GDR_TWSM_LEVEL = 4, // CIFStatic
    GDR_TWSM_ICON = 3, // CIFStatic
    GDR_TWSM_TEXT_LEV = 2, // CIFStatic
    GDR_TWSM_GAUGE_HPGAUGE = 1, // CIFGauge
    GDR_TWSM_TEXT_ID = 0, // CIFStatic
};

GFX_BEGIN_MESSAGE_MAP(CIFTargetWindowSpecialMob, CIFWnd)

GFX_END_MESSAGE_MAP()

bool CIFTargetWindowSpecialMob::OnCreate(long ln) {
    //printf("%s\n", __FUNCTION__);
    //return reinterpret_cast<bool (__thiscall *)(const CIFTargetWindowSpecialMob *, long)>(0x0069b410)(this, ln);

    //m_IRM.LoadFromFile("resinfo\\iftw_specialmob.txt");
    //m_IRM.CreateInterfaceSection("Create", this);

    //m_pGDR_TWSM_TEXT_LEV = m_IRM.GetResObj<CIFStatic>(GDR_TWSM_TEXT_ID, 1); // 0x370
    //m_pGDR_TWSM_GAUGE_HPGAUGE = m_IRM.GetResObj<CIFGauge>(GDR_TWSM_GAUGE_HPGAUGE, 1); // 0x374
    //m_pGDR_TWSM_TEXT_ID = m_IRM.GetResObj<CIFStatic>(GDR_TWSM_TEXT_LEV, 1); // 0x378

    m_pGDR_TWSM_GAUGE_HPGAUGE->field_0x38c = 0.1;
    m_pGDR_TWSM_GAUGE_HPGAUGE->SetGWndSize(208, 4);
    return true;
}

void CIFTargetWindowSpecialMob::OnUpdate() {
    //printf("%s\n", __FUNCTION__);
    reinterpret_cast<void(__thiscall *)(const CIFTargetWindowSpecialMob *)>(0x0056bb80)(this);
}

bool CIFTargetWindowSpecialMob::OnCreateIMPL(long ln) {

    //DWORD dwRetAddr = (DWORD) REMODEL_GET_RETURN_ADDRESS();
    //printf("%s - dwRetAddr %p\n", __FUNCTION__, dwRetAddr);

    bool b = reinterpret_cast<bool(__thiscall *)(CIFTargetWindowSpecialMob *, long)>(0x0056bd50)(this, ln);

    return b;
}

void CIFTargetWindowSpecialMob::OnUpdateIMPL() {
    reinterpret_cast<void(__thiscall *)(CIFTargetWindowSpecialMob *)>(0x0056bb80)(this);

    //CIFStatic *m_MobType = m_IRM.GetResObj<CIFStatic>(GDR_TWSM_TEXT_LEV, 1);
    //CIFStatic *m_MobIcon = m_IRM.GetResObj<CIFStatic>(GDR_TWSM_ICON, 1);
    //CIFStatic *m_pGDR_TWSM_TEXT_LEV = m_IRM.GetResObj<CIFStatic>(GDR_TWSM_TEXT_ID, 1);          
    //CIFStatic *m_pGDR_TWSM_GAUGE_HPGAUGE = m_IRM.GetResObj<CIFGauge>(GDR_TWSM_GAUGE_HPGAUGE, 1);
    //CIFWnd *m_parent = (CIFWnd *) this->GetParentControl();
    //CIFWnd *m_pCloseBtn = (CIFWnd *) m_parent->m_IRM.GetResObj<CIFWnd>(11, 1);

    //if (m_MobType != NULL) {

    //    if (wcscmp(m_MobType->ReturnText(), TSM_GETTEXTPTR(L"UIIT_CTL_WARENETWORK_DETAIL_NORMAL")) != 0 && !std::n_wstring(m_MobType->ReturnText()).empty()) {
    //        ChangeGWndSize(196, 78);
    //        //SetddjPath(L"interface\\ifcommon\\window_all.ddj", 1, 1);
    //        //m_MobType->SetddjPath("", 1, 1);
    //        m_MobType->ChangeGWndSize(70, 16);
    //        m_MobIcon->ShowWnd(true);
    //        return;
    //    }

    //    wnd_rect bounds = GetBounds();
    //    ChangeGWndSize(197, 55);
    //    //SetddjPath(L"", 0, 0);

    //    //m_MobType->SetddjPath(L"interface\\targetwindow\\tw_mob_wind.ddj", 1, 1);

    //    m_MobType->ChangeGWndSize(197, 55);
    //    m_MobType->ChangeText(L"");
    //    m_MobType->ChangePos(bounds.pos.x, bounds.pos.y);

    //    m_MobIcon->ShowWnd(false);
    //    m_pGDR_TWSM_TEXT_LEV->BringToUp();
    //    m_pGDR_TWSM_GAUGE_HPGAUGE->BringToUp();
    //}
}