# MC Clone Build Scripts

# Build script for Linux/macOS
build_desktop.sh

#!/bin/bash
set -e

echo "Building MC Clone for Desktop..."

# Create build directory
mkdir -p build

# Compile
gcc -std=c99 -Wall -Wextra -O2 \
    -Iinclude -Iinclude/core \
    src/core/*.c src/*.c \
    -o build/mc-clone \
    -lm -lraylib -lopengl32 -lgdi32 -lwinmm

echo "Build complete: build/mc-clone"