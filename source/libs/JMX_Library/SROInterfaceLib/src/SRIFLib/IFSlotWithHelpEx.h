#pragma once

#include "NIFWnd.h"
#include "IFSlotWithHelp.h"


class CIFSlotWithHelpEx : public CNIFWnd {
public:
    /// \address 00687d40
    bool OnCreate(long ln) override;

    /// \address 00687db0
    void Func_26(int a1) override;

public:
    CIFSlotWithHelp *m_pSlot; //0x0348

private:
//BEGIN_FIXTURE()
//        ENSURE_SIZE(0x34c)
//        ENSURE_OFFSET(m_pSlot, 0x0348)
//    END_FIXTURE()
//
//    RUN_FIXTURE(CIFSlotWithHelpEx)
};