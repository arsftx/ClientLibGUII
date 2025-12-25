#include "About.h"

#include <imgui/imgui.h>
#include <GInterface.h>

#include "SRIFLib/NInterfaceResource.h"
#include "SRIFLib/NIFWnd.h"

#include "Game.h"

void About::MenuItem() {
    ImGui::MenuItem("About", 0, &bShow);
}

About::About() : bShow(true), m_bButtonFloNotFound(false), m_bButtonQbiNotFound(false) {
}

void About::Render() {

    const ImVec2 vec(370.0f, 270.0f);

    if (!bShow)
        return;

    if (!ImGui::Begin("About", &bShow, vec)) {
        ImGui::End();
        return;
    }

    ImGui::TextWrapped("Welcome to SRO_DevKit, \n"
                       "\n"
                       "SRO_DevKit is a C++ framework for creating and extending "
                       "the game Silkroad Online. It aims to be binary compatible to "
                       "the original game client allowing the entire codebase to work "
                       "without assembly code. The framework can easily be extended "
                       "with interfaces for existing classes in the game.\n"
                       "\n"
                       "SRO_DevKit is free, open-source.");

    ImGui::TextWrapped("If you imported the files for the demo window correctly, "
                       "you can go ahead and press the button below to show it.");

    if (g_pCGInterface == NULL) {
        ImGui::TextWrapped("Log in to show the cif demo window btn.");
    } else {
        if (ImGui::Button("Show Demo CIF Window")) {
            //CIFWnd *demoWnd = g_pCGInterface->m_IRM.GetResObj(1338, 1);
            CIFWnd *demoWnd = 0;
            m_bButtonFloNotFound = demoWnd == 0;

            if (demoWnd != NULL) {
                demoWnd->ShowGWnd(true);
            }
        }

        if (m_bButtonFloNotFound) {
            ImGui::TextColored(ImColor(255, 0, 0), "Error: CIF Window could not be found.");
        }
    }

    if (ImGui::Button("Show Demo CNIF Window")) {
        if (g_sNewInterfaceMgr.GetInterfaceObj(125051) == NULL && !m_bButtonQbiNotFound)
            g_sNewInterfaceMgr.InstantiateDimensional("res_ui\\nifkyuubi09.2dt", theApp.GetCurrentProcess(), 0);

        CNIFWnd *demoWnd = g_sNewInterfaceMgr.GetInterfaceObj(125051);
        m_bButtonQbiNotFound = demoWnd == 0;

        if (demoWnd != NULL) {
            demoWnd->ShowGWnd(true);
        }

        if (m_bButtonQbiNotFound) {
            ImGui::TextColored(ImColor(255, 0, 0), "Error: CNIF Window could not be found.");
        }
    }

    ImGui::End();
}
