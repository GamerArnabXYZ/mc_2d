#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "raylib.h"
#include "mc2d_types.h"
#include "world/worldgen.h"
#include "physics/physics.h"
#include "renderer/renderer.h"
#include "input/input.h"
#include "player/player.h"
#include "ui/ui.h"

/* ─── Platform-aware entry ───────────────────────────────────── */
#if defined(PLATFORM_ANDROID)
    #include "raylib.h"   /* android_native_app_glue via Raylib */
#endif

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
    static void emscripten_loop(void* arg);
#endif

/* ─── Game context (global for Emscripten callback) ─────────── */
typedef struct {
    WorldCtx*     world;
    PlayerCtx*    player;
    RenderCtx*    renderer;
    InputCtx*     input;
    UICtx*        ui;
    GameStateEnum state;
    float         acc_dt;       /* fixed-step accumulator */
    bool          debug_mode;
} GameCtx;

static GameCtx G;

/* ─── Init ───────────────────────────────────────────────────── */
static void game_init(void) {
    int sw = GetScreenWidth();
    int sh = GetScreenHeight();

    G.world    = world_create(WORLD_SEED_DEFAULT);
    G.renderer = renderer_create(sw, sh);
    G.input    = input_create((float)sw, (float)sh);
    G.ui       = ui_create();
    G.state    = GAMESTATE_MENU;
    G.acc_dt   = 0.0f;
    G.debug_mode = false;

    renderer_load_assets(G.renderer);

    /* Player spawns after world exists */
    G.player = player_create(G.world->spawn_x * BLOCK_SIZE,
                              G.world->spawn_y);

    /* Aim camera at spawn */
    G.renderer->cam.x = G.player->entity.pos.x;
    G.renderer->cam.y = G.player->entity.pos.y;
}

/* ─── Resize handler ─────────────────────────────────────────── */
static void game_resize(void) {
    int sw = GetScreenWidth();
    int sh = GetScreenHeight();
    G.renderer->screen_w      = sw;
    G.renderer->screen_h      = sh;
    G.renderer->cam.screen_w  = (float)sw;
    G.renderer->cam.screen_h  = (float)sh;
    input_resize(G.input, (float)sw, (float)sh);
}

/* ─── Single frame ───────────────────────────────────────────── */
static void game_frame(void) {
    if (IsWindowResized()) game_resize();

    float dt = GetFrameTime();
    if (dt > 0.05f) dt = 0.05f; /* clamp spike */

    int sw = GetScreenWidth();
    int sh = GetScreenHeight();

    /* Update input every frame */
    input_update(G.input);
    const InputState* inp = &G.input->state;

    /* ── State machine ───────────────────────────────────────── */
    switch (G.state) {
        case GAMESTATE_MENU:
            BeginDrawing();
            ClearBackground(BLACK);
            ui_draw_main_menu(G.ui, sw, sh, &G.state);
            EndDrawing();
            return;

        case GAMESTATE_PLAYING:
            if (inp->pause) { G.state = GAMESTATE_PAUSED; break; }
            if (inp->inventory) { G.state = GAMESTATE_INVENTORY; break; }

            /* Fixed-step physics */
            G.acc_dt += dt;
            while (G.acc_dt >= FIXED_DT) {
                player_update(G.player, G.world, inp, &G.renderer->cam, FIXED_DT);
                G.acc_dt -= FIXED_DT;

                if (!G.player->entity.active)
                    G.state = GAMESTATE_GAMEOVER;
            }

            /* Camera smooth follow */
            float target_x = G.player->entity.pos.x + PLAYER_WIDTH  * 0.5f;
            float target_y = G.player->entity.pos.y + PLAYER_HEIGHT * 0.5f;
            camera_follow(&G.renderer->cam, target_x, target_y, 0.08f, dt);

            /* Update cursor world coords */
            camera_screen_to_world(&G.renderer->cam,
                                   inp->cursor_screen.x, inp->cursor_screen.y,
                                   &G.input->state.cursor_world.x,
                                   &G.input->state.cursor_world.y);
            break;

        case GAMESTATE_PAUSED:
            /* handled in draw below */
            break;

        case GAMESTATE_INVENTORY:
            if (inp->inventory || inp->pause) G.state = GAMESTATE_PLAYING;
            break;

        case GAMESTATE_GAMEOVER:
            /* player_update handles respawn internally */
            if (G.player->entity.active) G.state = GAMESTATE_PLAYING;
            player_update(G.player, G.world, inp, &G.renderer->cam, dt);
            break;

        default: break;
    }

    /* Toggle debug */
    if (IsKeyPressed(KEY_F3)) G.debug_mode = !G.debug_mode;

    /* ── Draw ────────────────────────────────────────────────── */
    BeginDrawing();
    ClearBackground((Color){100,180,255,255});

    if (G.state != GAMESTATE_MENU) {
        renderer_draw_world(G.renderer, G.world);
        renderer_draw_entity(G.renderer, &G.player->entity);

        /* HUD */
        player_draw_hotbar(G.player, sw, sh);
        input_draw_hud(G.input);

        if (G.debug_mode)
            renderer_draw_debug(G.renderer, G.world, &G.player->entity);

        /* FPS top-right */
        char fps_buf[16];
        int fps = GetFPS();
        snprintf(fps_buf, sizeof(fps_buf), "FPS:%d", fps);
        DrawText(fps_buf, sw - MeasureText(fps_buf, 16) - 8, 8, 16,
                 (Color){255,255,255,200});
    }

    /* State-specific overlays */
    if (G.state == GAMESTATE_PAUSED) {
        G.state = ui_draw_pause(G.ui, sw, sh);
    } else if (G.state == GAMESTATE_GAMEOVER) {
        ui_draw_gameover(G.ui, sw, sh, G.player->respawn_timer, &G.state);
    }

    EndDrawing();
}

/* ─── Cleanup ────────────────────────────────────────────────── */
static void game_shutdown(void) {
    player_destroy(G.player);
    renderer_destroy(G.renderer);
    input_destroy(G.input);
    ui_destroy(G.ui);
    world_destroy(G.world);
}

/* ─── Entry point ────────────────────────────────────────────── */
#if defined(PLATFORM_ANDROID)
void android_main(struct android_app* app) {
    InitWindow(0, 0, "MC2D");
    SetTargetFPS(TARGET_FPS);
    game_init();
    while (!WindowShouldClose()) game_frame();
    game_shutdown();
    CloseWindow();
}
#else
int main(void) {
    int sw = 800, sh = 450;

#if defined(PLATFORM_WEB)
    /* WASM: canvas size driven by HTML */
    sw = 960; sh = 540;
#endif

    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);
    InitWindow(sw, sh, "MC2D - ArnabLabZ Studio");
    SetTargetFPS(TARGET_FPS);

    game_init();

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop_arg(emscripten_loop, NULL, 0, 1);
#else
    while (!WindowShouldClose()) game_frame();
    game_shutdown();
    CloseWindow();
#endif
    return 0;
}

#if defined(PLATFORM_WEB)
static void emscripten_loop(void* arg) {
    (void)arg;
    game_frame();
}
#endif
#endif /* PLATFORM_ANDROID */
