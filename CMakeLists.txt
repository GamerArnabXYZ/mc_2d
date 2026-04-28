cmake_minimum_required(VERSION 3.20)
project(mc_2d LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(MC2D_ENABLE_LTO "Enable Link Time Optimization" ON)
option(MC2D_BUILD_TESTS "Build tests" OFF)

if(EMSCRIPTEN)
  set(CMAKE_EXECUTABLE_SUFFIX ".html")
  add_compile_definitions(MC2D_PLATFORM_WEB=1)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -sUSE_SDL=2 -sALLOW_MEMORY_GROWTH=1 -sASSERTIONS=0 -sMIN_WEBGL_VERSION=2 -sMAX_WEBGL_VERSION=2")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -sUSE_SDL=2 -sWASM=1 -sFETCH=1 --preload-file assets@/assets")
elseif(ANDROID)
  add_compile_definitions(MC2D_PLATFORM_ANDROID=1)
else()
  add_compile_definitions(MC2D_PLATFORM_DESKTOP=1)
endif()

if(MC2D_ENABLE_LTO)
  include(CheckIPOSupported)
  check_ipo_supported(RESULT ipo_supported)
  if(ipo_supported)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
  endif()
endif()

find_package(SDL2 REQUIRED)

set(MC2D_SOURCES
  src/main.cpp
  src/core/Game.cpp
  src/core/Config.cpp
  src/core/Timer.cpp
  src/game/GameState.cpp
  src/world/Block.cpp
  src/world/Chunk.cpp
  src/world/World.cpp
  src/world/WorldGenerator.cpp
  src/player/Player.cpp
  src/player/Inventory.cpp
  src/render/Camera.cpp
  src/render/Renderer.cpp
  src/platform/Input.cpp
  src/platform/TouchControls.cpp
  src/io/SaveSystem.cpp
  src/ui/Hud.cpp
)

add_executable(mc2d ${MC2D_SOURCES})
target_include_directories(mc2d PRIVATE src)
target_link_libraries(mc2d PRIVATE SDL2::SDL2 SDL2::SDL2main)

if(ANDROID)
  set_target_properties(mc2d PROPERTIES OUTPUT_NAME "main")
endif()
