#include "worldgen.h"
#include "../utils/noise.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* ─── Block Properties Table ─────────────────────────────────── */
static const BlockProps BLOCK_TABLE[BLOCK_COUNT] = {
/*  solid  trans  liq  light  hard  tex_top  tex_side  tex_bot */
    {0,    1,     0,   0,     0,    0,       0,        0  }, /* AIR */
    {1,    0,     0,   0,     2,    0,       1,        2  }, /* GRASS */
    {1,    0,     0,   0,     3,    2,       2,        2  }, /* DIRT */
    {1,    0,     0,   0,     8,    3,       3,        3  }, /* STONE */
    {1,    0,     0,   0,     3,    4,       4,        4  }, /* SAND */
    {1,    0,     0,   0,     4,    5,       5,        5  }, /* GRAVEL */
    {1,    0,     0,   0,     5,    6,       7,        6  }, /* WOOD */
    {1,    1,     0,   0,     1,    8,       8,        8  }, /* LEAVES */
    {1,    0,     0,   0,     9,    9,       9,        9  }, /* COAL_ORE */
    {1,    0,     0,   0,     9,    10,      10,       10 }, /* IRON_ORE */
    {1,    0,     0,   0,     10,   11,      11,       11 }, /* GOLD_ORE */
    {1,    0,     0,   0,     12,   12,      12,       12 }, /* DIAMOND_ORE */
    {0,    1,     1,   0,     0,    13,      13,       13 }, /* WATER */
    {0,    1,     1,   15,    0,    14,      14,       14 }, /* LAVA */
    {1,    0,     0,   0,     255,  15,      15,       15 }, /* BEDROCK */
    {1,    0,     0,   0,     3,    16,      16,       16 }, /* PLANKS */
    {1,    0,     0,   0,     7,    17,      17,       17 }, /* COBBLESTONE */
    {1,    1,     0,   0,     2,    18,      18,       18 }, /* GLASS */
    {1,    0,     0,   0,     4,    19,      20,       19 }, /* CRAFTING */
    {1,    0,     0,   0,     6,    21,      22,       21 }, /* FURNACE */
    {1,    0,     0,   0,     5,    23,      23,       23 }, /* CHEST */
    {0,    1,     0,   14,    1,    24,      24,       24 }, /* TORCH */
    {1,    0,     0,   0,     4,    25,      25,       25 }, /* TNT */
    {1,    0,     0,   0,     1,    26,      26,       26 }, /* SNOW */
    {1,    1,     0,   0,     2,    27,      27,       27 }, /* ICE */
    {0,    1,     0,   0,     2,    28,      28,       28 }, /* CACTUS */
};

const BlockProps* block_props(uint8_t type) {
    if (type >= BLOCK_COUNT) return &BLOCK_TABLE[0];
    return &BLOCK_TABLE[type];
}

/* ─── Biome selection (temperature + humidity from noise) ────── */
static Biome get_biome(float temperature, float humidity) {
    if (temperature < -0.3f) return BIOME_TUNDRA;
    if (temperature > 0.5f && humidity < -0.2f) return BIOME_DESERT;
    if (temperature > 0.3f && humidity > 0.3f) return BIOME_FOREST;
    if (temperature < 0.1f && humidity > 0.4f) return BIOME_OCEAN;
    if (temperature > 0.0f && humidity < 0.1f) return BIOME_MOUNTAINS;
    return BIOME_PLAINS;
}

/* ─── Tree generator ─────────────────────────────────────────── */
static void place_tree(Chunk* c, int lx, int base_y) {
    int trunk_h = 4 + ((int)(lx * 7 + base_y) % 3);
    /* Trunk */
    for (int y = base_y - trunk_h; y < base_y; y++) {
        if (y < 0 || y >= CHUNK_HEIGHT) continue;
        c->blocks[lx + y * CHUNK_WIDTH] = BLOCK_WOOD;
    }
    /* Leaf canopy */
    int cy = base_y - trunk_h;
    for (int dy = -2; dy <= 1; dy++) {
        int radius = (dy >= 0) ? 1 : 2;
        for (int dx = -radius; dx <= radius; dx++) {
            int nx = lx + dx;
            int ny = cy + dy;
            if (nx < 0 || nx >= CHUNK_WIDTH || ny < 0 || ny >= CHUNK_HEIGHT) continue;
            if (c->blocks[nx + ny * CHUNK_WIDTH] == BLOCK_AIR)
                c->blocks[nx + ny * CHUNK_WIDTH] = BLOCK_LEAVES;
        }
    }
}

/* ─── Cactus generator ───────────────────────────────────────── */
static void place_cactus(Chunk* c, int lx, int base_y) {
    int h = 2 + ((int)(lx * 3 + base_y) % 2);
    for (int y = base_y - h; y < base_y; y++) {
        if (y < 0 || y >= CHUNK_HEIGHT) continue;
        c->blocks[lx + y * CHUNK_WIDTH] = BLOCK_CACTUS;
    }
}

/* ─── Ore deposit ────────────────────────────────────────────── */
static void place_ore(Chunk* c, int lx, int ly, uint8_t ore_type, int radius) {
    for (int dy = -radius; dy <= radius; dy++) {
        for (int dx = -radius; dx <= radius; dx++) {
            if (dx*dx + dy*dy > radius*radius) continue;
            int nx = lx + dx, ny = ly + dy;
            if (nx < 0 || nx >= CHUNK_WIDTH || ny < 0 || ny >= CHUNK_HEIGHT) continue;
            if (c->blocks[nx + ny * CHUNK_WIDTH] == BLOCK_STONE)
                c->blocks[nx + ny * CHUNK_WIDTH] = ore_type;
        }
    }
}

/* ─── Core chunk generation ──────────────────────────────────── */
static void gen_chunk(WorldCtx* w, Chunk* c) {
    (void)w;
    memset(c->blocks, BLOCK_AIR, CHUNK_TOTAL);
    memset(c->light,  0,         CHUNK_TOTAL);

    int32_t cx = c->cx;

    for (int lx = 0; lx < CHUNK_WIDTH; lx++) {
        float wx = (float)(cx * CHUNK_WIDTH + lx);

        /* Biome noise */
        float temperature = octave_noise1d(wx * 0.003f,        3, 0.5f, 2.0f);
        float humidity    = octave_noise1d(wx * 0.003f + 500.f, 3, 0.5f, 2.0f);
        Biome biome       = get_biome(temperature, humidity);

        /* Height */
        float base_h   = 64.0f;
        float mountain = octave_noise1d(wx * 0.005f, 4, 0.5f, 2.0f);
        float detail   = octave_noise1d(wx * 0.02f,  2, 0.5f, 2.0f);

        float h_mod = 0.0f;
        switch (biome) {
            case BIOME_MOUNTAINS: h_mod = mountain * 60.0f + detail * 12.0f; break;
            case BIOME_PLAINS:    h_mod = detail   * 8.0f;                   break;
            case BIOME_DESERT:    h_mod = detail   * 6.0f;                   break;
            case BIOME_FOREST:    h_mod = detail   * 12.0f + mountain * 15.0f; break;
            case BIOME_TUNDRA:    h_mod = mountain * 20.0f + detail * 5.0f;  break;
            case BIOME_OCEAN:     h_mod = -20.0f + detail * 4.0f;            break;
            default:              h_mod = 0.0f;                               break;
        }

        int surface_y = (int)(base_h + h_mod);
        if (surface_y < 4)   surface_y = 4;
        if (surface_y > 250) surface_y = 250;

        /* Fill column */
        for (int y = 0; y < CHUNK_HEIGHT; y++) {
            uint8_t blk = BLOCK_AIR;

            if (y == CHUNK_HEIGHT - 1) {
                blk = BLOCK_BEDROCK;
            } else if (y > surface_y) {
                if (biome == BIOME_OCEAN && y <= 64) blk = BLOCK_WATER;
                else blk = BLOCK_AIR;
            } else if (y == surface_y) {
                switch (biome) {
                    case BIOME_DESERT:  blk = BLOCK_SAND;  break;
                    case BIOME_OCEAN:   blk = BLOCK_SAND;  break;
                    case BIOME_TUNDRA:  blk = BLOCK_SNOW;  break;
                    default:            blk = BLOCK_GRASS; break;
                }
            } else if (y > surface_y - 4) {
                blk = (biome == BIOME_DESERT || biome == BIOME_OCEAN) ? BLOCK_SAND : BLOCK_DIRT;
            } else if (y > surface_y - 8) {
                blk = (biome == BIOME_DESERT) ? BLOCK_SAND : BLOCK_DIRT;
            } else {
                blk = BLOCK_STONE;
            }

            c->blocks[lx + y * CHUNK_WIDTH] = blk;
        }

        /* Caves via 2D noise threshold */
        for (int y = surface_y - 10; y < CHUNK_HEIGHT - 2; y++) {
            if (c->blocks[lx + y * CHUNK_WIDTH] != BLOCK_STONE) continue;
            float cave = noise2d(wx * 0.04f, (float)y * 0.04f);
            if (cave > 0.35f) c->blocks[lx + y * CHUNK_WIDTH] = BLOCK_AIR;
        }

        /* Ore placement */
        for (int y = surface_y - 8; y < CHUNK_HEIGHT - 2; y++) {
            float ore_n = noise2d(wx * 0.1f + 1000.f, (float)y * 0.1f);
            if (ore_n > 0.42f) {
                uint8_t ore;
                if      (y > CHUNK_HEIGHT - 30) ore = BLOCK_DIAMOND_ORE;
                else if (y > CHUNK_HEIGHT - 50) ore = BLOCK_GOLD_ORE;
                else if (y > CHUNK_HEIGHT - 80) ore = BLOCK_IRON_ORE;
                else                             ore = BLOCK_COAL_ORE;
                place_ore(c, lx, y, ore, 1);
            }
        }

        /* Surface features */
        float feat_n = noise1d(wx * 0.3f + 200.f);
        if (biome == BIOME_FOREST && feat_n > 0.4f) {
            place_tree(c, lx, surface_y);
        } else if (biome == BIOME_DESERT && feat_n > 0.5f) {
            place_cactus(c, lx, surface_y);
        }
    }

    /* Propagate sky light downward */
    for (int lx = 0; lx < CHUNK_WIDTH; lx++) {
        uint8_t sky = 15;
        for (int y = 0; y < CHUNK_HEIGHT; y++) {
            uint8_t blk = c->blocks[lx + y * CHUNK_WIDTH];
            if (sky > 0 && !block_props(blk)->transparent && blk != BLOCK_AIR) sky = 0;
            c->light[lx + y * CHUNK_WIDTH] = sky;
        }
    }
}

/* ─── LRU cache helpers ──────────────────────────────────────── */
static int find_chunk(WorldCtx* w, int32_t cx) {
    for (int i = 0; i < w->cache_count; i++) {
        if (w->cache[i].loaded && w->cache[i].cx == cx) return i;
    }
    return -1;
}

static int evict_lru(WorldCtx* w) {
    if (w->cache_count < MAX_CHUNKS_CACHED) {
        return w->cache_count++;
    }
    int   oldest  = 0;
    uint64_t min_tick = w->cache[0].lru_tick;
    for (int i = 1; i < MAX_CHUNKS_CACHED; i++) {
        if (w->cache[i].lru_tick < min_tick) {
            min_tick = w->cache[i].lru_tick;
            oldest   = i;
        }
    }
    return oldest;
}

/* ─── Public API ─────────────────────────────────────────────── */
WorldCtx* world_create(uint32_t seed) {
    WorldCtx* w = (WorldCtx*)calloc(1, sizeof(WorldCtx));
    if (!w) return NULL;
    w->seed = seed;
    noise_init(seed);

    /* Find a solid spawn column */
    w->spawn_x = 8.0f;
    Chunk* sc = world_get_chunk(w, 0);
    for (int y = 0; y < CHUNK_HEIGHT; y++) {
        if (sc->blocks[8 + y * CHUNK_WIDTH] != BLOCK_AIR) {
            w->spawn_y = (float)((y - 2) * BLOCK_SIZE);
            break;
        }
    }
    return w;
}

void world_destroy(WorldCtx* w) {
    free(w);
}

Chunk* world_get_chunk(WorldCtx* w, int32_t cx) {
    int idx = find_chunk(w, cx);
    if (idx >= 0) {
        w->cache[idx].lru_tick = ++w->tick;
        return &w->cache[idx];
    }
    idx = evict_lru(w);
    Chunk* c   = &w->cache[idx];
    c->cx      = cx;
    c->loaded  = true;
    c->dirty   = true;
    c->lru_tick= ++w->tick;
    gen_chunk(w, c);
    return c;
}

uint8_t world_get_block(WorldCtx* w, int32_t wx, int32_t wy) {
    if (wy < 0 || wy >= CHUNK_HEIGHT) return BLOCK_AIR;
    int32_t cx = world_to_chunk_x(wx);
    int32_t lx = local_x(wx);
    Chunk*  c  = world_get_chunk(w, cx);
    return c->blocks[lx + wy * CHUNK_WIDTH];
}

void world_set_block(WorldCtx* w, int32_t wx, int32_t wy, uint8_t block) {
    if (wy < 0 || wy >= CHUNK_HEIGHT) return;
    int32_t cx = world_to_chunk_x(wx);
    int32_t lx = local_x(wx);
    Chunk*  c  = world_get_chunk(w, cx);
    c->blocks[lx + wy * CHUNK_WIDTH] = block;
    c->dirty = true;
}
