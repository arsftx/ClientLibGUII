#include "ItemData.h"

#include <Data/ItemData.h>
#include <GlobalDataManager.h>
#include <imgui/imgui.h>

void ItemData::MenuItem() {
    ImGui::MenuItem("Item Data", 0, &bShow);
}

void ItemData::Render() {

    if (!bShow) return;

    if (!ImGui::Begin("Item Data", &bShow)) {
        ImGui::End();
        return;
    }

    static int id = 0;
    static const CItemData *data = NULL;

    if (ImGui::InputInt("ID", &id)) {
        // Update
        data = g_CGlobalDataManager->GetItem(id);
    }

    if (data != NULL) {
        const SItemData &sItemData = data->GetData();
        ImGui::LabelText("CodeName", "%ls", sItemData.CodeName.c_str());
        ImGui::LabelText("Stack", "%d", sItemData.m_maxStack);
    }

    ImGui::End();
}
