#pragma once

#include "IFWnd.h"

class CIFTargetWindowPlayer : public CIFWnd {
    GFX_DECLARE_DYNAMIC_EXISTING(CIFTargetWindowPlayer, 0x00eea5dc)

    GFX_DECLARE_DYNCREATE_FN(CIFTargetWindowPlayer)

    GFX_DECLARE_MESSAGE_MAP(CIFTargetWindowPlayer)

public:
    /// \address 0069b180
    bool OnCreate(long ln) override;
    bool OnCreateIMPL(long ln);

    

    void OnUpdateIMPL();

    int sub_9D3B60(int a1);

    /// \address 0056ba10
    void FUN_0056ba10(int objectId);

    int sub_6B9320(char a2, char a3, char a4, char a5, char a6);

    void OnSpeicaltyRenderMySelf();

    void OnMyCIFCosInventoryRenderMySelf();

private:
    int m_objectId; //0x036C

private:
    /*BEGIN_FIXTURE()
        ENSURE_SIZE(0x370)
        ENSURE_OFFSET(m_objectId, 0x036C)
    END_FIXTURE()

    RUN_FIXTURE(CIFTargetWindowPlayer)*/
};

