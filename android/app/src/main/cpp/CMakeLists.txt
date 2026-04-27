cmake_minimum_required(VERSION 3.22.1)
project(mc2d C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# ── Source files ───────────────────────────────────────────────
set(SOURCES
    ${CMAKE_SOURCE_DIR}/../../../../../src/main.c
    ${CMAKE_SOURCE_DIR}/../../../../../src/world/worldgen.c
    ${CMAKE_SOURCE_DIR}/../../../../../src/physics/physics.c
    ${CMAKE_SOURCE_DIR}/../../../../../src/renderer/renderer.c
    ${CMAKE_SOURCE_DIR}/../../../../../src/input/input.c
    ${CMAKE_SOURCE_DIR}/../../../../../src/player/player.c
    ${CMAKE_SOURCE_DIR}/../../../../../src/ui/ui.c
    ${CMAKE_SOURCE_DIR}/../../../../../src/utils/noise.c
)

# ── Raylib static lib path (pre-built via workflow) ───────────
set(RAYLIB_DIR ${CMAKE_SOURCE_DIR}/../../../../../libs/raylib_android/${ANDROID_ABI})
set(RAYLIB_INCLUDE ${CMAKE_SOURCE_DIR}/../../../../../libs/raylib_android/include)

add_library(mc2d SHARED ${SOURCES})

target_include_directories(mc2d PRIVATE
    ${CMAKE_SOURCE_DIR}/../../../../../src
    ${RAYLIB_INCLUDE}
)

# ── Link raylib + system libs ──────────────────────────────────
target_link_libraries(mc2d
    ${RAYLIB_DIR}/libraylib.a
    android
    log
    EGL
    GLESv2
    m
    dl
    c
)

target_compile_options(mc2d PRIVATE
    -O2
    -Wall
    -DPLATFORM_ANDROID
    -DGRAPHICS_API_OPENGL_ES2
)
