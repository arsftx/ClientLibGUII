#include "CustomPlayerMiniInfo.h"
#include "CustomGUISession.h"
#include <imgui/imgui.h>
#include <GInterface.h>
#include <Game.h>
#include <ICPlayer.h>
#include <IFMainPopup.h>
#include <IFPlayerInfo.h>
#include <CharacterDependentData.h>
#include <GlobalDataManager.h>
#include <CharacterPortrait.h>
#include <BSLib/multibyte.h>
#include <ClientNet/MsgStreamBuffer.h>
#include <stdio.h>
#include <time.h>
#include <GFX3DFunction/GFXVideo3d.h>

// Opcode for selecting target (self-target on portrait click)
#define OPCODE_SELECT_TARGET 0x7045

// Address constants from ECSRO client
static const DWORD ADDR_PLAYER_PTR = 0x00A0465C;
static const DWORD ADDR_MAINPOPUP = 0xC5DD24;

// Native target function (from CIFPlayerMiniInfo::sub_519060)
typedef void (__thiscall *SelectTargetFn)(void* pMainPopup, DWORD uniqueID);
static const DWORD ADDR_SUB_4EA8D0 = 0x4EA8D0;

// Native visibility check (from CIFPlayerMiniInfo render - sub_89F430)
typedef bool (__thiscall *IsWindowVisibleFn)(void* pThis);
static const DWORD ADDR_SUB_89F430 = 0x89F430;

// Loading Manager global pointer (from sub_5ABE80 -> dword_A00524)
// Loading flag is at offset +0xBC (from sub_5E0370 analysis)
static const DWORD ADDR_LOADING_MANAGER = 0xA00524;
static const DWORD OFFSET_LOADING_FLAG = 0xBC;

// Helper to check if UI should be visible (not during loading/teleport)
static bool IsUIVisible() {
    // Check basic player validity first
    DWORD* pPlayerPtr = (DWORD*)ADDR_PLAYER_PTR;
    if (!pPlayerPtr || !*pPlayerPtr) return false;
    
    DWORD* pMainPopupPtr = (DWORD*)ADDR_MAINPOPUP;
    if (!pMainPopupPtr || !*pMainPopupPtr) return false;
    
    // Check loading state from Loading Manager
    DWORD loadingManager = *(DWORD*)ADDR_LOADING_MANAGER;
    if (loadingManager != 0) {
        BYTE isLoading = *(BYTE*)(loadingManager + OFFSET_LOADING_FLAG);
        if (isLoading != 0) {
            return false;  // Loading in progress, hide our UI
        }
    }
    
    return true;
}

// Helper function for self-target using native game function
static void SendSelfTargetPacket() {
    DWORD playerAddr = *(DWORD*)ADDR_PLAYER_PTR;
    if (!playerAddr) return;
    
    DWORD playerUniqueID = *(DWORD*)(playerAddr + 0xE0);
    if (playerUniqueID == 0) return;
    
    DWORD* pMainPopup = (DWORD*)ADDR_MAINPOPUP;
    if (!pMainPopup || !*pMainPopup) return;
    
    // Call native target function directly (instant, no delay!)
    SelectTargetFn selectTarget = (SelectTargetFn)ADDR_SUB_4EA8D0;
    selectTarget((void*)*pMainPopup, playerUniqueID);
}

// Static initialization tracking
static bool s_PlayerInfoInitialized = false;

// Singleton instance
CustomPlayerMiniInfo& CustomPlayerMiniInfo::Instance() {
    static CustomPlayerMiniInfo instance;
    return instance;
}

// Static render callback for CustomGUISession
static void PlayerMiniInfo_RenderCallback() {
    CustomPlayerMiniInfo::Instance().Render();
}

// Initialize - registers with CustomGUISession
bool CustomPlayerMiniInfo::Initialize() {
    if (s_PlayerInfoInitialized) return true;
    
    // Initialize character portrait renderer
    GetCharacterPortrait().Initialize(64, 64);
    
    // Register our render callback with the shared GUI session
    int callbackId = g_CustomGUI.RegisterRenderCallback(PlayerMiniInfo_RenderCallback);
    if (callbackId < 0) return false;
    
    s_PlayerInfoInitialized = true;
    return true;
} 

CustomPlayerMiniInfo::CustomPlayerMiniInfo() {
    // Initialize all members
    m_bShow = true;
    m_bEnabled = true;
    m_bShowStats = false;
    m_bShowStatsPopup = false;
    m_fPosX = 10.0f;
    m_fPosY = 10.0f;
    m_fAnimatedHP = 1.0f;
    m_fAnimatedMP = 1.0f;
    m_fAnimationSpeed = 0.05f;
    m_pCachedMiniInfo = NULL;
    
    // Default colors
    m_colors.hpBar[0] = 0.8f; m_colors.hpBar[1] = 0.2f; m_colors.hpBar[2] = 0.2f; m_colors.hpBar[3] = 1.0f;
    m_colors.mpBar[0] = 0.2f; m_colors.mpBar[1] = 0.4f; m_colors.mpBar[2] = 0.8f; m_colors.mpBar[3] = 1.0f;
    m_colors.hpCritical[0] = 1.0f; m_colors.hpCritical[1] = 0.0f; m_colors.hpCritical[2] = 0.0f; m_colors.hpCritical[3] = 1.0f;
    m_colors.background[0] = 0.1f; m_colors.background[1] = 0.1f; m_colors.background[2] = 0.15f; m_colors.background[3] = 0.85f;
    m_colors.text[0] = 1.0f; m_colors.text[1] = 1.0f; m_colors.text[2] = 1.0f; m_colors.text[3] = 1.0f;
    m_colors.levelText[0] = 1.0f; m_colors.levelText[1] = 0.85f; m_colors.levelText[2] = 0.0f; m_colors.levelText[3] = 1.0f;
}

void CustomPlayerMiniInfo::MenuItem() {
    if (ImGui::BeginMenu("Player Info")) {
        bool enabled = m_bEnabled;
        if (ImGui::MenuItem("Enable Custom UI", NULL, &enabled)) {
            if (enabled) {
                Enable();
            } else {
                Disable();
            }
        }
        
        ImGui::MenuItem("Show Stats Panel", NULL, &m_bShowStats);
        
        ImGui::EndMenu();
    }
}

void CustomPlayerMiniInfo::Render() {
    if (!m_bShow) return;
    
    // Check if UI should be visible (not during loading/teleport)
    if (!IsUIVisible()) {
        return;
    }
    
    // Get player pointer safely
    DWORD pPlayerDW = 0;
    __try {
        pPlayerDW = *(DWORD*)ADDR_PLAYER_PTR;
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        return;
    }
    
    if (pPlayerDW == 0) {
        return;  // Player not loaded yet
    }
    
    CICPlayerEcsro* pPlayerEcsro = (CICPlayerEcsro*)pPlayerDW;
    
    // Note: Loading state is already checked by IsUIVisible() using the loading manager flag
    // No need for additional HP checks here
    
    // Window flags - no titlebar, no resize, but allow interaction
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar |
                              ImGuiWindowFlags_NoResize |
                              ImGuiWindowFlags_NoScrollbar |
                              ImGuiWindowFlags_NoScrollWithMouse;
    
    if (m_bEnabled) {
        flags |= ImGuiWindowFlags_NoBackground;
    }
    
    ImGui::SetNextWindowPos(ImVec2(m_fPosX, m_fPosY), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(350, 100), ImGuiCond_Always);  // Wider for portrait
    
    // Custom styling
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(m_colors.background[0], 
                                                     m_colors.background[1], 
                                                     m_colors.background[2], 
                                                     m_colors.background[3]));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 10));
    
    if (ImGui::Begin("##CustomPlayerMiniInfo", NULL, flags)) {
        // Read player data safely - HP/MP are int, not short!
        int currentHP = 0, maxHP = 1, currentMP = 0, maxMP = 1;
        BYTE level = 0, hwanPoint = 0;
        short strength = 0, intelligence = 0, statPoints = 0;
        char* charName = NULL;
        
        __try {
            currentHP = pPlayerEcsro->RemaingHP;
            maxHP = pPlayerEcsro->MaxHP;
            currentMP = pPlayerEcsro->RemaingMP;
            maxMP = pPlayerEcsro->MaxMP;
            level = pPlayerEcsro->Level;
            strength = pPlayerEcsro->Strength;
            intelligence = pPlayerEcsro->Intelligence;
            hwanPoint = pPlayerEcsro->HwanPoint;
            charName = pPlayerEcsro->charname;
            statPoints = pPlayerEcsro->StatPoints;
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            maxHP = 1;
            maxMP = 1;
        }
        
        // Calculate percentages
        float hpPercent = (maxHP > 0) ? (float)currentHP / (float)maxHP : 0.0f;
        float mpPercent = (maxMP > 0) ? (float)currentMP / (float)maxMP : 0.0f;
        
        // Clamp percentages
        if (hpPercent < 0.0f) hpPercent = 0.0f;
        if (hpPercent > 1.0f) hpPercent = 1.0f;
        if (mpPercent < 0.0f) mpPercent = 0.0f;
        if (mpPercent > 1.0f) mpPercent = 1.0f;
        
        // Update animated values (smooth transition)
        float animSpeed = 0.1f;  // Faster animation
        m_fAnimatedHP += (hpPercent - m_fAnimatedHP) * animSpeed;
        m_fAnimatedMP += (mpPercent - m_fAnimatedMP) * animSpeed;
        
        // Clamp animated values too
        if (m_fAnimatedHP < 0.0f) m_fAnimatedHP = 0.0f;
        if (m_fAnimatedHP > 1.0f) m_fAnimatedHP = 1.0f;
        if (m_fAnimatedMP < 0.0f) m_fAnimatedMP = 0.0f;
        if (m_fAnimatedMP > 1.0f) m_fAnimatedMP = 1.0f;
        
        // === Character Portrait on the left ===
        CharacterPortrait& portrait = GetCharacterPortrait();
        portrait.Update();  // Update portrait texture
        
        IDirect3DTexture9* pPortraitTex = portrait.GetTexture();
        if (pPortraitTex && portrait.IsReady()) {
            // TODO: Display PNG loaded based on model ID
            ImGui::Image((ImTextureID)pPortraitTex, 
                        ImVec2((float)portrait.GetWidth(), (float)portrait.GetHeight()),
                        ImVec2(0, 0), ImVec2(1, 1),
                        ImVec4(1, 1, 1, 1),
                        ImVec4(0.3f, 0.3f, 0.3f, 0.8f));
            ImGui::SameLine();
        } else {
            // TODO: Load PNG face image based on model ID
            // Model ID at player+0x160: Chinese ~1907-1908, European ~14709-14714
            ImVec2 cursorPos = ImGui::GetCursorScreenPos();
            ImDrawList* drawList = ImGui::GetWindowDrawList();
            drawList->AddRectFilled(cursorPos, 
                                   ImVec2(cursorPos.x + 64, cursorPos.y + 64),
                                   IM_COL32(40, 40, 60, 220));
            drawList->AddRect(cursorPos,
                             ImVec2(cursorPos.x + 64, cursorPos.y + 64),
                             IM_COL32(100, 100, 140, 255));
            
            // Display model ID as text
            int modelID = portrait.GetModelID();
            char modelText[32];
            sprintf(modelText, "ID:%d", modelID);
            
            ImVec2 textSize = ImGui::CalcTextSize(modelText);
            float textX = cursorPos.x + (64 - textSize.x) * 0.5f;
            float textY = cursorPos.y + (64 - textSize.y) * 0.5f;
            drawList->AddText(ImVec2(textX, textY), IM_COL32(255, 255, 255, 255), modelText);
            
            ImGui::Dummy(ImVec2(64, 64));
            ImGui::SameLine();
        }
        
        // Begin group for name/level and bars
        ImGui::BeginGroup();
        
        // Character name and level
        if (charName && charName[0] != '\0') {
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "%s", charName);
            ImGui::SameLine();
        }
        ImGui::TextColored(ImVec4(m_colors.levelText[0], m_colors.levelText[1], 
                                   m_colors.levelText[2], m_colors.levelText[3]),
                          "Lv.%d", level);
        
        // Stats button [S] on same line
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.3f, 0.5f, 0.8f));
        if (ImGui::SmallButton("[S]")) {
            m_bShowStatsPopup = !m_bShowStatsPopup;
        }
        ImGui::PopStyleColor();
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Show Character Stats");
        }
        
        // RS button (Remaining Stats) - appears when stat points > 0
        if (statPoints > 0) {
            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.3f, 0.3f, 0.9f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.4f, 0.4f, 1.0f));
            if (ImGui::SmallButton("[RS]")) {
                // Call sub_4FC780 (0x4FC780) - THE EXACT function C key uses to toggle Character window
                // this = CGInterface pointer
                __try {
                    if (g_pCGInterface) {
                        typedef int (__thiscall *ToggleCharWindowFn)(void*);
                        ToggleCharWindowFn pToggle = (ToggleCharWindowFn)0x4FC780;
                        pToggle(g_pCGInterface);
                    }
                } __except(EXCEPTION_EXECUTE_HANDLER) {
                    // Ignore if failed
                }
            }
            ImGui::PopStyleColor(2);
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Remaining Stat Points: %d - Click to open Character Window", statPoints);
            }
        }
        
        // HP Bar - use actual percentage for color, animated for bar
        ImVec4 hpColor = (hpPercent < 0.25f) ? 
            ImVec4(m_colors.hpCritical[0], m_colors.hpCritical[1], m_colors.hpCritical[2], m_colors.hpCritical[3]) :
            ImVec4(m_colors.hpBar[0], m_colors.hpBar[1], m_colors.hpBar[2], m_colors.hpBar[3]);
        
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, hpColor);
        char hpText[64];
        sprintf(hpText, "HP: %d / %d", currentHP, maxHP);
        ImGui::ProgressBar(hpPercent, ImVec2(-1, 18), hpText);  // Use actual percent, not animated
        ImGui::PopStyleColor();
        
        // MP Bar
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, 
            ImVec4(m_colors.mpBar[0], m_colors.mpBar[1], m_colors.mpBar[2], m_colors.mpBar[3]));
        char mpText[64];
        sprintf(mpText, "MP: %d / %d", currentMP, maxMP);
        ImGui::ProgressBar(mpPercent, ImVec2(-1, 18), mpText);  // Use actual percent
        ImGui::PopStyleColor();
        
        // Hwan Points (if any)
        if (hwanPoint > 0) {
            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "Hwan: %d/5", hwanPoint);
        }
        
        ImGui::EndGroup();  // End portrait group
        
        // Self-target: Click anywhere on window to target self (like native PlayerMiniInfo)
        if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(0)) {
            SendSelfTargetPacket();
        }
    }
    ImGui::End();
    
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor();
    
    // Render stats popup if open
    if (m_bShowStatsPopup) {
        RenderStatsPopup(pPlayerEcsro);
    }
}

void CustomPlayerMiniInfo::RenderStatsPopup(CICPlayerEcsro* pPlayer) {
    ImGuiWindowFlags popupFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize;
    
    if (ImGui::Begin("Character Stats", &m_bShowStatsPopup, popupFlags)) {
        // Read basic stats safely
        BYTE level = 0;
        short str = 0, intel = 0, statPoints = 0;
        int currentHP = 0, maxHP = 1, currentMP = 0, maxMP = 1;
        
        // Combat stats (from combat stats struct at 0xA01510)
        int phyAtkMin = 0, phyAtkMax = 0;
        int magAtkMin = 0, magAtkMax = 0;
        int phyDef = 0, magDef = 0;
        int phyBalance = 0, magBalance = 0;
        int hitRatio = 0, parryRatio = 0;
        
        __try {
            // Basic stats from player struct
            level = pPlayer->Level;
            str = pPlayer->Strength;
            intel = pPlayer->Intelligence;
            statPoints = pPlayer->StatPoints;
            currentHP = pPlayer->RemaingHP;
            maxHP = pPlayer->MaxHP;
            currentMP = pPlayer->RemaingMP;
            maxMP = pPlayer->MaxMP;
            
            // Combat stats - read from global combat stats struct
            // From IDA: sub_62A6E0 returns this+252 (0xFC offset)
            // Combat stats struct is at 0xA01510 + 0xFC = 0xA0160C
            DWORD pCombatStats = 0xA01510 + 0xFC;  // 0xA0160C
            if (pCombatStats != 0) {
                phyAtkMin = *(int*)(pCombatStats + 0x00);    // PhyAtk Min
                phyAtkMax = *(int*)(pCombatStats + 0x04);    // PhyAtk Max
                magAtkMin = *(int*)(pCombatStats + 0x08);    // MagAtk Min
                magAtkMax = *(int*)(pCombatStats + 0x0C);    // MagAtk Max
                phyDef = *(short*)(pCombatStats + 0x10);     // PhyDef (short)
                magDef = *(short*)(pCombatStats + 0x12);     // MagDef (short)  
                hitRatio = *(short*)(pCombatStats + 0x14);   // Hit Ratio (short)
                parryRatio = *(short*)(pCombatStats + 0x16); // Parry Ratio (short)
                
                // Balance calculation from IDA sub_45D4D0:
                // PhyBalance = (STR + 2*Level + 14) * 100 / ((Level-1)*7 + 56) / 0.857
                // MagBalance = INT * 100 / (5 * (Level + 7)) / 0.8
                // Both capped at 120%
                if (level > 0) {
                    double phyBalanceCalc = (double)(str + 2 * level + 14) * 100.0 
                                          / ((double)(level - 1) * 7.0 + 56.0) / 0.857142857;
                    phyBalance = (int)phyBalanceCalc;
                    if (phyBalance > 120) phyBalance = 120;
                    
                    double magBalanceCalc = (double)intel * 100.0 
                                          / ((double)(5 * (level + 7))) / 0.8;
                    magBalance = (int)magBalanceCalc;
                    if (magBalance > 120) magBalance = 120;
                }
            }
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            // Use safe defaults
        }
        
        // Display stats in formatted layout
        ImGui::TextColored(ImVec4(1.0f, 0.9f, 0.4f, 1.0f), "=== Character Stats ===");
        ImGui::Separator();
        
        // Basic Info
        ImGui::Columns(2, NULL, false);
        ImGui::Text("Level:"); ImGui::NextColumn(); ImGui::Text("%d", level); ImGui::NextColumn();
        ImGui::Text("HP:"); ImGui::NextColumn(); ImGui::Text("%d / %d", currentHP, maxHP); ImGui::NextColumn();
        ImGui::Text("MP:"); ImGui::NextColumn(); ImGui::Text("%d / %d", currentMP, maxMP); ImGui::NextColumn();
        ImGui::Columns(1);
        
        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.5f, 1.0f, 0.5f, 1.0f), "=== Base Stats ===");
        
        ImGui::Columns(2, NULL, false);
        ImGui::Text("STR:"); ImGui::NextColumn(); ImGui::Text("%d", str); ImGui::NextColumn();
        ImGui::Text("INT:"); ImGui::NextColumn(); ImGui::Text("%d", intel); ImGui::NextColumn();
        ImGui::Text("Remaining Stat Points:"); ImGui::NextColumn(); ImGui::Text("%d", statPoints); ImGui::NextColumn();
        ImGui::Columns(1);
        
        ImGui::Separator();
        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.5f, 1.0f), "=== Combat Stats ===");
        
        ImGui::Columns(2, NULL, false);
        ImGui::Text("Phy. Atk:"); ImGui::NextColumn(); ImGui::Text("%d ~ %d", phyAtkMin, phyAtkMax); ImGui::NextColumn();
        ImGui::Text("Phy. Def:"); ImGui::NextColumn(); ImGui::Text("%d", phyDef); ImGui::NextColumn();
        ImGui::Text("Mag. Atk:"); ImGui::NextColumn(); ImGui::Text("%d ~ %d", magAtkMin, magAtkMax); ImGui::NextColumn();
        ImGui::Text("Mag. Def:"); ImGui::NextColumn(); ImGui::Text("%d", magDef); ImGui::NextColumn();
        ImGui::Separator();
        ImGui::Text("Phy. Balance:"); ImGui::NextColumn(); ImGui::Text("%d%%", phyBalance); ImGui::NextColumn();
        ImGui::Text("Mag. Balance:"); ImGui::NextColumn(); ImGui::Text("%d%%", magBalance); ImGui::NextColumn();
        ImGui::Text("Hit Ratio:"); ImGui::NextColumn(); ImGui::Text("%d", hitRatio); ImGui::NextColumn();
        ImGui::Text("Parry Ratio:"); ImGui::NextColumn(); ImGui::Text("%d", parryRatio); ImGui::NextColumn();
        ImGui::Columns(1);
    }
    ImGui::End();
}

void CustomPlayerMiniInfo::Enable() {
    m_bEnabled = true;
}

void CustomPlayerMiniInfo::Disable() {
    m_bEnabled = false;
}

bool CustomPlayerMiniInfo::IsEnabled() const {
    return m_bEnabled;
}

void CustomPlayerMiniInfo::SetPosition(float x, float y) {
    m_fPosX = x;
    m_fPosY = y;
}

void CustomPlayerMiniInfo::GetPosition(float& x, float& y) const {
    x = m_fPosX;
    y = m_fPosY;
}

// Stub implementations
void CustomPlayerMiniInfo::RenderHPBar(CICPlayerEcsro* pPlayer) {}
void CustomPlayerMiniInfo::RenderMPBar(CICPlayerEcsro* pPlayer) {}
void CustomPlayerMiniInfo::RenderLevelInfo(CICPlayerEcsro* pPlayer) {}
void CustomPlayerMiniInfo::RenderZerkPoints(CIFPlayerMiniInfo* pMiniInfo) {}
void CustomPlayerMiniInfo::RenderStatsPanel(CICPlayerEcsro* pPlayer) {}
void CustomPlayerMiniInfo::HideOriginalGUI() {}
void CustomPlayerMiniInfo::ShowOriginalGUI() {}
CIFPlayerMiniInfo* CustomPlayerMiniInfo::GetOriginalPlayerMiniInfo() { return NULL; }
