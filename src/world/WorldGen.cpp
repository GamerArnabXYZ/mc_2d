#include "WorldGen.h"
#include "../core/Config.h"
#include <cmath>
#include <cstdlib>

// ─── Constructor ──────────────────────────────────────────────────────────────
WorldGen::WorldGen(int seed) : m_seed(seed) {}

// ─── Integer noise (no deps, deterministic) ───────────────────────────────────
float WorldGen::noise1D(int x) const {
    // Hash-based pseudo random, deterministic per (seed, x)
    int n = x + m_seed * 57;
    n = (n << 13) ^ n;
    int t = (n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff;
    return 1.0f - (float)t / 1073741824.0f; // -1..1
}

float WorldGen::smoothNoise(float x, float scale) const {
    float sx = x / scale;
    int   ix = (int)floorf(sx);
    float fx = sx - (float)ix;
    // Cubic interpolation
    float u  = fx * fx * (3.0f - 2.0f * fx);
    return noise1D(ix) * (1.0f - u) + noise1D(ix + 1) * u;
}

float WorldGen::octaveNoise(float x, int octs, float freq, float amp) const {
    float total = 0.0f, maxV = 0.0f, a = amp, f = freq;
    for (int i = 0; i < octs; i++) {
        total += smoothNoise(x, 1.0f / f) * a;
        maxV  += a;
        a     *= 0.5f;
        f     *= 2.0f;
    }
    return total / maxV; // normalize to ~-1..1
}

// ─── Biome ────────────────────────────────────────────────────────────────────
int WorldGen::biomeAt(int worldX) const {
    float b = octaveNoise((float)worldX, 2, 0.003f, 1.0f);
    if (b >  0.5f) return 3; // snow
    if (b >  0.1f) return 2; // forest
    if (b < -0.3f) return 1; // desert
    return 0;                // plains
}

// ─── Main generation ──────────────────────────────────────────────────────────
void WorldGen::generateChunk(Chunk& chunk, int cx) {
    chunk.chunkX = cx;

    for (int lx = 0; lx < CHUNK_W; lx++) {
        int worldX  = cx * CHUNK_W + lx;
        int biome   = biomeAt(worldX);

        // Terrain height (0=top of world, CHUNK_H-1=bottom)
        float hRaw  = octaveNoise((float)worldX, 4, 0.01f, 1.0f);
        int   surfY = CHUNK_H - SURFACE_BASE + (int)(hRaw * 18.0f);
        surfY       = CLAMP(surfY, CHUNK_H / 4, CHUNK_H - 5);

        for (int ly = 0; ly < CHUNK_H; ly++) {
            uint8_t id = BLOCK_AIR;

            if (ly == CHUNK_H - 1) {
                id = BLOCK_BEDROCK;              // bottom layer = bedrock
            } else if (ly > surfY + 4) {
                id = BLOCK_STONE;
            } else if (ly > surfY) {
                id = (biome == 1) ? BLOCK_SAND : BLOCK_DIRT;
            } else if (ly == surfY) {
                switch (biome) {
                    case 1:  id = BLOCK_SAND;  break;
                    case 3:  id = BLOCK_SNOW;  break;
                    default: id = BLOCK_GRASS; break;
                }
            } else if (ly < surfY && ly >= SEA_LEVEL) {
                id = BLOCK_WATER;               // fill below sea to surface
            }
            chunk.set(lx, ly, id);
        }

        // ── Ore veins ─────────────────────────────────────────────────────────
        placeOres(chunk, lx, surfY, cx);

        // ── Surface features ──────────────────────────────────────────────────
        // Random check: use deterministic hash of (seed, worldX)
        int randVal = (m_seed * 1234 + worldX * 5678 + 91011) & 0xFFFF;

        if (biome == 2 && (randVal % 8 == 0)) {          // forest: tree every ~8 blocks
            placeTree(chunk, lx, surfY);
        } else if (biome == 1 && (randVal % 12 == 0)) {  // desert: cactus
            placeCactus(chunk, lx, surfY);
        } else if (biome == 0 && (randVal % 14 == 0)) {  // plains: occasional tree
            placeTree(chunk, lx, surfY);
        }
    }
    chunk.dirty = true;
}

// ─── Tree ─────────────────────────────────────────────────────────────────────
void WorldGen::placeTree(Chunk& chunk, int lx, int surfY) {
    int trunkH = 4 + ((m_seed + lx) % 3); // 4-6 blocks tall
    // Trunk
    for (int i = 0; i < trunkH; i++) {
        int ly = surfY - 1 - i;
        if (ly >= 0) chunk.set(lx, ly, BLOCK_WOOD);
    }
    // Leaves (3x3 top + 5x5 middle)
    int topY = surfY - 1 - trunkH;
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            int bx = lx + dx, by = topY + dy;
            if (bx >= 0 && bx < CHUNK_W && by >= 0 && by < CHUNK_H)
                if (chunk.get(bx, by) == BLOCK_AIR)
                    chunk.set(bx, by, BLOCK_LEAVES);
        }
    }
    // Wider middle layer
    for (int dx = -2; dx <= 2; dx++) {
        int bx = lx + dx, by = topY + 1;
        if (bx >= 0 && bx < CHUNK_W && by >= 0 && by < CHUNK_H)
            if (chunk.get(bx, by) == BLOCK_AIR)
                chunk.set(bx, by, BLOCK_LEAVES);
    }
}

// ─── Cactus ───────────────────────────────────────────────────────────────────
void WorldGen::placeCactus(Chunk& chunk, int lx, int surfY) {
    int h = 1 + ((m_seed + lx * 3) % 3); // 1-3 blocks tall
    for (int i = 0; i < h; i++) {
        int ly = surfY - 1 - i;
        if (ly >= 0) chunk.set(lx, ly, BLOCK_CACTUS);
    }
}

// ─── Ores ─────────────────────────────────────────────────────────────────────
void WorldGen::placeOres(Chunk& chunk, int lx, int surfY, int cx) {
    // Each ore has a depth range and frequency
    struct OreRule { uint8_t id; int minDepth; int maxDepth; int freq; };
    static const OreRule ores[] = {
        { BLOCK_COAL_ORE,    surfY +  4, CHUNK_H - 20,  6  },
        { BLOCK_IRON_ORE,    surfY + 10, CHUNK_H - 10,  8  },
        { BLOCK_GOLD_ORE,    surfY + 20, CHUNK_H - 8,   14 },
        { BLOCK_DIAMOND_ORE, surfY + 40, CHUNK_H - 5,   22 },
    };
    for (auto& ore : ores) {
        for (int ly = ore.minDepth; ly < ore.maxDepth; ly++) {
            int h = (m_seed * 31 + cx * 100 + lx * 7 + ly * 13) & 0xFFFF;
            if ((h % ore.freq) == 0 && chunk.get(lx, ly) == BLOCK_STONE)
                chunk.set(lx, ly, ore.id);
        }
    }
}
