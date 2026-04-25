# build-android/app/src/main/jni/CMakeLists.txt
cmake_minimum_required(VERSION 3.22.1)
project(mc2d C)

set(CMAKE_C_STANDARD 99)

# Source files (copied from project root by CI)
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

# Raylib — must be prebuilt for Android
# CI places it at: ${CMAKE_SOURCE_DIR}/../../../raylib-android/${ANDROID_ABI}/
set(RAYLIB_DIR "${CMAKE_SOURCE_DIR}/../../../raylib-android")
set(RAYLIB_LIB "${RAYLIB_DIR}/${ANDROID_ABI}/lib/libraylib.a")

# Shared library — Android Activity loads this
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
    -O2
    -ffunction-sections
    -fdata-sections
    -Wall
)

# Link: raylib + android system libs
target_link_libraries(mc2d
    ${RAYLIB_LIB}
    android
    log
    EGL
    GLESv2
    OpenSLES
    m
)

# Strip unused sections for smaller APK
set_target_properties(mc2d PROPERTIES
    LINK_FLAGS "-Wl,--gc-sections -Wl,-s"
)
