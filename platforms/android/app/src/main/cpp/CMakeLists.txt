# MC Clone - CMakeLists.txt for Android NDK
# Compile native C99 game engine

cmake_minimum_required(VERSION 3.22)

project(mc-clone C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Source files
file(GLOB SOURCES
    "${CMAKE_SOURCE_DIR}/../../src/core/*.c"
    "${CMAKE_SOURCE_DIR}/../../src/*.c"
)

# Include directories
include_directories(
    "${CMAKE_SOURCE_DIR}/../../include"
    "${CMAKE_SOURCE_DIR}/../../include/core"
)

# Compile options
add_compile_options(
    -Wall
    -Wextra
    -O2
    -ffast-math
    -fno-exceptions
    -fno-rtti
)

# Android-specific settings
if(ANDROID)
    add_definitions(-D__ANDROID__)
    add_definitions(-DANDROID_PLATFORM)

    # Link against Android libraries
    target_link_libraries(${PROJECT_NAME}
        android
        log
        EGL
        GLESv2
    )
endif()

# Build shared library
add_library(${PROJECT_NAME} SHARED ${SOURCES})

# Install APK hook
install(CODE "message(STATUS 'MC Clone library built successfully')")