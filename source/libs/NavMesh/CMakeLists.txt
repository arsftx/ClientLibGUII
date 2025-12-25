set(SOURCE_FILES 
    src/RTNavMeshTerrain.cpp
    src/NavMeshCollision.cpp
)

add_library(NavMesh STATIC ${SOURCE_FILES})
target_include_directories(NavMesh
        PUBLIC
        include/
        PRIVATE include/NavMesh)

target_link_libraries(NavMesh d3d9 d3dx9)
