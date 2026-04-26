cmake_minimum_required(VERSION 3.4.1)

# Raylib prebuilt static lib
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

target_include_directories(mc2d PRIVATE src ${RAYLIB_INC})

target_compile_definitions(mc2d PRIVATE
    PLATFORM_ANDROID
    GRAPHICS_API_OPENGL_ES2
)

target_compile_options(mc2d PRIVATE -O2 -Wall -ffunction-sections -fdata-sections)

# KEY FIX:
# --whole-archive forces ALL .o from libraylib.a to be linked (no gc-sections exclusion)
# -u ANativeActivity_onCreate forces the Android entry point symbol
# This ensures rcore.o (which defines android_fopen, TraceLog, MemAlloc etc) is included
target_link_libraries(mc2d
    -Wl,--whole-archive ${RAYLIB_LIB} -Wl,--no-whole-archive
    -u ANativeActivity_onCreate
    EGL GLESv2 OpenSLES android log m
)
