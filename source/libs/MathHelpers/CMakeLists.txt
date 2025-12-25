add_library(MathHelpers
        src/MathHelpers/hsl.cpp src/MathHelpers/hsl.h)

target_include_directories(MathHelpers
        PUBLIC
        src/)

target_link_libraries(MathHelpers d3d9 d3dx9)
