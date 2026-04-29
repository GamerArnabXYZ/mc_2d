# ─── Emscripten (WebAssembly) ─────────────────────────────────────────────────
message(STATUS "Building for Web (Emscripten)")

# SOURCES and SRC_INCLUDE_DIR are set in root CMakeLists.txt
add_executable(CraftSDL ${SOURCES})

target_include_directories(CraftSDL PRIVATE ${SRC_INCLUDE_DIR})

# Compile flags: SDL2/image/ttf via Emscripten ports (no system install needed)
target_compile_options(CraftSDL PRIVATE
    -O2
    "SHELL:-s USE_SDL=2"
    "SHELL:-s USE_SDL_IMAGE=2"
    "SHELL:-s USE_SDL_TTF=2"
)

# Link flags
target_link_options(CraftSDL PRIVATE
    -O2
    "SHELL:-s USE_SDL=2"
    "SHELL:-s USE_SDL_IMAGE=2"
    "SHELL:-s SDL2_IMAGE_FORMATS=[\"png\"]"
    "SHELL:-s USE_SDL_TTF=2"
    "SHELL:-s WASM=1"
    "SHELL:-s ALLOW_MEMORY_GROWTH=1"
    "SHELL:-s INITIAL_MEMORY=67108864"
    "SHELL:-s STACK_SIZE=1048576"
    "SHELL:-s ENVIRONMENT=web"
    "SHELL:-s ASSERTIONS=0"
    "SHELL:--preload-file ${CMAKE_CURRENT_SOURCE_DIR}/assets@/assets"
    "SHELL:--shell-file ${CMAKE_CURRENT_SOURCE_DIR}/web/shell.html"
)

# Emscripten outputs: CraftSDL.html + CraftSDL.js + CraftSDL.wasm [+ CraftSDL.data]
set_target_properties(CraftSDL PROPERTIES SUFFIX ".html")
