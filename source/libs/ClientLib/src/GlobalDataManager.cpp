#include "GlobalDataManager.h"
#include <BSLib/Debug.h>


undefined4 CGlobalDataManager::FUN_008cbac0(int a1) {
    return reinterpret_cast<undefined4(__thiscall *)(CGlobalDataManager *, int)>(0x008cbac0)(this, a1);
}

const CCharacterData *CGlobalDataManager::GetCharacter(int id) {
    std::n_map<unsigned __int32, CCharacterData *>::const_iterator it = m_characterDataMap.find(id);
    if (it == m_characterDataMap.end()) {
        if (LoadFromPk2(0, id) == 0) {
            return NULL;
        } else {
            it = m_characterDataMap.find(id);
        }
    }
    return it->second;
}

const SItemData &CGlobalDataManager::GetItemData(int refObjItemId) const {

    const CItemData *pItem = GetItem(refObjItemId);
    BS_ASSERT_MSG(pItem != NULL, "Item: %d", pItem);
    return pItem->GetData();
}

const CItemData *CGlobalDataManager::GetItem(int refObjItemId) const {
    return reinterpret_cast<const CItemData *(__thiscall *)(const CGlobalDataManager *, int)>(0x00616850)(this,
                                                                                                          refObjItemId);
}

//007a9660
const CLevelData *CGlobalDataManager::GetLevel(int level) const {
    return reinterpret_cast<const CLevelData *(__thiscall *) (const CGlobalDataManager *, int)>(0x007a9660)(this, level);
}

const SLevelData&CGlobalDataManager::GetLevelData(int level) const {
    return reinterpret_cast<const SLevelData &(__thiscall *) (const CGlobalDataManager *, int)>(0x007a9720)(this, level);
}

undefined CGlobalDataManager::FUN_00939a60(undefined4 param_1) {
    return reinterpret_cast<undefined (__thiscall *)(const CGlobalDataManager *, undefined4)>(0x00939a60)(this,
                                                                                                          param_1);
}

undefined4 CGlobalDataManager::FUN_0093a610(std::n_wstring param_1) {
    return reinterpret_cast<undefined4 (__thiscall *)(const CGlobalDataManager *, std::n_wstring)>(0x0093a610)(this, param_1);
}

undefined4 CGlobalDataManager::LoadFromPk2(int typeMAYBE, int id) {
    return reinterpret_cast<undefined4(__thiscall *)(CGlobalDataManager *, int, int)>(0x0093ef10)(this, typeMAYBE, id);
}
