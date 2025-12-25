/**
 * @file PetFilter.cpp
 * @brief Pet Item Filter System - ECSRO Version (VS2005 Compatible)
 */

#include "PetFilter.h"
#include "ItemDataGenerated.h"
#include "GInterface.h"
#include "ICPlayer.h"
#include "IFPetFilterSettings.h"
#include <ClientNet/MsgStreamBuffer.h>
#include "GlobalHelpersThatHaveNoHomeYet.h"
#include <cstdio>
#include <map>

/* ============================================
   MEMORY ADDRESSES - ECSRO VERSION
   ============================================ */

static const DWORD ADDR_PET_TICK_FUNC = 0x00647B70;
static const DWORD ADDR_GLOBAL_ENTITY_MAP = 0x00A0462C;
static const DWORD ADDR_IS_SAME_ITEM = 0x00898FD0;
static const DWORD ADDR_CLASS_CIITEM = 0x00A0454C;
static const DWORD ADDR_PLAYER_PTR = 0x00A0465C;
static const DWORD ADDR_GET_COS_MGR = 0x00659D70;

/* Bidirectional sync state */
static bool g_wasOriginalDisabledByUs = false;
static DWORD g_lastPickPetSCOSInfo = 0;  /* Store pet pointer for restore */

const double PetFilter::PICK_COOLDOWN = 0.4;

/* ============================================
   HELPER FUNCTIONS
   ============================================ */

static void PrintFilter(const char* format, ...) {
    va_list args;
    va_start(args, format);
    printf("[PetFilter] ");
    vprintf(format, args);
    fflush(stdout);
    va_end(args);
}

/* ============================================
   TID-BASED ITEM MATCHING
   ============================================ */

static bool CheckItemMatchesSettings(BYTE tid1, BYTE tid2, BYTE tid3, const PetFilterSettings& settings) {
    /* Weapons: tid1=1, tid2=6 */
    if (settings.eqSpear && tid1 == 1 && tid2 == 6 && tid3 == 4) return true;
    if (settings.eqGlaive && tid1 == 1 && tid2 == 6 && tid3 == 5) return true;
    if (settings.eqBow && tid1 == 1 && tid2 == 6 && tid3 == 6) return true;
    if (settings.eqBlade && tid1 == 1 && tid2 == 6 && tid3 == 3) return true;
    if (settings.eqSword && tid1 == 1 && tid2 == 6 && tid3 == 2) return true;
    
    /* Armor: tid1=1, tid2=3, tid3=1-6 */
    if (settings.eqHeavy && tid1 == 1 && tid2 == 3 && tid3 >= 1 && tid3 <= 6) return true;
    
    /* Protector: tid1=1, tid2=2, tid3=1-6 */
    if (settings.eqLight && tid1 == 1 && tid2 == 2 && tid3 >= 1 && tid3 <= 6) return true;
    
    /* Garment: tid1=1, tid2=1, tid3=1-6 */
    if (settings.eqClothes && tid1 == 1 && tid2 == 1 && tid3 >= 1 && tid3 <= 6) return true;
    
    /* Accessories: tid1=1, tid2=5 */
    if (settings.eqRing && tid1 == 1 && tid2 == 5 && tid3 == 3) return true;
    if (settings.eqNecklace && tid1 == 1 && tid2 == 5 && tid3 == 2) return true;
    if (settings.eqEarring && tid1 == 1 && tid2 == 5 && tid3 == 1) return true;
    
    /* Gold: tid1=3, tid2=5, tid3=0 */
    if (settings.gold && tid1 == 3 && tid2 == 5 && tid3 == 0) return true;
    
    /* Elixirs: tid1=3, tid2=10, tid3=1 */
    if (settings.alchemyElixirWeapon && tid1 == 3 && tid2 == 10 && tid3 == 1) return true;
    
    /* Potions: tid1=3, tid2=1 */
    if (settings.potionHP && tid1 == 3 && tid2 == 1) {
        if (tid3 == 1 || tid3 == 2 || tid3 == 3 || tid3 == 4 || tid3 == 6 || tid3 == 8 || tid3 == 9) return true;
    }
    
    /* Tablets: tid1=3, tid2=11, tid3=3 */
    if (settings.alchemyTabletBlue && tid1 == 3 && tid2 == 11 && tid3 == 3) return true;
    
    /* Materials: tid1=3, tid2=11, tid3=4,5 */
    if (settings.alchemyMaterial && tid1 == 3 && tid2 == 11 && (tid3 == 4 || tid3 == 5)) return true;
    
    /* Quests: tid1=3, tid2=9, tid3=0 */
    if (settings.universalPill && tid1 == 3 && tid2 == 9 && tid3 == 0) return true;
    
    /* Return Scroll: tid1=3, tid2=3, tid3=1 */
    if (settings.returnScroll && tid1 == 3 && tid2 == 3 && tid3 == 1) return true;
    
    return false;
}

/* ============================================
   PET FILTER SINGLETON
   ============================================ */

PetFilter::PetFilter() : m_lastPickTime(0) {
}

PetFilter& PetFilter::GetInstance() {
    static PetFilter instance;
    return instance;
}

/* ============================================
   TRAMPOLINE FOR ORIGINAL FUNCTION
   ============================================ */

static BYTE* g_pTrampoline = NULL;

static void CreateTrampoline(DWORD originalAddr) {
    g_pTrampoline = (BYTE*)VirtualAlloc(NULL, 32, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!g_pTrampoline) {
        PrintFilter("ERROR: Failed to allocate trampoline memory!\n");
        return;
    }
    
    memcpy(g_pTrampoline, (void*)originalAddr, 7);
    g_pTrampoline[7] = 0xE9;
    DWORD jumpBack = (originalAddr + 7) - (DWORD)(g_pTrampoline + 7) - 5;
    *(DWORD*)(g_pTrampoline + 8) = jumpBack;
}

static void CallOriginalTick(void* pThis, DWORD deltaTime) {
    if (g_pTrampoline) {
        typedef void (__thiscall *TrampolineFunc_t)(void* pThis, DWORD deltaTime);
        TrampolineFunc_t trampoline = (TrampolineFunc_t)g_pTrampoline;
        trampoline(pThis, deltaTime);
    }
}

/* ============================================
   RECENTLY PICKED ITEMS TRACKING
   ============================================ */

static void* g_pCurrentDropItemMgr = NULL;
static std::map<DWORD, DWORD> g_recentlyPickedItems;

static bool IsRecentlyPicked(DWORD itemID, DWORD currentTime) {
    std::map<DWORD, DWORD>::iterator it = g_recentlyPickedItems.find(itemID);
    if (it != g_recentlyPickedItems.end()) {
        if (currentTime - it->second < 30000) {
            return true;
        }
    }
    return false;
}

static void MarkAsPicked(DWORD itemID, DWORD currentTime) {
    g_recentlyPickedItems[itemID] = currentTime;
    
    std::map<DWORD, DWORD>::iterator it = g_recentlyPickedItems.begin();
    while (it != g_recentlyPickedItems.end()) {
        if (currentTime - it->second > 60000) {
            it = g_recentlyPickedItems.erase(it);
        } else {
            ++it;
        }
    }
}

/* ============================================
   MAIN TICK PROCESSING
   ============================================ */

void PetFilter::ProcessPickPets(void* pDropItemMgr, DWORD deltaTime) {
    if (!pDropItemMgr) return;
    
    g_pCurrentDropItemMgr = pDropItemMgr;
    PetFilter& filter = GetInstance();
    
    if (!filter.IsEnabled()) {
        CallOriginalTick(pDropItemMgr, deltaTime);
        return;
    }
    
    static DWORD lastPickTime = 0;
    DWORD currentTime = GetTickCount();
    if (currentTime - lastPickTime < 500) {
        CallOriginalTick(pDropItemMgr, deltaTime);
        return;
    }
    lastPickTime = currentTime;
    
    /* Get pick pet unique ID */
    DWORD pickPetUniqueID = 0;
    DWORD pPlayer = *(DWORD*)ADDR_PLAYER_PTR;
    if (pPlayer == 0) {
        CallOriginalTick(pDropItemMgr, deltaTime);
        return;
    }
    
    typedef DWORD (__thiscall *GetCosMgr_t)(DWORD pPlayer);
    GetCosMgr_t GetCosMgr = (GetCosMgr_t)ADDR_GET_COS_MGR;
    DWORD pCosMgr = GetCosMgr(pPlayer);
    if (pCosMgr != 0) {
        DWORD mapPtr = *(DWORD*)(pCosMgr + 4);
        if (mapPtr != 0) {
            DWORD node = *(DWORD*)(mapPtr + 8);
            DWORD sentinel = mapPtr;
            int petCount = 0;
            
            while (node != 0 && node != sentinel && petCount < 20) {
                petCount++;
                DWORD pSCOSInfo = *(DWORD*)(node + 0x14);
                if (pSCOSInfo != 0 && pSCOSInfo > 0x10000) {
                    BYTE petType = *(BYTE*)(pSCOSInfo + 0x14);
                    if (petType == 2) {
                        pickPetUniqueID = *(DWORD*)(pSCOSInfo + 0x04);
                        
                        /* BIDIRECTIONAL SYNC between our filter and original grab system
                         * 
                         * Logic:
                         * - If player manually enables original grab (0x80 set) → they want original system
                         *   So we disable our filter and let original work
                         * - If original grab is disabled (0x80 clear) → our filter stays active
                         *   We keep it disabled so our filter handles picking
                         */
                        BYTE* pGrabSettings = (BYTE*)(pSCOSInfo + 0x87F8);
                        BYTE grabByte = *pGrabSettings;
                        
                        if (grabByte & 0x80) {
                            /* Original grab is ENABLED - check if player did this */
                            if (g_wasOriginalDisabledByUs) {
                                /* Player manually re-enabled original grab */
                                PrintFilter("Player enabled original grab - switching to original system\n");
                                filter.SetEnabled(false);
                                CIFPetFilterSettings::SwitchPetFilter = false;
                                g_wasOriginalDisabledByUs = false;
                                
                                /* Let original system handle picking */
                                CallOriginalTick(pDropItemMgr, deltaTime);
                                return;
                            } else {
                                /* First time seeing original enabled - disable it for our filter */
                                *pGrabSettings &= ~0x80;
                                g_wasOriginalDisabledByUs = true;
                                g_lastPickPetSCOSInfo = pSCOSInfo;  /* Store for restore */
                                PrintFilter("Original grab disabled (our filter taking over)\n");
                            }
                        }
                        
                        break;
                    }
                }
                node = *(DWORD*)(node + 0x08) ? *(DWORD*)(*(DWORD*)(node + 0x08)) : *(DWORD*)(node + 0x04);
                if (node == sentinel) break;
            }
        }
    }
    
    if (pickPetUniqueID == 0) {
        CallOriginalTick(pDropItemMgr, deltaTime);
        return;
    }
    
    /* Iterate dropped items using global entity map */
    DWORD mapHeader = *(DWORD*)ADDR_GLOBAL_ENTITY_MAP;
    if (mapHeader == 0 || mapHeader < 0x10000) {
        CallOriginalTick(pDropItemMgr, deltaTime);
        return;
    }
    
    DWORD rootNode = *(DWORD*)(mapHeader + 4);
    if (rootNode == 0 || rootNode < 0x10000 || rootNode == mapHeader) {
        CallOriginalTick(pDropItemMgr, deltaTime);
        return;
    }
    
    typedef bool (__thiscall *IsSame_t)(void* pThis, DWORD classPtr);
    IsSame_t IsSame = (IsSame_t)ADDR_IS_SAME_ITEM;
    
    DWORD node = rootNode;
    while (*(DWORD*)(node + 0x08) != 0 && *(DWORD*)(node + 0x08) != mapHeader) {
        node = *(DWORD*)(node + 0x08);
    }
    
    int entityCount = 0;
    
    while (node != 0 && node != mapHeader && entityCount < 500) {
        entityCount++;
        
        DWORD entityUniqueID = *(DWORD*)(node + 0x10);
        DWORD pEntity = *(DWORD*)(node + 0x14);
        
        if (pEntity != 0 && pEntity > 0x10000 && entityUniqueID != 0) {
            bool isItem = false;
            if (!IsBadReadPtr((void*)pEntity, 4)) {
                isItem = IsSame((void*)pEntity, ADDR_CLASS_CIITEM);
            }
            
            if (isItem && !IsRecentlyPicked(entityUniqueID, currentTime)) {
                DWORD ownerCheck = *(DWORD*)(pEntity + 0x1D0);
                
                if (ownerCheck == 0) {
                    DWORD refObjID = *(DWORD*)(pEntity + 0x160);
                    
                    if (refObjID > 0 && refObjID < 20000) {
                        const PetFilterItemData* itemData = GetPetFilterItemByID(refObjID);
                        
                        if (itemData != NULL) {
                            bool shouldPick = CheckItemMatchesSettings(
                                itemData->tid1, 
                                itemData->tid2, 
                                itemData->tid3, 
                                filter.GetSettings()
                            );
                            
                            if (shouldPick) {
                                PrintFilter("Item: %s - PICKING\n", itemData->displayName);
                                
                                filter.SendPickPacket(pickPetUniqueID, entityUniqueID);
                                MarkAsPicked(entityUniqueID, currentTime);
                                
                                CallOriginalTick(pDropItemMgr, deltaTime);
                                return;
                            }
                        }
                    }
                }
            }
        }
        
        /* Move to next node (in-order traversal) */
        DWORD rightChild = *(DWORD*)(node + 0x0C);
        
        if (rightChild != 0 && rightChild != mapHeader) {
            node = rightChild;
            DWORD leftChild = *(DWORD*)(node + 0x08);
            while (leftChild != 0 && leftChild != mapHeader) {
                node = leftChild;
                leftChild = *(DWORD*)(node + 0x08);
            }
        } else {
            DWORD parent = *(DWORD*)(node + 0x04);
            while (parent != 0 && parent != mapHeader) {
                DWORD parentRightChild = *(DWORD*)(parent + 0x0C);
                if (node != parentRightChild) {
                    break;
                }
                node = parent;
                parent = *(DWORD*)(node + 0x04);
            }
            node = parent;
        }
        
        if (node == 0 || node == mapHeader) {
            break;
        }
    }
    
    CallOriginalTick(pDropItemMgr, deltaTime);
}

/* ============================================
   HOOK FUNCTIONS
   ============================================ */

__declspec(naked) void PetFilter_TickHook() {
    __asm {
        pushad
        pushfd
        
        mov eax, [esp + 36 + 4]
        push eax
        push ecx
        call PetFilter::ProcessPickPets
        add esp, 8
        
        popfd
        popad
        
        ret 4
    }
}

void PetFilter::InstallHook() {
    PrintFilter("Installing pick hook for ECSRO...\n");
    
    DWORD hookAddr = ADDR_PET_TICK_FUNC;
    DWORD hookFunc = (DWORD)PetFilter_TickHook;
    
    CreateTrampoline(hookAddr);
    
    if (!g_pTrampoline) {
        PrintFilter("ERROR: Trampoline creation failed!\n");
        return;
    }
    
    DWORD relativeJump = hookFunc - hookAddr - 5;
    
    DWORD oldProtect;
    if (!VirtualProtect((LPVOID)hookAddr, 16, PAGE_EXECUTE_READWRITE, &oldProtect)) {
        PrintFilter("ERROR: VirtualProtect failed!\n");
        return;
    }
    
    *(BYTE*)hookAddr = 0xE9;
    *(DWORD*)(hookAddr + 1) = relativeJump;
    *(BYTE*)(hookAddr + 5) = 0x90;
    *(BYTE*)(hookAddr + 6) = 0x90;
    
    VirtualProtect((LPVOID)hookAddr, 16, oldProtect, &oldProtect);
    
    PrintFilter("Hook installed successfully!\n");
}

/* ============================================
   PUBLIC FUNCTIONS
   ============================================ */

bool PetFilter::IsEquipmentFilterEnabled() const {
    if (m_settings.eqSpear || m_settings.eqGlaive || m_settings.eqBow ||
        m_settings.eqBlade || m_settings.eqSword) {
        return true;
    }
    if (m_settings.eqRing || m_settings.eqEarring || m_settings.eqNecklace) {
        return true;
    }
    if (m_settings.eqHeavy || m_settings.eqLight || m_settings.eqClothes) {
        return true;
    }
    return false;
}

void PetFilter::SendPickPacket(int petUniqueID, int itemUniqueID) {
    NEWMSG(0x70C5)
    pReq << petUniqueID;
    pReq << (BYTE)8;
    pReq << itemUniqueID;
    SENDMSG()
}

void PetFilter::SetEnabled(bool enabled) {
    m_settings.enabled = enabled;
    
    if (enabled) {
        /* When we're enabled, immediately disable original grab if pet is available */
        if (g_lastPickPetSCOSInfo != 0) {
            BYTE* pGrabSettings = (BYTE*)(g_lastPickPetSCOSInfo + 0x87F8);
            if (*pGrabSettings & 0x80) {
                *pGrabSettings &= ~0x80;  /* Clear grab enabled bit */
                g_wasOriginalDisabledByUs = true;
                PrintFilter("Original grab disabled on enable\n");
            }
        }
    } else {
        /* When we're disabled, restore original grab if we disabled it */
        if (g_wasOriginalDisabledByUs && g_lastPickPetSCOSInfo != 0) {
            BYTE* pGrabSettings = (BYTE*)(g_lastPickPetSCOSInfo + 0x87F8);
            *pGrabSettings |= 0x80;  /* Set grab enabled bit */
            PrintFilter("Original grab restored (0x80 bit set back)\n");
        }
        
        /* Reset sync state when we're disabled */
        g_wasOriginalDisabledByUs = false;
        g_lastPickPetSCOSInfo = 0;
    }
    
    PrintFilter("Filter %s\n", enabled ? "ENABLED" : "DISABLED");
}

void PetFilter::EnableAllFilters() {
    m_settings.enabled = true;
    m_settings.gold = true;
    m_settings.eqSpear = true;
    m_settings.eqGlaive = true;
    m_settings.eqBow = true;
    m_settings.eqBlade = true;
    m_settings.eqSword = true;
    m_settings.eqHeavy = true;
    m_settings.eqLight = true;
    m_settings.eqClothes = true;
    m_settings.eqRing = true;
    m_settings.eqEarring = true;
    m_settings.eqNecklace = true;
    m_settings.alchemyElixirWeapon = true;
    m_settings.alchemyTabletBlue = true;
    m_settings.alchemyMaterial = true;
    m_settings.potionHP = true;
    m_settings.returnScroll = true;
    PrintFilter("All filters enabled!\n");
}

void PetFilter::PrintSettings() const {
    PrintFilter("=== PetFilter Settings ===\n");
    PrintFilter("Enabled: %s\n", m_settings.enabled ? "YES" : "NO");
    PrintFilter("Gold: %s\n", m_settings.gold ? "YES" : "NO");
    PrintFilter("==========================\n");
}

void SetPetFilterEnabled(bool enabled) {
    g_PetFilter.SetEnabled(enabled);
}
