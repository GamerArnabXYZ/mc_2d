cmake_minimum_required(VERSION 3.22.1)
project(mc2d C)
set(CMAKE_C_STANDARD 99)

set(MC2D_SRCS
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

set(RAYLIB_DIR "${CMAKE_SOURCE_DIR}/raylib-android")
set(RAYLIB_LIB "${RAYLIB_DIR}/${ANDROID_ABI}/lib/libraylib.a")

add_library(mc2d SHARED ${MC2D_SRCS})

target_include_directories(mc2d PRIVATE
    src
    ${RAYLIB_DIR}/include
)

target_compile_definitions(mc2d PRIVATE
    PLATFORM_ANDROID
    GRAPHICS_API_OPENGL_ES2
)

target_compile_options(mc2d PRIVATE
    -O2 -ffunction-sections -fdata-sections -Wall
)

# FIX: Raylib 6.0 uses __wrap_fopen internally (linker wrap mechanism)
# We must pass --wrap=fopen so linker provides __real_fopen automatically
target_link_libraries(mc2d
    ${RAYLIB_LIB}
    android log EGL GLESv2 OpenSLES m
    -Wl,--wrap=fopen
    -Wl,--wrap=fclose
    -Wl,--wrap=fread
    -Wl,--wrap=fwrite
    -Wl,--wrap=fseek
    -Wl,--wrap=ftell
    -Wl,--wrap=feof
    -Wl,--wrap=fgets
)

set_target_properties(mc2d PROPERTIES
    LINK_FLAGS "-Wl,--gc-sections"
)
