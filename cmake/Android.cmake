# ─── Android NDK ──────────────────────────────────────────────────────────────
message(STATUS "Building for Android (NDK)")

# SDL2 should be present as a submodule or fetched in android/app/jni/SDL
# For GitHub Actions we'll use the prebuilt approach via Gradle

add_library(CraftSDL SHARED ${SOURCES})

target_include_directories(CraftSDL PRIVATE
    src
    ${SDL2_SOURCE_DIR}/include   # set by Gradle CMakeOptions
)

target_link_libraries(CraftSDL PRIVATE
    SDL2
    SDL2_image
    SDL2_ttf
    android
    log
    m
)

target_compile_options(CraftSDL PRIVATE
    -O2
    -fvisibility=hidden
    -ffunction-sections
    -fdata-sections
)

target_link_options(CraftSDL PRIVATE
    -Wl,--gc-sections
)
