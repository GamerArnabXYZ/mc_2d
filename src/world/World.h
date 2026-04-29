#pragma once
#include "Chunk.h"
#include "WorldGen.h"
#include "../core/Config.h"
#include <vector>
#include <cmath>
#include <string>

// ─── World ────────────────────────────────────────────────────────────────────
// Manages array of chunks, block access, day-night, save/load per chunk
class World {
public:
    explicit World(int seed = WORLD_SEED_DEFAULT);
    ~World();

    // ── Chunk management ──────────────────────────────────────────────────────
    void      setSaveDir(const std::string& dir) { m_saveDir = dir; }
    void      ensureChunksAround(int centerChunkX); // load/gen chunks near player
    Chunk*    getChunk(int cx);                      // nullptr if not loaded

    // ── Block access (world block coords) ────────────────────────────────────
    uint8_t   getBlock(int bx, int by) const;
    void      setBlock(int bx, int by, uint8_t id);

    // ── Convert helpers ───────────────────────────────────────────────────────
    // pixel → block coord
    static inline int pixToBlock(float px) { return (int)floorf(px / BLOCK_SIZE); }
    // block coord → chunk index
    static inline int blockToChunk(int bx)  {
        return (bx < 0) ? (bx - CHUNK_W + 1) / CHUNK_W : bx / CHUNK_W;
    }
    static inline int blockLocalX(int bx)   {
        int lx = bx % CHUNK_W;
        return (lx < 0) ? lx + CHUNK_W : lx;
    }

    // ── Day-Night cycle ───────────────────────────────────────────────────────
    void  update(float dt);
    float getDayFraction() const { return m_dayFrac; }  // 0..1
    // Returns ambient light 0..255
    uint8_t getAmbientLight() const;

    // ── Save / Load ───────────────────────────────────────────────────────────
    void saveAll(const std::string& dir);
    void loadAll(const std::string& dir);

    int getSeed() const { return m_seed; }

private:
    int               m_seed;
    WorldGen          m_gen;
    std::vector<Chunk> m_chunks;   // indexed 0..WORLD_CHUNKS-1
    int               m_chunkOffset; // world chunk index of m_chunks[0]
    float             m_dayFrac;
    std::string       m_saveDir;

    void  shiftChunks(int newOffset);
    bool  saveChunk(const Chunk& c, const std::string& dir);
    bool  loadChunk(Chunk& c, int cx, const std::string& dir);
};
