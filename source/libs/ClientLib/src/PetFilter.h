/**
 * @file PetFilter.h
 * @brief Pet Item Filter System - ECSRO Version (VS2005 Compatible)
 */

#ifndef PETFILTER_H
#define PETFILTER_H

#include <Windows.h>
#include <ctime>

/* ============================================
   PET FILTER SETTINGS STRUCTURE
   ============================================ */

struct PetFilterSettings {
    /* Master switch */
    bool enabled;
    
    /* Equipment degree filters */
    bool eqDegree[15];
    
    /* Weapon type filters (tid1=1, tid2=6) */
    bool eqSword;     /* tid3=2 */
    bool eqBlade;     /* tid3=3 */
    bool eqSpear;     /* tid3=4 */
    bool eqGlaive;    /* tid3=5 */
    bool eqBow;       /* tid3=6 */
    bool eqWeapon;    /* Legacy - all weapons */
    
    /* Accessory type filters (tid1=1, tid2=5) */
    bool eqRing;      /* tid3=3 */
    bool eqEarring;   /* tid3=1 */
    bool eqNecklace;  /* tid3=2 */
    bool eqAccessory; /* Legacy - all accessories */
    
    /* Armor filters (tid1=1, tid2=1-3) */
    bool eqHeavy;     /* tid2=3 - Armor */
    bool eqLight;     /* tid2=2 - Protector */
    bool eqClothes;   /* tid2=1 - Garment */
    
    /* Gender filters */
    bool eqMale;
    bool eqFemale;
    
    /* Alchemy degree filters */
    bool alchemyDegree[15];
    
    /* Alchemy item types */
    bool alchemyElixirWeapon;
    bool alchemyElixirShield;
    bool alchemyElixirProtector;
    bool alchemyElixirAccessory;
    bool alchemyTabletBlue;
    bool alchemyTabletRed;
    bool alchemyStoneBlue;
    bool alchemyStoneRed;
    bool alchemyMaterial;
    
    /* Consumables */
    bool potionHP;
    bool potionMP;
    bool potionVigor;
    bool grainHP;
    bool grainMP;
    bool grainVigor;
    bool universalPill;
    bool returnScroll;
    bool arrows;
    bool bolts;
    bool coins;
    
    /* Gold */
    bool gold;
    
    PetFilterSettings() {
        int i;
        enabled = false;
        
        for (i = 0; i < 15; i++) {
            eqDegree[i] = false;
            alchemyDegree[i] = false;
        }
        
        eqSword = false;
        eqBlade = false;
        eqSpear = false;
        eqGlaive = false;
        eqBow = false;
        eqWeapon = false;
        
        eqRing = false;
        eqEarring = false;
        eqNecklace = false;
        eqAccessory = false;
        
        eqHeavy = false;
        eqLight = false;
        eqClothes = false;
        eqMale = false;
        eqFemale = false;
        
        alchemyElixirWeapon = false;
        alchemyElixirShield = false;
        alchemyElixirProtector = false;
        alchemyElixirAccessory = false;
        alchemyTabletBlue = false;
        alchemyTabletRed = false;
        alchemyStoneBlue = false;
        alchemyStoneRed = false;
        alchemyMaterial = false;
        
        potionHP = false;
        potionMP = false;
        potionVigor = false;
        grainHP = false;
        grainMP = false;
        grainVigor = false;
        universalPill = false;
        returnScroll = false;
        arrows = false;
        bolts = false;
        coins = false;
        
        gold = false;
    }
};

/* ============================================
   PET FILTER MANAGER
   ============================================ */

class PetFilter {
public:
    static PetFilter& GetInstance();
    
    PetFilterSettings& GetSettings() { return m_settings; }
    const PetFilterSettings& GetSettings() const { return m_settings; }
    
    void SetEnabled(bool enabled);
    bool IsEnabled() const { return m_settings.enabled; }
    
    static void InstallHook();
    static void ProcessPickPets(void* pDropItemMgr, DWORD deltaTime);
    
    void EnableAllFilters();
    void PrintSettings() const;
    
private:
    PetFilter();
    ~PetFilter() {}
    
    PetFilterSettings m_settings;
    time_t m_lastPickTime;
    static const double PICK_COOLDOWN;
    
    void SendPickPacket(int petUniqueID, int itemUniqueID);
    bool IsEquipmentFilterEnabled() const;
};

#define g_PetFilter PetFilter::GetInstance()

/* Global function for macro window */
void SetPetFilterEnabled(bool enabled);

#endif /* PETFILTER_H */
