#pragma once

// ─── Window ───────────────────────────────────────────────────────────────────
#define WINDOW_TITLE       "CraftSDL"
#define WINDOW_W           480
#define WINDOW_H           854

// ─── World ────────────────────────────────────────────────────────────────────
#define CHUNK_W            16        // blocks wide
#define CHUNK_H            128       // blocks tall
#define WORLD_CHUNKS       32        // total horizontal chunks loaded
#define BLOCK_SIZE         32        // pixels per block (base)
#define SEA_LEVEL          64        // sea level block Y
#define SURFACE_BASE       72       // avg surface height

// ─── Physics ──────────────────────────────────────────────────────────────────
#define GRAVITY            980.0f    // px/s²
#define JUMP_FORCE        -380.0f
#define MOVE_SPEED         120.0f
#define MAX_FALL_SPEED     600.0f
#define PLAYER_W           24
#define PLAYER_H           44

// ─── Rendering ────────────────────────────────────────────────────────────────
#define ATLAS_BLOCK_SIZE   16        // each block in atlas is 16x16 px
#define ATLAS_COLS         16        // atlas grid columns
#define TARGET_FPS         60

// ─── Day-Night ────────────────────────────────────────────────────────────────
#define DAY_DURATION       600.0f    // seconds per full cycle
#define DAWN_START         0.0f
#define NOON               0.25f
#define DUSK_START         0.5f
#define NIGHT              0.75f

// ─── Inventory ────────────────────────────────────────────────────────────────
#define HOTBAR_SLOTS       9
#define INV_ROWS           3
#define INV_COLS           9
#define INV_TOTAL_SLOTS    (INV_ROWS * INV_COLS + HOTBAR_SLOTS)
#define MAX_STACK          64

// ─── Reach ────────────────────────────────────────────────────────────────────
#define BREAK_REACH        160.0f    // pixels
#define PLACE_REACH        160.0f

// ─── Touch Controls ───────────────────────────────────────────────────────────
#define TOUCH_JOYSTICK_R   55.0f     // joystick radius px
#define TOUCH_BTN_SIZE     52.0f     // action button size

// ─── Platform Macros ─────────────────────────────────────────────────────────
#ifdef __EMSCRIPTEN__
  #define PLATFORM_WEB  1
#elif defined(__ANDROID__)
  #define PLATFORM_ANDROID 1
#else
  #define PLATFORM_DESKTOP 1
#endif

// ─── Save ─────────────────────────────────────────────────────────────────────
#define SAVE_DIR           "save"
#define WORLD_SEED_DEFAULT 12345

// ─── Helpers ──────────────────────────────────────────────────────────────────
#define CLAMP(v,lo,hi) ((v)<(lo)?(lo):((v)>(hi)?(hi):(v)))
#define MIN(a,b)       ((a)<(b)?(a):(b))
#define MAX(a,b)       ((a)>(b)?(a):(b))
