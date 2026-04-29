cmake_minimum_required(VERSION 3.18)
project(CraftSDL VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# CMAKE_SOURCE_DIR = always the root dir where cmake was invoked (-S .)
# This is safe across include() calls unlike CMAKE_CURRENT_SOURCE_DIR
set(ROOT ${CMAKE_SOURCE_DIR})

# ─── All source files ─────────────────────────────────────────────────────────
set(SOURCES
    ${ROOT}/src/main.cpp
    ${ROOT}/src/core/Game.cpp
    ${ROOT}/src/core/Timer.cpp
    ${ROOT}/src/world/World.cpp
    ${ROOT}/src/world/WorldGen.cpp
    ${ROOT}/src/player/Player.cpp
    ${ROOT}/src/player/Inventory.cpp
    ${ROOT}/src/renderer/Renderer.cpp
    ${ROOT}/src/renderer/TextureAtlas.cpp
    ${ROOT}/src/renderer/Camera.cpp
    ${ROOT}/src/input/InputManager.cpp
    ${ROOT}/src/save/SaveManager.cpp
    ${ROOT}/src/ui/CraftingUI.cpp
)

set(SRC_INCLUDE_DIR ${ROOT}/src)

# ─── Platform detection ───────────────────────────────────────────────────────
if(EMSCRIPTEN)
    include(${ROOT}/cmake/Emscripten.cmake)
elseif(ANDROID)
    include(${ROOT}/cmake/Android.cmake)
else()
    include(${ROOT}/cmake/Desktop.cmake)
endif()
