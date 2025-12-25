#pragma once

#define g_hdc (*((HDC *) 0x00ed3ad0))

#include "Process.h"
#include "ICharactor.h"
#include "GameCfg.h"
#include "IFWnd.h"
#include "IFSlotWithHelp.h"

#include <GFX3DFunction/GFXVideo3d.h>
#include <BSLib/BSLib.h>

#define ARRAY_SIZE(ar) ((int)(sizeof(ar)/sizeof(*ar)))

#define g_currentTime (*reinterpret_cast<int *>(0x00ed3820))
#define DAT_00eef5e8 (*reinterpret_cast<undefined4 *>(0x00cb3d40))

// This is used for some debugging stuff ... no idea what this exactly is
#define dword_EECC18 (*reinterpret_cast<int *>(0x00cb19bc))

// extern GlobalPtr<CGWndBase, 0x0110F60C> g_CurrentIF_UnderCursor;

enum ChatType : char {
    CHAT_All = 1,
    CHAT_PM = 2,
    CHAT_AllGM = 3,
    CHAT_Party = 4,
    CHAT_Guild = 5,
    CHAT_Global = 6,
    CHAT_Notice = 7,
    CHAT_Stall = 9,
    CHAT_Ask = 10,
    CHAT_Union = 11,
    CHAT_NPC = 13,
    CHAT_Academy = 16,
};

bool TryCreateCompatibleDC();

bool StartNetEngine();


void DrawRect(int x, int y, int height, int width, int color);
void DrawRect(int x, int y, int height, int width);

void sub_BBDA70(int a1);
void PopulateCharPositionsForNameChange(CGFXVideo3d *p);

void SendRestartRequest(char type);

int sub_4F9C50();
int GetIDOfInterfaceUnderCursor();
CICharactor *GetCharacterObjectByID_MAYBE(int uniqueid);

/// \address 0098caa0
std::n_string GetKindredTextureFilePath(char param_1, int param_2);

/// Write a received message to the chat window
/// \remark This belongs to somewhere near CNetProcessIn
/// \address 0x00876810
void __stdcall WriteToChatWindow(ChatType type, const std::n_wstring &strRecipient, int uniqueid,
                        const std::n_wstring &strMessage, char direction);

GameCfg* Fun_GetCfgGame();

void __stdcall FUN_00bbda70(undefined4 a1);

/// \address 00ba1140
CIFWnd *GetWndByGID(int hgWnd);

struct SOldTriJobRecord {
    BYTE HunterLvl;
    int HunterExp;
    BYTE TraderLvl;
    int TraderExp;
    BYTE ThiefLvl;
    int ThiefExp;

    SOldTriJobRecord()
        : HunterLvl(1),
          HunterExp(0),
          TraderLvl(1),
          TraderExp(0),
          ThiefLvl(1),
          ThiefExp(0) {}
};

class GameDataExt {
public:
    GameDataExt();
    const bool IsHasJobExp() const { return MyJobInfo.HunterExp > 0 || MyJobInfo.ThiefExp > 0 || MyJobInfo.TraderExp > 0; }

public:
    SOldTriJobRecord MyJobInfo;
};

extern GameDataExt *p_GameData;
#define g_pMyGameData p_GameData