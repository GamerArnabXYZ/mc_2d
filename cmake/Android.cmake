# ─── Android NDK ──────────────────────────────────────────────────────────────
message(STATUS "Building for Android NDK (ABI=${ANDROID_ABI})")

set(SDL2_SRC     "${CMAKE_SOURCE_DIR}/deps/SDL2")
set(SDL2_IMG_SRC "${CMAKE_SOURCE_DIR}/deps/SDL2_image")
set(SDL2_TTF_SRC "${CMAKE_SOURCE_DIR}/deps/SDL2_ttf")

foreach(SDL_DIR ${SDL2_SRC} ${SDL2_IMG_SRC} ${SDL2_TTF_SRC})
    if(NOT EXISTS "${SDL_DIR}")
        message(FATAL_ERROR "Not found: ${SDL_DIR}")
    endif()
endforeach()

set(SDL2TTF_VENDORED ON CACHE BOOL "" FORCE)

add_subdirectory(${SDL2_SRC}     ${CMAKE_BINARY_DIR}/sdl2)
add_subdirectory(${SDL2_IMG_SRC} ${CMAKE_BINARY_DIR}/sdl2_image)
add_subdirectory(${SDL2_TTF_SRC} ${CMAKE_BINARY_DIR}/sdl2_ttf)

add_library(CraftSDL SHARED ${SOURCES})

# SDL2 SDLActivity.java loads "libmain.so" — output must be named "main"
set_target_properties(CraftSDL PROPERTIES OUTPUT_NAME "main")

target_include_directories(CraftSDL PRIVATE
    ${SRC_INCLUDE_DIR}
    ${SDL2_SRC}/include
    ${SDL2_IMG_SRC}/include
    # SDL2_ttf: source root has SDL_ttf.h directly
    # CMake generates SDL2/SDL_ttf.h wrapper in build dir
    ${SDL2_TTF_SRC}
    ${CMAKE_BINARY_DIR}/sdl2_ttf/include
)

# SDL2_ttf target_link propagates its INTERFACE_INCLUDE_DIRECTORIES automatically
target_link_libraries(CraftSDL PRIVATE
    SDL2
    SDL2_image
    SDL2_ttf
    android
    log
    m
)

target_compile_options(CraftSDL PRIVATE
    -O2 -fvisibility=hidden -ffunction-sections -fdata-sections
)
target_link_options(CraftSDL PRIVATE -Wl,--gc-sections)
