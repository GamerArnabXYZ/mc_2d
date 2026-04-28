# ─── Desktop (Linux / Windows) ────────────────────────────────────────────────
message(STATUS "Building for Desktop")

find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_ttf REQUIRED)

add_executable(CraftSDL ${SOURCES})

target_include_directories(CraftSDL PRIVATE
    ${SDL2_INCLUDE_DIRS}
    ${SDL2_IMAGE_INCLUDE_DIRS}
    ${SDL2_TTF_INCLUDE_DIRS}
    src
)

target_link_libraries(CraftSDL PRIVATE
    ${SDL2_LIBRARIES}
    SDL2_image
    SDL2_ttf
    m
)

# Copy assets next to binary
add_custom_command(TARGET CraftSDL POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:CraftSDL>/assets
)

# Optimization flags
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(CraftSDL PRIVATE -O2 -DNDEBUG)
endif()
