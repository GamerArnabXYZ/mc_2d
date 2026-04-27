#include "physics.h"
#include <math.h>

/* ─── Block solid check ──────────────────────────────────────── */
static bool is_solid(WorldCtx* w, int bx, int by) {
    uint8_t blk = world_get_block(w, bx, by);
    return block_props(blk)->solid;
}

/* Pixel coords → block coords */
static int px_to_bx(float px) {
    return (int)floorf(px / (float)BLOCK_SIZE);
}
static int px_to_by(float py) {
    return (int)floorf(py / (float)BLOCK_SIZE);
}

/* ─── X-axis sweep ───────────────────────────────────────────── */
bool physics_sweep_x(WorldCtx* w, AABB* box, float* vx, float dt) {
    float dx     = (*vx) * dt;
    float new_x  = box->x + dx;
    bool  hit    = false;

    /* Determine block columns to check */
    int by_top = px_to_by(box->y + 1.0f);
    int by_bot = px_to_by(box->y + box->h - 1.0f);

    if (dx > 0.0f) {
        int bx = px_to_bx(new_x + box->w);
        for (int by = by_top; by <= by_bot; by++) {
            if (is_solid(w, bx, by)) {
                new_x = (float)(bx * BLOCK_SIZE) - box->w - 0.001f;
                *vx   = 0.0f;
                hit   = true;
                break;
            }
        }
    } else if (dx < 0.0f) {
        int bx = px_to_bx(new_x);
        for (int by = by_top; by <= by_bot; by++) {
            if (is_solid(w, bx, by)) {
                new_x = (float)((bx + 1) * BLOCK_SIZE) + 0.001f;
                *vx   = 0.0f;
                hit   = true;
                break;
            }
        }
    }

    box->x = new_x;
    return hit;
}

/* ─── Y-axis sweep ───────────────────────────────────────────── */
bool physics_sweep_y(WorldCtx* w, AABB* box, float* vy, float dt) {
    float dy    = (*vy) * dt;
    float new_y = box->y + dy;
    bool  hit   = false;

    int bx_left  = px_to_bx(box->x + 1.0f);
    int bx_right = px_to_bx(box->x + box->w - 1.0f);

    if (dy > 0.0f) {
        int by = px_to_by(new_y + box->h);
        for (int bx = bx_left; bx <= bx_right; bx++) {
            if (is_solid(w, bx, by)) {
                new_y = (float)(by * BLOCK_SIZE) - box->h - 0.001f;
                *vy   = 0.0f;
                hit   = true;
                break;
            }
        }
    } else if (dy < 0.0f) {
        int by = px_to_by(new_y);
        for (int bx = bx_left; bx <= bx_right; bx++) {
            if (is_solid(w, bx, by)) {
                new_y = (float)((by + 1) * BLOCK_SIZE) + 0.001f;
                *vy   = 0.0f;
                hit   = true;
                break;
            }
        }
    }

    box->y = new_y;
    return hit;
}

/* ─── Full entity physics update with 4× sub-stepping ────────── */
void physics_update(Entity* e, WorldCtx* w, float dt) {
    float sub_dt = dt / (float)SUBSTEP_COUNT;

    /* Apply gravity */
    e->vel.y += GRAVITY * dt;
    if (e->vel.y > TERMINAL_VELOCITY) e->vel.y = TERMINAL_VELOCITY;

    bool landed = false;

    for (int step = 0; step < SUBSTEP_COUNT; step++) {
        physics_sweep_x(w, &e->hitbox, &e->vel.x, sub_dt);
        bool y_hit = physics_sweep_y(w, &e->hitbox, &e->vel.y, sub_dt);
        if (y_hit && e->vel.y == 0.0f) landed = true;
    }

    e->on_ground = landed;
    /* Sync position from hitbox */
    e->pos.x = e->hitbox.x;
    e->pos.y = e->hitbox.y;

    /* Friction only when grounded */
    if (e->on_ground) {
        e->vel.x *= 0.75f;
        if (fabsf(e->vel.x) < 0.5f) e->vel.x = 0.0f;
    }
}

/* ─── Utilities ──────────────────────────────────────────────── */
bool physics_overlaps_block(WorldCtx* w, float wx, float wy) {
    return is_solid(w, px_to_bx(wx), px_to_by(wy));
}

bool aabb_overlap(const AABB* a, const AABB* b) {
    return (a->x < b->x + b->w) && (a->x + a->w > b->x) &&
           (a->y < b->y + b->h) && (a->y + a->h > b->y);
}
