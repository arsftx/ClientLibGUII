#include "ICharactor.h"
#include <memory/util.h>

CLASSLINK_STATIC_IMPL(CICharactor)

void CICharactor::ShowMessageAboveEntity(const std::n_wstring &message, D3DCOLOR color) {
    reinterpret_cast<void(__thiscall *)(CICharactor *, const std::n_wstring *, D3DCOLOR)>(0x0081b1b0)(this, &message, color);
}

unsigned int CICharactor::GetMaxHp() const {
    return MEMUTIL_READ_BY_PTR_OFFSET(this, 0x3c8, unsigned int);
}

unsigned int CICharactor::GetCurrentHp() const {
    return MEMUTIL_READ_BY_PTR_OFFSET(this, 0x3c0, unsigned int);
}

unsigned int CICharactor::FUN_009c7880() const {
    return reinterpret_cast<unsigned int(__thiscall *)(const CICharactor *)>(0x0081b270)(this);
}

unsigned int CICharactor::GetCurrentMp() const {
    return MEMUTIL_READ_BY_PTR_OFFSET(this, 0x3bc, unsigned int);
}

unsigned int CICharactor::GetMaxMp() const {
    return MEMUTIL_READ_BY_PTR_OFFSET(this, 0x3c4, unsigned int);

    return m_mpMax;
}

void CICharactor::Func_15(int param_1, float *param_2) {
    //printf("%s\n", __FUNCTION__);
    reinterpret_cast<void (__thiscall *)(const CICharactor *, int param_1, float *param_2)>(0x009cf1b0)(this, param_1, param_2);
}

void CICharactor::Func_15_impl(int param_1, float *param_2) {
    CICharactor::Func_15(param_1, param_2);
}
