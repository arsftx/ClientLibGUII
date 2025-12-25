set(SOURCE_FILES src/hook.cpp src/util.cpp src/RemodelUtility.cpp)

add_library(memory STATIC ${SOURCE_FILES})
target_include_directories(memory
        PUBLIC include/
        PRIVATE include/memory)
target_link_libraries(memory d3d9)
