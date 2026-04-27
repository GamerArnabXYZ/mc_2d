# MC2D Makefile — Desktop (Linux/macOS/Windows) + Web (WASM)
# Usage:
#   make              → native desktop build
#   make web          → WASM build (requires Emscripten)
#   make clean        → remove build artifacts

CC      = gcc
EMSDK   = emcc

TARGET  = mc2d
WEB_OUT = web/index.html

SRCS    = src/main.c \
          src/world/worldgen.c \
          src/physics/physics.c \
          src/renderer/renderer.c \
          src/input/input.c \
          src/player/player.c \
          src/ui/ui.c \
          src/utils/noise.c

CFLAGS  = -std=c99 -O2 -Wall -Wextra \
          -Isrc \
          $(shell pkg-config --cflags raylib 2>/dev/null || echo "-I/usr/local/include")

LDFLAGS = $(shell pkg-config --libs raylib 2>/dev/null || echo "-lraylib") \
          -lm -ldl -lpthread \
          $(shell pkg-config --libs gl 2>/dev/null || echo "-lGL")

# ── Desktop ───────────────────────────────────────────────────
.PHONY: all
all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $@ $(LDFLAGS)
	@echo "Built: $(TARGET)"

# ── Web / WASM ────────────────────────────────────────────────
.PHONY: web
web: $(SRCS)
	@mkdir -p web
	$(EMSDK) $(SRCS) \
	    -std=c99 -O2 \
	    -Isrc \
	    -I$(EMSDK_PATH)/upstream/emscripten/cache/sysroot/include \
	    -DPLATFORM_WEB \
	    -s WASM=1 \
	    -s ALLOW_MEMORY_GROWTH=1 \
	    -s INITIAL_MEMORY=67108864 \
	    -s MAX_MEMORY=268435456 \
	    -s USE_GLFW=3 \
	    -s FULL_ES2=1 \
	    -s ASYNCIFY \
	    -s EXPORTED_RUNTIME_METHODS='["ccall","cwrap"]' \
	    --shell-file web/shell.html \
	    -lraylib \
	    --preload-file assets \
	    -o $(WEB_OUT)
	@echo "Built WASM: $(WEB_OUT)"

# ── Clean ─────────────────────────────────────────────────────
.PHONY: clean
clean:
	rm -f $(TARGET) web/index.html web/index.js web/index.wasm web/index.data
	@echo "Cleaned."

# ── Syntax check only ─────────────────────────────────────────
.PHONY: check
check:
	$(CC) -std=c99 -fsyntax-only -Wall $(CFLAGS) $(SRCS)
	@echo "Syntax OK."
