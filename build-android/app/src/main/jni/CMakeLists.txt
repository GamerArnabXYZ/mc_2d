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

# Raylib 6.0 Android: __wrap_fopen is defined INSIDE libraylib.a itself.
# We must NOT pass --wrap flags from outside — that causes __wrap_* undefined errors.
# Instead: link raylib FIRST, allow-multiple-definition for any overlap,
# and use --whole-archive so all symbols from raylib are pulled in.
target_link_libraries(mc2d
    -Wl,--whole-archive
    ${RAYLIB_LIB}
    -Wl,--no-whole-archive
    android log EGL GLESv2 OpenSLES m
    -Wl,--allow-multiple-definition
)

set_target_properties(mc2d PROPERTIES
    LINK_FLAGS "-Wl,--gc-sections"
)
