#pragma once
// chunk.h - 16x16 Dynamic Chunk Loading/Unloading System
// Memory layout: column-major for cache efficiency in 2D side-scroller

#include "blocks.h"
#include <stdint.h>
#include <stdbool.h>

#define CHUNK_W         16      // blocks wide
#define CHUNK_H         256     // blocks tall (full column height)
#define CHUNK_POOL_MAX  64      // max chunks in memory at once
#define CHUNK_LOAD_DIST 4       // chunks to load around player
#define CHUNK_UNLOAD_DIST 6     // chunks to unload beyond this

// Block is just a uint8_t (block ID) — 4KB per chunk (16*256)
// Using column-major: chunk->blocks[x][y] for vertical access patterns
typedef struct {
    int32_t  cx;                        // Chunk X coordinate (world / CHUNK_W)
    uint8_t  blocks[CHUNK_W][CHUNK_H];  // [x][y] column-major
    bool     dirty;                     // Needs mesh rebuild
    bool     generated;                 // World gen complete
    bool     active;                    // In use (pool slot occupied)
    uint32_t last_accessed;             // Frame counter for LRU eviction
} Chunk;

// Pool allocator — no malloc per chunk
typedef struct {
    Chunk    pool[CHUNK_POOL_MAX];
    int      count;                     // Active chunks
} ChunkManager;

extern ChunkManager g_chunks;

void   chunk_manager_init(void);
Chunk* chunk_get_or_create(int32_t cx);     // Returns existing or generates new
Chunk* chunk_find(int32_t cx);              // Returns NULL if not loaded
void   chunk_update_active(int32_t player_cx); // Load/unload based on player pos
void   chunk_mark_dirty(int32_t cx);

// Block access — handles chunk boundary crossing
uint8_t  world_get_block(int32_t wx, int32_t wy);
void     world_set_block(int32_t wx, int32_t wy, uint8_t id);

// Coordinate helpers
static inline int32_t world_to_chunk_x(int32_t wx) {
    return (wx < 0) ? ((wx + 1) / CHUNK_W) - 1 : (wx / CHUNK_W);
}
static inline int32_t world_to_local_x(int32_t wx) {
    int32_t lx = wx % CHUNK_W;
    return (lx < 0) ? lx + CHUNK_W : lx;
}
