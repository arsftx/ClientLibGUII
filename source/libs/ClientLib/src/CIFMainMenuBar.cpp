#include "CIFMainMenuBar.h"
#include "IFNormalTile.h"
#include "ICPlayer.h"
#include "Game.h"
#include "IFChatViewer.h"
#include <ctime>
// #include <SilkRoadWeb/SilkroadWeb.h> // Not available in ClientLib
#include "GInterface.h"
// #include "CPSMission.h" // Not available in ClientLib
#include "GlobalDataManager.h"
#include "BSLib/multibyte.h"
#include "IFMessageBox.h"
#include <shellapi.h> // For ShellExecute
#include "TextStringManager.h" // For TSM_GETTEXTPTR

#define CLIENT_SCREEN_WIDTH (theApp.GetRes().res[0].width)
#define CLIENT_SCREEN_HEIGHT (theApp.GetRes().res[0].height)

#define GDR_MAINMENU_RT_LABEL 10
#define GDR_MAINMENU_RT_BTN 11
#define GDR_MAINMENU_RT_LABEL_TIME 13
#define GDR_MAINMENU_RT_MAIN_FRAME 20
#define GDR_MAINMENU_RT_MAIN_FRAME1 31
#define GDR_MAINMENU_RT_MAIN_FRAME2 32
#define GDR_MAINMENU_RT_MAIN_FRAME3 33
#define GDR_MAINMENU_RT_BTN_DISCORD 34
#define GDR_MAINMENU_RT_BTN_FACEBOOK 35
#define GDR_MAINMENU_RT_BTN_WEB 36
#define GDR_MAINMENU_RT_CHARLOCK 37
#define GDR_MAINMENU_RT_CHARNAME 38
#define GDR_MAINMENU_RT_BTN1 21
#define GDR_MAINMENU_RT_BTN2 23
#define GDR_MAINMENU_RT_BTN3 24
#define GDR_MAINMENU_RT_BTN4 25
#define GDR_MAINMENU_RT_BTN5 26
#define GDR_MAINMENU_RT_BTN6 27
#define GDR_MAINMENU_RT_BTN7 28
#define GDR_MAINMENU_RT_BTN8 29

GFX_IMPLEMENT_DYNCREATE(CIFMainMenuBar, CIFMainFrame)

GFX_BEGIN_MESSAGE_MAP(CIFMainMenuBar, CIFMainFrame)
                    ONG_COMMAND(GDR_MAINMENU_RT_BTN, &On_BtnClick_ChangeGrantName)
                    ONG_COMMAND(GDR_MAINMENU_RT_BTN1, &On_BtnClickCharLock)
                    ONG_COMMAND(GDR_MAINMENU_RT_BTN2, &On_BtnClick)
                    ONG_COMMAND(GDR_MAINMENU_RT_BTN3, &On_BtnClick_Title)
                    ONG_COMMAND(GDR_MAINMENU_RT_BTN4, &On_BtnClick_New)
                    ONG_COMMAND(GDR_MAINMENU_RT_BTN5, &On_BtnClick_EventTimer)
                    ONG_COMMAND(GDR_MAINMENU_RT_BTN6, &On_BtnClick_Unknow)
                    ONG_COMMAND(GDR_MAINMENU_RT_BTN7, &On_BtnClick_SendPVP)
                    ONG_COMMAND(GDR_MAINMENU_RT_BTN8, &On_BtnClick_Test)
                    ONG_COMMAND(GDR_MAINMENU_RT_BTN_DISCORD, &On_BtnClick_Discord)
                    ONG_COMMAND(GDR_MAINMENU_RT_BTN_FACEBOOK, &On_BtnClick_Facebook)
                    ONG_COMMAND(GDR_MAINMENU_RT_BTN_WEB, &On_BtnClick_Web)
GFX_END_MESSAGE_MAP()


CIFMainMenuBar::CIFMainMenuBar() :
        m_custom_label(NULL) {

}

bool CIFMainMenuBar::OnCreate(long ln) {
    CIFMainFrame::OnCreate(ln);

    // Set own title
    TB_Func_13("interface\\frame\\mframe_wnd_", 1, 0);

    wnd_size sz = this->GetSize();
    RECT rect_frame = { 17,45,179,74 };

    CIFFrame *frame = (CIFFrame *) CreateInstance(this,
                                                  GFX_RUNTIME_CLASS(CIFFrame),
                                                  rect_frame,
                                                  GDR_MAINMENU_RT_MAIN_FRAME,
                                                  1);
    frame->TB_Func_13("interface\\frame\\frameg_wnd_", 1, 0);

    RECT rect_background = {rect_frame.left + 10,
                            rect_frame.top + 10,
                            rect_frame.right - 20,
                            rect_frame.bottom - 20};

    CIFNormalTile *tile = (CIFNormalTile *) CreateInstance(this,
                                                           GFX_RUNTIME_CLASS(CIFNormalTile),
                                                           rect_background,
                                                           GDR_MAINMENU_RT_MAIN_FRAME,
                                                           1);
    tile->TB_Func_13("interface\\ifcommon\\bg_tile\\com_bg_tile_f.ddj", 1, 0);

    ////char_icon_frame
    RECT rect_frame_char_icon = { 34,64,42,42 };

    CIFFrame *frame_char_icon = (CIFFrame *) CreateInstance(this,
                                                  GFX_RUNTIME_CLASS(CIFFrame),
                                                            rect_frame_char_icon,
                                                  GDR_MAINMENU_RT_MAIN_FRAME,
                                                  1);
    frame_char_icon->TB_Func_13("interface\\frame\\frame_make_", 1, 0);

    RECT rect_background_char_icon = {rect_frame_char_icon.left + 3,
                                      rect_frame_char_icon.top + 3,
                                      rect_frame_char_icon.right - 6,
                                      rect_frame_char_icon.bottom - 6};

    CIFNormalTile *tile_char_icon = (CIFNormalTile *) CreateInstance(this,
                                                           GFX_RUNTIME_CLASS(CIFNormalTile),
                                                                     rect_background_char_icon,
                                                           GDR_MAINMENU_RT_MAIN_FRAME,
                                                           1);
    tile_char_icon->TB_Func_13("interface\\ifcommon\\bg_tile\\com_bg_tile_d.ddj", 1, 0);

    RECT rect_m_char_icon = {rect_frame_char_icon.left + 3,
                                      rect_frame_char_icon.top + 3,
                                      rect_frame_char_icon.right - 6,
                                      rect_frame_char_icon.bottom - 6};
    m_char_icon = (CIFStatic *) CGWnd::CreateInstance(this,
                                                         GFX_RUNTIME_CLASS(CIFStatic),
                                                      rect_m_char_icon,
                                                         GDR_MAINMENU_RT_LABEL,
                                                         1);
    ///////// charname frame
    RECT rect_m_char_name = { 88,85,94,18 };
    m_char_name = (CIFStatic *) CGWnd::CreateInstance(this,
                                                      GFX_RUNTIME_CLASS(CIFStatic),
                                                      rect_m_char_name,
                                                      GDR_MAINMENU_RT_CHARNAME,
                                                      1);
    RECT rect_m_frame_label = { 45,12,118,15 };
    m_frame_label = (CIFStatic *) CGWnd::CreateInstance(this,
                                                      GFX_RUNTIME_CLASS(CIFStatic),
                                                        rect_m_frame_label,
                                                      GDR_MAINMENU_RT_CHARNAME,
                                                      1);
    ////char_lock_frame
    RECT rect_frame_char_lock = { 83,60,99,24 };

    CIFFrame *frame_char_lock = (CIFFrame *) CreateInstance(this,
                                                            GFX_RUNTIME_CLASS(CIFFrame),
                                                            rect_frame_char_lock,
                                                            GDR_MAINMENU_RT_MAIN_FRAME,
                                                            1);
    frame_char_lock->TB_Func_13("interface\\frame\\ub_pet_wnd_", 1, 0);

    RECT rect_background_char_lock = {rect_frame_char_lock.left + 3,
                                      rect_frame_char_lock.top + 3,
                                      rect_frame_char_lock.right - 6,
                                      rect_frame_char_lock.bottom - 6};

    CIFNormalTile *tile_char_lock = (CIFNormalTile *) CreateInstance(this,
                                                                     GFX_RUNTIME_CLASS(CIFNormalTile),
                                                                     rect_background_char_lock,
                                                                     GDR_MAINMENU_RT_MAIN_FRAME,
                                                                     1);
    tile_char_lock->TB_Func_13("interface\\ifcommon\\bg_tile\\com_bg_tile_d.ddj", 1, 0);

    RECT rect_m_char_lock_lable = {rect_frame_char_lock.left + 3,
                             rect_frame_char_lock.top + 3,
                             rect_frame_char_lock.right - 6,
                             rect_frame_char_lock.bottom - 6};
    m_char_lock_lable = (CIFStatic *) CGWnd::CreateInstance(this,
                                                      GFX_RUNTIME_CLASS(CIFStatic),
                                                            rect_m_char_lock_lable,
                                                            GDR_MAINMENU_RT_CHARLOCK,
                                                      1);
    /////////// mainmenu frame
    RECT rect_frame1 = { 17,121,179,385 };

    CIFFrame *frame1 = (CIFFrame *) CreateInstance(this,
                                                  GFX_RUNTIME_CLASS(CIFFrame),
                                                  rect_frame1,
                                                  GDR_MAINMENU_RT_MAIN_FRAME1,
                                                  1);
    frame1->TB_Func_13("interface\\frame\\frameg_wnd_", 1, 0);

    RECT rect_background1 = {rect_frame1.left + 10,
                            rect_frame1.top + 10,
                            rect_frame1.right - 20,
                            rect_frame1.bottom - 20};

    CIFNormalTile *tile1 = (CIFNormalTile *) CreateInstance(this,
                                                           GFX_RUNTIME_CLASS(CIFNormalTile),
                                                           rect_background1,
                                                           GDR_MAINMENU_RT_MAIN_FRAME1,
                                                           1);
    tile1->TB_Func_13("interface\\ifcommon\\bg_tile\\com_bg_tile_f.ddj", 1, 0);
    /////////
    RECT rect_frame2 = { 17,507,179,58 };

    CIFFrame *frame2 = (CIFFrame *) CreateInstance(this,
                                                   GFX_RUNTIME_CLASS(CIFFrame),
                                                   rect_frame2,
                                                   GDR_MAINMENU_RT_MAIN_FRAME1,
                                                   1);
    frame2->TB_Func_13("interface\\frame\\frameg_wnd_", 1, 0);

    RECT rect_background2 = {rect_frame2.left + 10,
                             rect_frame2.top + 10,
                             rect_frame2.right - 20,
                             rect_frame2.bottom - 20};

    CIFNormalTile *tile2 = (CIFNormalTile *) CreateInstance(this,
                                                            GFX_RUNTIME_CLASS(CIFNormalTile),
                                                            rect_background2,
                                                            GDR_MAINMENU_RT_MAIN_FRAME1,
                                                            1);
    tile2->TB_Func_13("interface\\ifcommon\\bg_tile\\com_bg_tile_f.ddj", 1, 0);
    ////// discord_icon
    RECT rect_discord_icon_button = {33,515,40,40};
    CIFButton *discord_icon_button = (CIFButton *) CGWnd::CreateInstance(this,
                                                         GFX_RUNTIME_CLASS(CIFButton),
                                                         rect_discord_icon_button,
                                                                         GDR_MAINMENU_RT_BTN_DISCORD,
                                                         1);

    discord_icon_button->TB_Func_13("interface\\ifcommon\\thaidu0ngpr0\\discord.ddj", 1, 1);
    ////// facebook_icon
    RECT rect_facebook_icon_button = {88,515,40,40};
    CIFButton *facebook_icon_button = (CIFButton *) CGWnd::CreateInstance(this,
                                                                         GFX_RUNTIME_CLASS(CIFButton),
                                                                          rect_facebook_icon_button,
                                                                         GDR_MAINMENU_RT_BTN_FACEBOOK,
                                                                         1);

    facebook_icon_button->TB_Func_13("interface\\ifcommon\\thaidu0ngpr0\\facebook.ddj", 1, 1);
    ////// web_icon
    RECT rect_web_icon_button = {144,515,40,40};
    CIFButton *web_icon_button = (CIFButton *) CGWnd::CreateInstance(this,
                                                                          GFX_RUNTIME_CLASS(CIFButton),
                                                                          rect_web_icon_button,
                                                                          GDR_MAINMENU_RT_BTN_WEB,
                                                                          1);

    web_icon_button->TB_Func_13("interface\\ifcommon\\thaidu0ngpr0\\website.ddj", 1, 1);
    ////////
    RECT rect_custom_label = {50, 50, 60, 20};
    m_custom_label = (CIFStatic *) CGWnd::CreateInstance(this,
                                                         GFX_RUNTIME_CLASS(CIFStatic),
                                                         rect_custom_label,
                                                         GDR_MAINMENU_RT_LABEL,
                                                         1);


    RECT rect_label_time = {50, 70, 60, 20};
    m_time_label = (CIFStatic *) CGWnd::CreateInstance(this,
                                                       GFX_RUNTIME_CLASS(CIFStatic),
                                                       rect_label_time,
                                                       GDR_MAINMENU_RT_LABEL_TIME,
                                                       1);
///////// button
    RECT rect_button = {rect_frame1.left + 20,
                         rect_frame1.top + 60,
                         rect_frame1.right - 40,
                         30};

   btn = (CIFButton *) CGWnd::CreateInstance(this,
                                                         GFX_RUNTIME_CLASS(CIFButton),
                                                         rect_button,
                                                         GDR_MAINMENU_RT_BTN,
                                                         1);
    wchar_t buffer[255];
    swprintf_s(buffer, L"Event Register"); // TSM_GETTEXTPTR(L"UIIT_STT_EVENTREG4_NAME")
    // btn ->Func_51(buffer);
    btn->TB_Func_13("interface\\ifcommon\\com_mid_button.ddj", 1, 1);

    RECT rect_button1 = {rect_frame1.left + 20,
                        rect_frame1.top + 20,
                        rect_frame1.right - 40,
                        30};
   btn1 = (CIFButton *) CGWnd::CreateInstance(this,
                                                         GFX_RUNTIME_CLASS(CIFButton),
                                                         rect_button1,
                                                         GDR_MAINMENU_RT_BTN1,
                                                         1);

    wchar_t buffer1[255];
    swprintf_s(buffer1, L"Item Lock"); // TSM_GETTEXTPTR(L"UIIT_ST_ITEM_LOCK")
    // btn1 ->Func_51(buffer1);
    btn1->TB_Func_13("interface\\ifcommon\\com_mid_button.ddj", 1, 1);


    RECT rect_button2 = {rect_frame1.left + 20,
                         rect_frame1.top + 100,
                         rect_frame1.right - 40,
                         30};
    btn2 = (CIFButton *) CGWnd::CreateInstance(this,
                                                          GFX_RUNTIME_CLASS(CIFButton),
                                                          rect_button2,
                                                          GDR_MAINMENU_RT_BTN2,
                                                          1);

    // btn2->Func_51(L"Unique History"); // TSM_GETTEXTPTR(L"UIIT_ST_UNIQUE_LABLE")
    btn2->TB_Func_13("interface\\ifcommon\\com_mid_button.ddj", 1, 1);

    RECT rect_button3 = {rect_frame1.left + 20,
                         rect_frame1.top + 140,
                         rect_frame1.right - 40,
                         30};
    btn3 = (CIFButton *) CGWnd::CreateInstance(this,
                                                          GFX_RUNTIME_CLASS(CIFButton),
                                                          rect_button3,
                                                          GDR_MAINMENU_RT_BTN3,
                                                          1);

    // btn3->Func_51(L"Title Switch"); // TSM_GETTEXTPTR(L"UIIT_ST_TITLE_SWITCH")
    btn3->TB_Func_13("interface\\ifcommon\\com_mid_button.ddj", 1, 1);
    RECT rect_button4 = {rect_frame1.left + 20,
                         rect_frame1.top + 180,
                         rect_frame1.right - 40,
                         30};
    btn4 = (CIFButton *) CGWnd::CreateInstance(this,
                                                          GFX_RUNTIME_CLASS(CIFButton),
                                                          rect_button4,
                                                          GDR_MAINMENU_RT_BTN4,
                                                          1);

    // btn4->Func_51(L"Custom Rank"); // TSM_GETTEXTPTR(L"UIIT_ST_CUSTOMRANK_TILE")
    btn4->TB_Func_13("interface\\ifcommon\\com_mid_button.ddj", 1, 1);

    RECT rect_button5 = {rect_frame1.left + 20,
                         rect_frame1.top + 220,
                         rect_frame1.right - 40,
                         30};
    btn5 = (CIFButton *) CGWnd::CreateInstance(this,
                                                          GFX_RUNTIME_CLASS(CIFButton),
                                                          rect_button5,
                                                          GDR_MAINMENU_RT_BTN5,
                                                          1);

    // btn5->Func_51(L"Event Timer"); // TSM_GETTEXTPTR(L"UIIT_ST_EVENTTIMER_TILE")
    btn5->TB_Func_13("interface\\ifcommon\\com_mid_button.ddj", 1, 1);

    RECT rect_button6 = {rect_frame1.left + 20,
                         rect_frame1.top + 260,
                         rect_frame1.right - 40,
                         30};
    btn6 = (CIFButton *) CGWnd::CreateInstance(this,
                                                          GFX_RUNTIME_CLASS(CIFButton),
                                                          rect_button6,
                                                          GDR_MAINMENU_RT_BTN6,
                                                          1);

    // btn6->Func_51(L"Total Point"); // TSM_GETTEXTPTR(L"UIIT_ST_TOTAL_POINT")
    btn6->TB_Func_13("interface\\ifcommon\\com_mid_button.ddj", 1, 1);

    RECT rect_button7 = {rect_frame1.left + 20,
                         rect_frame1.top + 300,
                         rect_frame1.right - 40,
                         30};
    btn7 = (CIFButton *) CGWnd::CreateInstance(this,
                                                          GFX_RUNTIME_CLASS(CIFButton),
                                                          rect_button7,
                                                          GDR_MAINMENU_RT_BTN7,
                                                          1);

    // btn7->Func_51(L"PVP Challenge"); // TSM_GETTEXTPTR(L"UIIT_STT_PVP_CHALLENGE")
    btn7->TB_Func_13("interface\\ifcommon\\com_mid_button.ddj", 1, 1);

    RECT rect_button8 = {rect_frame1.left + 20,
                         rect_frame1.top + 340,
                         rect_frame1.right - 40,
                         30};
    btn8 = (CIFButton *) CGWnd::CreateInstance(this,
                                                          GFX_RUNTIME_CLASS(CIFButton),
                                                          rect_button8,
                                                          GDR_MAINMENU_RT_BTN8,
                                                          1);

    // btn8->Func_51(L"NPC Shop"); // TSM_GETTEXTPTR(L"UIIT_ST_CUSTOMNPC_SHOP")
    btn8->TB_Func_13("interface\\ifcommon\\com_mid_button.ddj", 1, 1);
    
    // m_frame_label ->Func_51(L"ST Filter Menu");
    this->ShowGWnd(false);
    return true;
}



void CIFMainMenuBar::OnUpdate() {
    time_t rawtime;
    struct tm *timeinfo;
    wchar_t buffer[80];

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    wcsftime(buffer, sizeof(buffer), L"%d-%m-%Y %H:%M:%S", timeinfo);

    if (m_char_name) {
        if (g_pCICPlayer) {
            // m_char_name->Func_51(g_pCICPlayer->GetCharName().c_str());
        }
        m_char_name->SetFont(theApp.GetFont(0));
        m_char_name->TB_Func_5(0); //can le
        m_char_name->TB_Func_6(2); // co chu
    }

    // CPSMission checks removed for now as we don't have that class fully integrated yet
    // We can re-enable them later if we port CPSMission
}

int CIFMainMenuBar::OnMouseMove(int a1, int x, int y) {
    this->BringToFront();
    return 0;
}

void CIFMainMenuBar::On_BtnClick() {
    g_pCGInterface->ShowMessage_Notice(L"Unique History Clicked");
}

void CIFMainMenuBar::On_BtnClick_Test() {
     g_pCGInterface->ShowMessage_Notice(L"Test Clicked");
}

void CIFMainMenuBar::On_BtnClick_SendPVP() {
    g_pCGInterface->ShowMessage_Notice(L"PVP Challenge Clicked");
}

void CIFMainMenuBar::On_BtnClickCharLock() {
    g_pCGInterface->ShowMessage_Notice(L"Char Lock Clicked");
}

void CIFMainMenuBar::On_BtnClick_Title() {
    g_pCGInterface->ShowMessage_Notice(L"Title Switch Clicked");
}

void CIFMainMenuBar::On_BtnClick_ChangeGrantName() {
    g_pCGInterface->ShowMessage_Notice(L"Grant Name Clicked");
}

void CIFMainMenuBar::On_BtnClick_New() {
    g_pCGInterface->ShowMessage_Notice(L"Custom Rank Clicked");
}

void CIFMainMenuBar::On_BtnClick_EventTimer() {
    g_pCGInterface->ShowMessage_Notice(L"Event Timer Clicked");
}

void CIFMainMenuBar::On_BtnClick_Unknow()  {
    g_pCGInterface->ShowMessage_Notice(L"Total Point Clicked");
}

void CIFMainMenuBar::On_BtnClick_Discord() {
    ShellExecute(NULL, "open", "https://discord.gg/yourserver", NULL, NULL, SW_SHOWNORMAL);
}

void CIFMainMenuBar::On_BtnClick_Facebook() {
    ShellExecute(NULL, "open", "https://facebook.com/yourpage", NULL, NULL, SW_SHOWNORMAL);
}

void CIFMainMenuBar::On_BtnClick_Web() {
    ShellExecute(NULL, "open", "https://yourwebsite.com", NULL, NULL, SW_SHOWNORMAL);
}

void CIFMainMenuBar::ResetPosition()
{
    USHORT PosX = 0, PosY = 165;
    PosY = CLIENT_SCREEN_HEIGHT - 600;
    PosX = CLIENT_SCREEN_WIDTH - 305;
    this->MoveGWnd(PosX, PosY);
}
