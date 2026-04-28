#pragma once
#include <cstdint>

// ─── Block IDs ────────────────────────────────────────────────────────────────
enum BlockID : uint8_t {
    BLOCK_AIR = 0,
    BLOCK_GRASS,      // 1
    BLOCK_DIRT,       // 2
    BLOCK_STONE,      // 3
    BLOCK_SAND,       // 4
    BLOCK_GRAVEL,     // 5
    BLOCK_WOOD,       // 6
    BLOCK_LEAVES,     // 7
    BLOCK_WATER,      // 8
    BLOCK_COAL_ORE,   // 9
    BLOCK_IRON_ORE,   // 10
    BLOCK_GOLD_ORE,   // 11
    BLOCK_DIAMOND_ORE,// 12
    BLOCK_PLANKS,     // 13
    BLOCK_COBBLESTONE,// 14
    BLOCK_BEDROCK,    // 15
    BLOCK_GLASS,      // 16
    BLOCK_TORCH,      // 17
    BLOCK_CRAFTING,   // 18
    BLOCK_CHEST,      // 19
    BLOCK_SNOW,       // 20
    BLOCK_ICE,        // 21
    BLOCK_CACTUS,     // 22
    BLOCK_COUNT       // keep last
};

// ─── Block properties (POD, cache-friendly) ───────────────────────────────────
struct BlockDef {
    const char* name;
    uint8_t     atlasX;        // column in texture atlas
    uint8_t     atlasY;        // row in texture atlas
    uint8_t     atlasXTop;     // top-face column (for grass etc.)
    uint8_t     atlasYTop;
    bool        solid;         // collides with player
    bool        transparent;   // don't cull neighbors
    bool        liquid;
    float       hardness;      // break time in seconds
    uint8_t     dropID;        // block dropped on break (255 = self)
};

// ─── Global block table ───────────────────────────────────────────────────────
// Atlas layout (16-col atlas, each cell = one block face):
//  Row 0: grass-side, dirt, stone, sand, gravel, wood-side, leaves, water,
//         coal_ore, iron_ore, gold_ore, diamond_ore, planks, cobble, bedrock, glass
//  Row 1: grass-top, wood-top, snow, ice, cactus-side, torch, crafting-top, ...

static const BlockDef BLOCK_DEFS[BLOCK_COUNT] = {
//  name            aX aY  aXT aYT solid  transp  liquid  hard  drop
    {"Air",          0, 0,  0,  0, false, true,  false, 0.0f, 0   }, // AIR
    {"Grass",        0, 0,  0,  1, true,  false, false, 0.6f, 2   }, // GRASS -> dirt
    {"Dirt",         1, 0,  1,  0, true,  false, false, 0.5f, 255 }, // DIRT
    {"Stone",        2, 0,  2,  0, true,  false, false, 1.5f, 14  }, // STONE -> cobble
    {"Sand",         3, 0,  3,  0, true,  false, false, 0.5f, 255 }, // SAND
    {"Gravel",       4, 0,  4,  0, true,  false, false, 0.6f, 255 }, // GRAVEL
    {"Wood",         5, 0,  1,  1, true,  false, false, 2.0f, 255 }, // WOOD
    {"Leaves",       6, 0,  6,  0, true,  true,  false, 0.2f, 255 }, // LEAVES
    {"Water",        7, 0,  7,  0, false, true,  true,  0.0f, 255 }, // WATER
    {"Coal Ore",     8, 0,  8,  0, true,  false, false, 3.0f, 255 }, // COAL_ORE
    {"Iron Ore",     9, 0,  9,  0, true,  false, false, 3.0f, 255 }, // IRON_ORE
    {"Gold Ore",    10, 0, 10,  0, true,  false, false, 3.0f, 255 }, // GOLD_ORE
    {"Diamond Ore", 11, 0, 11,  0, true,  false, false, 5.0f, 255 }, // DIAMOND_ORE
    {"Planks",      12, 0, 12,  0, true,  false, false, 2.0f, 255 }, // PLANKS
    {"Cobblestone", 13, 0, 13,  0, true,  false, false, 2.0f, 255 }, // COBBLESTONE
    {"Bedrock",     14, 0, 14,  0, true,  false, false,99.0f, 255 }, // BEDROCK
    {"Glass",       15, 0, 15,  0, true,  true,  false, 0.3f, 255 }, // GLASS
    {"Torch",        5, 1,  5,  1, false, true,  false, 0.0f, 255 }, // TORCH
    {"Crafting",     6, 1,  6,  1, true,  false, false, 2.5f, 255 }, // CRAFTING
    {"Chest",        7, 1,  7,  1, true,  false, false, 2.5f, 255 }, // CHEST
    {"Snow",         2, 1,  2,  1, true,  false, false, 0.2f, 255 }, // SNOW
    {"Ice",          3, 1,  3,  1, true,  true,  false, 0.5f, 20  }, // ICE
    {"Cactus",       4, 1,  4,  1, false, true,  false, 0.4f, 255 }, // CACTUS
};

// ─── Inline helpers ───────────────────────────────────────────────────────────
inline bool blockIsSolid(uint8_t id) {
    return (id < BLOCK_COUNT) && BLOCK_DEFS[id].solid;
}
inline bool blockIsTransparent(uint8_t id) {
    return (id < BLOCK_COUNT) && BLOCK_DEFS[id].transparent;
}
inline bool blockIsAir(uint8_t id) {
    return id == BLOCK_AIR;
}
