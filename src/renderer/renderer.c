#include "renderer.h"
#include <stdlib.h>
#include <math.h>
#include <stdio.h>

/* ─── Colour palette for procedural block tiles ──────────────── */
typedef struct { unsigned char r,g,b,a; } RGBA;

static const RGBA TILE_COLORS[BLOCK_COUNT][3] = {
/*                       top              side             bottom */
 [BLOCK_AIR]        = {{0,0,0,0},        {0,0,0,0},       {0,0,0,0}},
 [BLOCK_GRASS]      = {{106,176,76,255}, {134,96,67,255}, {139,109,77,255}},
 [BLOCK_DIRT]       = {{139,109,77,255}, {139,109,77,255},{139,109,77,255}},
 [BLOCK_STONE]      = {{128,128,128,255},{128,128,128,255},{128,128,128,255}},
 [BLOCK_SAND]       = {{220,210,140,255},{220,210,140,255},{220,210,140,255}},
 [BLOCK_GRAVEL]     = {{150,140,130,255},{150,140,130,255},{150,140,130,255}},
 [BLOCK_WOOD]       = {{160,120,60,255}, {140,100,50,255},{160,120,60,255}},
 [BLOCK_LEAVES]     = {{60,120,50,200},  {60,120,50,200}, {60,120,50,200}},
 [BLOCK_COAL_ORE]   = {{90,90,90,255},   {90,90,90,255},  {90,90,90,255}},
 [BLOCK_IRON_ORE]   = {{190,160,130,255},{190,160,130,255},{190,160,130,255}},
 [BLOCK_GOLD_ORE]   = {{200,180,60,255}, {200,180,60,255},{200,180,60,255}},
 [BLOCK_DIAMOND_ORE]= {{80,200,220,255}, {80,200,220,255},{80,200,220,255}},
 [BLOCK_WATER]      = {{50,100,200,180}, {50,100,200,180},{50,100,200,180}},
 [BLOCK_LAVA]       = {{220,80,20,255},  {220,80,20,255}, {220,80,20,255}},
 [BLOCK_BEDROCK]    = {{50,50,50,255},   {50,50,50,255},  {50,50,50,255}},
 [BLOCK_PLANKS]     = {{175,140,90,255}, {175,140,90,255},{175,140,90,255}},
 [BLOCK_COBBLESTONE]= {{110,110,110,255},{110,110,110,255},{110,110,110,255}},
 [BLOCK_GLASS]      = {{200,230,255,120},{200,230,255,120},{200,230,255,120}},
 [BLOCK_CRAFTING]   = {{140,100,60,255}, {140,100,60,255},{140,100,60,255}},
 [BLOCK_FURNACE]    = {{100,100,100,255},{100,100,100,255},{100,100,100,255}},
 [BLOCK_CHEST]      = {{170,130,60,255}, {170,130,60,255},{170,130,60,255}},
 [BLOCK_TORCH]      = {{255,220,100,255},{255,220,100,255},{255,220,100,255}},
 [BLOCK_TNT]        = {{200,40,40,255},  {200,40,40,255}, {200,40,40,255}},
 [BLOCK_SNOW]       = {{240,245,255,255},{200,220,240,255},{240,245,255,255}},
 [BLOCK_ICE]        = {{160,200,255,200},{160,200,255,200},{160,200,255,200}},
 [BLOCK_CACTUS]     = {{60,150,50,255},  {60,150,50,255}, {60,150,50,255}},
};

/* ─── Draw a procedural 16×16 tile into an Image pixel array ─── */
static void draw_tile(Color* px, int tile_col, int tile_row, RGBA base, RGBA accent) {
    int ox = tile_col * ATLAS_TILE_SIZE;
    int oy = tile_row * ATLAS_TILE_SIZE;
    int W  = ATLAS_COLS * ATLAS_TILE_SIZE;

    for (int y = 0; y < ATLAS_TILE_SIZE; y++) {
        for (int x = 0; x < ATLAS_TILE_SIZE; x++) {
            /* Checkerboard noise */
            bool noisy = ((x ^ y ^ tile_col ^ tile_row) & 3) == 0;
            RGBA c = noisy ? accent : base;

            /* Border darkening */
            if (x == 0 || y == 0) {
                c.r = (unsigned char)(c.r * 0.7f);
                c.g = (unsigned char)(c.g * 0.7f);
                c.b = (unsigned char)(c.b * 0.7f);
            }
            px[(oy + y) * W + (ox + x)] = (Color){c.r, c.g, c.b, c.a};
        }
    }
}

/* ─── Generate full atlas from palette ──────────────────────────*/
void renderer_gen_atlas(RenderCtx* r) {
    /* Atlas: 8 cols × ceil(BLOCK_COUNT/8) rows, each tile 16px */
    int rows  = ((BLOCK_COUNT * 3) + ATLAS_COLS - 1) / ATLAS_COLS; /* 3 faces */
    int img_w = ATLAS_COLS * ATLAS_TILE_SIZE;
    int img_h = rows       * ATLAS_TILE_SIZE;

    Image img = GenImageColor(img_w, img_h, BLANK);

    for (int b = 0; b < BLOCK_COUNT; b++) {
        for (int face = 0; face < 3; face++) {
            int idx = b * 3 + face;
            int col = idx % ATLAS_COLS;
            int row = idx / ATLAS_COLS;
            RGBA base   = TILE_COLORS[b][face];
            RGBA accent = {
                (unsigned char)((int)base.r * 85 / 100),
                (unsigned char)((int)base.g * 85 / 100),
                (unsigned char)((int)base.b * 85 / 100),
                base.a
            };
            /* Draw tile using ImageDrawPixel loop */
            int ox = col * ATLAS_TILE_SIZE;
            int oy = row * ATLAS_TILE_SIZE;
            for (int y = 0; y < ATLAS_TILE_SIZE; y++) {
                for (int x = 0; x < ATLAS_TILE_SIZE; x++) {
                    bool noisy = ((x ^ y ^ col ^ row) & 3) == 0;
                    RGBA c     = noisy ? accent : base;
                    if (x == 0 || y == 0) {
                        c.r = (unsigned char)(c.r * 70 / 100);
                        c.g = (unsigned char)(c.g * 70 / 100);
                        c.b = (unsigned char)(c.b * 70 / 100);
                    }
                    ImageDrawPixel(&img, ox + x, oy + y, (Color){c.r,c.g,c.b,c.a});
                }
            }
        }
    }

    r->assets.atlas  = LoadTextureFromImage(img);
    UnloadImage(img);
}

/* ─── Player spritesheet (simple procedural) ─────────────────── */
static void gen_player_texture(RenderCtx* r) {
    Image img = GenImageColor(16, 32, BLANK);
    /* Body */
    for (int y = 8; y < 24; y++)
        for (int x = 2; x < 14; x++)
            ImageDrawPixel(&img, x, y, (Color){60,120,200,255});
    /* Head */
    for (int y = 0; y < 8; y++)
        for (int x = 3; x < 13; x++)
            ImageDrawPixel(&img, x, y, (Color){220,180,140,255});
    /* Eyes */
    ImageDrawPixel(&img, 5, 3, (Color){30,30,30,255});
    ImageDrawPixel(&img, 10,3, (Color){30,30,30,255});
    /* Legs */
    for (int y = 24; y < 32; y++) {
        for (int x = 2; x < 7; x++)
            ImageDrawPixel(&img, x, y, (Color){80,60,40,255});
        for (int x = 9; x < 14; x++)
            ImageDrawPixel(&img, x, y, (Color){80,60,40,255});
    }
    r->assets.player = LoadTextureFromImage(img);
    UnloadImage(img);
}

/* ─── Camera utilities ───────────────────────────────────────── */
void camera_follow(Camera2D_MC* cam, float tx, float ty, float speed, float dt) {
    float dx  = tx - cam->x;
    float dy  = ty - cam->y;
    float t   = 1.0f - powf(1.0f - speed, dt * 60.0f);
    cam->x   += dx * t;
    cam->y   += dy * t;
}

void camera_world_to_screen(const Camera2D_MC* cam, float wx, float wy, float* sx, float* sy) {
    *sx = (wx - cam->x) * cam->zoom + cam->screen_w * 0.5f;
    *sy = (wy - cam->y) * cam->zoom + cam->screen_h * 0.5f;
}

void camera_screen_to_world(const Camera2D_MC* cam, float sx, float sy, float* wx, float* wy) {
    *wx = (sx - cam->screen_w  * 0.5f) / cam->zoom + cam->x;
    *wy = (sy - cam->screen_h * 0.5f) / cam->zoom + cam->y;
}

/* ─── Visible chunk range ────────────────────────────────────── */
static void get_visible_blocks(const Camera2D_MC* cam,
                                int* bx_min, int* bx_max,
                                int* by_min, int* by_max) {
    float half_w = (cam->screen_w  * 0.5f) / cam->zoom;
    float half_h = (cam->screen_h * 0.5f) / cam->zoom;
    float left   = cam->x - half_w - BLOCK_SIZE;
    float right  = cam->x + half_w + BLOCK_SIZE;
    float top    = cam->y - half_h - BLOCK_SIZE;
    float bottom = cam->y + half_h + BLOCK_SIZE;
    *bx_min = (int)floorf(left   / BLOCK_SIZE);
    *bx_max = (int)ceilf (right  / BLOCK_SIZE);
    *by_min = (int)floorf(top    / BLOCK_SIZE);
    *by_max = (int)ceilf (bottom / BLOCK_SIZE);
    if (*by_min < 0)             *by_min = 0;
    if (*by_max >= CHUNK_HEIGHT) *by_max = CHUNK_HEIGHT - 1;
}

/* ─── Get texture rect for a block face ─────────────────────── */
static Rectangle get_block_uv(uint8_t blk, int face) {
    /* face: 0=top 1=side 2=bottom */
    int idx = blk * 3 + face;
    int col = idx % ATLAS_COLS;
    int row = idx / ATLAS_COLS;
    return (Rectangle){
        (float)(col * ATLAS_TILE_SIZE),
        (float)(row * ATLAS_TILE_SIZE),
        (float)ATLAS_TILE_SIZE,
        (float)ATLAS_TILE_SIZE
    };
}

/* ─── Main world renderer ────────────────────────────────────── */
void renderer_draw_world(RenderCtx* r, WorldCtx* w) {
    int bx_min, bx_max, by_min, by_max;
    get_visible_blocks(&r->cam, &bx_min, &bx_max, &by_min, &by_max);

    float tile = (float)BLOCK_SIZE * r->cam.zoom;
    float half_w = r->cam.screen_w  * 0.5f;
    float half_h = r->cam.screen_h * 0.5f;

    /* Sky gradient background */
    DrawRectangleGradientV(0, 0, r->screen_w, r->screen_h,
                           (Color){100,180,255,255},
                           (Color){50,120,220,255});

    BeginScissorMode(0, 0, r->screen_w, r->screen_h);

    for (int bx = bx_min; bx <= bx_max; bx++) {
        for (int by = by_min; by <= by_max; by++) {
            uint8_t blk = world_get_block(w, bx, by);
            if (blk == BLOCK_AIR) continue;

            float sx = (bx * BLOCK_SIZE - r->cam.x) * r->cam.zoom + half_w;
            float sy = (by * BLOCK_SIZE - r->cam.y) * r->cam.zoom + half_h;

            Rectangle src  = get_block_uv(blk, 1); /* side face for 2D */
            Rectangle dest = {sx, sy, tile, tile};

            /* Light modulation */
            uint8_t li = 200; /* default ambient */
            Color tint = {li, li, li, 255};

            DrawTexturePro(r->assets.atlas, src, dest, (Vector2){0,0}, 0.0f, tint);
        }
    }

    EndScissorMode();
}

/* ─── Entity draw ────────────────────────────────────────────── */
void renderer_draw_entity(RenderCtx* r, const Entity* e) {
    float sx, sy;
    camera_world_to_screen(&r->cam, e->pos.x, e->pos.y, &sx, &sy);
    float pw = PLAYER_WIDTH  * r->cam.zoom;
    float ph = PLAYER_HEIGHT * r->cam.zoom;

    Rectangle src  = {0, 0, 16, 32};
    Rectangle dest = {sx, sy, pw, ph};
    DrawTexturePro(r->assets.player, src, dest, (Vector2){0,0}, 0.0f, WHITE);
}

/* ─── Debug overlay ──────────────────────────────────────────── */
void renderer_draw_debug(RenderCtx* r, WorldCtx* w, const Entity* pe) {
    (void)r; (void)w;
    char buf[128];
    int bx = (int)floorf(pe->pos.x / BLOCK_SIZE);
    int by = (int)floorf(pe->pos.y / BLOCK_SIZE);
    snprintf(buf, sizeof(buf), "X:%.0f Y:%.0f  Block:%d,%d  Vel:%.0f,%.0f",
             pe->pos.x, pe->pos.y, bx, by, pe->vel.x, pe->vel.y);
    DrawText(buf, 8, 8, 16, (Color){255,255,255,200});
}

/* ─── Create / Destroy ───────────────────────────────────────── */
RenderCtx* renderer_create(int sw, int sh) {
    RenderCtx* r   = (RenderCtx*)calloc(1, sizeof(RenderCtx));
    r->screen_w    = sw;
    r->screen_h    = sh;
    r->cam.screen_w= (float)sw;
    r->cam.screen_h= (float)sh;
    r->cam.zoom    = 2.0f;
    return r;
}

void renderer_load_assets(RenderCtx* r) {
    renderer_gen_atlas(r);
    gen_player_texture(r);
    r->assets.loaded = true;
}

void renderer_destroy(RenderCtx* r) {
    if (r->assets.loaded) {
        UnloadTexture(r->assets.atlas);
        UnloadTexture(r->assets.player);
    }
    free(r);
}
