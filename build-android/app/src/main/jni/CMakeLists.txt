# Exact same pattern as ClassiCube Android build
cmake_minimum_required(VERSION 3.4.1)
project(mc2d C)

# Raylib .a is pre-built by CI and placed here
# No FetchContent, no external CMake — pure ClassiCube style
set(RAYLIB_LIB "${CMAKE_SOURCE_DIR}/raylib-prebuilt/${ANDROID_ABI}/libraylib.a")
set(RAYLIB_INC "${CMAKE_SOURCE_DIR}/raylib-prebuilt/include")

add_library(mc2d SHARED
    src/main.c
    src/world/blocks.c
    src/world/chunk.c
    src/world/worldgen.c
    src/player/player.c
    src/player/inventory.c
    src/renderer/textures.c
    src/renderer/camera.c
    src/ui/hud.c
    src/input/input.c
)

target_include_directories(mc2d PRIVATE
    src
    ${RAYLIB_INC}
)

target_compile_definitions(mc2d PRIVATE
    PLATFORM_ANDROID
    GRAPHICS_API_OPENGL_ES2
)

target_compile_options(mc2d PRIVATE -O2 -Wall -ffunction-sections -fdata-sections)

# ClassiCube style: just link the .a + system libs, no wrap flags
target_link_libraries(mc2d
    ${RAYLIB_LIB}
    EGL
    GLESv2
    OpenSLES
    android
    log
    m
)
