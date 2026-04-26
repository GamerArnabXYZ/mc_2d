cmake_minimum_required(VERSION 3.22.1)
project(mc2d C)
set(CMAKE_C_STANDARD 99)

# ── FetchContent: Download & build Raylib from source ─────────────────────────
# This is the OFFICIAL approach from raylib/projects/CMake/CMakeLists.txt
# Building from source in the same CMake context fixes __wrap_fopen/__real_fopen
# because the linker sees all symbols together — no prebuilt .a needed.
include(FetchContent)

set(RAYLIB_VERSION 6.0)
set(FETCHCONTENT_QUIET NO)

FetchContent_Declare(
    raylib
    DOWNLOAD_EXTRACT_TIMESTAMP OFF
    URL https://github.com/raysan5/raylib/archive/refs/tags/${RAYLIB_VERSION}.tar.gz
)

# Raylib build options — must be set BEFORE FetchContent_MakeAvailable
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE)
set(PLATFORM "Android" CACHE STRING "" FORCE)
set(GRAPHICS "GRAPHICS_API_OPENGL_ES2" CACHE STRING "" FORCE)

FetchContent_MakeAvailable(raylib)
# ──────────────────────────────────────────────────────────────────────────────

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

add_library(mc2d SHARED ${MC2D_SRCS})

target_include_directories(mc2d PRIVATE src)

target_compile_definitions(mc2d PRIVATE
    PLATFORM_ANDROID
    GRAPHICS_API_OPENGL_ES2
)

target_compile_options(mc2d PRIVATE
    -O2 -ffunction-sections -fdata-sections -Wall
)

# From official Raylib Makefile.Android:
# -u ANativeActivity_onCreate forces symbol inclusion
# -lraylib links the freshly built raylib (same cmake context = no wrap issues)
target_link_libraries(mc2d
    raylib
    android log EGL GLESv2 OpenSLES m
    -u ANativeActivity_onCreate
)
