#include "IFInventory.h"

CSOItem *CIFInventory::GetItemBySlot(int slotId) {
    return reinterpret_cast<CSOItem *(__thiscall *)(CIFInventory *, int)>(0x004584A0)(this, slotId);
}
