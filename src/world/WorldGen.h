#pragma once
#include "Chunk.h"
#include <cstdint>

// ─── WorldGen ─────────────────────────────────────────────────────────────────
// Procedural generation: terrain heights via simple noise, biomes, ores, trees
class WorldGen {
public:
    explicit WorldGen(int seed);

    // Generate one chunk at world-chunk index cx
    void generateChunk(Chunk& chunk, int cx);

private:
    int m_seed;

    // ── Noise helpers (no external lib, fast integer noise) ──────────────────
    float noise1D(int x) const;
    float smoothNoise(float x, float scale) const;
    float octaveNoise(float x, int octs, float freq, float amp) const;

    // ── Biome detection ───────────────────────────────────────────────────────
    // Returns 0=plains, 1=desert, 2=forest, 3=snow
    int biomeAt(int worldX) const;

    // ── Feature placement ─────────────────────────────────────────────────────
    void placeTree   (Chunk& chunk, int lx, int surfY);
    void placeCactus (Chunk& chunk, int lx, int surfY);
    void placeOres   (Chunk& chunk, int lx, int surfY, int cx);
};
