//
// Created by Kurama on 2/1/2023.
//

#include "NInterfaceResource.h"

void CNInterfaceManager::InstantiateDimensional(const char *p2DFilePath, CGWnd *pParent, bool b) {
    reinterpret_cast<void (__thiscall *)(CNInterfaceManager *, const char *, CGWnd *, bool)>(0x00469140)(this,
                                                                                                           p2DFilePath,
                                                                                                           pParent, b);
}

CNIFWnd *CNInterfaceManager::GetInterfaceObj(int nId) {
    std::n_map<DWORD, CNIFWnd *>::const_iterator it = m_mapInterface.find(nId);
    // if Id not found,
    if (it == m_mapInterface.end())
        return NULL;
    return it->second;
}
