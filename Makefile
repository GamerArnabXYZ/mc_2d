# MC2D Engine - Universal Makefile
PROJECT    := mc2d
RAYLIB_VER := 5.0

SRCS := src/main.c \
        src/world/blocks.c \
        src/world/chunk.c \
        src/world/worldgen.c \
        src/player/player.c \
        src/player/inventory.c \
        src/renderer/textures.c \
        src/renderer/camera.c \
        src/ui/hud.c \
        src/input/input.c

INCLUDES := -Isrc

UNAME := $(shell uname -s)

# ---- Native ----
CC      := gcc
CFLAGS  := -std=c99 -O2 -Wall -Wextra $(INCLUDES)
LDFLAGS := -lraylib -lm -lpthread -ldl

ifeq ($(UNAME), Darwin)
    LDFLAGS += -framework OpenGL -framework Cocoa -framework IOKit
else ifeq ($(UNAME), Linux)
    LDFLAGS += -lGL -lX11
endif

native: $(SRCS)
	@echo ">>> Building native..."
	$(CC) $(CFLAGS) $(SRCS) -o $(PROJECT) $(LDFLAGS)
	@echo ">>> Done: ./$(PROJECT)"

run: native
	./$(PROJECT)

# ---- Web / WASM ----
EMCC    := emcc
WEB_OUT := web/build

web: $(SRCS)
	@mkdir -p $(WEB_OUT)
	$(EMCC) -std=c99 -O2 $(INCLUDES) \
		-DPLATFORM_WEB \
		-s USE_GLFW=3 -s ASYNCIFY \
		-s TOTAL_MEMORY=134217728 -s ALLOW_MEMORY_GROWTH=1 \
		-s WASM=1 -s MAX_WEBGL_VERSION=1 \
		--shell-file web/shell.html \
		--preload-file assets \
		-lraylib \
		$(SRCS) -o $(WEB_OUT)/index.html
	@echo ">>> WASM done: $(WEB_OUT)/index.html"

web-serve: web
	cd $(WEB_OUT) && python3 -m http.server 8080

# ---- Android ----
android:
	cd build-android && ./gradlew assembleRelease

clean:
	rm -f $(PROJECT)
	rm -rf $(WEB_OUT)

.PHONY: native run web web-serve android clean
