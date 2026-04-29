# ─── Desktop (Linux / Windows / macOS) ───────────────────────────────────────
message(STATUS "Building for Desktop")

find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_ttf REQUIRED)

add_executable(CraftSDL ${SOURCES})

target_include_directories(CraftSDL PRIVATE
    ${SRC_INCLUDE_DIR}
    ${SDL2_INCLUDE_DIRS}
)

target_link_libraries(CraftSDL PRIVATE
    ${SDL2_LIBRARIES}
    SDL2_image
    SDL2_ttf
    m
)

target_compile_options(CraftSDL PRIVATE -O2)

add_custom_command(TARGET CraftSDL POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:CraftSDL>/assets
    COMMENT "Copying assets..."
)
