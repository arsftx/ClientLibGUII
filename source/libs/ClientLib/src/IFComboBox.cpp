#include "IFComboBox.h"

void CIFComboBox::SetMaxSlotInList(int nCount) {
    if (nCount < 1)
        nCount = 1;

}

size_t CIFComboBox::GetTextureListSize() const {
    return reinterpret_cast<size_t (__thiscall *)(const CIFComboBox *)>(0x00707160)(this);
}

int CIFComboBox::GetSelectedSlotId() const {
    return m_nSelectedSlotNum;
}
void CIFComboBox::SetSelectedSlotId(int value)
{
    MEMUTIL_WRITE_BY_PTR_OFFSET(this, m_nSelectedSlotNum, int, value);
}
void CIFComboBox::SelectSlotList(int nSlotId) {
    reinterpret_cast<void (__thiscall *)(CIFComboBox *, int)>(0x0042D290)(this, nSlotId);
}

void CIFComboBox::EraseSlotFromList(size_t nSlotId) {
    reinterpret_cast<int (__thiscall *)(CIFComboBox *, size_t)>(0x00708600)(this, nSlotId);
}

void CIFComboBox::InsertSlotToList(int nSlotId, const char *wstr) {
    reinterpret_cast<void (__thiscall *)(CIFComboBox *, int, const char *)>(0x0042D460)(this, nSlotId, wstr);
}
