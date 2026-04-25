// player.c - Player Physics, AABB Collision, State Machine
#include "player.h"
#include "../world/chunk.h"
#include "../world/blocks.h"
#include <math.h>
#include <string.h>
#include <stdio.h>

// ============================================================
// AABB Swept Collision — Pixel-perfect resolution
// ============================================================

// Test if a single point overlaps a solid block
static bool is_solid_at(float wx, float wy) {
    if (wy < 0) return true; // Below world = solid
    uint8_t id = world_get_block((int32_t)floorf(wx), (int32_t)floorf(wy));
    return block_is_solid(id);
}

// Check corners of AABB against world blocks
static bool aabb_overlaps_solid(float x, float y, float w, float h) {
    // Check 9 points: 4 corners + 4 edge midpoints + center
    float pts_x[] = {x, x+w*0.5f, x+w-0.01f, x,         x+w-0.01f};
    float pts_y[] = {y, y,         y,          y+h-0.01f, y+h-0.01f};
    for (int i = 0; i < 5; i++) {
        if (is_solid_at(pts_x[i], pts_y[i])) return true;
    }
    return false;
}

void aabb_resolve(AABB* box, float* vel_x, float* vel_y, bool* on_ground, bool* in_water) {
    *on_ground = false;
    *in_water  = false;

    float x = box->x, y = box->y;
    float w = box->w, h = box->h;

    // --- X movement ---
    float nx = x + *vel_x;
    if (!aabb_overlaps_solid(nx, y, w, h)) {
        x = nx;
    } else {
        // Binary search for exact contact point
        float lo = 0.0f, hi = fabsf(*vel_x);
        float dir = (*vel_x > 0) ? 1.0f : -1.0f;
        for (int i = 0; i < 8; i++) {
            float mid = (lo + hi) * 0.5f;
            if (!aabb_overlaps_solid(x + dir * mid, y, w, h)) lo = mid;
            else hi = mid;
        }
        x += dir * lo;
        *vel_x = 0;
    }

    // --- Y movement ---
    float ny = y + *vel_y;
    if (!aabb_overlaps_solid(x, ny, w, h)) {
        y = ny;
    } else {
        float lo = 0.0f, hi = fabsf(*vel_y);
        float dir = (*vel_y > 0) ? 1.0f : -1.0f;
        for (int i = 0; i < 8; i++) {
            float mid = (lo + hi) * 0.5f;
            if (!aabb_overlaps_solid(x, y + dir * mid, w, h)) lo = mid;
            else hi = mid;
        }
        y += dir * lo;
        if (*vel_y > 0) *on_ground = true;  // Moving down + hit = on ground
        *vel_y = 0;
    }

    // Check if standing on ground (foot probe)
    if (!*on_ground && aabb_overlaps_solid(x, y + h, w, 0.05f)) {
        *on_ground = true;
    }

    // Check water submersion (center of player)
    uint8_t center_block = world_get_block(
        (int32_t)floorf(x + w * 0.5f),
        (int32_t)floorf(y + h * 0.5f)
    );
    *in_water = block_is_liquid(center_block);

    box->x = x;
    box->y = y;
}

// ============================================================
// Player Init
// ============================================================
void player_init(Player* p, float start_x, float start_y) {
    memset(p, 0, sizeof(Player));
    p->box.x      = start_x;
    p->box.y      = start_y;
    p->box.w      = PLAYER_W;
    p->box.h      = PLAYER_H;
    p->move_speed = MOVE_SPEED;
    p->jump_force = JUMP_FORCE;
    p->gravity    = GRAVITY;
    p->swim_speed = SWIM_SPEED;
    p->facing     = 1;
    p->health     = 20;
    p->max_health = 20;
    p->state      = PLAYER_STATE_IDLE;
}

// ============================================================
// State Machine Update
// ============================================================
static void player_update_state(Player* p) {
    float spd = fabsf(p->vel_x);

    if (p->in_water) {
        p->state = PLAYER_STATE_SWIM;
    } else if (!p->on_ground) {
        p->state = (p->vel_y < 0) ? PLAYER_STATE_JUMP : PLAYER_STATE_FALL;
    } else if (p->is_mining) {
        p->state = PLAYER_STATE_MINE;
    } else if (spd > 0.1f) {
        p->state = PLAYER_STATE_RUN;
    } else {
        p->state = PLAYER_STATE_IDLE;
    }
    p->state_timer += 0.016f; // Will be replaced with real dt
}

// ============================================================
// Player Update — call each frame with dt in seconds
// ============================================================
void player_update(Player* p, float dt) {
    // Apply gravity
    if (p->in_water) {
        p->vel_y += p->gravity * 0.3f * dt;  // Reduced gravity in water
        if (p->vel_y > p->swim_speed) p->vel_y = p->swim_speed;
    } else {
        p->vel_y += p->gravity * dt;
        if (p->vel_y > MAX_FALL_SPEED) p->vel_y = MAX_FALL_SPEED;
    }

    // Friction / drag
    if (p->on_ground) {
        p->vel_x *= 0.75f; // Ground friction
    } else {
        p->vel_x *= 0.92f; // Air resistance
    }

    // AABB collision resolution
    float step_x = p->vel_x * dt;
    float step_y = p->vel_y * dt;

    // Sub-step for fast movement (prevents tunneling)
    int steps = 1;
    float sx = fabsf(step_x), sy = fabsf(step_y);
    if (sx > 0.4f || sy > 0.4f) steps = 4;

    for (int i = 0; i < steps; i++) {
        float sub_vx = p->vel_x / steps;
        float sub_vy = p->vel_y / steps;
        p->box.x += sub_vx * dt;
        p->box.y += sub_vy * dt;
        aabb_resolve(&p->box, &p->vel_x, &p->vel_y, &p->on_ground, &p->in_water);
    }

    // Mining progress
    if (p->is_mining) {
        const BlockDef* bd = block_get(world_get_block(p->mine_x, p->mine_y));
        if (bd->hardness < 0) {
            p->mine_progress = 0; // Unbreakable
        } else {
            float rate = (bd->hardness > 0) ? (1.0f / bd->hardness) : 10.0f;
            p->mine_progress += rate * dt;
            if (p->mine_progress >= 1.0f) {
                // Break block — drop item
                uint8_t drop = bd->drop_id;
                world_set_block(p->mine_x, p->mine_y, BLOCK_ID_AIR);
                p->mine_progress = 0;
                p->is_mining = false;
                printf("[Player] Broke block %d, dropped %d\n", bd->id, drop);
                // TODO: Add drop to inventory
            }
        }
    }

    // Facing direction
    if (p->vel_x > 0.01f)       p->facing =  1;
    else if (p->vel_x < -0.01f) p->facing = -1;

    player_update_state(p);
}

void player_jump(Player* p) {
    if (p->on_ground) {
        p->vel_y = -p->jump_force; // Negative Y = up
        p->on_ground = false;
    } else if (p->in_water) {
        p->vel_y = -p->swim_speed * 1.5f; // Water jump/swim up
    }
}

void player_move(Player* p, float dir, float dt) {
    float target_spd = p->in_water ? p->swim_speed : p->move_speed;
    p->vel_x += dir * target_spd * dt * 15.0f; // Acceleration
    if (p->vel_x >  target_spd) p->vel_x =  target_spd;
    if (p->vel_x < -target_spd) p->vel_x = -target_spd;
}

void player_start_mine(Player* p, int32_t bx, int32_t by) {
    // Reach check
    float px = p->box.x + p->box.w * 0.5f;
    float py = p->box.y + p->box.h * 0.5f;
    float dx = bx + 0.5f - px;
    float dy = by + 0.5f - py;
    float dist = sqrtf(dx*dx + dy*dy);
    if (dist > MINE_REACH) return;

    uint8_t id = world_get_block(bx, by);
    if (id == BLOCK_ID_AIR || id == BLOCK_ID_BEDROCK) return;

    if (p->mine_x != bx || p->mine_y != by) {
        p->mine_progress = 0; // Reset if target changed
    }
    p->mine_x   = bx;
    p->mine_y   = by;
    p->is_mining= true;
}

void player_stop_mine(Player* p) {
    p->is_mining     = false;
    p->mine_progress = 0;
}

bool player_place_block(Player* p, int32_t bx, int32_t by, uint8_t id) {
    // Reach check
    float px = p->box.x + p->box.w * 0.5f;
    float py = p->box.y + p->box.h * 0.5f;
    float dx = bx + 0.5f - px;
    float dy = by + 0.5f - py;
    if (sqrtf(dx*dx + dy*dy) > MINE_REACH) return false;

    // Can't place inside player
    AABB test = {(float)bx, (float)by, 1.0f, 1.0f};
    AABB* pp  = &p->box;
    bool overlap = (test.x < pp->x + pp->w && test.x + test.w > pp->x &&
                    test.y < pp->y + pp->h && test.y + test.h > pp->y);
    if (overlap) return false;

    if (world_get_block(bx, by) != BLOCK_ID_AIR) return false;

    world_set_block(bx, by, id);
    return true;
}
