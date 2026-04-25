# MC Clone Game Engine - Makefile for Desktop Builds

# Compiler settings
CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -O2 -Iinclude -Iinclude/core
LDFLAGS = -lm

# Raylib (adjust path as needed)
RAYLIB_PATH = /usr/local
RAYLIB_LIBS = -lraylib -lopengl32 -lgdi32 -lwinmm

# Platform detection
ifeq ($(OS),Windows_NT)
    PLATFORM = windows
    EXE_EXT = .exe
else
    ifeq ($(shell uname),Darwin)
        PLATFORM = macos
        EXE_EXT = .app
    else
        PLATFORM = linux
        EXE_EXT =
    endif()
endif()

# Directories
SRC_DIR = src
BUILD_DIR = build
BIN_DIR = bin

# Source files
C_SOURCES = $(wildcard $(SRC_DIR)/*.c)
C_SOURCES += $(wildcard $(SRC_DIR)/core/*.c)

# Object files
OBJECTS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(C_SOURCES))

# Target executable
TARGET = $(BIN_DIR)/mc-clone$(EXE_EXT)

# Default target
all: directories $(TARGET)

# Create directories
directories:
	@mkdir -p $(BUILD_DIR)/core
	@mkdir -p $(BIN_DIR)
	@mkdir -p $(BUILD_DIR)/assets

# Compile C files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/core/%.o: $(SRC_DIR)/core/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Link executable
$(TARGET): $(OBJECTS)
	$(CC) $^ -o $@ $(LDFLAGS) $(RAYLIB_LIBS)

# Clean build
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

# Run game
run: $(TARGET)
	./$(TARGET)

# Debug build
debug: CFLAGS += -g -DDEBUG
debug: all

# Web build (requires Emscripten)
web: CFLAGS = -std=c99 -Wall -O2 -s USE_WEBGL2=1 -s ALLOW_MEMORY_GROWTH=1
web: LDFLAGS = -s WASM=1 -s USE_WEBGL2=1
web: CC = emcc
web: TARGET = $(BIN_DIR)/mc-clone.html
web: all

# Android build (requires Android NDK)
android:
	cd platforms/android && ./gradlew assembleDebug

# Android release
android-release:
	cd platforms/android && ./gradlew assembleRelease

# Install
install: $(TARGET)
	install -Dm 755 $(TARGET) /usr/local/bin/mc-clone

# Package for distribution
package: clean all
	@echo "Creating distribution package..."
	@mkdir -p dist/mc-clone-$(GAME_VERSION)
	@cp -r $(BIN_DIR)/* dist/mc-clone-$(GAME_VERSION)/
	@cp -r assets dist/mc-clone-$(GAME_VERSION)/
	@cd dist && tar -czvf mc-clone-$(GAME_VERSION).tar.gz mc-clone-$(GAME_VERSION)/

.PHONY: all directories clean run debug web android android-release install package