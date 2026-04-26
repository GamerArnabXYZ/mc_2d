#pragma once
// textures.h - Resilient Texture Loader
// Zero-crash guarantee: Falls back to hardcoded XBM byte arrays if PNG missing

#include "raylib.h"
#include <stdint.h>
#include <stdbool.h>

#define ATLAS_TILE_SIZE   16    // Each tile = 16x16 pixels
#define ATLAS_COLS        16    // Atlas is 16 tiles wide = 256px
#define MAX_ATLAS_TILES   256

// ============================================================
// Hardcoded fallback textures as C arrays (XBM-style raw RGBA)
// Each is 16x16 RGBA = 1024 bytes
// Generated via simple patterns — no file dependency
// ============================================================

// Checkerboard pattern generator (used for missing textures)
// Colors encoded as 0xRRGGBBAA
static inline uint32_t xbm_checkerboard(int x, int y, uint32_t ca, uint32_t cb) {
    return ((x / 4 + y / 4) % 2 == 0) ? ca : cb;
}

typedef struct {
    Texture2D atlas;        // GPU texture (atlas of all tiles)
    bool      loaded;       // true = from file, false = fallback
    Image     fallback_img; // CPU-side fallback
} TextureSystem;

extern TextureSystem g_textures;

// Init: try load atlas.png, generate fallback on failure
void textures_init(const char* atlas_path);
void textures_unload(void);

// Get source rect for tile index in atlas
Rectangle tex_get_tile_rect(int tile_index);

// Draw a tile at world position (call from renderer)
void tex_draw_tile(int tile_index, float screen_x, float screen_y, float size, Color tint);

// Generate programmatic fallback atlas (pure CPU, no file I/O)
Image textures_generate_fallback_atlas(void);
