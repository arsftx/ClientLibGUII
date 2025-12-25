#pragma once
#include "Windows.h"
#include <map>
#include "ICPlayer.h"
#include <ClientNet/MsgStreamBuffer.h>

#define FN_CICPLAYER_ICON_MAP_OFFSET 0x00ecf14c
#define p_MiniIcon_TexturePtrs (*reinterpret_cast<IDirect3DBaseTexture9 ***>(FN_CICPLAYER_ICON_MAP_OFFSET))

#define __CHAR_ICON_CHARNAME_ICON_PATH_MAP std::map<std::wstring, std::string>
#define __CHAR_ICON_CHARNAME_ICON_PATH_MAP_IT __CHAR_ICON_CHARNAME_ICON_PATH_MAP::iterator

class CCustomCharIcons {
private:
    static __CHAR_ICON_CHARNAME_ICON_PATH_MAP s_mapCharIconCharnamePathLeft;

    static void __stdcall HasPlayerCustomIcon(CICPlayer *pPlayer);

    BOOL __fastcall MyCICPlayer_SetIcon(void *pSelf, int a2, void *pTextureData);

public:
    int OnPacketRecv(CMsgStreamBuffer *msg);
    static void Initialize();
    static void *lastFoundedIcon;
};