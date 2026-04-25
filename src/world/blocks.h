#pragma once
// blocks.h - Block Registry & JSON-based Block Definitions
// C99 compatible, zero external deps for core logic

#include <stdint.h>
#include <stdbool.h>

#define MAX_BLOCK_TYPES     64
#define BLOCK_ID_AIR        0
#define BLOCK_ID_DIRT       1
#define BLOCK_ID_GRASS      2
#define BLOCK_ID_STONE      3
#define BLOCK_ID_WOOD       4
#define BLOCK_ID_LEAVES     5
#define BLOCK_ID_SAND       6
#define BLOCK_ID_WATER      7
#define BLOCK_ID_BEDROCK    8
#define BLOCK_ID_COAL_ORE   9
#define BLOCK_ID_IRON_ORE   10
#define BLOCK_ID_TORCH      11
#define BLOCK_ID_COBBLE     12

// Block solidity flags
#define BLOCK_FLAG_SOLID        (1 << 0)
#define BLOCK_FLAG_TRANSPARENT  (1 << 1)
#define BLOCK_FLAG_LIQUID       (1 << 2)
#define BLOCK_FLAG_CLIMBABLE    (1 << 3)
#define BLOCK_FLAG_PLACEABLE    (1 << 4)
#define BLOCK_FLAG_BREAKABLE    (1 << 5)

typedef struct {
    uint8_t  id;
    char     name[32];
    uint8_t  flags;
    float    hardness;      // Mining time in seconds
    uint8_t  drop_id;       // Block ID dropped on break
    uint8_t  tex_top;       // Texture atlas index (top face)
    uint8_t  tex_side;      // Side face
    uint8_t  tex_bottom;    // Bottom face
    uint8_t  light_emit;    // 0=none, 15=max
    uint8_t  friction;      // 0-255 mapped to 0.0-1.0
} BlockDef;

typedef struct {
    BlockDef defs[MAX_BLOCK_TYPES];
    int      count;
} BlockRegistry;

// Global registry - single instance pattern
extern BlockRegistry g_blocks;

void blocks_init_defaults(void);
bool blocks_load_json(const char* path);  // Loads blocks.json, falls back to defaults
const BlockDef* block_get(uint8_t id);
bool block_is_solid(uint8_t id);
bool block_is_transparent(uint8_t id);
bool block_is_liquid(uint8_t id);
