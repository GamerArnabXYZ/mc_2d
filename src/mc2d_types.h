#ifndef MC2D_TYPES_H
#define MC2D_TYPES_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* ─── Constants ─────────────────────────────────────────────── */
#define MC2D_VERSION_MAJOR   1
#define MC2D_VERSION_MINOR   0

#define CHUNK_WIDTH          16
#define CHUNK_HEIGHT         256
#define CHUNK_TOTAL          (CHUNK_WIDTH * CHUNK_HEIGHT)

#define BLOCK_SIZE           16      /* pixels per block at zoom=1 */
#define WORLD_SEED_DEFAULT   42069

#define MAX_CHUNKS_CACHED    64      /* LRU cache limit */
#define MAX_ENTITIES         256
#define INVENTORY_SLOTS      36
#define HOTBAR_SLOTS         9

#define GRAVITY              980.0f  /* pixels/sec^2 */
#define TERMINAL_VELOCITY    1200.0f
#define JUMP_VELOCITY        -480.0f
#define PLAYER_SPEED         200.0f
#define PLAYER_WIDTH         14.0f
#define PLAYER_HEIGHT        28.0f

#define SUBSTEP_COUNT        4       /* AABB tunneling prevention */

#define TARGET_FPS           60
#define FIXED_DT             (1.0f / TARGET_FPS)

/* ─── Block Types ────────────────────────────────────────────── */
typedef enum {
    BLOCK_AIR        = 0,
    BLOCK_GRASS      = 1,
    BLOCK_DIRT       = 2,
    BLOCK_STONE      = 3,
    BLOCK_SAND       = 4,
    BLOCK_GRAVEL     = 5,
    BLOCK_WOOD       = 6,
    BLOCK_LEAVES     = 7,
    BLOCK_COAL_ORE   = 8,
    BLOCK_IRON_ORE   = 9,
    BLOCK_GOLD_ORE   = 10,
    BLOCK_DIAMOND_ORE= 11,
    BLOCK_WATER      = 12,
    BLOCK_LAVA       = 13,
    BLOCK_BEDROCK    = 14,
    BLOCK_PLANKS     = 15,
    BLOCK_COBBLESTONE= 16,
    BLOCK_GLASS      = 17,
    BLOCK_CRAFTING   = 18,
    BLOCK_FURNACE    = 19,
    BLOCK_CHEST      = 20,
    BLOCK_TORCH      = 21,
    BLOCK_TNT        = 22,
    BLOCK_SNOW       = 23,
    BLOCK_ICE        = 24,
    BLOCK_CACTUS     = 25,
    BLOCK_COUNT      = 26
} BlockType;

/* ─── Block Properties ───────────────────────────────────────── */
typedef struct {
    uint8_t  solid;
    uint8_t  transparent;
    uint8_t  liquid;
    uint8_t  light_emission; /* 0–15 */
    uint8_t  hardness;       /* dig time ticks */
    uint8_t  tex_top;
    uint8_t  tex_side;
    uint8_t  tex_bottom;
} BlockProps;

/* ─── Chunk ──────────────────────────────────────────────────── */
typedef struct {
    int32_t  cx;                        /* chunk x-index */
    uint8_t  blocks[CHUNK_TOTAL];       /* [x + y*CHUNK_WIDTH] */
    uint8_t  light [CHUNK_TOTAL];       /* block light 0–15 */
    bool     dirty;                     /* needs re-mesh */
    bool     loaded;
    uint64_t lru_tick;
} Chunk;

/* ─── AABB ───────────────────────────────────────────────────── */
typedef struct {
    float x, y, w, h;
} AABB;

/* ─── Vec2 ───────────────────────────────────────────────────── */
typedef struct { float x, y; } Vec2;
typedef struct { int   x, y; } Vec2i;

/* ─── Entity ─────────────────────────────────────────────────── */
typedef enum {
    ENTITY_PLAYER = 0,
    ENTITY_ZOMBIE = 1,
    ENTITY_ITEM   = 2
} EntityType;

typedef struct {
    EntityType type;
    Vec2       pos;       /* world pixels, top-left */
    Vec2       vel;
    AABB       hitbox;
    bool       on_ground;
    bool       active;
    int32_t    health;
    int32_t    health_max;
} Entity;

/* ─── Inventory ──────────────────────────────────────────────── */
typedef struct {
    uint8_t  block;
    uint8_t  count;
} ItemStack;

typedef struct {
    ItemStack slots[INVENTORY_SLOTS];
    int32_t   selected_slot;
    bool      open;
} Inventory;

/* ─── Camera ─────────────────────────────────────────────────── */
typedef struct {
    float  x, y;   /* world-space center */
    float  zoom;
    float  screen_w, screen_h;
} Camera2D_MC;

/* ─── Input State ────────────────────────────────────────────── */
typedef struct {
    /* Keyboard / digital */
    bool  move_left;
    bool  move_right;
    bool  jump;
    bool  dig;
    bool  place;
    bool  inventory;
    bool  pause;

    /* Touch joystick (normalised -1..1) */
    float joy_x;
    float joy_y;

    /* Touch action buttons */
    bool  btn_jump;
    bool  btn_dig;
    bool  btn_place;

    /* Mouse / pointer for block selection */
    Vec2  cursor_world;
    Vec2  cursor_screen;
    bool  pointer_down;

    /* Scroll for hotbar */
    int32_t scroll_delta;
} InputState;

/* ─── Game State ─────────────────────────────────────────────── */
typedef enum {
    GAMESTATE_MENU = 0,
    GAMESTATE_PLAYING,
    GAMESTATE_PAUSED,
    GAMESTATE_INVENTORY,
    GAMESTATE_GAMEOVER
} GameStateEnum;

/* ─── Forward declarations ───────────────────────────────────── */
typedef struct WorldCtx  WorldCtx;
typedef struct RenderCtx RenderCtx;

#endif /* MC2D_TYPES_H */
