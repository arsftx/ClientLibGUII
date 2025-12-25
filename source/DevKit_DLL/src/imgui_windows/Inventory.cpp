#include "Inventory.h"

#include "imgui/imgui.h"

#include <GInterface.h>

Inventory::Inventory() : bShow(false) {

}

void Inventory::MenuItem() {
    ImGui::MenuItem("Inventory", 0, &bShow);
}

void Inventory::Render() {

    if (!bShow)
        return;


    if (!ImGui::Begin("Inventory", &bShow)) {
        ImGui::End();
        return;
    }

    CIFInventory *inventory = g_pCGInterface->GetMainPopup()->GetInventory();

    for (int i = 0; i < 20; i++) {
        CSOItem *item = inventory->GetItemBySlot(i);

        if (item == NULL) {
            ImGui::Text("Item %d: NONE", i);
            continue;
        }

        if (item->m_refObjItemId == 0) {
            ImGui::Text("Item %d: (empty)", i);
            continue;
        }

        // Display item info using ECSRO offsets
        ImGui::Text("Item %d: TID=%d, +%d", i, item->m_refObjItemId, item->m_OptLevel);
        ImGui::Text("  Durability: %d", item->m_CurrDurability);
        ImGui::Text("  PhyAtk: %d-%d, MagAtk: %d-%d", 
                   item->m_PhyAtkPwrMin, item->m_PhyAtkPwrMax,
                   item->m_MagAtkPwrMin, item->m_MagAtkPwrMax);
        ImGui::Text("  PhyDef: %.1f, MagDef: %.1f", 
                   item->m_PhyDefPwrValue, item->m_MagDefPwrValue);
        
        // TODO: Re-implement blue options using ECSRO magic attribute list (CSOItem+0x84)
        // Blue options (STR, INT, HP, MP etc.) are stored differently in ECSRO
    }

    ImGui::End();
}

