// chunk.c - Chunk Manager with Pool Allocator & LRU Eviction
#include "chunk.h"
#include "worldgen.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

ChunkManager g_chunks = {0};
static uint32_t s_frame = 0;

void chunk_manager_init(void) {
    memset(&g_chunks, 0, sizeof(ChunkManager));
}

// Find existing chunk (O(n), n≤64 so fine)
Chunk* chunk_find(int32_t cx) {
    for (int i = 0; i < CHUNK_POOL_MAX; i++) {
        if (g_chunks.pool[i].active && g_chunks.pool[i].cx == cx)
            return &g_chunks.pool[i];
    }
    return NULL;
}

// Allocate from pool — LRU eviction if full
static Chunk* chunk_alloc(void) {
    // Find free slot first
    for (int i = 0; i < CHUNK_POOL_MAX; i++) {
        if (!g_chunks.pool[i].active) {
            memset(&g_chunks.pool[i], 0, sizeof(Chunk));
            g_chunks.pool[i].active = true;
            g_chunks.count++;
            return &g_chunks.pool[i];
        }
    }
    // Pool full — evict LRU
    int lru_idx = 0;
    uint32_t oldest = g_chunks.pool[0].last_accessed;
    for (int i = 1; i < CHUNK_POOL_MAX; i++) {
        if (g_chunks.pool[i].last_accessed < oldest) {
            oldest  = g_chunks.pool[i].last_accessed;
            lru_idx = i;
        }
    }
    printf("[Chunk] Evicting chunk %d (LRU)\n", g_chunks.pool[lru_idx].cx);
    memset(&g_chunks.pool[lru_idx], 0, sizeof(Chunk));
    g_chunks.pool[lru_idx].active = true;
    return &g_chunks.pool[lru_idx];
}

Chunk* chunk_get_or_create(int32_t cx) {
    Chunk* c = chunk_find(cx);
    if (c) {
        c->last_accessed = s_frame;
        return c;
    }
    Chunk* new_c = chunk_alloc();
    new_c->cx           = cx;
    new_c->last_accessed= s_frame;
    new_c->generated    = false;
    new_c->dirty        = true;
    worldgen_generate_chunk(new_c);
    return new_c;
}

void chunk_update_active(int32_t player_cx) {
    s_frame++;

    // Load chunks in range
    for (int32_t cx = player_cx - CHUNK_LOAD_DIST; cx <= player_cx + CHUNK_LOAD_DIST; cx++) {
        chunk_get_or_create(cx);
    }

    // Mark chunks beyond unload distance as inactive
    for (int i = 0; i < CHUNK_POOL_MAX; i++) {
        if (!g_chunks.pool[i].active) continue;
        int32_t dist = g_chunks.pool[i].cx - player_cx;
        if (dist < 0) dist = -dist;
        if (dist > CHUNK_UNLOAD_DIST) {
            printf("[Chunk] Unloading chunk %d\n", g_chunks.pool[i].cx);
            g_chunks.pool[i].active = false;
            g_chunks.count--;
        }
    }
}

void chunk_mark_dirty(int32_t cx) {
    Chunk* c = chunk_find(cx);
    if (c) c->dirty = true;
}

// World block access — handles chunk boundaries
uint8_t world_get_block(int32_t wx, int32_t wy) {
    if (wy < 0 || wy >= CHUNK_H) return BLOCK_ID_BEDROCK;
    int32_t cx = world_to_chunk_x(wx);
    int32_t lx = world_to_local_x(wx);
    Chunk* c = chunk_find(cx);
    if (!c) return BLOCK_ID_AIR;
    return c->blocks[lx][wy];
}

void world_set_block(int32_t wx, int32_t wy, uint8_t id) {
    if (wy < 0 || wy >= CHUNK_H) return;
    int32_t cx = world_to_chunk_x(wx);
    int32_t lx = world_to_local_x(wx);
    Chunk* c = chunk_get_or_create(cx);
    c->blocks[lx][wy] = id;
    c->dirty = true;

    // Mark adjacent chunks dirty for border blocks
    if (lx == 0)          chunk_mark_dirty(cx - 1);
    if (lx == CHUNK_W-1)  chunk_mark_dirty(cx + 1);
}
