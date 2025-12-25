#pragma once

#include "IFWnd.h"

class CIFBuffViewer : public CIFWnd {
    GFX_DECLARE_DYNAMIC_EXISTING(CIFBuffViewer, 0x00CB16AC)

    GFX_DECLARE_DYNCREATE_FN(CIFBuffViewer)

    GFX_DECLARE_MESSAGE_MAP(CIFBuffViewer)

    struct SBuffListData {
    private:
        char pad[0x30];
    };

public:

    CIFBuffViewer();

    /// \address 007fab80
    bool OnCreate(long ln) override;
private:
    undefined4 field1_0x36c;
    undefined4 field2_0x370;
    undefined4 field3_0x374;
    undefined4 field4_0x378;
    undefined4 field5_0x37c;
    undefined4 field6_0x380;
    undefined4 field7_0x384;
    std::n_list<SBuffListData> field8_0x388;
    std::n_list<SBuffListData> field9_0x394;
    undefined4 field10_0x3a0;
    undefined4 field11_0x3a4;

public:
    // === ICONS PER ROW OFFSET (from IDA analysis) ===
    // CIFBuffViewer size: 736 bytes (0x2E0) - ECSRO client
    // this[175] = offset 700 (0x2BC) = iconsPerRow (default 8)
    // this[176] = offset 704 (0x2C0) = iconWidth (default 21)
    // this[177] = offset 708 (0x2C4) = iconHeight (default 21)
    static const int OFFSET_ICONS_PER_ROW = 0x2BC;  // 700
    static const int OFFSET_ICON_WIDTH = 0x2C0;     // 704
    static const int OFFSET_ICON_HEIGHT = 0x2C4;    // 708

private:
   /* BEGIN_FIXTURE()
        ENSURE_SIZE(0x3a8)
    END_FIXTURE()

    RUN_FIXTURE(CIFBuffViewer)*/
};
