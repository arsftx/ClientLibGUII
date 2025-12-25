#pragma once

#include "IClientNet.h"

class CClientNet : public IClientNet
{
	public:
    static CClientNet *get() {
        return *reinterpret_cast<CClientNet **>(0x00f1f0a8);
    }

};

