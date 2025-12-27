#include "CustomMinimap.h"
#include "CustomGUISession.h"
#include <TextStringManager.h>  // For region name lookup
#include <GFX3DFunction/GFXVideo3d.h>
#include <ECSRO_Classes.h>  // For GetAllEntitiesRaw and entity iteration
#include <IGIDObject.h>     // For CGfxRuntimeClass
#include <CIFMinimap.h>     // For native minimap access
#include <stdio.h>
#include <cstring>  // For strcmp
#include <cmath>    // For fabsf, sinf, cosf

// Debug log function - writes to file for easy viewing
static void DebugLog(const char* msg) {
    OutputDebugStringA("[CustomMinimap] ");
    OutputDebugStringA(msg);
    OutputDebugStringA("\n");
    
    // Also write to file
    FILE* fp = fopen("minimap_debug.txt", "a");
    if (fp) {
        fprintf(fp, "[CustomMinimap] %s\n", msg);
        fclose(fp);
    }
}

// Global instance
static CustomMinimap* g_pCustomMinimap = NULL;

// Render callback for CustomGUISession
static void Minimap_RenderCallback() {
    if (g_pCustomMinimap) {
        g_pCustomMinimap->Render();
    }
}

// Initialize function - call from DllMain or elsewhere
void InitializeCustomMinimap() {
    DebugLog("InitializeCustomMinimap: START");
    if (!g_pCustomMinimap) {
        DebugLog("InitializeCustomMinimap: Creating CustomMinimap");
        g_pCustomMinimap = new CustomMinimap();
        DebugLog("InitializeCustomMinimap: CustomMinimap created");
        
        // Don't access g_CD3DApplication here - it may not be ready yet
        // Device will be obtained lazily in Render() when ImGui is ready
        DebugLog("InitializeCustomMinimap: Skipping device init (lazy)");
        
        DebugLog("InitializeCustomMinimap: Registering callback");
        g_CustomGUI.RegisterRenderCallback(Minimap_RenderCallback);
        DebugLog("InitializeCustomMinimap: Callback registered");
    }
    DebugLog("InitializeCustomMinimap: END");
}

// Player pointer address
static const DWORD ADDR_PLAYER_PTR = 0x00A0465C;

// Native constants from sub_53A5A0 ASM analysis
static const float COORD_SCALE = 10.0f;     // flt_94AE04 = 10.0f
static const float COORD_SCALE_2 = 0.01f;   // flt_94AE08 = 0.01f (1/100)
static const float REGION_SIZE = 192.0f;    // flt_93B838 = 192.0f

// Entity Linked List (from IDA: dword_9C99A4)
static const DWORD ADDR_ENTITY_LIST_HEAD = 0x009C99A4;

// RuntimeClass pointers for entity type checking (from IDA analysis)
static const DWORD RUNTIMECLASS_MONSTER = 0x00A04320;  // CICMonster
static const DWORD RUNTIMECLASS_NPC     = 0x00A01DD8;  // CICNPC
static const DWORD RUNTIMECLASS_ITEM    = 0x00A0436C;  // CICPickedItem
static const DWORD RUNTIMECLASS_PLAYER  = 0x00A04490;  // CICPlayer

// Entity offsets (from IDA sub_53AD20)
static const DWORD ENTITY_OFFSET_STATE      = 0x1E6;   // Dead state = 4
static const DWORD ENTITY_OFFSET_POSEX      = 0x84;    // X position (float)
static const DWORD ENTITY_OFFSET_POSEZ      = 0x8C;    // Z position (float)
static const DWORD ENTITY_OFFSET_MONSTERTYPE = 0x668;  // Monster type (3 = Unique)
static const DWORD ENTITY_LINKED_OFFSET     = 436;     // v31 = v30 - 436

// Player rotation offset (from sub_53A5A0: player+0x80)
static const DWORD PLAYER_ROTATION_OFFSET   = 0x80;    // Player rotation float (radians)

// Loading Manager (from CustomPlayerMiniInfo - hide during loading/teleport)
static const DWORD ADDR_LOADING_MANAGER = 0xA00524;
static const DWORD OFFSET_LOADING_FLAG = 0xBC;

// Party Manager (from RE: sub_629510 returns this+24)
static const DWORD ADDR_PARTY_MANAGER = 0xA01510;       // unk_A01510
static const DWORD PARTY_DATA_OFFSET = 24;              // sub_629510: this+24
static const DWORD PARTY_MEMBER_LIST_OFFSET = 28;       // partyData+28 = member list
static const DWORD PARTY_SELF_ID_OFFSET = 20;           // partyData+20 = self world ID (partyData[5])
static const DWORD PMEMBER_ID_OFFSET = 36;              // memberNode+36 = partyID
static const DWORD PMEMBER_POSX_OFFSET = 64;            // memberNode+64 = X
static const DWORD PMEMBER_POSZ_OFFSET = 72;            // memberNode+72 = Z

// Quest Manager chain for ACTIVE quest target detection
// dword_C5DD24[362] = QuestManager, sub_645B80(qm)+24 = target list
static const DWORD ADDR_GAME_MANAGER = 0xC5DD24;
static const DWORD OFFSET_QUEST_MANAGER = 0x5A8;       // GameManager[362] = 362 * 4
static const DWORD OFFSET_QUEST_TARGET_LIST = 0x18;    // from sub_645B80 result + 24
static const DWORD OFFSET_TARGET_ENTITY_ID = 0x14;     // each target node + 20

// FindEntityByID - converts entity ID to entity pointer (from CustomDamageRenderer)
typedef void* (__cdecl *FindEntityByIDFunc)(DWORD entityID);
// ============================================================================
// Player Quest Manager Access (reverse engineered from CICPlayer)
// ============================================================================
// CICPlayer+5144 (offset 1286) = PlayerQuestManager (48 bytes, sub_5F33F0)
// PlayerQuestManager+28 (offset 7) = Quest Map (std::map<questID, QuestData*>)
// QuestData structure: offset+20 (5) = target list header
// Target node+20 = target entity ID
// ============================================================================

static const DWORD OFFSET_PLAYER_QUEST_MANAGER = 5144;  // CICPlayer[1286] = 1286 * 4
static const DWORD OFFSET_QUEST_MAP = 28;               // PlayerQuestManager[7] = offset 28
static const DWORD OFFSET_QUEST_TARGET_LIST_IN_QUEST = 20;  // Quest data offset for target list

// Debug flag for quest detection
static bool g_bQuestDebugOnce = true;
static DWORD g_dwLastQuestLogTime = 0;

// ============================================================================
// GetVisibleEntities - Combines BOTH entity sources:
// 1. Native linked list (dword_9C99A4) - nearby entities
// 2. EntityManager map - all entities including other players
// ============================================================================
static std::vector<DWORD> GetVisibleEntities() {
    std::vector<DWORD> entities;
    entities.reserve(200);
    
    // Note: Cannot use __try here because std::vector requires object unwinding
    // Using IsBadReadPtr checks for safety instead
    
    // Source 1: Native linked list at dword_9C99A4 (nearby entities)
    DWORD node = *(DWORD*)ADDR_ENTITY_LIST_HEAD;
    for (int i = 0; i < 1000 && node != 0; i++) {
        if (IsBadReadPtr((void*)node, 0x10)) {
            break;
        }
        DWORD entityPtr = node - ENTITY_LINKED_OFFSET;
        if (entityPtr && !IsBadReadPtr((void*)entityPtr, 0x100)) {
            entities.push_back(entityPtr);
        }
        DWORD nextNode = *(DWORD*)(node + 12);
        if (nextNode == node) break;
        node = nextNode;
    }
    
    // Source 2: EntityManager map (includes OTHER PLAYERS not in linked list)
    // This is same as GetAllEntitiesRaw() but we merge with existing list
    DWORD entityMgrPtr = *(DWORD*)0x00C5DCF0;  // ECSRO_ADDR_ENTITY_MANAGER
    if (entityMgrPtr && !IsBadReadPtr((void*)entityMgrPtr, 0x40)) {
        DWORD mapHead = *(DWORD*)(entityMgrPtr + 0x1C);  // ECSRO_ENTITYMGR_MAP_HEAD
        if (mapHead && !IsBadReadPtr((void*)mapHead, 0x18)) {
            DWORD rootNode = *(DWORD*)(mapHead + 0x04);  // ECSRO_NODE_PARENT
            if (rootNode && rootNode != mapHead && !IsBadReadPtr((void*)rootNode, 0x18)) {
                // Simple BFS traversal of map
                std::vector<DWORD> stack;
                stack.push_back(rootNode);
                
                while (!stack.empty() && entities.size() < 500) {
                    DWORD curr = stack.back();
                    stack.pop_back();
                    
                    if (!curr || curr == mapHead || IsBadReadPtr((void*)curr, 0x18)) {
                        continue;
                    }
                    
                    // Get entity pointer (node + 20 = value)
                    DWORD entityPtr = *(DWORD*)(curr + 0x14);
                    if (entityPtr && !IsBadReadPtr((void*)entityPtr, 0x100)) {
                        // Check if not already in list (avoid duplicates)
                        bool found = false;
                        for (size_t j = 0; j < entities.size(); j++) {
                            if (entities[j] == entityPtr) {
                                found = true;
                                break;
                            }
                        }
                        if (!found) {
                            entities.push_back(entityPtr);
                        }
                    }
                    
                    // Add children to stack
                    DWORD left = *(DWORD*)(curr + 0x08);
                    DWORD right = *(DWORD*)(curr + 0x0C);
                    if (left && left != mapHead) stack.push_back(left);
                    if (right && right != mapHead) stack.push_back(right);
                }
            }
        }
    }
    
    return entities;
}

// Native function: sub_62A1E0 - finds party member by World ID
// Signature: _DWORD* __thiscall sub_62A1E0(_DWORD *partyManager, int worldID)
// Address: 0x62A1E0
// Searches member list at PartyManager[13] (offset +52), compares node[9] (offset +36) with worldID
// Returns: pointer to member name if found, NULL otherwise
typedef void* (__thiscall *fnFindPartyMemberByWorldID)(void* partyManager, int worldID);
static fnFindPartyMemberByWorldID g_pfnFindPartyMemberByWorldID = (fnFindPartyMemberByWorldID)0x0062A1E0;

// Native typedef for IsInParty check (sub_62A6C0)
// Returns: BYTE (1 = in party, 0 = not in party)
typedef BYTE (__thiscall *fnIsInParty)(void* partyManager);
static fnIsInParty g_pfnIsInParty = (fnIsInParty)0x0062A6C0;

// Native typedef for getting Player World ID (sub_65A5D0)
// Returns: this[1279] = Player+5116 = World ID
typedef DWORD (__thiscall *fnGetPlayerWorldID)(void* player);
static fnGetPlayerWorldID g_pfnGetPlayerWorldID = (fnGetPlayerWorldID)0x0065A5D0;

// Check if a player entity (CICUser) is in our party
// Uses native game function for reliable party member detection
static bool IsEntityInParty(DWORD entityPtr) {
    __try {
        if (!entityPtr || IsBadReadPtr((void*)entityPtr, 0x200)) {
            return false;
        }
        
        // First check: Are we even in a party?
        BYTE inParty = g_pfnIsInParty((void*)ADDR_PARTY_MANAGER);
        if (!inParty) {
            return false;  // Not in party, no need to check further
        }
        
        // Get entity's UNIQUE ID at offset +412 (NOT +224!)
        // Source: PartyManager.h - CICUSER_OFF_UNIQUE_ID = 412
        // source_part_9.cpp line 22276: sub_62A6F0(*(_DWORD *)(v10 + 412))
        // This is compared with party member node+36
        DWORD entityUniqueID = *(DWORD*)(entityPtr + 412);
        if (entityUniqueID == 0) return false;
        
        // Get self Unique ID from local Player at same offset +412
        DWORD playerPtr = *(DWORD*)ADDR_PLAYER_PTR;
        DWORD selfUniqueID = 0;
        if (playerPtr && !IsBadReadPtr((void*)playerPtr, 0x1500)) {
            selfUniqueID = *(DWORD*)(playerPtr + 412);
        }
        
        // Skip if this entity is ourselves
        if (entityUniqueID == selfUniqueID && selfUniqueID != 0) {
            return false;
        }
        
        // Use native function to check if Unique ID exists in party
        // sub_62A1E0(&unk_A01510, uniqueID) returns member data if found, NULL otherwise
        void* memberData = g_pfnFindPartyMemberByWorldID((void*)ADDR_PARTY_MANAGER, (int)entityUniqueID);
        
        // DEBUG: Log periodically with party member list dump
        static DWORD lastDebugTime = 0;
        DWORD now = GetTickCount();
        if (now - lastDebugTime > 3000) {
            lastDebugTime = now;
            
            char buf[512];
            sprintf(buf, "IsEntityInParty: entity=0x%X, entityUniqueID=%d, selfUniqueID=%d, inParty=%d, memberData=0x%X", 
                    entityPtr, entityUniqueID, selfUniqueID, inParty, (DWORD)memberData);
            DebugLog(buf);
            
            // Dump party member IDs from linked list for debugging
            // PartyManager+52 = member list head (sentinel)
            DWORD memberListHead = *(DWORD*)(ADDR_PARTY_MANAGER + 52);
            if (memberListHead && !IsBadReadPtr((void*)memberListHead, 8)) {
                DWORD node = *(DWORD*)memberListHead;  // First node
                for (int i = 0; i < 8 && node && node != memberListHead; i++) {
                    if (IsBadReadPtr((void*)node, 0x50)) break;
                    DWORD nodeID = *(DWORD*)(node + 36);  // node+36 = party member unique ID
                    sprintf(buf, "  PartyMember[%d]: node=0x%X, uniqueID=%d", i, node, nodeID);
                    DebugLog(buf);
                    node = *(DWORD*)node;  // Next node
                }
            }
        }
        
        if (memberData != NULL) {
            char buf[128];
            sprintf(buf, "PARTY MATCH! entity=0x%X, uniqueID=%d, memberData=0x%X", 
                    entityPtr, entityUniqueID, (DWORD)memberData);
            DebugLog(buf);
            return true;
        }
        
        return false;
    }
    __except(1) { return false; }
}




// Check if NPC is a target of ANY active quest the player has
static bool IsNPCQuestTarget(DWORD npcEntityPtr) {
    __try {
        if (!npcEntityPtr || IsBadReadPtr((void*)npcEntityPtr, 0x100)) {
            return false;
        }
        
        // Get player pointer
        DWORD playerPtr = *(DWORD*)ADDR_PLAYER_PTR;
        if (!playerPtr || IsBadReadPtr((void*)playerPtr, 0x1500)) {
            return false;
        }
        
        // Get NPC's RefObjID for comparison (entity + 0x160)
        DWORD npcRefObjID = *(DWORD*)(npcEntityPtr + 0x160);
        
        // Get PlayerQuestManager from CICPlayer+5144
        DWORD playerQuestMgr = *(DWORD*)(playerPtr + OFFSET_PLAYER_QUEST_MANAGER);
        if (!playerQuestMgr || IsBadReadPtr((void*)playerQuestMgr, 0x30)) {
            return false;
        }
        
        // Get quest map header (PlayerQuestManager+28)
        DWORD questMapPtr = playerQuestMgr + OFFSET_QUEST_MAP;
        if (IsBadReadPtr((void*)questMapPtr, 0x18)) {
            return false;
        }
        
        // std::map structure: [sentinel][size] where sentinel+8 = first node
        DWORD mapSentinel = *(DWORD*)questMapPtr;
        if (!mapSentinel || IsBadReadPtr((void*)mapSentinel, 0x18)) {
            return false;
        }
        
        // Periodic debug logging
        DWORD now = GetTickCount();
        if (now - g_dwLastQuestLogTime > 5000) {
            g_dwLastQuestLogTime = now;
            
            DWORD mapSize = *(DWORD*)(questMapPtr + 4);
            char buf[256];
            sprintf(buf, "PlayerQuest: Player=0x%X, QuestMgr=0x%X, MapSize=%d, NPC RefID=0x%X", 
                    playerPtr, playerQuestMgr, mapSize, npcRefObjID);
            DebugLog(buf);
        }
        
        // Get first node from map (sentinel + 8 = leftmost/first)
        DWORD currentNode = *(DWORD*)(mapSentinel + 8);
        
        // Traverse quest map (std::map nodes: [left][right][parent][key][value])
        for (int i = 0; i < 100 && currentNode != 0 && currentNode != mapSentinel; i++) {
            if (IsBadReadPtr((void*)currentNode, 0x20)) {
                break;
            }
            
            // Get quest data pointer (node + 20 = value, which is QuestData*)
            DWORD questData = *(DWORD*)(currentNode + 20);
            
            // Debug log once per 5 seconds
            if (now - g_dwLastQuestLogTime < 100 && i == 0) {
                char buf[256];
                sprintf(buf, "  Quest[%d]: node=0x%X, questData=0x%X", i, currentNode, questData);
                DebugLog(buf);
            }
            
            if (questData && !IsBadReadPtr((void*)questData, 0x30)) {
                // QuestData+32/36/40 is a vector containing TARGET NPC RefObjIDs!
                DWORD vecStart = *(DWORD*)(questData + 32);
                DWORD vecEnd = *(DWORD*)(questData + 36);
                
                // Debug logging (periodic)
                if (now - g_dwLastQuestLogTime < 100 && i == 0) {
                    char buf[256];
                    DWORD questID = *(DWORD*)(questData + 4);
                    sprintf(buf, "    QuestID=0x%X, vecStart=0x%X, vecEnd=0x%X (want NPC 0x%X)", 
                            questID, vecStart, vecEnd, npcRefObjID);
                    DebugLog(buf);
                }
                
                // Check if vector has elements
                if (vecStart && vecEnd && vecStart < vecEnd && !IsBadReadPtr((void*)vecStart, 0x10)) {
                    // Iterate through vector elements (each is a DWORD = RefObjID)
                    for (DWORD* pRef = (DWORD*)vecStart; pRef < (DWORD*)vecEnd; pRef++) {
                        DWORD targetRefObjID = *pRef;
                        
                        // Debug (only first quest, first few elements)
                        if (now - g_dwLastQuestLogTime < 100 && i == 0 && (pRef - (DWORD*)vecStart) < 3) {
                            char buf[128];
                            sprintf(buf, "      Vec[%d]=0x%X (match NPC 0x%X? %s)", 
                                    (int)(pRef - (DWORD*)vecStart), targetRefObjID, npcRefObjID,
                                    (targetRefObjID == npcRefObjID) ? "YES!" : "no");
                            DebugLog(buf);
                        }
                        
                        // MATCH! This NPC is a quest target!
                        if (targetRefObjID == npcRefObjID) {
                            return true;
                        }
                    }
                }
            }
            
            // Move to next node in map (in-order traversal)
            // Simple approach: just follow the linked structure
            DWORD nextNode = *(DWORD*)(currentNode + 8);  // right child
            if (nextNode && nextNode != mapSentinel) {
                // Go to leftmost of right subtree
                currentNode = nextNode;
                while (true) {
                    DWORD left = *(DWORD*)(currentNode + 0);
                    if (!left || left == mapSentinel || IsBadReadPtr((void*)left, 0x10)) {
                        break;
                    }
                    currentNode = left;
                }
            } else {
                // Go up until we come from left
                DWORD parent = *(DWORD*)(currentNode + 4);
                while (parent && parent != mapSentinel) {
                    if (IsBadReadPtr((void*)parent, 0x10)) break;
                    if (*(DWORD*)(parent + 0) == currentNode) {
                        // Coming from left child, parent is next
                        currentNode = parent;
                        break;
                    }
                    currentNode = parent;
                    parent = *(DWORD*)(currentNode + 4);
                }
                if (!parent || parent == mapSentinel) {
                    break;  // Done traversing
                }
            }
        }
        
        return false;
    }
    __except(1) { return false; }
}

// Helper to check if UI should be visible (not during loading/teleport)
static bool IsMinimapVisible() {
    // Check if player exists
    DWORD playerPtr = *(DWORD*)ADDR_PLAYER_PTR;
    if (playerPtr == 0) {
        return false;  // No player, don't show
    }
    
    // Check loading state from Loading Manager
    DWORD loadingManager = *(DWORD*)ADDR_LOADING_MANAGER;
    if (loadingManager != 0) {
        BYTE isLoading = *(BYTE*)(loadingManager + OFFSET_LOADING_FLAG);
        if (isLoading != 0) {
            return false;  // Loading in progress, hide minimap
        }
    }
    
    return true;
}

// Get minimap data directly from player pointer (like native minimap does)
// Native code reads from dword_A0465C (player ptr) + offsets
// Fixed: Use GLOBAL coordinate formula matching native minimap display
static bool GetMinimapData(int& regionX, int& regionY, int& displayX, int& displayY, const char*& regionName) {
    __try {
        // Read player pointer
        DWORD playerPtr = *(DWORD*)ADDR_PLAYER_PTR;
        if (playerPtr == 0) return false;
        
        // Read RegionID from player + 0x70 (112 decimal)
        // WORD format: low byte = X, high byte = Y
        WORD regionID = *(WORD*)(playerPtr + 0x70);
        regionX = (unsigned char)(regionID & 0xFF);        // Low byte
        regionY = (unsigned char)((regionID >> 8) & 0xFF); // High byte
        
        // Read position from player + 0x74, 0x78, 0x7C
        float posX = *(float*)(playerPtr + 0x74);
        // float posY = *(float*)(playerPtr + 0x78);  // Height, not used
        float posZ = *(float*)(playerPtr + 0x7C);
        
        // Calculate GLOBAL display coordinates (matches native minimap X:6290 Y:499)
        // SRO global coordinate formula:
        // GlobalX = (RegionX - 135) * 192 + (localPosX / 10)
        // GlobalY = (RegionY - 92) * 192 + (localPosZ / 10)
        // Region 135,92 is the center reference point
        displayX = (regionX - 135) * 192 + (int)(posX / COORD_SCALE);
        displayY = (regionY - 92) * 192 + (int)(posZ / COORD_SCALE);
        
        // Get region name from TextStringManager
        // Native: sprintf("%d", regionID); TSM->GetString(buffer);
        static char regionIdBuffer[16];
        sprintf(regionIdBuffer, "%d", (int)regionID);
        regionName = g_CTextStringManager->GetString3(regionIdBuffer);
        
        return true;
    }
    __except (1) { return false; }
}

CustomMinimap::CustomMinimap() {
    m_bVisible = true;
    m_bInitialized = false;
    m_nRegionX = 0;
    m_nRegionY = 0;
    m_nDisplayX = 0;
    m_nDisplayY = 0;
    m_pRegionName = NULL;
    m_fPlayerPosX = 0.0f;
    m_fPlayerPosZ = 0.0f;
    m_fPlayerRotation = 0.0f;
    m_fMinimapSize = 192.0f;
    m_vMinimapPos = ImVec2(10.0f, 240.0f);  // Below PlayerMiniInfo
    m_fZoomFactor = 2.5f;  // Default to middle zoom
    m_fArrowOffsetX = 0.0f;
    m_fArrowOffsetY = 0.0f;
    m_pDevice = NULL;
    m_pTexCharacter = NULL;
    m_pTexPartyArrow = NULL;
    m_pTexNPC = NULL;
    m_pTexMonster = NULL;
    m_pTexPartyMember = NULL;
}

CustomMinimap::~CustomMinimap() {
    Shutdown();
}

bool CustomMinimap::Initialize(IDirect3DDevice9* pDevice) {
    if (!pDevice) return false;
    
    m_pDevice = pDevice;
    
    // Don't load textures here - too early!
    // LoadTextures will be called lazily in Render() when game is ready
    
    m_bInitialized = true;
    return true;
}

void CustomMinimap::Shutdown() {
    m_bInitialized = false;
    m_pDevice = NULL;
}

void CustomMinimap::UpdatePlayerPosition() {
    // Get coordinates and region name from native GetMinimapData
    GetMinimapData(m_nRegionX, m_nRegionY, m_nDisplayX, m_nDisplayY, m_pRegionName);
    
    // Try to get native CIFMinimap instance for rotation and position data
    // NOTE: We do NOT read native zoom - our zoom scale is different (0.5-8.0 vs 160.0)
    CIFMinimap* pMinimap = CIFMinimap::GetInstance();
    if (pMinimap) {
        // DON'T read native zoom - it's incompatible (160.0f default)
        
        // Get player rotation (offset 0x320)
        m_fPlayerRotation = pMinimap->GetPlayerRotation();
        
        // Get arrow screen position (offsets 0x338, 0x33C)
        m_fArrowOffsetX = pMinimap->GetArrowScreenX();
        m_fArrowOffsetY = pMinimap->GetArrowScreenY();
        
        // Get player position cache (offsets 0x314, 0x31C)
        m_fPlayerPosX = pMinimap->GetPlayerPosX();
        m_fPlayerPosZ = pMinimap->GetPlayerPosZ();
    }
}

void CustomMinimap::Render() {
    if (!m_bVisible) return;
    
    // Lazy initialization - set initialized on first render
    if (!m_bInitialized) {
        m_bInitialized = true;
    }
    
    // Lazy texture loading - only load once when game is ready
    if (m_pTexCharacter == NULL) {
        LoadTextures();
    }
    
    // Don't render during loading/teleport
    if (!IsMinimapVisible()) {
        return;
    }
    
    // HIDE native CIFMinimap to prevent overlap
    CIFMinimap* pNativeMinimap = CIFMinimap::GetInstance();
    if (pNativeMinimap) {
        pNativeMinimap->ShowGWnd(false);  // Hide native minimap
    }
    
    // Update position data
    UpdatePlayerPosition();
    
    // Setup ImGui window flags
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | 
                             ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoScrollbar |
                             ImGuiWindowFlags_NoBackground;
    
    ImGui::SetNextWindowPos(m_vMinimapPos, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(m_fMinimapSize + 30, m_fMinimapSize + 50));  // Compact size
    
    if (ImGui::Begin("CustomMinimap", NULL, flags)) {
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 windowPos = ImGui::GetWindowPos();
        
        // Minimap background position
        ImVec2 mapPos = ImVec2(windowPos.x + 10, windowPos.y + 10);
        
        // Draw minimap background
        DrawMinimapBackground(drawList, mapPos, m_fMinimapSize);
        
        // Draw entity markers (monsters, NPCs, players, items)
        // NOTE: Other players are drawn as GREEN here, party members handled separately
        DrawEntityMarkers(drawList, mapPos, m_fMinimapSize);
        
        // Draw party members as CYAN markers (native approach: separate from entity loop)
        // Get player position for DrawPartyMembers
        DWORD playerPtr = GetPlayerAddressRaw();
        if (playerPtr) {
            D3DVECTOR playerLoc = GetLocationRaw(playerPtr);
            ImVec2 center = ImVec2(mapPos.x + m_fMinimapSize * 0.5f, mapPos.y + m_fMinimapSize * 0.5f);
            float baseRange = 100.0f;
            float minimapRange = baseRange * m_fZoomFactor;
            float scale = (m_fMinimapSize * 0.5f) / minimapRange;
            DrawPartyMembers(drawList, mapPos, m_fMinimapSize, playerLoc.x, playerLoc.z, center, scale, minimapRange);
        }
        
        // Draw player marker with rotation at calculated position
        DrawPlayerMarker(drawList, mapPos, m_fMinimapSize);
        
        // Draw zoom controls (+/- buttons only)
        DrawZoomControls(mapPos, m_fMinimapSize);
        
        // Draw coordinates below map
        ImVec2 coordPos = ImVec2(mapPos.x, mapPos.y + m_fMinimapSize + 5);
        DrawCoordinates(drawList, coordPos);
    }
    ImGui::End();
}

void CustomMinimap::DrawMinimapBackground(ImDrawList* drawList, const ImVec2& pos, float size) {
    ImVec2 min = pos;
    ImVec2 max = ImVec2(pos.x + size, pos.y + size);
    
    // Background - dark with some transparency
    drawList->AddRectFilled(min, max, IM_COL32(20, 20, 30, 200));
    
    // Border
    drawList->AddRect(min, max, IM_COL32(80, 80, 100, 255), 0.0f, 0, 2.0f);
    
    // Grid lines for visual reference
    ImU32 gridColor = IM_COL32(40, 40, 60, 150);
    float gridSize = size / 4.0f;
    
    for (int i = 1; i < 4; i++) {
        float offset = gridSize * i;
        // Vertical lines
        drawList->AddLine(
            ImVec2(min.x + offset, min.y),
            ImVec2(min.x + offset, max.y),
            gridColor);
        // Horizontal lines
        drawList->AddLine(
            ImVec2(min.x, min.y + offset),
            ImVec2(max.x, min.y + offset),
            gridColor);
    }
}

void CustomMinimap::DrawPlayerMarker(ImDrawList* drawList, const ImVec2& mapPos, float mapSize) {
    // Player arrow should ALWAYS be at the CENTER of the minimap
    // Only rotation changes based on player's world heading
    
    float centerX = mapPos.x + mapSize * 0.5f;
    float centerY = mapPos.y + mapSize * 0.5f;
    ImVec2 arrowCenter = ImVec2(centerX, centerY);
    
    // Player arrow/triangle with rotation
    float arrowSize = 10.0f;
    
    // Read player rotation DIRECTLY from player object (more reliable than cached)
    // Native formula: player+0x80 (rotation in radians)
    float rotation = 0.0f;
    DWORD playerPtr = *(DWORD*)ADDR_PLAYER_PTR;
    if (playerPtr && !IsBadReadPtr((void*)playerPtr, 0x84)) {
        rotation = *(float*)(playerPtr + PLAYER_ROTATION_OFFSET);
    }
    
    // PI - rotation: flip 180 degrees AND mirror horizontally to match native
    const float PI = 3.14159265f;
    rotation = PI - rotation;
    
    // Calculate rotated triangle points
    // Arrow pointing "up" (North) at rotation=0
    float cosR = cosf(rotation);
    float sinR = sinf(rotation);
    
    // Original triangle points (arrow pointing up = North before rotation)
    float p1x = 0, p1y = -arrowSize;                    // Top (tip)
    float p2x = -arrowSize * 0.5f, p2y = arrowSize * 0.4f;   // Bottom left
    float p3x = arrowSize * 0.5f, p3y = arrowSize * 0.4f;    // Bottom right
    
    // Apply rotation (clockwise rotation in screen coordinates)
    ImVec2 p1 = ImVec2(arrowCenter.x + (p1x * cosR - p1y * sinR),
                       arrowCenter.y + (p1x * sinR + p1y * cosR));
    ImVec2 p2 = ImVec2(arrowCenter.x + (p2x * cosR - p2y * sinR),
                       arrowCenter.y + (p2x * sinR + p2y * cosR));
    ImVec2 p3 = ImVec2(arrowCenter.x + (p3x * cosR - p3y * sinR),
                       arrowCenter.y + (p3x * sinR + p3y * cosR));
    
    // Filled triangle - yellow/gold color like native
    drawList->AddTriangleFilled(p1, p2, p3, IM_COL32(255, 215, 0, 255));
    
    // Border for visibility
    drawList->AddTriangle(p1, p2, p3, IM_COL32(0, 0, 0, 200), 2.0f);
}

void CustomMinimap::DrawCoordinates(ImDrawList* drawList, const ImVec2& pos) {
    // Display coordinates like native minimap: "X:123  Y:456"
    char coordText[64];
    sprintf(coordText, "X:%d  Y:%d", m_nDisplayX, m_nDisplayY);
    
    // Shadow
    drawList->AddText(ImVec2(pos.x + 1, pos.y + 1), IM_COL32(0, 0, 0, 200), coordText);
    // Text
    drawList->AddText(pos, IM_COL32(200, 200, 255, 255), coordText);
    
    // Region name on second line (from TextStringManager)
    const char* regionName = m_pRegionName ? m_pRegionName : "Unknown";
    drawList->AddText(ImVec2(pos.x + 1, pos.y + 15), IM_COL32(0, 0, 0, 200), regionName);
    drawList->AddText(ImVec2(pos.x, pos.y + 14), IM_COL32(255, 220, 100, 255), regionName);
}

// Native DDJ texture loader function pointer (sub_409E10)
// Takes std::string* as parameter and returns texture pointer
typedef void* (__cdecl* LoadDDJTextureFunc)(void* pStdString);
static LoadDDJTextureFunc g_pfnLoadDDJTexture = (LoadDDJTextureFunc)0x00409E10;

// std::string constructor at 0x406190 - ACTUALLY __thiscall!
// Signature: int __thiscall sub_406190(int *this, void *Src, int a3)
// this = pointer to string struct (ECX)
// Src = start of char buffer
// a3 = end of char buffer (Src + len)
static const DWORD ADDR_StdStringCtor = 0x00406190;

// Helper to call __thiscall std::string constructor
// Uses inline assembly because VS2005 doesn't support __thiscall function pointers easily
static void CallStdStringCtor(void* pThis, const char* begin, const char* end) {
    __asm {
        push end          // a3 - end pointer
        push begin        // Src - start pointer
        mov ecx, pThis    // this pointer in ECX (__thiscall)
        mov eax, ADDR_StdStringCtor
        call eax
    }
}

// Helper to create std::string and load texture
static void* LoadDDJTexture(const char* path) {
    // Simple std::string structure (12 bytes for VS2005)
    char strBuffer[16] = {0};
    
    // Create std::string with path using correct __thiscall convention
    size_t len = strlen(path);
    CallStdStringCtor(strBuffer, path, path + len);
    
    // Load texture using native function
    void* pTexture = g_pfnLoadDDJTexture(strBuffer);
    
    // Note: strBuffer will be cleaned up when function returns (leak, but small)
    return pTexture;
}

// Entity type enum for clarity
enum EntityType {
    ENTITY_UNKNOWN = 0,
    ENTITY_MONSTER,
    ENTITY_MONSTER_UNIQUE,
    ENTITY_NPC,
    ENTITY_PLAYER,
    ENTITY_ITEM,
    ENTITY_PET        // COS - our pets (horse, devil, etc)
};

// Get entity type using RuntimeClass className (proven approach from AutoTargetController)
static EntityType GetEntityTypeByRuntimeClass(DWORD entityPtr) {
    __try {
        if (!entityPtr || IsBadReadPtr((void*)entityPtr, 0x80)) {
            return ENTITY_UNKNOWN;
        }
        
        // Get VTable
        DWORD vtable = *(DWORD*)entityPtr;
        if (!vtable || IsBadReadPtr((void*)vtable, 4)) {
            return ENTITY_UNKNOWN;
        }
        
        // Call GetRuntimeClass (first virtual function)
        typedef CGfxRuntimeClass* (__thiscall *GetRuntimeClassFn)(void*);
        GetRuntimeClassFn getRuntimeClass = *(GetRuntimeClassFn*)vtable;
        
        if (IsBadReadPtr((void*)getRuntimeClass, 4)) {
            return ENTITY_UNKNOWN;
        }
        
        CGfxRuntimeClass const* rtClass = getRuntimeClass((void*)entityPtr);
        
        if (!rtClass || IsBadReadPtr((void*)rtClass, 4)) {
            return ENTITY_UNKNOWN;
        }
        
        const char* className = rtClass->m_lpszClassName;
        if (!className || IsBadReadPtr((void*)className, 1)) {
            return ENTITY_UNKNOWN;
        }
        
        // Check entity type by class name (same as AutoTargetController)
        if (strcmp(className, "CICMonster") == 0) {
            // Check if unique/elite monster (offset 0x668 == 3)
            BYTE monsterType = *(BYTE*)(entityPtr + ENTITY_OFFSET_MONSTERTYPE);
            if (monsterType == 3) {
                return ENTITY_MONSTER_UNIQUE;
            }
            return ENTITY_MONSTER;
        }
        
        if (strcmp(className, "CICNPC") == 0) {
            return ENTITY_NPC;
        }
        
        if (strcmp(className, "CICPlayer") == 0) {
            return ENTITY_PLAYER;
        }
        
        // CICUser = OTHER players (not the local player)
        // ECSRO uses CICPlayer for self, CICUser for others
        if (strcmp(className, "CICUser") == 0) {
            return ENTITY_PLAYER;
        }
        
        if (strcmp(className, "CICPickedItem") == 0) {
            return ENTITY_ITEM;
        }
        
        // COS = pets/mounts (horse, devil, etc) - our own pets
        // RuntimeClass from functions_index.h line 9736: "CICCos"
        if (strcmp(className, "CICCos") == 0) {
            return ENTITY_PET;
        }
        
        // DEBUG: Log unknown class name
        static DWORD lastUnknownLogTime = 0;
        DWORD tick = GetTickCount();
        if (tick - lastUnknownLogTime > 2000) {
            lastUnknownLogTime = tick;
            char buf[128];
            sprintf(buf, "UNKNOWN entity class: '%s' at 0x%X", className, entityPtr);
            DebugLog(buf);
        }
        
        return ENTITY_UNKNOWN;
    }
    __except (1) { return ENTITY_UNKNOWN; }
}

// Check if entity is dead or invalid (same logic as AutoTargetController)
static bool IsEntityDeadOrInvalid(DWORD entityPtr) {
    __try {
        // ActionState offset 0x1AF: 2 = dead
        // State offset 0x1E6: 4 = dead
        // InvalidFlag offset 0x63C: non-zero = invalid
        BYTE actionState = *(BYTE*)(entityPtr + 0x1AF);
        BYTE state = *(BYTE*)(entityPtr + 0x1E6);
        BYTE invalidFlag = *(BYTE*)(entityPtr + 0x63C);
        
        return (actionState == 2 || state == 4 || invalidFlag != 0);
    }
    __except (1) { return true; }
}

void CustomMinimap::DrawEntityMarkers(ImDrawList* drawList, const ImVec2& mapPos, float mapSize) {
    // Note: Cannot use __try here because std::vector requires object unwinding
    // Using IsBadReadPtr checks for safety instead
    
    // Get player pointer for position reference
    DWORD playerPtr = GetPlayerAddressRaw();
    if (playerPtr == 0) return;
    
    // Player position and region (for cross-region entity handling)
    D3DVECTOR playerLoc = GetLocationRaw(playerPtr);
    float playerX = playerLoc.x;
    float playerZ = playerLoc.z;
    
    // Minimap center
    ImVec2 center = ImVec2(mapPos.x + mapSize * 0.5f, mapPos.y + mapSize * 0.5f);
    
    // Minimap range (world units visible on minimap)
    // Native zoom logic from screenshots:
    // - Higher zoom = see MORE area (wider view, zoom out)
    // - Lower zoom = see LESS area (closer view, zoom in)
    // Base range is 100 units at zoom 1.0
    float baseRange = 100.0f;
    float minimapRange = baseRange * m_fZoomFactor;  // Higher zoom = larger range
    float scale = (mapSize * 0.5f) / minimapRange;
    
    // Get all visible entities using native linked list (dword_9C99A4)
    std::vector<DWORD> allEntities = GetVisibleEntities();
    
    // DEBUG: Log entity count every 2 seconds
    static DWORD lastEntityDebugTime = 0;
    DWORD now = GetTickCount();
    if (now - lastEntityDebugTime > 2000) {
        lastEntityDebugTime = now;
        char buf[256];
        sprintf(buf, "Minimap: %d entities, range=%.1f, playerPos=(%.1f,%.1f), zoom=%.1f", 
                (int)allEntities.size(), minimapRange, playerX, playerZ, m_fZoomFactor);
        DebugLog(buf);
        
        // Log first 5 entity positions using CORRECT offsets (0x84/0x8C)
        for (size_t dbg = 0; dbg < allEntities.size() && dbg < 5; dbg++) {
            DWORD ep = allEntities[dbg];
            if (ep && !IsBadReadPtr((void*)ep, 0x90)) {
                // Use native offsets like CIFMinimap (entity+0x84, entity+0x8C)
                float entX = *(float*)(ep + ENTITY_OFFSET_POSEX);  // 0x84
                float entZ = *(float*)(ep + ENTITY_OFFSET_POSEZ);  // 0x8C
                EntityType type = GetEntityTypeByRuntimeClass(ep);
                float rx = entX - playerX;
                float rz = entZ - playerZ;
                sprintf(buf, "  Entity[%d] type=%d pos=(%.1f,%.1f) rel=(%.1f,%.1f) inRange=%s",
                        (int)dbg, (int)type, entX, entZ, rx, rz,
                        (fabsf(rx) < minimapRange && fabsf(rz) < minimapRange) ? "YES" : "NO");
                DebugLog(buf);
            }
        }
    }
    
    for (size_t i = 0; i < allEntities.size(); i++) {
        DWORD entityPtr = allEntities[i];
        
        // Skip invalid pointers
        if (!entityPtr || IsBadReadPtr((void*)entityPtr, 0x80)) {
            continue;
        }
        
        // Skip dead/invalid entities
        if (IsEntityDeadOrInvalid(entityPtr)) {
            continue;
        }
        
        // Get entity position using NATIVE offsets (0x84/0x8C)
        // Native CIFMinimap (sub_53AD20 line 12620-12621) uses:
        //   entity+132 (0x84) for X, entity+140 (0x8C) for Z
        // NOT the CIObject base offsets 0x74/0x7C!
        float entityX = *(float*)(entityPtr + ENTITY_OFFSET_POSEX);  // 0x84
        float entityZ = *(float*)(entityPtr + ENTITY_OFFSET_POSEZ);  // 0x8C
        
        // Calculate relative position from player
        float relX = entityX - playerX;
        float relZ = entityZ - playerZ;
        
        // Check if in minimap range
        if (fabsf(relX) < minimapRange && fabsf(relZ) < minimapRange) {
            // Convert to screen position
            // Note: Z is typically north (up on minimap), X is east (right)
            float screenX = center.x + (relX * scale);
            float screenY = center.y - (relZ * scale);  // Invert Z for screen coords
            
            // Clamp to minimap bounds
            float margin = 4.0f;
            screenX = max(mapPos.x + margin, min(mapPos.x + mapSize - margin, screenX));
            screenY = max(mapPos.y + margin, min(mapPos.y + mapSize - margin, screenY));
            
            // Get entity type using RuntimeClass (proven approach)
            EntityType type = GetEntityTypeByRuntimeClass(entityPtr);
            ImVec2 markerPos = ImVec2(screenX, screenY);
            float markerSize = 4.0f;
            
            switch (type) {
                case ENTITY_MONSTER:
                    // Normal monster - red circle
                    drawList->AddCircleFilled(markerPos, markerSize, IM_COL32(255, 80, 80, 220));
                    break;
                    
                case ENTITY_MONSTER_UNIQUE:
                    // Unique/Elite monster - orange star-like marker
                    drawList->AddCircleFilled(markerPos, markerSize + 2, IM_COL32(255, 165, 0, 255));
                    drawList->AddCircle(markerPos, markerSize + 4, IM_COL32(255, 255, 100, 180), 8, 2.0f);
                    break;
                    
                case ENTITY_NPC:
                    // NPC - check if quest NPC (gold) or normal NPC (blue)
                    // Uses native sub_605040 to check quest status
                    if (IsNPCQuestTarget(entityPtr)) {
                        // Quest NPC - GOLD square
                        drawList->AddRectFilled(
                            ImVec2(markerPos.x - markerSize, markerPos.y - markerSize),
                            ImVec2(markerPos.x + markerSize, markerPos.y + markerSize),
                            IM_COL32(255, 200, 50, 255));  // Gold
                        drawList->AddRect(
                            ImVec2(markerPos.x - markerSize, markerPos.y - markerSize),
                            ImVec2(markerPos.x + markerSize, markerPos.y + markerSize),
                            IM_COL32(180, 140, 30, 255), 0.0f, 0, 1.5f);  // Dark gold border
                    } else {
                        // Normal NPC - BLUE square
                        drawList->AddRectFilled(
                            ImVec2(markerPos.x - markerSize, markerPos.y - markerSize),
                            ImVec2(markerPos.x + markerSize, markerPos.y + markerSize),
                            IM_COL32(100, 180, 255, 255));  // Blue
                        drawList->AddRect(
                            ImVec2(markerPos.x - markerSize, markerPos.y - markerSize),
                            ImVec2(markerPos.x + markerSize, markerPos.y + markerSize),
                            IM_COL32(50, 100, 180, 255), 0.0f, 0, 1.5f);  // Dark blue border
                    }
                    break;
                    
                case ENTITY_PLAYER:
                    // Other player - skip self
                    if (entityPtr != playerPtr) {
                        ImVec2 p1 = ImVec2(markerPos.x, markerPos.y - markerSize - 2);
                        ImVec2 p2 = ImVec2(markerPos.x - markerSize, markerPos.y + markerSize);
                        ImVec2 p3 = ImVec2(markerPos.x + markerSize, markerPos.y + markerSize);
                        
                        // DEBUG: Log entity ID for other players
                        static DWORD lastPlayerDebugTime = 0;
                        DWORD nowP = GetTickCount();
                        if (nowP - lastPlayerDebugTime > 2000) {
                            lastPlayerDebugTime = nowP;
                            DWORD uniqueId = *(DWORD*)(entityPtr + 412);  // Unique ID at offset 0x19C (for party matching)
                            char buf[256];
                            sprintf(buf, "OTHER_PLAYER: entity=0x%X, UniqueID(+412)=%d", entityPtr, uniqueId);
                            DebugLog(buf);
                        }
                        
                        // All other players drawn as GREEN triangles here
                        // Party members are drawn separately via DrawPartyMembers() (native approach)
                        drawList->AddTriangleFilled(p1, p2, p3, IM_COL32(50, 255, 50, 255));   // Bright green
                        drawList->AddTriangle(p1, p2, p3, IM_COL32(0, 100, 0, 255), 1.5f);     // Dark green border
                    }
                    break;
                    
                case ENTITY_ITEM:
                    // Picked item - small yellow diamond
                    drawList->AddQuadFilled(
                        ImVec2(markerPos.x, markerPos.y - 3),
                        ImVec2(markerPos.x + 3, markerPos.y),
                        ImVec2(markerPos.x, markerPos.y + 3),
                        ImVec2(markerPos.x - 3, markerPos.y),
                        IM_COL32(255, 255, 100, 200));
                    break;
                    
                case ENTITY_PET:
                    // Our pet/COS (horse, devil, etc) - orange circle
                    drawList->AddCircleFilled(markerPos, markerSize + 1, IM_COL32(255, 140, 0, 255));  // Orange
                    drawList->AddCircle(markerPos, markerSize + 1, IM_COL32(200, 100, 0, 255), 8, 1.5f); // Dark orange border
                    break;
                    
                default:
                    // Unknown - skip
                    break;
            }
        }
        // Note: Quest NPC arrows disabled - entities outside range are not shown
    }
    
    // Party members are drawn separately via DrawPartyMembers() called from Render()
    // This follows native CIFMinimap approach (sub_53AD20 lines 12773-12984)
}

// ============================================================================
// DrawPartyMembers - Renders party member markers on minimap
// Uses native PartyManager (unk_A01510) structure from sub_53AD20 lines 12773-12900
// ============================================================================
void CustomMinimap::DrawPartyMembers(ImDrawList* drawList, const ImVec2& mapPos, float mapSize,
                                      float playerX, float playerZ, const ImVec2& center, 
                                      float scale, float minimapRange) {
    __try {
        // sub_629510(&unk_A01510) returns &unk_A01510 + 24
        // unk_A01510 IS the PartyManager object, not a pointer to it
        // So partyData = 0xA01510 + 24 (direct address calculation)
        DWORD partyData = ADDR_PARTY_MANAGER + PARTY_DATA_OFFSET;  // 0xA01510 + 24
        
        // Get self ID to skip drawing ourselves (partyData + 24)
        DWORD selfID = *(DWORD*)(partyData + PARTY_SELF_ID_OFFSET);
        
        // Get party member list header pointer (partyData + 28)
        DWORD memberListHead = *(DWORD*)(partyData + PARTY_MEMBER_LIST_OFFSET);
        if (!memberListHead || IsBadReadPtr((void*)memberListHead, 0x10)) {
            return;
        }
        
        // DEBUG: Log party info periodically
        static DWORD lastPartyDebugTime = 0;
        DWORD now = GetTickCount();
        if (now - lastPartyDebugTime > 3000) {
            lastPartyDebugTime = now;
            char buf[256];
            sprintf(buf, "PARTY: partyData=0x%X, selfID=0x%X, memberHead=0x%X", 
                    partyData, selfID, memberListHead);
            DebugLog(buf);
        }
        
        // Iterate party member linked list
        // List structure: [next][prev]... memberNode+36=ID, +64=X, +72=Z
        DWORD node = *(DWORD*)memberListHead;  // First member
        
        for (int i = 0; i < 8 && node != 0 && node != memberListHead; i++) {
            if (IsBadReadPtr((void*)node, 0x50)) {
                break;
            }
            
            // Get member ID
            DWORD memberID = *(DWORD*)(node + PMEMBER_ID_OFFSET);
            
            // Skip self
            if (memberID != selfID) {
                // Get member region for cross-region adjustment (node+60)
                WORD memberRegion = *(WORD*)(node + 60);  // PMEMBER region at offset 60
                int memberRegionX = memberRegion & 0xFF;
                int memberRegionY = (memberRegion >> 8) & 0xFF;
                
                // Get player region for comparison
                DWORD playerPtr = GetPlayerAddressRaw();
                int playerRegionX = 0, playerRegionY = 0;
                if (playerPtr && !IsBadReadPtr((void*)playerPtr, 0x74)) {
                    WORD playerReg = *(WORD*)(playerPtr + 0x70);
                    playerRegionX = playerReg & 0xFF;
                    playerRegionY = (playerReg >> 8) & 0xFF;
                }
                
                // Get member position and adjust for region difference
                float memberX = *(float*)(node + PMEMBER_POSX_OFFSET);
                float memberZ = *(float*)(node + PMEMBER_POSZ_OFFSET);
                
                // Apply cross-region adjustment (192 units per region)
                memberX += (memberRegionX - playerRegionX) * 192.0f;
                memberZ += (memberRegionY - playerRegionY) * 192.0f;
                
                // Calculate relative position
                float relX = memberX - playerX;
                float relZ = memberZ - playerZ;
                
                // Check if in range
                if (fabsf(relX) < minimapRange && fabsf(relZ) < minimapRange) {
                    // Convert to screen position
                    float screenX = center.x + (relX * scale);
                    float screenY = center.y - (relZ * scale);
                    
                    // Clamp to minimap bounds
                    float margin = 4.0f;
                    screenX = max(mapPos.x + margin, min(mapPos.x + mapSize - margin, screenX));
                    screenY = max(mapPos.y + margin, min(mapPos.y + mapSize - margin, screenY));
                    
                    ImVec2 markerPos = ImVec2(screenX, screenY);
                    float markerSize = 5.0f;
                    
                    // Party member - CYAN diamond (distinct from other markers)
                    drawList->AddQuadFilled(
                        ImVec2(markerPos.x, markerPos.y - markerSize - 1),
                        ImVec2(markerPos.x + markerSize + 1, markerPos.y),
                        ImVec2(markerPos.x, markerPos.y + markerSize + 1),
                        ImVec2(markerPos.x - markerSize - 1, markerPos.y),
                        IM_COL32(0, 255, 255, 255));  // Cyan
                    drawList->AddQuad(
                        ImVec2(markerPos.x, markerPos.y - markerSize - 1),
                        ImVec2(markerPos.x + markerSize + 1, markerPos.y),
                        ImVec2(markerPos.x, markerPos.y + markerSize + 1),
                        ImVec2(markerPos.x - markerSize - 1, markerPos.y),
                        IM_COL32(0, 150, 150, 255), 1.5f);  // Dark cyan border
                }
            }
            
            // Next member
            node = *(DWORD*)node;
        }
    }
    __except(1) {
        // Exception occurred, skip party rendering
    }
}

void CustomMinimap::DrawZoomControls(const ImVec2& mapPos, float mapSize) {
    // Position zoom controls below coordinates
    ImVec2 zoomPos = ImVec2(mapPos.x, mapPos.y + mapSize + 35);
    
    ImGui::SetCursorScreenPos(zoomPos);
    
    // +/- buttons only (no slider)
    if (ImGui::Button("-", ImVec2(30, 20))) {
        m_fZoomFactor = max(0.5f, m_fZoomFactor - 0.5f);
        CIFMinimap* pMinimap = CIFMinimap::GetInstance();
        if (pMinimap) pMinimap->SetZoomFactor(m_fZoomFactor);
    }
    ImGui::SameLine(0, 5);
    
    // Display current zoom level
    ImGui::Text("%.1fx", m_fZoomFactor);
    
    ImGui::SameLine(0, 5);
    if (ImGui::Button("+", ImVec2(30, 20))) {
        m_fZoomFactor = min(8.0f, m_fZoomFactor + 0.5f);  // Max zoom out = 8x
        CIFMinimap* pMinimap = CIFMinimap::GetInstance();
        if (pMinimap) pMinimap->SetZoomFactor(m_fZoomFactor);
    }
}

void CustomMinimap::LoadTextures() {
    // Load minimap marker textures using native DDJ loader
    m_pTexCharacter = LoadDDJTexture("interface\\minimap\\mm_sign_character.ddj");
    m_pTexPartyArrow = LoadDDJTexture("interface\\minimap\\mm_sign_partyarrow.ddj");
    m_pTexNPC = LoadDDJTexture("interface\\minimap\\mm_sign_npc.ddj");
    m_pTexPartyMember = LoadDDJTexture("interface\\minimap\\mm_sign_partymember.ddj");
    
    // Note: mm_sign_monster.ddj may not exist, skip for now
    // m_pTexMonster = LoadDDJTexture("interface\\minimap\\mm_sign_monster.ddj");
}
