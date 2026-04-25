# MC Clone Game Engine - CMakeLists.txt
# Cross-platform build configuration

cmake_minimum_required(VERSION 3.16)

project(mc-clone C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Version
set(GAME_VERSION "1.0.0")
set(GAME_NAME "MC Clone")

# Compiler flags
if(MSVC)
    add_compile_options(/W4 /WX-)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Raylib configuration
option(BUILD_EXAMPLES "Build examples" OFF)

# Platform-specific settings
if(WEB)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -s USE_WEBGL2=1 -s ALLOW_MEMORY_GROWTH=1")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s WASM=1")
endif()

# Source files
file(GLOB SOURCES
    "${CMAKE_SOURCE_DIR}/src/core/*.c"
    "${CMAKE_SOURCE_DIR}/src/*.c"
)

# Header files
file(GLOB HEADERS
    "${CMAKE_SOURCE_DIR}/include/**/*.h"
)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/core
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Link Raylib
find_package(raylib QUIET)
if(raylib_FOUND)
    target_link_libraries(${PROJECT_NAME} raylib)
else()
    # Fallback: download and build raylib
    include(FetchContent)
    FetchContent_Declare(
        raylib
        GIT_REPOSITORY https://github.com/raysan5/raylib.git
        GIT_TAG 4.5.0
    )
    FetchContent_MakeAvailable(raylib)
    target_link_libraries(${PROJECT_NAME} raylib)
endif()

# Install targets
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
install(DIRECTORY assets/ DESTINATION share/${PROJECT_NAME}/assets)

# Debug symbols
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    if(MSVC)
        target_compile_options(${PROJECT_NAME} /Zi)
    else()
        target_compile_options(${PROJECT_NAME} -g)
    endif()
endif()

# Generate assembly for analysis (optional)
if(GENERATE_ASM)
    if(CLANG)
        set_target_properties(${PROJECT_NAME} PROPERTIES
            ASM_INSTRUCTIONS_FILE "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.asm")
    endif()
endif()