#pragma once

#include "ICUser.h"
#include "SOItem.h"

struct SCOSInfo {
    // ========== SCOSInfo Structure (from IDA analysis of sub_632E10) ==========
    DWORD m_refObjID;        // +0x00 - RefObjID / CharDataID
    DWORD m_uniqueID;        // +0x04 - Unique game ID
    DWORD m_currentHP;       // +0x08 - Current HP
    DWORD m_maxHP;           // +0x0C - Max HP
    WORD  m_hgp;             // +0x10 - HGP (Hunger/Growth Points, max 10000)
    BYTE  m_unknown12;       // +0x12 - Unknown (possibly level-related)
    BYTE  m_padding13;       // +0x13 - Padding
    BYTE  m_petType;         // +0x14 - Pet Type (0=Horse, 1=Attack/Pick, 2=?, 3=Fellow, 4=?)
    BYTE  m_padding15[3];    // +0x15-0x17 - Padding
    DWORD m_status;          // +0x18 - Status flags
    std::n_wstring m_name;   // +0x1C - Pet name (n_wstring = 12 bytes: begin, end, capacity)
    char  m_padding28[0x18]; // +0x28-0x3F - Padding to align EXP
    DWORD m_currentExpLow;   // +0x40 - Current EXP (low 32 bits)
    DWORD m_currentExpHigh;  // +0x44 - Current EXP (high 32 bits)
    DWORD m_maxExpLow;       // +0x48 - Max EXP (low 32 bits, Fellow Pet only)
    DWORD m_maxExpHigh;      // +0x4C - Max EXP (high 32 bits, Fellow Pet only)
    char  m_padding50[0x60]; // +0x50-0xAF - Padding
    CSOItem m_soItem[7 * 28];// +0x0B0 - Inventory items
public:
    // Getters
    byte GetInventoryType() const;
    int GetUniqueId() const;
    int GetOwnerUniqueId() const;
    int GetItemId() const;
    int GetMaxStack() const;
    int GetTotalInvSlotCount() const;
    std::n_wstring GetPetName() const;
    
    // New getters for pet stats
    DWORD GetCurrentHP() const { return m_currentHP; }
    DWORD GetMaxHP() const { return m_maxHP; }
    WORD GetHGP() const { return m_hgp; }
    BYTE GetPetType() const { return m_petType; }
    DWORD GetStatus() const { return m_status; }
    unsigned long long GetCurrentEXP() const { 
        return ((unsigned long long)m_currentExpHigh << 32) | m_currentExpLow; 
    }
    unsigned long long GetMaxEXP() const { 
        return ((unsigned long long)m_maxExpHigh << 32) | m_maxExpLow; 
    }
};

class CCOSDataMgr {
public:
    SCOSInfo *GetSCOSInfo(int UniqueId) const;
    SCOSInfo *GetCOSInfoBase();
    SCOSInfo *GetTradeInfo();

    int GetSelectedPetGameId() const;

private:
    virtual ~CCOSDataMgr() = 0;

private:
    char pad_0004[4];//0x0004
public:
    std::n_map<DWORD, SCOSInfo *> m_mapCOSData;//0x4
    int m_selectedPetUniqueId;                 //0x10
private:
    BEGIN_FIXTURE()
    ENSURE_SIZE(0x18)
    ENSURE_OFFSET(m_mapCOSData, 0x8)
    ENSURE_OFFSET(m_selectedPetUniqueId, 0x14)
    END_FIXTURE()
    RUN_FIXTURE(CCOSDataMgr)
};

class CICPlayer : public CICUser {
    GFX_DECLARE_DYNAMIC_EXISTING(CICPlayer, 0x0A04418)
public:

    /// \address 009d49c0
    void Func_15(int param_1, float *param_2) override;
    void Func_15_impl(int param_1, float *param_2);

    bool IsGameMaster();

    std::n_wstring *sub_9D6580(std::n_wstring *str);

    /// \address 0x8288C0
    std::n_wstring GetCharName() const;
	
    unsigned char GetCurrentLevel() const;

    long long int GetCurrentExp() const;

    short GetStatPointAvailable() const;

    short GetSkillPoints() const;

    short GetStrength() const;

    short GetIntelligence() const;

    /// \address 00828900
    /// \remark Optimized to return const reference instead of object
    const std::n_wstring &GetJobAlias() const;

    /// \address 009d4d20
    int GetCurrentJobExperiencePoints() const;

    /// \address 009d68f0
    undefined4 FUN_009d68f0();

    CCOSDataMgr *GetCosMgr();

private:
    char pad_082C[32]; //0x082C
    std::n_wstring m_charname; //0x084C
public:
    unsigned char m_level; //0x0868 level of your character, crashes when set too high
private:
    char pad_0869[7]; //0x0869
public:
    long long int m_exp_current; //0x0870
    int m_skillpoint_progress; //0x0878
private:
    short m_str_stat; //0x087C
    short m_int_stat; //0x087E
public:
    int m_skillpoint; //0x0880
private:
    short m_statpoint_available; //0x0884
    char pad_0886[26]; //0x0886
    CSOItem N0000947A; //0x08A0
    CSOItem N0000947B; //0x0A70
    CSOItem N0000947C; //0x0C40
    CSOItem N0000947D; //0x0E10
    CSOItem N0000947E; //0x0FE0
    CSOItem N0000947F; //0x11B0
    CSOItem N00009480; //0x1380
    CSOItem N00009481; //0x1550
    CSOItem N00009482; //0x1720
    CSOItem N00009483; //0x18F0
    CSOItem N00009484; //0x1AC0
    CSOItem N00009485; //0x1C90
    CSOItem N00009486; //0x1E60
    char pad_2030[96]; //0x2030
    char N000094A7; //0x2090 bit 0 = isGameMaster
    char pad_2091[7]; //0x2091
    std::n_wstring m_jobAlias; // 0x2098
    char pad_20B4[48]; //0x20B4
    short m_WorldID; //0x20E4
    char pad_20E6[50]; //0x20E6


    /*BEGIN_FIXTURE()
        ENSURE_SIZE(0x2118)

        ENSURE_OFFSET(m_charname, 0x084C)
        ENSURE_OFFSET(m_level, 0x0868)
        ENSURE_OFFSET(m_exp_current, 0x0870)
        ENSURE_OFFSET(m_skillpoint_progress, 0x0878)
        ENSURE_OFFSET(m_str_stat, 0x087C)
        ENSURE_OFFSET(m_int_stat, 0x087E)
        ENSURE_OFFSET(m_skillpoint, 0x0880)
        ENSURE_OFFSET(m_statpoint_available, 0x0884)
        ENSURE_OFFSET(m_jobAlias, 0x2098)
        ENSURE_OFFSET(m_WorldID, 0x20E4)

        END_FIXTURE()

    RUN_FIXTURE(CICPlayer)*/
};

#define g_pCICPlayer (*((CICPlayer **)0xA0465C))

// NOTE: Renamed from CharacterData to avoid conflict with imgui_windows/CharacterData.h
class CharacterDataEcsro
{
public:
	char pad_0000[1732]; //0x0000
	std::string charname; //0x06C4
	char pad_06C8[120]; //0x06C8
};
class CICPlayerEcsro
{
public:
	char pad_0000[112]; //0x0000
	short RegionID; //0x0070
	char pad_0072[2]; //0x0072
	float RegionX; //0x0074
	float RegionY; //0x0078
	float RegionZ; //0x007C
	char pad_0080[224]; //0x0080
	int ObjChar; //0x0160
	char pad_0164[132]; //0x0164
	int AbnormalState; //0x01E8
	char pad_01EC[356]; //0x01EC
	int RemaingHP; //0x0350
	int RemaingMP; //0x0354
	int MaxHP; //0x0358
	int MaxMP; //0x035C
	char pad_0360[868]; //0x0360 - 0x6C3
	char* charname; //0x06C4 - Character name pointer
	char pad_06C8[8]; //0x06C8 - 0x6CF
	BYTE Level; //0x06D0 - Character level
	char pad_06D1[19]; //0x06D1 - 0x6E3
	short Strength; //0x06E4 - STR stat (verified from IDA OnUpdate)
	short Intelligence; //0x06E6 - INT stat (verified from IDA OnUpdate)
	char pad_06E8[12]; //0x06E8 - 0x6F3
	short StatPoints; //0x06F4 - Available stat points
	char pad_06F6[1]; //0x06F6
	BYTE HwanPoint; //0x06F7 - Hwan gauge value (0-5, use at 5)
	char pad_06F8[80]; //0x06F8 - 0x747 (remaining padding)
};

#define g_pCICPlayerEcsro (*((CICPlayerEcsro **)0xA0465C))