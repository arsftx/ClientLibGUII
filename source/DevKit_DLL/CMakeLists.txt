set(SOURCE_FILES
        src/DllMain.cpp
        src/SoundBugFix.cpp src/SoundBugFix.h
        src/DamageFix.cpp src/DamageFix.h
        src/WindowedMode.cpp src/WindowedMode.h
        src/hooks/Hooks.cpp src/hooks/Hooks.h
        src/hooks/GFXVideo3d_Hook.cpp src/hooks/GFXVideo3d_Hook.h
        src/hooks/WndProc_Hook.cpp src/hooks/WndProc_Hook.h
        src/hooks/CGame_Hook.cpp src/hooks/CGame_Hook.h
        src/hooks/PlayerMiniInfo_Hook.cpp src/hooks/PlayerMiniInfo_Hook.h
        src/examples/IFflorian0.cpp src/examples/IFflorian0.h
        src/examples/NIFKyuubi09.cpp src/examples/NIFKyuubi09.h
        src/imgui_windows/ImGui_Windows.cpp src/imgui_windows/ImGui_Windows.h
        src/imgui_windows/InterfaceDebugger.cpp src/imgui_windows/InterfaceDebugger.h
        src/imgui_windows/SoundTool.cpp src/imgui_windows/SoundTool.h
        src/imgui_windows/NavMeshTool.cpp src/imgui_windows/NavMeshTool.h
        src/imgui_windows/EntityExplorer.cpp src/imgui_windows/EntityExplorer.h
        src/imgui_windows/NotificationTool.cpp src/imgui_windows/NotificationTool.h
        src/imgui_windows/About.cpp src/imgui_windows/About.h
        src/imgui_windows/SystemMessage.cpp src/imgui_windows/SystemMessage.h
        src/imgui_windows/ErrorMessageTool.h src/imgui_windows/ErrorMessageTool.cpp
        src/imgui_windows/InterfaceTree.cpp src/imgui_windows/InterfaceTree.h
        src/imgui_windows/ProcessViewer.cpp src/imgui_windows/ProcessViewer.h
        src/imgui_windows/Inventory.h src/imgui_windows/Inventory.cpp
        src/imgui_windows/PartyInfo.cpp src/imgui_windows/PartyInfo.h
        src/imgui_windows/CharacterData.cpp src/imgui_windows/CharacterData.h
        src/imgui_windows/CustomPlayerMiniInfo.cpp src/imgui_windows/CustomPlayerMiniInfo.h
        src/imgui_windows/CustomGUISession.cpp src/imgui_windows/CustomGUISession.h
        src/imgui_windows/ItemData.cpp src/imgui_windows/ItemData.h
        src/Util.cpp src/Util.h
        src/QuickStart.cpp src/QuickStart.h
        src/MathUtil.cpp src/MathUtil.h
        src/CustomOpcodeHandler.cpp src/CustomOpcodeHandler.h
        src/CustomDamageRenderer.cpp src/CustomDamageRenderer.h
        src/WindowedModeManager.cpp src/WindowedModeManager.h
        src/imgui_windows/NativeBarRenderer.cpp src/imgui_windows/NativeBarRenderer.h
        src/imgui_windows/UIRenderer.h
        src/imgui_windows/SROProgressBar.h
        src/imgui_windows/CustomMinimap.cpp src/imgui_windows/CustomMinimap.h
        src/ClientValidation.cpp src/ClientValidation.h)

add_library(DevKit_DLL SHARED ${SOURCE_FILES})
target_link_libraries(DevKit_DLL BSLib SROInterfaceLIB ClientLib MathHelpers memory winmm d3d9 d3dx9 crypt32 iphlpapi advapi32 rpcrt4)

set_target_properties(DevKit_DLL
        PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/BinOut/${CMAKE_BUILD_TYPE}/"
        LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/BinOut/${CMAKE_BUILD_TYPE}/"
        RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/BinOut/${CMAKE_BUILD_TYPE}/"
        OUTPUT_NAME "ClientLib"
        )
