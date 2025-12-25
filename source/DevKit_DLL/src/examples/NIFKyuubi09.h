//
// Created by Kurama on 2/25/2023.
//
#pragma once

#include "SRIFLib/NIFMainFrame.h"

class CNIFKyuubi09 : public CNIFMainFrame {
GFX_DECLARE_DYNCREATE(CNIFKyuubi09)

GFX_DECLARE_MESSAGE_MAP(CNIFKyuubi09)

public:
    void OnUpdate() override;

private:
    // OnCreate is useless if u create the class from 2dt file
    // so this will be the new OnCreate
    // called when parse 2dt file end
    int OnSROInterfaceCreate(int, int);

private:
    CNIFStatic* m_pTime_label;
    CNIFStatic* m_pCustom_label;
};