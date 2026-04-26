#pragma once
// player.h - Player Entity: AABB Collision, Gravity, State Machine

#include <stdint.h>
#include <stdbool.h>

// ============================================================
// Player State Machine
// ============================================================
typedef enum {
    PLAYER_STATE_IDLE   = 0,
    PLAYER_STATE_RUN    = 1,
    PLAYER_STATE_JUMP   = 2,
    PLAYER_STATE_FALL   = 3,
    PLAYER_STATE_SWIM   = 4,
    PLAYER_STATE_MINE   = 5,  // Animation state
} PlayerState;

// ============================================================
// AABB — Axis-Aligned Bounding Box
// pos = top-left corner in world units (pixels / BLOCK_SIZE)
// ============================================================
typedef struct {
    float x, y;        // World-space position (in block units)
    float w, h;        // Size in block units (e.g. 0.6, 1.8)
} AABB;

typedef struct {
    AABB         box;
    float        vel_x, vel_y;
    bool         on_ground;
    bool         in_water;

    // State machine
    PlayerState  state;
    float        state_timer;   // Time in current state (seconds)

    // Mining
    int32_t      mine_x, mine_y;   // Block being mined
    float        mine_progress;    // 0.0 to 1.0
    bool         is_mining;

    // Movement config
    float        move_speed;    // Blocks/second
    float        jump_force;    // Upward impulse
    float        gravity;       // Blocks/s²
    float        swim_speed;

    // Direction: -1 = left, 1 = right
    int          facing;

    // Health
    int          health, max_health;
} Player;

// Physics constants
#define PLAYER_W        0.6f
#define PLAYER_H        1.8f
#define GRAVITY         20.0f       // blocks/s²
#define JUMP_FORCE      9.0f
#define MOVE_SPEED      5.0f
#define SWIM_SPEED      3.0f
#define MAX_FALL_SPEED  30.0f
#define MINE_REACH      5.0f        // Max mining distance in blocks

void  player_init(Player* p, float start_x, float start_y);
void  player_update(Player* p, float dt);
void  player_jump(Player* p);
void  player_move(Player* p, float dir, float dt);  // dir: -1/0/+1
void  player_start_mine(Player* p, int32_t bx, int32_t by);
void  player_stop_mine(Player* p);
bool  player_place_block(Player* p, int32_t bx, int32_t by, uint8_t id);

// AABB collision response — returns penetration resolved position
void  aabb_resolve(AABB* box, float* vel_x, float* vel_y, bool* on_ground, bool* in_water);

// Pixel-perfect block coordinate from world pos
static inline int32_t world_to_block(float w) {
    return (int32_t)__builtin_floorf(w);
}
