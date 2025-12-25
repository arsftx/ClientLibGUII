#include "IFTargetWindowJobPlayer.h"
#include "unsorted.h"
#include "IFStatic.h"
#include "TextStringManager.h"
#include "ICUser.h"

#include <remodel/MemberFunctionHook.h>

GFX_IMPLEMENT_DYNAMIC_EXISTING(CIFTargetWindowJobPlayer, 0x00eea5bc)

GFX_IMPLEMENT_DYNCREATE_FN(CIFTargetWindowJobPlayer, CIFWnd)

enum {
    GDR_TWJP_JOB_ALIAS = 1, // CIFStatic
    GDR_TWJP_JOB_ICON = 10, // CIFStatic
    GDR_TWJP_JOB_GRADENAME = 11, // CIFStatic
    GDR_TWJP_JOB_GRADE = 12, // CIFStatic
    GDR_TWJP_KINDRED_MARK = 20, // CIFStatic
};

GFX_BEGIN_MESSAGE_MAP(CIFTargetWindowJobPlayer, CIFWnd)

GFX_END_MESSAGE_MAP()

bool CIFTargetWindowJobPlayer::OnCreate(long ln) {
    //printf("%s\n", __FUNCTION__);
    //return reinterpret_cast<bool (__thiscall *)(const CIFTargetWindowJobPlayer *, long)>(0x0069ac40)(this, ln);

    //m_IRM.LoadFromFile("resinfo\\iftw_jobplayer_trijob2.txt");
    //m_IRM.CreateInterfaceSection("Create", this);
    return true;
}

void CIFTargetWindowJobPlayer::FUN_0056b5d0(int objectId) {
    reinterpret_cast<void(__thiscall *)(const CIFTargetWindowJobPlayer *, int)>(0x0056b5d0)(this, objectId);
   /* m_IRM.GetResObj<CIFStatic>(GDR_TWJP_JOB_GRADE, 1)->ShowWnd(false);*/
}
