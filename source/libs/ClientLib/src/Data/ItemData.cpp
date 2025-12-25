#include "ItemData.h"

const SItemData &CItemData::GetData() const {
    return mData;
}

bool SItemData::IsGlobalMessageScroll() const {
    return m_typeId.Is(TypeIdRegistry::ITEM_ETC_SCROLL_GLOBALCHATTING);
}
