# MC2D Android Build — Based on ClassiCube Android CMakeLists pattern
# cmake_minimum_required stays same as ClassiCube (3.4.1) — proven working

cmake_minimum_required(VERSION 3.4.1)

# Raylib prebuilt .a placed by CI in jni/raylib-prebuilt/
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

target_compile_options(mc2d PRIVATE
    -O2 -ffunction-sections -fdata-sections -Wall
)

# ClassiCube style: minimal system libs
target_link_libraries(mc2d
    ${RAYLIB_LIB}
    EGL
    GLESv2
    OpenSLES
    android
    log
    m
)
