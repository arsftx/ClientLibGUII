set(SOURCE_FILES
        imgui/imgui.cpp
        imgui/imgui_widgets.cpp
        imgui/imgui_draw.cpp
        imgui/examples/imgui_impl_dx9.cpp
        imgui/examples/imgui_impl_win32.cpp
        imgui/misc/freetype/imgui_freetype.cpp)

# Find FreeType library
find_package(Freetype QUIET)
if(FREETYPE_FOUND)
    message(STATUS "FreeType found: ${FREETYPE_LIBRARIES}")
else()
    # Try to find in common vcpkg locations
    if(EXISTS "C:/vcpkg/installed/x86-windows/lib/freetype.lib")
        set(FREETYPE_LIBRARIES "C:/vcpkg/installed/x86-windows/lib/freetype.lib")
        set(FREETYPE_INCLUDE_DIRS "C:/vcpkg/installed/x86-windows/include")
        set(FREETYPE_FOUND TRUE)
        message(STATUS "FreeType found via vcpkg: ${FREETYPE_LIBRARIES}")
    else()
        message(WARNING "FreeType NOT found - font rendering will use fallback")
    endif()
endif()

add_library(imgui STATIC ${SOURCE_FILES})
target_include_directories(imgui
        PUBLIC ./
        PRIVATE imgui/
        PRIVATE imgui/misc/freetype/)

if(FREETYPE_FOUND)
    target_include_directories(imgui PRIVATE ${FREETYPE_INCLUDE_DIRS})
    target_link_libraries(imgui d3d9 d3dx9 ${FREETYPE_LIBRARIES})
    target_compile_definitions(imgui PRIVATE IMGUI_ENABLE_FREETYPE)
else()
    target_link_libraries(imgui d3d9 d3dx9)
endif()
# target_link_directories(imgui PUBLIC ${DirectX_LIBRARY_PATHS})

add_executable(imgui-demo-win32-dx9
        imgui/examples/example_win32_directx9/main.cpp
        imgui/imgui_demo.cpp)

target_link_libraries(imgui-demo-win32-dx9 imgui d3d9)

target_include_directories(imgui-demo-win32-dx9
        PRIVATE
        imgui/
        imgui/examples/)
