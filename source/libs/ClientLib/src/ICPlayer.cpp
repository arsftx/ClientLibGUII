#include "ICPlayer.h"
#include "GInterface.h"

GFX_IMPLEMENT_DYNAMIC_EXISTING(CICPlayer, 0x0A04418)


// CICPlayer::IsGameMasterMAYBE(void) .text 00826e10 00000009   R . . . . T .
bool CICPlayer::IsGameMaster() {
    return reinterpret_cast<bool(__thiscall *) (CICPlayer *)>(0x00826e10)(this);
}

// sub_8288C0 .text 8288C0 00000032 00000008 00000004 R . . . . T .
// I am pretty sure this func simply returned a copy of the players name ... ridiculous!
std::n_wstring *CICPlayer::sub_9D6580(std::n_wstring *str) {
    return reinterpret_cast<std::n_wstring *(__thiscall *) (CICPlayer *, std::n_wstring *)>(0x008288C0)(this, str);
}

std::n_wstring CICPlayer::GetCharName() const {
    return MEMUTIL_READ_BY_PTR_OFFSET(this, 0x77c, std::n_wstring);
}

unsigned char CICPlayer::GetCurrentLevel() const {
    return MEMUTIL_READ_BY_PTR_OFFSET(this, 0x798, unsigned char);
}

long long int CICPlayer::GetCurrentExp() const {
    return MEMUTIL_READ_BY_PTR_OFFSET(this, 0x7a4, long long int);
}

short CICPlayer::GetStatPointAvailable() const {
    return MEMUTIL_READ_BY_PTR_OFFSET(this, 0x7b4, short);
}

short CICPlayer::GetSkillPoints() const {
    return MEMUTIL_READ_BY_PTR_OFFSET(this, 0x7b0, short);
}

short CICPlayer::GetStrength() const {
    return MEMUTIL_READ_BY_PTR_OFFSET(this, 0x7ac, short);
}

short CICPlayer::GetIntelligence() const {
    return MEMUTIL_READ_BY_PTR_OFFSET(this, 0x7ae, short);
}

const std::n_wstring &CICPlayer::GetJobAlias() const {
    return MEMUTIL_READ_BY_PTR_OFFSET(this, 0x179c, std::n_wstring);
}

int CICPlayer::GetCurrentJobExperiencePoints() const {
    //offset 0x17d0
    return reinterpret_cast<int(__thiscall *)(const CICPlayer *)>(0x00826ec0)(this);
}

void CICPlayer::Func_15(int param_1, float *param_2) {
    //printf("%s %d %p\n", __FUNCTION__, param_1, param_2);
    //reinterpret_cast<void (__thiscall *)(const CICharactor *, int param_1, float *param_2)>(0x009d49c0)(this, param_1, param_2);
    if ((param_1 == 3) && (g_pCGInterface != NULL)) {
        g_pCGInterface->FUN_00777a70(0,1);
    }
    CICUser::Func_15(param_1,param_2);
}

void CICPlayer::Func_15_impl(int param_1, float *param_2) {
    CICPlayer::Func_15(param_1, param_2);
}

undefined4 CICPlayer::FUN_009d68f0() {
    return reinterpret_cast<undefined4 (__thiscall *)(const CICPlayer*)>(0x009d68f0)(this);
}

CCOSDataMgr *CICPlayer::GetCosMgr() {
    return MEMUTIL_READ_BY_PTR_OFFSET(this, 0x17CC, CCOSDataMgr *);
}

SCOSInfo *CCOSDataMgr::GetSCOSInfo(int UniqueId) const {
    return reinterpret_cast<SCOSInfo *(__thiscall *) (const CCOSDataMgr *, int)>(0x007f5b00 /*0x004C0FD0*/)(this, UniqueId);
}

SCOSInfo *CCOSDataMgr::GetCOSInfoBase() {
    std::n_map<DWORD, SCOSInfo *>::const_iterator it = m_mapCOSData.begin();

    for (; it != m_mapCOSData.end(); ++it) {
        if (it->first == m_selectedPetUniqueId) {
            if (it->second->GetInventoryType() == 3 ||
                it->second->GetInventoryType() == 9 ||
                it->second->GetInventoryType() == 2) {
                return it->second;
            }
        }
    }

    return NULL;
}

SCOSInfo *CCOSDataMgr::GetTradeInfo() {
    std::n_map<DWORD, SCOSInfo *>::const_iterator it = m_mapCOSData.begin();

    for (; it != m_mapCOSData.end(); ++it) {
        if (it->first == m_selectedPetUniqueId) {
            if (it->second->GetInventoryType() == 1) {
                return it->second;
            }
        }
    }

    return NULL;
}

int CCOSDataMgr::GetSelectedPetGameId() const {
    return MEMUTIL_READ_BY_PTR_OFFSET(this, 0x10, int);
}

byte SCOSInfo::GetInventoryType() const {
    // Note: inventoryType is now at a different location, using petType as fallback
    return m_petType;  // Or access raw offset if needed
}

int SCOSInfo::GetUniqueId() const {
    return m_uniqueID;
}

int SCOSInfo::GetOwnerUniqueId() const {
    return m_refObjID;  // RefObjID acts as owner reference
}

int SCOSInfo::GetItemId() const {
    return m_refObjID;
}

int SCOSInfo::GetMaxStack() const {
    return MEMUTIL_READ_BY_PTR_OFFSET(this, 0x10, int);  // Keep for compatibility
}

int SCOSInfo::GetTotalInvSlotCount() const {
    return MEMUTIL_READ_BY_PTR_OFFSET(this, 0x18, int);  // Keep for compatibility
}

std::n_wstring SCOSInfo::GetPetName() const {
    return m_name;
}
