#include "player.h"
#include "../physics/physics.h"
#include "../renderer/renderer.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

#define PLAYER_REACH_PX  80.0f
#define RESPAWN_DELAY    3.0f

PlayerCtx* player_create(float spawn_x, float spawn_y) {
    PlayerCtx* p = (PlayerCtx*)calloc(1, sizeof(PlayerCtx));
    p->entity.type       = ENTITY_PLAYER;
    p->entity.pos.x      = spawn_x;
    p->entity.pos.y      = spawn_y;
    p->entity.vel.x      = 0.0f;
    p->entity.vel.y      = 0.0f;
    p->entity.hitbox     = (AABB){spawn_x, spawn_y, PLAYER_WIDTH, PLAYER_HEIGHT};
    p->entity.health     = 20;
    p->entity.health_max = 20;
    p->entity.active     = true;
    p->reach_px          = PLAYER_REACH_PX;
    p->dig_bx            = -99999;
    p->dig_by            = -99999;

    /* Default hotbar blocks */
    static const uint8_t defaults[HOTBAR_SLOTS] = {
        BLOCK_GRASS, BLOCK_DIRT, BLOCK_STONE, BLOCK_SAND, BLOCK_WOOD,
        BLOCK_PLANKS, BLOCK_COBBLESTONE, BLOCK_GLASS, BLOCK_TORCH
    };
    for (int i = 0; i < HOTBAR_SLOTS; i++) {
        p->inv.slots[i].block = defaults[i];
        p->inv.slots[i].count = 64;
    }
    p->selected_block = BLOCK_GRASS;
    return p;
}

void player_destroy(PlayerCtx* p) { free(p); }

/* ─── Dig logic ──────────────────────────────────────────────── */
bool player_try_dig(PlayerCtx* p, WorldCtx* w, float target_wx, float target_wy, float dt) {
    /* Check reach */
    float cx  = p->entity.pos.x + PLAYER_WIDTH  * 0.5f;
    float cy  = p->entity.pos.y + PLAYER_HEIGHT * 0.5f;
    float dx  = target_wx - cx;
    float dy  = target_wy - cy;
    float dist= sqrtf(dx*dx + dy*dy);
    if (dist > p->reach_px) { p->dig_progress = 0.0f; return false; }

    int bx = (int)floorf(target_wx / BLOCK_SIZE);
    int by = (int)floorf(target_wy / BLOCK_SIZE);
    uint8_t blk = world_get_block(w, bx, by);
    if (blk == BLOCK_AIR || blk == BLOCK_BEDROCK) return false;

    /* Reset if switched target */
    if (bx != p->dig_bx || by != p->dig_by) {
        p->dig_progress = 0.0f;
        p->dig_bx = bx;
        p->dig_by = by;
    }

    const BlockProps* bp = block_props(blk);
    float hardness = (float)bp->hardness;
    float dig_speed = p->creative_mode ? 9999.0f : (hardness > 0 ? 1.0f / hardness * 3.0f : 9999.0f);
    p->dig_progress += dig_speed * dt;

    if (p->dig_progress >= 1.0f) {
        p->dig_progress = 0.0f;
        world_set_block(w, bx, by, BLOCK_AIR);
        player_give_item(p, blk, 1);
        return true;
    }
    return false;
}

/* ─── Place logic ────────────────────────────────────────────── */
bool player_try_place(PlayerCtx* p, WorldCtx* w, float target_wx, float target_wy) {
    int bx = (int)floorf(target_wx / BLOCK_SIZE);
    int by = (int)floorf(target_wy / BLOCK_SIZE);

    uint8_t existing = world_get_block(w, bx, by);
    if (existing != BLOCK_AIR) return false;

    /* Check player overlap */
    AABB block_box = {(float)(bx*BLOCK_SIZE), (float)(by*BLOCK_SIZE),
                      (float)BLOCK_SIZE, (float)BLOCK_SIZE};
    if (aabb_overlap(&p->entity.hitbox, &block_box)) return false;

    ItemStack* slot = &p->inv.slots[p->inv.selected_slot];
    if (slot->count <= 0) return false;

    world_set_block(w, bx, by, slot->block);
    if (!p->creative_mode) slot->count--;
    return true;
}

/* ─── Inventory helpers ──────────────────────────────────────── */
void player_give_item(PlayerCtx* p, uint8_t block, int count) {
    /* First fill existing stack */
    for (int i = 0; i < INVENTORY_SLOTS; i++) {
        if (p->inv.slots[i].block == block && p->inv.slots[i].count < 64) {
            int space = 64 - p->inv.slots[i].count;
            int add   = count < space ? count : space;
            p->inv.slots[i].count += (uint8_t)add;
            count -= add;
            if (count <= 0) return;
        }
    }
    /* Then empty slot */
    for (int i = 0; i < INVENTORY_SLOTS; i++) {
        if (p->inv.slots[i].count == 0) {
            p->inv.slots[i].block = block;
            p->inv.slots[i].count = (uint8_t)(count > 64 ? 64 : count);
            return;
        }
    }
}

/* ─── Update ─────────────────────────────────────────────────── */
void player_update(PlayerCtx* p, WorldCtx* w,
                   const InputState* inp, const Camera2D_MC* cam, float dt) {
    if (!p->entity.active) {
        p->respawn_timer -= dt;
        if (p->respawn_timer <= 0.0f) {
            p->entity.active  = true;
            p->entity.health  = p->entity.health_max;
            p->entity.vel.x   = 0.0f;
            p->entity.vel.y   = 0.0f;
        }
        return;
    }

    /* Horizontal movement */
    if (inp->move_left)  p->entity.vel.x = -PLAYER_SPEED;
    if (inp->move_right) p->entity.vel.x =  PLAYER_SPEED;
    if (!inp->move_left && !inp->move_right && fabsf(inp->joy_x) < 0.3f)
        p->entity.vel.x *= 0.8f;
    /* Joystick override */
    if (fabsf(inp->joy_x) >= 0.3f)
        p->entity.vel.x = inp->joy_x * PLAYER_SPEED;

    /* Jump */
    if (inp->jump && p->entity.on_ground)
        p->entity.vel.y = JUMP_VELOCITY;

    /* Update physics */
    physics_update(&p->entity, w, dt);

    /* Hotbar scroll */
    p->inv.selected_slot -= inp->scroll_delta;
    if (p->inv.selected_slot < 0)                p->inv.selected_slot = HOTBAR_SLOTS - 1;
    if (p->inv.selected_slot >= HOTBAR_SLOTS)    p->inv.selected_slot = 0;

    /* Update selected block */
    p->selected_block = p->inv.slots[p->inv.selected_slot].block;

    /* Dig / Place */
    float cwx, cwy;
    camera_screen_to_world(cam, inp->cursor_screen.x, inp->cursor_screen.y, &cwx, &cwy);
    if (inp->dig)   player_try_dig  (p, w, cwx, cwy, dt);
    else            p->dig_progress = 0.0f;
    static bool place_last = false;
    if (inp->place && !place_last) player_try_place(p, w, cwx, cwy);
    place_last = inp->place;

    /* Fall damage / death */
    if (p->entity.pos.y > (float)(CHUNK_HEIGHT * BLOCK_SIZE + 200)) {
        p->entity.health = 0;
    }
    if (p->entity.health <= 0) {
        p->entity.active   = false;
        p->respawn_timer   = RESPAWN_DELAY;
    }
}

/* ─── Hotbar HUD ─────────────────────────────────────────────── */
void player_draw_hotbar(const PlayerCtx* p, int sw, int sh) {
    int slot_size = 50;
    int total_w   = HOTBAR_SLOTS * slot_size + (HOTBAR_SLOTS - 1) * 4;
    int start_x   = (sw - total_w) / 2;
    int start_y   = sh - slot_size - 8;

    for (int i = 0; i < HOTBAR_SLOTS; i++) {
        int sx = start_x + i * (slot_size + 4);
        bool sel = (i == p->inv.selected_slot);
        Color bg  = sel ? (Color){255,255,255,200} : (Color){80,80,80,180};
        Color bdr = sel ? (Color){255,220,50,255}  : (Color){150,150,150,200};
        DrawRectangleRounded((Rectangle){(float)sx,(float)start_y,(float)slot_size,(float)slot_size},
                             0.2f, 6, bg);
        DrawRectangleRoundedLines((Rectangle){(float)sx,(float)start_y,(float)slot_size,(float)slot_size},
                                  0.2f, 6, 2.0f, bdr);
        uint8_t blk = p->inv.slots[i].block;
        if (blk != BLOCK_AIR) {
            /* Draw small colour swatch */
            DrawRectangle(sx+6, start_y+6, slot_size-12, slot_size-12,
                          (Color){100,100,100,180});
            char cnt[4];
            snprintf(cnt, sizeof(cnt), "%d", p->inv.slots[i].count);
            DrawText(cnt, sx + 2, start_y + slot_size - 14, 10, WHITE);
        }
    }

    /* Health bar */
    int hbx = start_x;
    int hby = start_y - 16;
    int hbw = total_w;
    DrawRectangle(hbx, hby, hbw, 10, (Color){80,20,20,200});
    float hfrac = (float)p->entity.health / (float)p->entity.health_max;
    DrawRectangle(hbx, hby, (int)(hbw * hfrac), 10, (Color){220,50,50,220});
    DrawRectangleLines(hbx, hby, hbw, 10, (Color){255,255,255,150});

    /* Dig progress bar */
    if (p->dig_progress > 0.0f) {
        int dpx = start_x;
        int dpy = start_y - 32;
        DrawRectangle(dpx, dpy, (int)(total_w * p->dig_progress), 8,
                      (Color){255,180,50,220});
        DrawRectangleLines(dpx, dpy, total_w, 8, (Color){255,255,255,150});
    }
}
