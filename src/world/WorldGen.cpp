#include "WorldGen.h"
#include "../core/Config.h"
#include <cmath>

WorldGen::WorldGen(int seed) : m_seed(seed) {}

// ─── Deterministic hash noise ─────────────────────────────────────────────────
float WorldGen::noise1D(int x) const {
    int n = x * 1619 + m_seed * 31337;
    n = (n << 13) ^ n;
    return 1.0f - ((n * (n*n*15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0f;
}

float WorldGen::smoothNoise(float x, float scale) const {
    float sx = x * scale;
    int   ix = (int)floorf(sx);
    float fx = sx - ix;
    float u  = fx * fx * (3.0f - 2.0f * fx); // smoothstep
    return noise1D(ix) * (1.0f - u) + noise1D(ix + 1) * u;
}

float WorldGen::octaveNoise(float x, int octs, float freq, float amp) const {
    float total = 0, maxV = 0, a = amp, f = freq;
    for (int i = 0; i < octs; i++) {
        total += smoothNoise(x, f) * a;
        maxV  += a; a *= 0.5f; f *= 2.0f;
    }
    return total / maxV;
}

int WorldGen::biomeAt(int wx) const {
    float b = octaveNoise((float)wx, 2, 0.002f, 1.0f);
    if (b >  0.45f) return 3; // snow
    if (b >  0.05f) return 2; // forest
    if (b < -0.35f) return 1; // desert
    return 0; // plains
}

// ─── Main generation ──────────────────────────────────────────────────────────
void WorldGen::generateChunk(Chunk& chunk, int cx) {
    chunk.chunkX = cx;

    for (int lx = 0; lx < CHUNK_W; lx++) {
        int   wx    = cx * CHUNK_W + lx;
        int   biome = biomeAt(wx);

        // Surface height: y from TOP. Small y = high mountain, large y = low valley
        float h = octaveNoise((float)wx, 5, 0.008f, 1.0f);
        // Map -1..1 → surface y range (20..90 from top)
        int surfY = (int)(SURFACE_AVG + h * 20.0f);
        surfY = CLAMP(surfY, 15, CHUNK_H - 10);

        for (int ly = 0; ly < CHUNK_H; ly++) {
            uint8_t id = BLOCK_AIR;

            if (ly >= CHUNK_H - 1) {
                id = BLOCK_BEDROCK;
            } else if (ly > surfY + 5) {
                id = BLOCK_STONE;
            } else if (ly > surfY + 1) {
                id = (biome == 1) ? BLOCK_SAND : BLOCK_DIRT;
            } else if (ly == surfY + 1) {
                // Just below surface = dirt
                id = (biome == 1) ? BLOCK_SAND : BLOCK_DIRT;
            } else if (ly == surfY) {
                // Surface block
                switch(biome) {
                    case 1:  id = BLOCK_SAND;  break;
                    case 3:  id = BLOCK_SNOW;  break;
                    default: id = BLOCK_GRASS; break;
                }
            } else {
                // Above surface: air or water
                if (ly > SEA_LEVEL && ly < surfY) {
                    id = BLOCK_WATER;
                }
            }
            chunk.set(lx, ly, id);
        }

        // Ores (only in stone layer)
        placeOres(chunk, lx, surfY, cx);

        // Surface features
        int rng = (m_seed * 1000003 + wx * 999983) & 0x7FFF;
        if (biome == 2 && (rng % 6 == 0)) {
            placeTree(chunk, lx, surfY);
        } else if (biome == 0 && (rng % 10 == 0)) {
            placeTree(chunk, lx, surfY);
        } else if (biome == 1 && (rng % 8 == 0)) {
            placeCactus(chunk, lx, surfY);
        }
    }
    chunk.dirty = true;
}

void WorldGen::placeTree(Chunk& chunk, int lx, int surfY) {
    int trunkH = 4 + ((m_seed + lx * 7) % 3);
    for (int i = 1; i <= trunkH; i++) {
        int ly = surfY - i;
        if (ly >= 0) chunk.set(lx, ly, BLOCK_WOOD);
    }
    // Leaves: 3 layers
    int top = surfY - trunkH - 1;
    for (int dy = -1; dy <= 1; dy++) {
        int radius = (dy == 0) ? 2 : 1;
        for (int dx = -radius; dx <= radius; dx++) {
            int bx = lx + dx, by = top + dy;
            if (bx >= 0 && bx < CHUNK_W && by >= 0 && by < CHUNK_H)
                if (chunk.get(bx, by) == BLOCK_AIR)
                    chunk.set(bx, by, BLOCK_LEAVES);
        }
    }
}

void WorldGen::placeCactus(Chunk& chunk, int lx, int surfY) {
    int h = 1 + ((m_seed + lx) % 3);
    for (int i = 1; i <= h; i++) {
        int ly = surfY - i;
        if (ly >= 0) chunk.set(lx, ly, BLOCK_CACTUS);
    }
}

void WorldGen::placeOres(Chunk& chunk, int lx, int surfY, int cx) {
    struct OreRule { uint8_t id; int minY; int maxY; int freq; };
    static const OreRule ores[] = {
        { BLOCK_COAL_ORE,     surfY+6,  CHUNK_H-20,  5  },
        { BLOCK_IRON_ORE,     surfY+12, CHUNK_H-12,  8  },
        { BLOCK_GOLD_ORE,     surfY+25, CHUNK_H-8,   14 },
        { BLOCK_DIAMOND_ORE,  surfY+40, CHUNK_H-4,   22 },
    };
    
    // Seed and chunk part are constant for the entire LX column
    const int base_h = m_seed * 31 + cx * 997 + lx * 13;

    for (auto& o : ores) {
        for (int ly = o.minY; ly < o.maxY && ly < CHUNK_H; ly++) {
            int h = (base_h + ly * 7) & 0x7FFF;
            if ((h % o.freq) == 0 && chunk.get(lx,ly) == BLOCK_STONE)
                chunk.set(lx, ly, o.id);
        }
    }
}
