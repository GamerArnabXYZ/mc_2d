#pragma once
// worldgen.h - Perlin Noise-based World Generation

#include "chunk.h"
#include <stdint.h>

// Noise configuration for terrain layers
typedef struct {
    uint32_t seed;
    float    surface_scale;     // Horizontal frequency (bigger = smoother)
    int      surface_amplitude; // Max height variation
    int      base_height;       // Y of average surface
    int      water_level;       // Y below which water fills
    int      stone_depth;       // Blocks of dirt before stone
    float    cave_threshold;    // 0.6-0.8 for cave density
} WorldgenConfig;

extern WorldgenConfig g_worldgen;

void worldgen_init(uint32_t seed);
void worldgen_generate_chunk(Chunk* chunk);  // Fill chunk->blocks[][]

// Perlin noise — pure C, no deps
float perlin2d(float x, float y, float freq, int depth, uint32_t seed);

// Structure placement (trees, etc.)
void worldgen_place_tree(int32_t wx, int32_t wy);
