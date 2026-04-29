#include "World.h"
#include <cstdio>
#include <cstring>
#include <cmath>
#include <algorithm>

// ─── Constructor / Destructor ─────────────────────────────────────────────────
World::World(int seed)
    : m_seed(seed)
    , m_gen(seed)
    , m_chunkOffset(0)
    , m_dayFrac(0.25f)   // start at noon
{
    m_chunks.resize(WORLD_CHUNKS);
}

World::~World() {}

// ─── Chunk management ─────────────────────────────────────────────────────────
// Ensure the WORLD_CHUNKS window is centered around the player's chunk
void World::ensureChunksAround(int centerCX) {
    int newOffset = centerCX - WORLD_CHUNKS / 2;

    if (newOffset != m_chunkOffset) {
        shiftChunks(newOffset);
    }

    // Generate/Load any empty/uninitialized chunks
    for (int i = 0; i < WORLD_CHUNKS; i++) {
        Chunk& c = m_chunks[i];
        int    cx = m_chunkOffset + i;
        if (c.chunkX != cx || c.dirty) { // c.dirty or uninitialized
            if (!m_saveDir.empty() && loadChunk(c, cx, m_saveDir)) {
                // loaded from disk
            } else {
                m_gen.generateChunk(c, cx);
            }
        }
    }
}

// Shift the chunk window left or right
void World::shiftChunks(int newOffset) {
    int shift = newOffset - m_chunkOffset;

    if (abs(shift) >= WORLD_CHUNKS) {
        // Complete reload: save all current, then load/gen new
        if (!m_saveDir.empty()) {
            for (auto& c : m_chunks) saveChunk(c, m_saveDir);
        }
        m_chunkOffset = newOffset;
        for (int i = 0; i < WORLD_CHUNKS; i++) {
            int cx = m_chunkOffset + i;
            if (m_saveDir.empty() || !loadChunk(m_chunks[i], cx, m_saveDir)) {
                m_gen.generateChunk(m_chunks[i], cx);
            }
        }
        return;
    }

    if (shift > 0) {
        // Shift left: drop first `shift` chunks, generate new ones at end
        if (!m_saveDir.empty()) {
            for (int i = 0; i < shift; i++) saveChunk(m_chunks[i], m_saveDir);
        }
        for (int i = 0; i < WORLD_CHUNKS - shift; i++)
            m_chunks[i] = m_chunks[i + shift];
        m_chunkOffset = newOffset;
        for (int i = WORLD_CHUNKS - shift; i < WORLD_CHUNKS; i++) {
            int cx = m_chunkOffset + i;
            if (m_saveDir.empty() || !loadChunk(m_chunks[i], cx, m_saveDir)) {
                m_gen.generateChunk(m_chunks[i], cx);
            }
        }
    } else {
        // Shift right
        shift = -shift;
        if (!m_saveDir.empty()) {
            for (int i = WORLD_CHUNKS - 1; i >= WORLD_CHUNKS - shift; i--)
                saveChunk(m_chunks[i], m_saveDir);
        }
        for (int i = WORLD_CHUNKS - 1; i >= shift; i--)
            m_chunks[i] = m_chunks[i - shift];
        m_chunkOffset = newOffset;
        for (int i = 0; i < shift; i++) {
            int cx = m_chunkOffset + i;
            if (m_saveDir.empty() || !loadChunk(m_chunks[i], cx, m_saveDir)) {
                m_gen.generateChunk(m_chunks[i], cx);
            }
        }
    }
}

Chunk* World::getChunk(int cx) {
    int idx = cx - m_chunkOffset;
    if (idx < 0 || idx >= WORLD_CHUNKS) return nullptr;
    return &m_chunks[idx];
}

// ─── Block access ─────────────────────────────────────────────────────────────
uint8_t World::getBlock(int bx, int by) const {
    if (by < 0) return BLOCK_AIR;
    if (by >= CHUNK_H) return BLOCK_BEDROCK;

    int cx = blockToChunk(bx);
    int idx = cx - m_chunkOffset;
    if (idx < 0 || idx >= WORLD_CHUNKS) return BLOCK_AIR;

    return m_chunks[idx].get(blockLocalX(bx), by);
}

void World::setBlock(int bx, int by, uint8_t id) {
    if (by < 0 || by >= CHUNK_H) return;

    int cx  = blockToChunk(bx);
    int idx = cx - m_chunkOffset;
    if (idx < 0 || idx >= WORLD_CHUNKS) return;

    m_chunks[idx].set(blockLocalX(bx), by, id);
}

// ─── Day-Night ────────────────────────────────────────────────────────────────
void World::update(float dt) {
    m_dayFrac += dt / DAY_DURATION;
    if (m_dayFrac >= 1.0f) m_dayFrac -= 1.0f;
}

uint8_t World::getAmbientLight() const {
    // 0=midnight(40), 0.25=noon(255), smooth cosine
    float angle = m_dayFrac * 2.0f * 3.14159f;
    float light = (cosf(angle) + 1.0f) * 0.5f; // 0..1
    return (uint8_t)(40 + light * 215);
}

// ─── Save / Load ──────────────────────────────────────────────────────────────
void World::saveAll(const std::string& dir) {
    for (auto& c : m_chunks) saveChunk(c, dir);
}

void World::loadAll(const std::string& dir) {
    for (int i = 0; i < WORLD_CHUNKS; i++) {
        int cx = m_chunkOffset + i;
        if (!loadChunk(m_chunks[i], cx, dir))
            m_gen.generateChunk(m_chunks[i], cx);
    }
}

bool World::saveChunk(const Chunk& c, const std::string& dir) {
    char path[256], tmpPath[256];
    snprintf(path, sizeof(path), "%s/chunk_%d.bin", dir.c_str(), c.chunkX);
    snprintf(tmpPath, sizeof(tmpPath), "%s/chunk_%d.bin.tmp", dir.c_str(), c.chunkX);
    
    FILE* f = fopen(tmpPath, "wb");
    if (!f) return false;
    if (fwrite(c.blocks, 1, sizeof(c.blocks), f) != sizeof(c.blocks)) {
        fclose(f);
        remove(tmpPath);
        return false;
    }
    fclose(f);
    
#ifdef _WIN32
    remove(path); // Windows rename doesn't overwrite
#endif
    if (rename(tmpPath, path) != 0) {
        remove(tmpPath);
        return false;
    }
    return true;
}

bool World::loadChunk(Chunk& c, int cx, const std::string& dir) {
    char path[256];
    snprintf(path, sizeof(path), "%s/chunk_%d.bin", dir.c_str(), cx);
    FILE* f = fopen(path, "rb");
    if (!f) return false;
    fread(c.blocks, 1, sizeof(c.blocks), f);
    fclose(f);
    c.chunkX = cx;
    c.dirty  = true;
    return true;
}
