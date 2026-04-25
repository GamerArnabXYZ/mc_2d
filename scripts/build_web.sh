#!/bin/bash
# MC Clone Web Build Script

set -e

echo "Building MC Clone for Web..."

# Check Emscripten
if [ -z "$EMSCRIPTEN" ]; then
    source ~/emsdk/emsdk_env.sh
fi

# Create build directory
mkdir -p build-web/deploy

# Build with Emscripten
emcc -std=c99 -O2 \
    -s USE_WEBGL2=1 \
    -s ALLOW_MEMORY_GROWTH=1 \
    -s WASM=1 \
    -s ALIASING_FUNCTION_POINTERS=1 \
    -Iinclude -Iinclude/core \
    src/core/*.c src/*.c \
    -o build-web/deploy/mc-clone.js

echo "Web build complete: build-web/deploy/"
echo "Host the deploy directory on a web server"