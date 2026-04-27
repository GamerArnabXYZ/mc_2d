#ifndef MC2D_WORLDGEN_H
#define MC2D_WORLDGEN_H

#include "../mc2d_types.h"

/* ─── Biome types ────────────────────────────────────────────── */
typedef enum {
    BIOME_PLAINS   = 0,
    BIOME_FOREST   = 1,
    BIOME_DESERT   = 2,
    BIOME_MOUNTAINS= 3,
    BIOME_TUNDRA   = 4,
    BIOME_OCEAN    = 5,
    BIOME_COUNT    = 6
} Biome;

/* ─── World Context ──────────────────────────────────────────── */
struct WorldCtx {
    uint32_t seed;

    /* LRU cache */
    Chunk    cache[MAX_CHUNKS_CACHED];
    uint64_t tick;               /* monotonic counter for LRU */
    int32_t  cache_count;

    /* Spawn */
    float    spawn_x, spawn_y;
};

WorldCtx* world_create(uint32_t seed);
void      world_destroy(WorldCtx* w);

/* Returns pointer into LRU cache — may evict old chunk */
Chunk*    world_get_chunk(WorldCtx* w, int32_t cx);

uint8_t   world_get_block(WorldCtx* w, int32_t wx, int32_t wy);
void      world_set_block(WorldCtx* w, int32_t wx, int32_t wy, uint8_t block);

/* Block properties table (global) */
const BlockProps* block_props(uint8_t type);

/* Helpers */
static inline int32_t world_to_chunk_x(int32_t wx) {
    return wx < 0 ? (wx + 1) / CHUNK_WIDTH - 1 : wx / CHUNK_WIDTH;
}
static inline int32_t local_x(int32_t wx) {
    int32_t lx = wx % CHUNK_WIDTH;
    return lx < 0 ? lx + CHUNK_WIDTH : lx;
}

#endif /* MC2D_WORLDGEN_H */
