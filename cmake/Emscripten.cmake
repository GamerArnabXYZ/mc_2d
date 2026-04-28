# ─── Emscripten (WebAssembly) ─────────────────────────────────────────────────
message(STATUS "Building for Web (Emscripten)")

add_executable(CraftSDL ${SOURCES})

target_include_directories(CraftSDL PRIVATE src)

# Emscripten compile flags (SDL2 via ports)
target_compile_options(CraftSDL PRIVATE
    -O2
    -sUSE_SDL=2
    -sUSE_SDL_IMAGE=2
    -sUSE_SDL_TTF=2
)

# Emscripten link flags
target_link_options(CraftSDL PRIVATE
    -O2
    -sUSE_SDL=2
    -sUSE_SDL_IMAGE=2
    "-sSDL2_IMAGE_FORMATS=[\"png\"]"
    -sUSE_SDL_TTF=2
    -sWASM=1
    -sALLOW_MEMORY_GROWTH=1
    -sINITIAL_MEMORY=67108864
    -sSTACK_SIZE=1048576
    -sENVIRONMENT=web
    -sASSERTIONS=0
    "--preload-file=${CMAKE_SOURCE_DIR}/assets@/assets"
    "--shell-file=${CMAKE_SOURCE_DIR}/web/shell.html"
)

# Output .html (Emscripten generates .html + .js + .wasm)
set_target_properties(CraftSDL PROPERTIES SUFFIX ".html")
