#include "CustomCharIcons.h"
#include <BSLib/_internal/custom_stl.h>
#include <memory/util.h>
#include <GInterface.h>
#include <memory/RemodelUtility.h>
#include <memory/hook.h>
#include <TextStringManager.h>


#define FN_CICPLAYER_GET_ICON_STATE_OFFSET 0x0082e570
#define FN_CICPLAYER_IMPL_JOB_ICON_OFFSET 0x0082e4e3

#define FN_CICPLAYER_GET_ICON_STATE_HOOK_OFFSET 0x0082e576
const DWORD onCharIconHookAddrJmpback = 0x0082e57e;

void *CCustomCharIcons::lastFoundedIcon;

#define CICPLAYER_VFT_ADDR 0xBD8B34
#define CICUSER_VFT_ADDR 0xBD8D44


//009e49b0

int CCustomCharIcons::OnPacketRecv(CMsgStreamBuffer *msg) {

    switch (msg->msgid()) {
        case 0x5010: {

            BYTE Type;
            *msg >> Type;
            switch (Type) {
                case 1: {
                    BYTE TraderLvl;
                    *msg >> TraderLvl;
                    int TraderExp;
                    *msg >> TraderExp;

                    //printf("%s TraderLvl %d TraderExp %d\n", __FUNCTION__, TraderLvl, TraderExp);

                    BYTE HunterLvl;
                    *msg >> HunterLvl;
                    int HunterExp;
                    *msg >> HunterExp;

                    // printf("%s HunterLvl %d HunterExp %d\n", __FUNCTION__, HunterLvl, HunterExp);

                    BYTE ThiefLvl;
                    *msg >> ThiefLvl;
                    int ThiefExp;
                    *msg >> ThiefExp;

                    // printf("%s ThiefLvl %d ThiefExp %d\n", __FUNCTION__, ThiefLvl, ThiefExp);

                    g_pMyGameData->MyJobInfo.TraderLvl = TraderLvl;
                    g_pMyGameData->MyJobInfo.TraderExp = TraderExp;

                    g_pMyGameData->MyJobInfo.HunterLvl = HunterLvl;
                    g_pMyGameData->MyJobInfo.HunterExp = HunterExp;

                    g_pMyGameData->MyJobInfo.ThiefLvl = ThiefLvl;
                    g_pMyGameData->MyJobInfo.ThiefExp = ThiefExp;
                } break;
                case 2: {

                    TRIJOB_TYPE JobType;
                    *msg >> JobType;

                    int ExpToLose;
                    *msg >> ExpToLose;

                    wchar_t *Key;
                    const wchar_t *TxtUi = L"UIIT_STT_JOB_EXP_%s_LOST";

                    switch (JobType) {
                        case TRIJOB_HUNTER:
                            Key = L"HUNTER";

                            //if (g_pMyGameData->MyJobInfo.HunterExp - ExpToLose < 0)
                            //    g_pMyGameData->MyJobInfo.HunterExp = 0;
                            //else
                            //    g_pMyGameData->MyJobInfo.HunterExp -= ExpToLose;

                            break;
                        case TRIJOB_THIEF:
                            Key = L"THIEF";

                            //if (g_pMyGameData->MyJobInfo.ThiefExp - ExpToLose < 0)
                            //    g_pMyGameData->MyJobInfo.ThiefExp = 0;
                            //else
                            //    g_pMyGameData->MyJobInfo.ThiefExp -= ExpToLose;

                            break;
                        case TRIJOB_TRADER:
                            Key = L"MERCHANT";
                            //if (g_pMyGameData->MyJobInfo.TraderExp - ExpToLose < 0)
                            //    g_pMyGameData->MyJobInfo.TraderExp = 0;
                            //else
                            //    g_pMyGameData->MyJobInfo.TraderExp -= ExpToLose;
                            break;
                    }

                    wchar_t buffer[0x100];
                    swprintf(buffer, size(buffer), TxtUi, Key);
                    
                    //wprintf(L"%s - buffer %s\n", __FUNCTIONW__, buffer);

                    const wchar_t *FinalTxt = TSM_GETTEXTPTR(buffer);
                    swprintf(buffer, size(buffer), FinalTxt, ExpToLose);
                    //wprintf(L"%s - FinalTxt %s\n", __FUNCTIONW__, buffer);

                    g_pCGInterface->WriteSystemMessage(SYSLOG_ERR, buffer);

                } break;
            }

            msg->FlushRemaining();
        } break;
    }

    return reinterpret_cast<int(__thiscall *)(CCustomCharIcons *, CMsgStreamBuffer *)>(0x006f9b10)(this, msg);
}

__declspec(naked) void IL_MyCICPlayer_GetIconState() {
    __asm
    {
		REMODEL_SAVE_REGISTERS_BEFORE_STDCALL;
		PUSH ESI;
		CALL CCustomCharIcons::HasPlayerCustomIcon;
		REMODEL_RESTORE_REGISTERS_AFTER_STDCALL;

        MOV EAX, FN_CICPLAYER_ICON_MAP_OFFSET;
		MOV EAX, CCustomCharIcons::lastFoundedIcon;
		jmp onCharIconHookAddrJmpback;
    }
}

void CCustomCharIcons::Initialize() {

    CCustomCharIcons::lastFoundedIcon = NULL;

    MEMUTIL_NOP(FN_CICPLAYER_GET_ICON_STATE_OFFSET, 6);
    MEMUTIL_NOP(FN_CICPLAYER_GET_ICON_STATE_HOOK_OFFSET, 8);
    MEMUTIL_NOP(0x0082e683, 6);//show guild while jobbing

    //* inside job name
    MEMUTIL_WRITE_VALUE(BYTE, 0x00bb1d7c, 0x00);

    MEMUTIL_SETUP_HOOK(LongJump,
                       (uintptr_t) FN_CICPLAYER_GET_ICON_STATE_HOOK_OFFSET,
                       (uintptr_t) IL_MyCICPlayer_GetIconState);

    //job icons
    MEMUTIL_WRITE_VALUE(BYTE, FN_CICPLAYER_IMPL_JOB_ICON_OFFSET, MEMUTIL_ASM_OPCODE_SHORT_JUMP);
    replaceOffset(0x0082e466, addr_from_this(&CCustomCharIcons::MyCICPlayer_SetIcon));
}

void __stdcall CCustomCharIcons::HasPlayerCustomIcon(CICPlayer *pPlayer) {
    if (g_pCICPlayer && pPlayer && ((*(DWORD *) pPlayer == CICPLAYER_VFT_ADDR || *(DWORD *) pPlayer == CICUSER_VFT_ADDR))) {

        //undefined1 trijobType = pPlayer->FUN_009db0d0();//reg type not the state
        TRIJOB_TYPE trijobType = pPlayer->GetJobState();
        // printf("trijobType %d\n", trijobType);

        if (trijobType == TRIJOB_TRADER) {
            lastFoundedIcon = p_MiniIcon_TexturePtrs[0x00];
        } else if (trijobType == TRIJOB_THIEF) {
            lastFoundedIcon = p_MiniIcon_TexturePtrs[0x1];
        } else if (trijobType == TRIJOB_HUNTER) {
            lastFoundedIcon = p_MiniIcon_TexturePtrs[0x2];
        } else {
            lastFoundedIcon = p_MiniIcon_TexturePtrs[0x04];
        }
    }
}

BOOL CCustomCharIcons::MyCICPlayer_SetIcon(void *pSelf, int a2, void *pTextureData)
{
    return TRUE;
    pTextureData = NULL;
    return reinterpret_cast<BOOL(__thiscall *)(void *, int, void *)>(0x004c20c0)(pSelf, a2, pTextureData);
}