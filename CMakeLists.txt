cmake_minimum_required(VERSION 3.18)
project(CraftSDL VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ─── Source files ─────────────────────────────────────────────────────────────
set(SOURCES
    src/main.cpp
    src/core/Game.cpp
    src/core/Timer.cpp
    src/world/World.cpp
    src/world/WorldGen.cpp
    src/player/Player.cpp
    src/player/Inventory.cpp
    src/renderer/Renderer.cpp
    src/renderer/TextureAtlas.cpp
    src/renderer/Camera.cpp
    src/input/InputManager.cpp
    src/save/SaveManager.cpp
    src/ui/CraftingUI.cpp
)

# ─── Platform detection ───────────────────────────────────────────────────────
if(EMSCRIPTEN)
    include(cmake/Emscripten.cmake)
elseif(ANDROID)
    include(cmake/Android.cmake)
else()
    include(cmake/Desktop.cmake)
endif()
