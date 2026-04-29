cmake_minimum_required(VERSION 3.18)
project(CraftSDL VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ─── Source files (absolute paths — works from any build dir) ─────────────────
set(SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/Game.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/Timer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/world/World.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/world/WorldGen.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/player/Player.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/player/Inventory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/renderer/Renderer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/renderer/TextureAtlas.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/renderer/Camera.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/input/InputManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/save/SaveManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/CraftingUI.cpp
)

# ─── Include dir (also absolute) ─────────────────────────────────────────────
set(SRC_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

# ─── Platform detection ───────────────────────────────────────────────────────
if(EMSCRIPTEN)
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/Emscripten.cmake)
elseif(ANDROID)
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/Android.cmake)
else()
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/Desktop.cmake)
endif()
