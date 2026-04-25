// worldgen.c - Perlin Noise World Generation
// Self-contained C99 implementation, no external noise library needed

#include "worldgen.h"
#include "chunk.h"
#include "blocks.h"
#include <math.h>
#include <string.h>
#include <stdio.h>

WorldgenConfig g_worldgen = {0};

// ============================================================
// Permutation table — seeded Perlin noise
// ============================================================
static uint8_t perm[512];
static bool    perm_inited = false;

static void perm_init(uint32_t seed) {
    // Fill 0..255
    for (int i = 0; i < 256; i++) perm[i] = (uint8_t)i;
    // Fisher-Yates with LCG seeded by user seed
    uint32_t state = seed ^ 0xDEADBEEF;
    for (int i = 255; i > 0; i--) {
        state = state * 1664525u + 1013904223u;
        int j = (int)(state >> 16) % (i + 1);
        uint8_t tmp = perm[i]; perm[i] = perm[j]; perm[j] = tmp;
    }
    for (int i = 0; i < 256; i++) perm[i + 256] = perm[i];
    perm_inited = true;
}

static float fade(float t) { return t * t * t * (t * (t * 6 - 15) + 10); }
static float lerp_f(float a, float b, float t) { return a + t * (b - a); }

static float grad(int hash, float x, float y) {
    switch (hash & 3) {
        case 0: return  x + y;
        case 1: return -x + y;
        case 2: return  x - y;
        default:return -x - y;
    }
}

// Raw Perlin [-1, +1]
static float perlin_raw(float x, float y, uint32_t seed) {
    if (!perm_inited) perm_init(seed);

    int xi = (int)floorf(x) & 255;
    int yi = (int)floorf(y) & 255;
    float xf = x - floorf(x);
    float yf = y - floorf(y);

    float u = fade(xf), v = fade(yf);

    int aa = perm[perm[xi]     + yi];
    int ab = perm[perm[xi]     + yi + 1];
    int ba = perm[perm[xi + 1] + yi];
    int bb = perm[perm[xi + 1] + yi + 1];

    return lerp_f(
        lerp_f(grad(aa, xf,   yf  ), grad(ba, xf-1, yf  ), u),
        lerp_f(grad(ab, xf,   yf-1), grad(bb, xf-1, yf-1), u),
        v
    );
}

// Octave Perlin — [0, 1] range
float perlin2d(float x, float y, float freq, int depth, uint32_t seed) {
    float val = 0.0f, amp = 1.0f, max_amp = 0.0f;
    for (int i = 0; i < depth; i++) {
        val     += perlin_raw(x * freq, y * freq, seed) * amp;
        max_amp += amp;
        amp     *= 0.5f;
        freq    *= 2.0f;
    }
    return (val / max_amp + 1.0f) * 0.5f; // normalize to [0,1]
}

// ============================================================
// World Generation
// ============================================================

void worldgen_init(uint32_t seed) {
    g_worldgen.seed             = seed;
    g_worldgen.surface_scale    = 0.03f;
    g_worldgen.surface_amplitude= 30;
    g_worldgen.base_height      = 80;
    g_worldgen.water_level      = 72;
    g_worldgen.stone_depth      = 5;
    g_worldgen.cave_threshold   = 0.72f;
    perm_init(seed);
    perm_inited = true;
}

void worldgen_place_tree(int32_t wx, int32_t wy) {
    // Trunk (4-6 tall)
    int height = 4 + (int)(perlin2d((float)wx * 0.7f, 0.5f, 1.0f, 1, g_worldgen.seed + 77) * 3);
    for (int i = 0; i < height; i++) world_set_block(wx, wy + i, BLOCK_ID_WOOD);
    // Leaves canopy
    int top = wy + height;
    for (int lx = -2; lx <= 2; lx++) {
        for (int ly = -2; ly <= 2; ly++) {
            if (abs(lx) == 2 && abs(ly) == 2) continue; // Round corners
            world_set_block(wx + lx, top + ly, BLOCK_ID_LEAVES);
        }
    }
}

void worldgen_generate_chunk(Chunk* chunk) {
    if (chunk->generated) return;

    uint32_t seed = g_worldgen.seed;
    int32_t  cx   = chunk->cx;

    // Generate per column
    for (int lx = 0; lx < CHUNK_W; lx++) {
        int32_t wx = cx * CHUNK_W + lx;

        // Surface height — octave perlin for smooth terrain
        float nv = perlin2d((float)wx * g_worldgen.surface_scale, 0.0f, 1.0f, 4, seed);
        int surface_y = g_worldgen.base_height + (int)(nv * g_worldgen.surface_amplitude) - g_worldgen.surface_amplitude / 2;
        surface_y = (surface_y < 1) ? 1 : surface_y;

        for (int y = 0; y < CHUNK_H; y++) {
            uint8_t block = BLOCK_ID_AIR;

            if (y == 0) {
                block = BLOCK_ID_BEDROCK;
            } else if (y < surface_y - g_worldgen.stone_depth) {
                // Check caves
                float cave = perlin2d((float)wx * 0.05f, (float)y * 0.05f, 1.0f, 3, seed + 1);
                if (cave > g_worldgen.cave_threshold) {
                    block = BLOCK_ID_AIR;
                } else {
                    // Ore veins
                    float ore = perlin2d((float)wx * 0.15f, (float)y * 0.15f, 1.0f, 2, seed + 99);
                    if (ore > 0.82f && y < 30)      block = BLOCK_ID_IRON_ORE;
                    else if (ore > 0.78f && y < 50) block = BLOCK_ID_COAL_ORE;
                    else                             block = BLOCK_ID_STONE;
                }
            } else if (y < surface_y - 1) {
                block = BLOCK_ID_DIRT;
            } else if (y == surface_y - 1) {
                block = (surface_y - 1 <= g_worldgen.water_level) ? BLOCK_ID_SAND : BLOCK_ID_GRASS;
            } else if (y < g_worldgen.water_level && y >= surface_y) {
                block = BLOCK_ID_WATER;
            }

            chunk->blocks[lx][y] = block;
        }

        // Tree spawn — ~10% chance on grass
        if (chunk->blocks[lx][surface_y - 1] == BLOCK_ID_GRASS) {
            float tree_noise = perlin2d((float)wx * 1.3f, 42.0f, 1.0f, 1, seed + 7);
            if (tree_noise > 0.88f) {
                worldgen_place_tree(wx, surface_y);
            }
        }
    }

    chunk->generated = true;
    chunk->dirty = true;
    printf("[WorldGen] Chunk %d generated.\n", cx);
}
