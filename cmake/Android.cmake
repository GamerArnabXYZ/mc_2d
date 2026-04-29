# ─── Android NDK ──────────────────────────────────────────────────────────────
# CMAKE_SOURCE_DIR = repo root (where CMakeLists.txt is)
# deps/ is downloaded to repo root by CI, so CMAKE_SOURCE_DIR/deps/SDL2 is correct
message(STATUS "Building for Android NDK (ABI=${ANDROID_ABI})")

set(SDL2_SRC     "${CMAKE_SOURCE_DIR}/deps/SDL2")
set(SDL2_IMG_SRC "${CMAKE_SOURCE_DIR}/deps/SDL2_image")
set(SDL2_TTF_SRC "${CMAKE_SOURCE_DIR}/deps/SDL2_ttf")

# Validate SDL2 dirs exist before proceeding
foreach(SDL_DIR ${SDL2_SRC} ${SDL2_IMG_SRC} ${SDL2_TTF_SRC})
    if(NOT EXISTS "${SDL_DIR}")
        message(FATAL_ERROR "SDL2 directory not found: ${SDL_DIR}\n"
            "Run the CI workflow or manually place SDL2 sources in deps/")
    endif()
endforeach()

add_subdirectory(${SDL2_SRC}     ${CMAKE_BINARY_DIR}/sdl2)
add_subdirectory(${SDL2_IMG_SRC} ${CMAKE_BINARY_DIR}/sdl2_image)
add_subdirectory(${SDL2_TTF_SRC} ${CMAKE_BINARY_DIR}/sdl2_ttf)

add_library(CraftSDL SHARED ${SOURCES})

target_include_directories(CraftSDL PRIVATE
    ${SRC_INCLUDE_DIR}
    ${SDL2_SRC}/include
    ${SDL2_IMG_SRC}
    ${SDL2_TTF_SRC}
)

target_link_libraries(CraftSDL PRIVATE
    SDL2 SDL2_image SDL2_ttf android log m
)

target_compile_options(CraftSDL PRIVATE
    -O2 -fvisibility=hidden -ffunction-sections -fdata-sections
)
target_link_options(CraftSDL PRIVATE -Wl,--gc-sections)
