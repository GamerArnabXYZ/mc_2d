#pragma once
#include "Block.h"
#include "../core/Config.h"
#include <cstdint>
#include <cstring>

// ─── Chunk ────────────────────────────────────────────────────────────────────
// A column of CHUNK_W × CHUNK_H blocks. X stored as column, Y as row (0=top).
// Memory: 16*128 = 2048 bytes per chunk, very low footprint.
struct Chunk {
    uint8_t blocks[CHUNK_W][CHUNK_H]; // [x][y], y=0 is top
    bool    dirty;                    // needs re-render
    int     chunkX;                   // world chunk index

    Chunk() : dirty(true), chunkX(0) {
        memset(blocks, BLOCK_AIR, sizeof(blocks));
    }

    // ── Block access (bounds checked in debug) ────────────────────────────────
    inline uint8_t get(int lx, int ly) const {
        if (lx < 0 || lx >= CHUNK_W || ly < 0 || ly >= CHUNK_H)
            return BLOCK_BEDROCK;
        return blocks[lx][ly];
    }
    inline void set(int lx, int ly, uint8_t id) {
        if (lx < 0 || lx >= CHUNK_W || ly < 0 || ly >= CHUNK_H) return;
        blocks[lx][ly] = id;
        dirty = true;
    }

    // ── World-space pixel origin of this chunk ────────────────────────────────
    inline int worldPixelX() const { return chunkX * CHUNK_W * BLOCK_SIZE; }
};
