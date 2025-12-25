//
// Created by Kurama on 2/25/2023.
//

#include <ctime>
#include "NIFKyuubi09.h"
#include "SRIFLib/NIFWnd.h"

#define GDR_KYUUBI09_LABEL 10
#define GDR_KYUUBI09_LABEL_TIME 13

GFX_IMPLEMENT_DYNCREATE(CNIFKyuubi09, CNIFMainFrame)

GFX_BEGIN_MESSAGE_MAP(CNIFKyuubi09, CNIFMainFrame)
                    ONG_SRO_INTERFACE_PARSE()
GFX_END_MESSAGE_MAP()

void CNIFKyuubi09::OnUpdate() {
    time_t rawtime;
    struct tm *timeinfo;
    wchar_t buffer[80];

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    wcsftime(buffer, sizeof(buffer), L"%d-%m-%Y %H:%M:%S", timeinfo);

    m_pTime_label->SetText(buffer);
}

int CNIFKyuubi09::OnSROInterfaceCreate(int, int) {
    SetText(L"Kyuubi's Window");

    m_pTime_label = GetResObj<CNIFStatic>(GDR_KYUUBI09_LABEL_TIME);
    m_pCustom_label = GetResObj<CNIFStatic>(GDR_KYUUBI09_LABEL);

    m_pCustom_label->SetText(L"Kyuubi is a fox and he has 9 tails.");

    return 0;
}

