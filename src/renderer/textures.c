// textures.c - Resilient Texture System
// If atlas.png is missing/corrupt, generates procedural fallback textures in VRAM
// Zero file dependency at runtime — works on any platform

#include "textures.h"
#include <string.h>
#include <stdio.h>
#include <math.h>

TextureSystem g_textures = {0};

// ============================================================
// Procedural tile generators — each returns 16x16 RGBA image
// These replicate classic MC2D look without any asset files
// ============================================================

// Write single pixel to raw RGBA buffer
static inline void px(uint8_t* buf, int x, int y, uint8_t r, uint8_t g, uint8_t b) {
    int idx = (y * 16 + x) * 4;
    buf[idx+0] = r; buf[idx+1] = g; buf[idx+2] = b; buf[idx+3] = 255;
}

// Tile 0: AIR (transparent)
static void gen_air(uint8_t* b) {
    memset(b, 0, 16*16*4); // fully transparent
}

// Tile 1: DIRT
static void gen_dirt(uint8_t* b) {
    for (int y=0;y<16;y++) for (int x=0;x<16;x++) {
        int v = 130 + ((x*3+y*7+x^y)%20) - 10;
        px(b,x,y, v, v-40, v-60);
    }
}

// Tile 2: GRASS TOP
static void gen_grass_top(uint8_t* b) {
    for (int y=0;y<16;y++) for (int x=0;x<16;x++) {
        int v = 100 + ((x*5+y*3)%30);
        px(b,x,y, 50, v, 30);
    }
}

// Tile 3: GRASS SIDE
static void gen_grass_side(uint8_t* b) {
    for (int y=0;y<16;y++) for (int x=0;x<16;x++) {
        if (y < 4) { int v=100+((x*5)%20); px(b,x,y,50,v,30); }
        else       { int v=130+((x*3+y*7)%20)-10; px(b,x,y,v,v-40,v-60); }
    }
}

// Tile 4: STONE
static void gen_stone(uint8_t* b) {
    for (int y=0;y<16;y++) for (int x=0;x<16;x++) {
        int v = 120 + ((x*x+y*y+x*y)%30)-15;
        px(b,x,y, v,v,v);
    }
}

// Tile 5: WOOD TOP (rings)
static void gen_wood_top(uint8_t* b) {
    for (int y=0;y<16;y++) for (int x=0;x<16;x++) {
        int dx=x-8, dy=y-8;
        int ring = (int)sqrtf((float)(dx*dx+dy*dy)) % 3;
        int v = 110 + ring*20;
        px(b,x,y, v, v-30, v-60);
    }
}

// Tile 6: WOOD SIDE (vertical grain)
static void gen_wood_side(uint8_t* b) {
    for (int y=0;y<16;y++) for (int x=0;x<16;x++) {
        int v = 100 + (x%4)*10;
        px(b,x,y, v, v-30, v-55);
    }
}

// Tile 7: LEAVES
static void gen_leaves(uint8_t* b) {
    for (int y=0;y<16;y++) for (int x=0;x<16;x++) {
        int dot = ((x+y)%5==0 || (x*2+y)%7==0) ? 1 : 0;
        int v   = 60 + dot*40 + ((x*y)%15);
        px(b,x,y, 30, v, 20);
    }
}

// Tile 8: SAND
static void gen_sand(uint8_t* b) {
    for (int y=0;y<16;y++) for (int x=0;x<16;x++) {
        int v = 190 + ((x*7+y*3)%20)-10;
        px(b,x,y, v, v-10, v-60);
    }
}

// Tile 9: WATER
static void gen_water(uint8_t* b) {
    for (int y=0;y<16;y++) for (int x=0;x<16;x++) {
        int wave = (int)(sinf((x+y)*0.8f)*15);
        px(b,x,y, 30, 80+wave, 180+wave);
        b[(y*16+x)*4+3] = 180; // semi-transparent
    }
}

// Tile 10: BEDROCK
static void gen_bedrock(uint8_t* b) {
    for (int y=0;y<16;y++) for (int x=0;x<16;x++) {
        int v = 50 + ((x*x^y*y)%30);
        px(b,x,y, v,v,v);
    }
}

// Tile 11: COAL ORE
static void gen_coal_ore(uint8_t* b) {
    gen_stone(b);
    for (int y=4;y<12;y++) for (int x=4;x<12;x++) {
        if ((x+y)%3==0) px(b,x,y, 30,30,30);
    }
}

// Tile 12: IRON ORE
static void gen_iron_ore(uint8_t* b) {
    gen_stone(b);
    for (int y=4;y<12;y++) for (int x=4;x<12;x++) {
        if ((x*y)%5==0) px(b,x,y, 180,140,100);
    }
}

// Tile 13: TORCH
static void gen_torch(uint8_t* b) {
    memset(b, 0, 16*16*4);
    for (int y=6;y<16;y++) { px(b,7,y,120,80,40); px(b,8,y,120,80,40); }
    px(b,7,5, 255,200,50); px(b,8,5,255,200,50);
    px(b,7,4, 255,240,100); px(b,8,4,255,240,100);
}

// Tile 14: COBBLESTONE
static void gen_cobble(uint8_t* b) {
    for (int y=0;y<16;y++) for (int x=0;x<16;x++) {
        int grid = ((x/4)+(y/4))%2;
        int v = 100 + grid*30 + ((x^y)%15);
        px(b,x,y, v,v,v);
    }
}

// Tile 15: MISSING / ERROR (magenta checkerboard)
static void gen_missing(uint8_t* b) {
    for (int y=0;y<16;y++) for (int x=0;x<16;x++) {
        int check = ((x/4)+(y/4))%2;
        px(b,x,y, check?255:0, 0, check?0:255);
    }
}

// ============================================================
// Atlas Builder — packs all tiles into one ATLAS_COLS-wide image
// ============================================================
Image textures_generate_fallback_atlas(void) {
    typedef void (*GenFn)(uint8_t*);
    GenFn generators[] = {
        gen_air, gen_dirt, gen_grass_top, gen_grass_side,
        gen_stone, gen_wood_top, gen_wood_side, gen_leaves,
        gen_sand, gen_water, gen_bedrock, gen_coal_ore,
        gen_iron_ore, gen_torch, gen_cobble, gen_missing
    };
    int num_gen = sizeof(generators) / sizeof(generators[0]);

    int atlas_w = ATLAS_COLS * ATLAS_TILE_SIZE;
    int atlas_h = ((MAX_ATLAS_TILES + ATLAS_COLS - 1) / ATLAS_COLS) * ATLAS_TILE_SIZE;

    Image img = {0};
    img.width  = atlas_w;
    img.height = atlas_h;
    img.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
    img.mipmaps= 1;
    img.data   = MemAlloc(atlas_w * atlas_h * 4);
    memset(img.data, 0, atlas_w * atlas_h * 4);

    uint8_t tile_buf[16*16*4];

    for (int tile = 0; tile < MAX_ATLAS_TILES; tile++) {
        int tx = (tile % ATLAS_COLS) * ATLAS_TILE_SIZE;
        int ty = (tile / ATLAS_COLS) * ATLAS_TILE_SIZE;

        // Use matching generator or missing tile
        int gen_idx = (tile < num_gen) ? tile : (num_gen - 1);
        generators[gen_idx](tile_buf);

        // Copy tile into atlas
        for (int py = 0; py < ATLAS_TILE_SIZE; py++) {
            uint8_t* src = tile_buf + py * ATLAS_TILE_SIZE * 4;
            uint8_t* dst = (uint8_t*)img.data + ((ty + py) * atlas_w + tx) * 4;
            memcpy(dst, src, ATLAS_TILE_SIZE * 4);
        }
    }

    return img;
}

// ============================================================
// Public API
// ============================================================
void textures_init(const char* atlas_path) {
    // Try loading real atlas first
    if (FileExists(atlas_path)) {
        Image img = LoadImage(atlas_path);
        if (img.data != NULL) {
            g_textures.atlas  = LoadTextureFromImage(img);
            g_textures.loaded = true;
            UnloadImage(img);
            printf("[Textures] Loaded atlas from '%s'\n", atlas_path);
            return;
        }
    }

    // Fallback — generate procedural atlas
    printf("[Textures] '%s' not found, generating fallback atlas.\n", atlas_path);
    g_textures.fallback_img = textures_generate_fallback_atlas();
    g_textures.atlas        = LoadTextureFromImage(g_textures.fallback_img);
    SetTextureFilter(g_textures.atlas, TEXTURE_FILTER_POINT); // Pixel-perfect
    g_textures.loaded = false;
}

void textures_unload(void) {
    UnloadTexture(g_textures.atlas);
    if (!g_textures.loaded && g_textures.fallback_img.data) {
        UnloadImage(g_textures.fallback_img);
    }
}

Rectangle tex_get_tile_rect(int tile_index) {
    if (tile_index < 0 || tile_index >= MAX_ATLAS_TILES) tile_index = 15; // missing
    return (Rectangle){
        .x = (float)((tile_index % ATLAS_COLS) * ATLAS_TILE_SIZE),
        .y = (float)((tile_index / ATLAS_COLS) * ATLAS_TILE_SIZE),
        .width  = ATLAS_TILE_SIZE,
        .height = ATLAS_TILE_SIZE
    };
}

void tex_draw_tile(int tile_index, float sx, float sy, float size, Color tint) {
    Rectangle src = tex_get_tile_rect(tile_index);
    Rectangle dst = { sx, sy, size, size };
    DrawTexturePro(g_textures.atlas, src, dst, (Vector2){0,0}, 0.0f, tint);
}
