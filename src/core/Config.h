#pragma once

// ─── Window ───────────────────────────────────────────────────────────────────
#define WINDOW_TITLE       "CraftSDL"
#define WINDOW_W           480
#define WINDOW_H           854

// ─── World ────────────────────────────────────────────────────────────────────
#define CHUNK_W            16
#define CHUNK_H            128
#define WORLD_CHUNKS       24        // loaded chunks (12 left, 12 right of player)
#define BLOCK_SIZE         40        // bigger blocks = easier to tap on mobile
#define SEA_LEVEL          72        // y index from top (higher = deeper sea)
#define SURFACE_AVG        55        // average surface y from top of chunk

// ─── Physics ──────────────────────────────────────────────────────────────────
#define GRAVITY            900.0f
#define JUMP_FORCE        -360.0f
#define MOVE_SPEED         130.0f
#define MAX_FALL_SPEED     550.0f
#define PLAYER_W           26
#define PLAYER_H           48

// ─── Rendering ────────────────────────────────────────────────────────────────
#define ATLAS_BLOCK_SIZE   16
#define ATLAS_COLS         16
#define TARGET_FPS         60

// ─── Day-Night ────────────────────────────────────────────────────────────────
#define DAY_DURATION       480.0f   // 8 min cycle

// ─── Inventory ────────────────────────────────────────────────────────────────
#define HOTBAR_SLOTS       9
#define INV_ROWS           3
#define INV_COLS           9
#define INV_TOTAL_SLOTS    (INV_ROWS * INV_COLS + HOTBAR_SLOTS)
#define MAX_STACK          64

// ─── Interaction ──────────────────────────────────────────────────────────────
#define BREAK_REACH        5.5f     // in blocks
#define PLACE_REACH        5.5f

// ─── Touch Controls ───────────────────────────────────────────────────────────
#define TOUCH_JOYSTICK_R   60.0f
#define TOUCH_BTN_SIZE     58.0f

// ─── Platform ─────────────────────────────────────────────────────────────────
#ifdef __EMSCRIPTEN__
  #define PLATFORM_WEB 1
#elif defined(__ANDROID__)
  #define PLATFORM_ANDROID 1
#else
  #define PLATFORM_DESKTOP 1
#endif

// ─── Save ─────────────────────────────────────────────────────────────────────
#define SAVE_DIR           "save"
#define WORLD_SEED_DEFAULT 42069

// ─── Helpers ──────────────────────────────────────────────────────────────────
#define CLAMP(v,lo,hi) ((v)<(lo)?(lo):((v)>(hi)?(hi):(v)))
#define MIN(a,b)       ((a)<(b)?(a):(b))
#define MAX(a,b)       ((a)>(b)?(a):(b))
