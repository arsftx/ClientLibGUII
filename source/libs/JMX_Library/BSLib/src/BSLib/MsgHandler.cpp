#include "MsgHandler.h"


// 0xBB0110
void CMsgHandler::sub_BB0110(int width, int height) {
    reinterpret_cast<void(__thiscall *)(CMsgHandler *, int, int)>(0x008AD150)(this, width, height); //ECSRO
}
