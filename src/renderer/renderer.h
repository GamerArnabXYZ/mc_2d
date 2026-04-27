#ifndef MC2D_RENDERER_H
#define MC2D_RENDERER_H

#include "../mc2d_types.h"
#include "../world/worldgen.h"
#include "raylib.h"

/* ─── Tileset atlas — 32 cols × N rows, each tile 16px ───────── */
#define ATLAS_TILE_SIZE  16
#define ATLAS_COLS       8

typedef struct {
    Texture2D atlas;   /* block texture atlas */
    Texture2D player;  /* player spritesheet  */
    bool      loaded;
} RenderAssets;

struct RenderCtx {
    RenderAssets assets;
    Camera2D_MC  cam;
    int          screen_w;
    int          screen_h;
};

RenderCtx* renderer_create(int sw, int sh);
void       renderer_destroy(RenderCtx* r);
void       renderer_load_assets(RenderCtx* r);

/* Main draw call — call between BeginDrawing()/EndDrawing() */
void renderer_draw_world(RenderCtx* r, WorldCtx* w);
void renderer_draw_entity(RenderCtx* r, const Entity* e);
void renderer_draw_debug(RenderCtx* r, WorldCtx* w, const Entity* player_e);

/* Camera */
void camera_follow(Camera2D_MC* cam, float target_x, float target_y, float lerp_speed, float dt);
void camera_world_to_screen(const Camera2D_MC* cam, float wx, float wy, float* sx, float* sy);
void camera_screen_to_world(const Camera2D_MC* cam, float sx, float sy, float* wx, float* wy);

/* Procedural atlas generator (no file needed) */
void renderer_gen_atlas(RenderCtx* r);

#endif /* MC2D_RENDERER_H */
