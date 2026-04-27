#ifndef MC2D_PLAYER_H
#define MC2D_PLAYER_H

#include "../mc2d_types.h"
#include "../world/worldgen.h"
#include "../input/input.h"

typedef struct {
    Entity     entity;
    Inventory  inv;
    float      dig_progress;    /* 0.0–1.0 */
    int32_t    dig_bx, dig_by;  /* block being dug */
    float      reach_px;        /* max reach in pixels */
    uint8_t    selected_block;  /* for placement */
    float      respawn_timer;
    bool       creative_mode;
} PlayerCtx;

PlayerCtx* player_create(float spawn_x, float spawn_y);
void       player_destroy(PlayerCtx* p);
void       player_update(PlayerCtx* p, WorldCtx* w,
                         const InputState* inp,
                         const Camera2D_MC* cam, float dt);

/* Returns true if a block was broken */
bool player_try_dig  (PlayerCtx* p, WorldCtx* w, float target_wx, float target_wy, float dt);
bool player_try_place(PlayerCtx* p, WorldCtx* w, float target_wx, float target_wy);

void player_give_item(PlayerCtx* p, uint8_t block, int count);
void player_draw_hotbar(const PlayerCtx* p, int sw, int sh);

#endif /* MC2D_PLAYER_H */
