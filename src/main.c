// main.c - MC2D Engine Entry Point (v2 — camera + hud integrated)
// C99 | Raylib | Desktop · Web (Emscripten) · Android

#include "raylib.h"
#include "world/blocks.h"
#include "world/chunk.h"
#include "world/worldgen.h"
#include "player/player.h"
#include "player/inventory.h"
#include "renderer/textures.h"
#include "renderer/camera.h"
#include "input/input.h"
#include "ui/hud.h"
#include <stdio.h>
#include <math.h>
#include <time.h>

#ifdef PLATFORM_WEB
#include <emscripten/emscripten.h>
#endif

#define WINDOW_TITLE  "MC2D — ArnabLabZ"
#define BLOCK_SIZE    32.0f
#define TARGET_FPS    60
#define SPAWN_BX      0

typedef struct {
    Player    player;
    Inventory inventory;
    float     time_of_day;
    bool      paused;
    int       screen_w, screen_h;
} GameState;

static GameState G = {0};

static void render_sky(void) {
    float t = G.time_of_day;
    Color top, bot;
    if (t > 0.25f && t < 0.75f) {
        float b = (t < 0.35f) ? (t-0.25f)/0.1f : (t > 0.65f) ? 1.0f-(t-0.65f)/0.1f : 1.0f;
        top = (Color){(uint8_t)(5+45*b),(uint8_t)(5+115*b),(uint8_t)(30+170*b),255};
        bot = (Color){(uint8_t)(20+115*b),(uint8_t)(40+160*b),(uint8_t)(50+190*b),255};
    } else {
        top = (Color){3,5,22,255}; bot = (Color){8,10,40,255};
    }
    DrawRectangleGradientV(0, 0, G.screen_w, G.screen_h, top, bot);

    if (t < 0.22f || t > 0.78f) {
        static bool si = false; static int sx[80], sy[80];
        if (!si) { for(int i=0;i<80;i++){sx[i]=GetRandomValue(0,1280);sy[i]=GetRandomValue(0,360);} si=true; }
        float v = (t<0.22f)?(0.22f-t)/0.05f:(t-0.78f)/0.05f;
        if(v>1.f)v=1.f;
        for(int i=0;i<80;i++) DrawPixel(sx[i]%G.screen_w, sy[i]%(G.screen_h/2),
            (Color){255,255,255,(uint8_t)(v*200)});
    }
}

static void render_world(void) {
    int sbx = (int)(g_camera.position.x/BLOCK_SIZE)-1;
    int ebx = sbx + G.screen_w/(int)BLOCK_SIZE + 3;
    int sby = (int)(g_camera.position.y/BLOCK_SIZE)-1;
    int eby = sby + G.screen_h/(int)BLOCK_SIZE + 3;

    for (int bx=sbx; bx<=ebx; bx++) {
        for (int by=sby; by<=eby; by++) {
            if (by<0||by>=CHUNK_H) continue;
            uint8_t id = world_get_block(bx, by);
            if (id == BLOCK_ID_AIR) continue;
            const BlockDef* bd = block_get(id);
            Vector2 sp = camera_world_to_screen((float)bx,(float)by,BLOCK_SIZE);
            if (sp.x>G.screen_w+BLOCK_SIZE||sp.x<-BLOCK_SIZE) continue;
            if (sp.y>G.screen_h+BLOCK_SIZE||sp.y<-BLOCK_SIZE) continue;

            Color tint = WHITE;
            if (id==BLOCK_ID_WATER) tint=(Color){80,140,255,170};
            tex_draw_tile(bd->tex_side, sp.x, sp.y, BLOCK_SIZE, tint);

            if (g_hud.highlight_valid && g_hud.highlight_bx==bx && g_hud.highlight_by==by)
                DrawRectangleLinesEx((Rectangle){sp.x,sp.y,BLOCK_SIZE,BLOCK_SIZE},2.f,(Color){255,255,255,180});

            if (G.player.is_mining && G.player.mine_x==bx && G.player.mine_y==by) {
                float p = G.player.mine_progress;
                DrawRectangle((int)sp.x,(int)sp.y,(int)BLOCK_SIZE,(int)BLOCK_SIZE,(Color){0,0,0,(uint8_t)(p*160)});
                int cracks=(int)(p*5);
                for(int c=0;c<cracks;c++) DrawLine((int)sp.x+c*6+3,(int)sp.y+2,(int)sp.x+c*6+7,(int)(sp.y+BLOCK_SIZE-2),(Color){20,20,20,180});
            }
        }
    }
}

static void render_player(void) {
    Vector2 sp = camera_world_to_screen(G.player.box.x, G.player.box.y, BLOCK_SIZE);
    float pw = G.player.box.w*BLOCK_SIZE, ph = G.player.box.h*BLOCK_SIZE;
    DrawRectangle((int)(sp.x+pw*.2f),(int)(sp.y+ph*.35f),(int)(pw*.6f),(int)(ph*.45f),(Color){70,130,200,255});
    DrawRectangle((int)(sp.x+pw*.15f),(int)sp.y,(int)(pw*.7f),(int)(ph*.35f),(Color){255,200,150,255});
    int ey=(int)(sp.y+ph*.12f);
    if(G.player.facing>0) DrawRectangle((int)(sp.x+pw*.55f),ey,4,5,BLACK);
    else                  DrawRectangle((int)(sp.x+pw*.22f),ey,4,5,BLACK);
    float ro=(G.player.state==PLAYER_STATE_RUN)?sinf(GetTime()*10.f)*4.f:0.f;
    DrawRectangle((int)(sp.x+pw*.2f),(int)(sp.y+ph*.78f+ro),(int)(pw*.27f),(int)(ph*.22f),(Color){50,80,150,255});
    DrawRectangle((int)(sp.x+pw*.53f),(int)(sp.y+ph*.78f-ro),(int)(pw*.27f),(int)(ph*.22f),(Color){50,80,150,255});
}

static void update_highlight(void) {
    if (!g_input.interact_valid) { hud_set_highlight(0,0,false); return; }
    Vector2 wp = camera_screen_to_world(g_input.interact_screen.x, g_input.interact_screen.y, BLOCK_SIZE);
    hud_set_highlight((int32_t)floorf(wp.x),(int32_t)floorf(wp.y),true);
}

static void game_frame(void) {
    float dt = GetFrameTime();
    if (dt>0.05f) dt=0.05f;
    G.screen_w = GetScreenWidth();
    G.screen_h = GetScreenHeight();

    input_update(G.screen_w, G.screen_h);

    if (IsWindowResized()) {
        camera_resize(G.screen_w, G.screen_h);
        hud_layout(G.screen_w, G.screen_h);
        input_layout(G.screen_w, G.screen_h);
    }

    if (g_input.pause) G.paused = !G.paused;

    if (!G.paused) {
        if (g_input.move_x!=0) player_move(&G.player, g_input.move_x, dt);
        if (g_input.jump)      player_jump(&G.player);

        update_highlight();

        if (g_input.interact_valid) {
            Vector2 wp = camera_screen_to_world(g_input.interact_screen.x, g_input.interact_screen.y, BLOCK_SIZE);
            int32_t bx=(int32_t)floorf(wp.x), by=(int32_t)floorf(wp.y);
            if (g_input.mine)  player_start_mine(&G.player, bx, by);
            else               player_stop_mine(&G.player);
            if (g_input.place) {
                ItemSlot sel = inv_get_selected(&G.inventory);
                if (sel.block_id!=0 && player_place_block(&G.player, bx, by, sel.block_id)) {
                    inv_remove(&G.inventory, G.inventory.selected, 1);
                    camera_shake(0.08f, 3.f);
                }
            }
        } else { player_stop_mine(&G.player); }

        if (g_input.hotbar_delta!=0) {
            int ns = G.inventory.selected + g_input.hotbar_delta;
            if (ns<0) ns=INV_HOTBAR_SIZE-1;
            if (ns>=INV_HOTBAR_SIZE) ns=0;
            inv_select(&G.inventory, ns);
        }
        for (int k=0; k<INV_HOTBAR_SIZE; k++)
            if (IsKeyPressed(KEY_ONE+k)) inv_select(&G.inventory, k);

        player_update(&G.player, dt);
        chunk_update_active(world_to_chunk_x((int32_t)G.player.box.x));
        camera_update(dt, G.player.box.x+G.player.box.w*.5f, G.player.box.y+G.player.box.h*.5f, BLOCK_SIZE);

        G.time_of_day += dt/600.f;
        if (G.time_of_day>=1.f) G.time_of_day-=1.f;

        hud_update(g_input.inventory);
    }

    BeginDrawing();
    ClearBackground(BLACK);
    render_sky();
    render_world();
    render_player();
    hud_draw(&G.player, &G.inventory, G.time_of_day);
    hud_draw_debug(&G.player, g_chunks.count);
    input_draw_virtual();
    if (!g_input.is_touch) hud_draw_crosshair(G.screen_w, G.screen_h);

    if (G.paused) {
        DrawRectangle(0,0,G.screen_w,G.screen_h,(Color){0,0,0,130});
        const char* pt="PAUSED"; int tw=MeasureText(pt,48);
        DrawText(pt,(G.screen_w-tw)/2,G.screen_h/2-30,48,WHITE);
        DrawText("[ESC] Resume",(G.screen_w-MeasureText("[ESC] Resume",18))/2,G.screen_h/2+30,18,(Color){200,200,200,200});
    }
    EndDrawing();
}

int main(void) {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE|FLAG_MSAA_4X_HINT|FLAG_VSYNC_HINT);
    InitWindow(800, 480, WINDOW_TITLE);
    SetTargetFPS(TARGET_FPS);
    G.screen_w=GetScreenWidth(); G.screen_h=GetScreenHeight();

    blocks_init_defaults();
    blocks_load_json("assets/blocks.json");
    worldgen_init((uint32_t)time(NULL));
    chunk_manager_init();
    textures_init("assets/atlas.png");
    camera_init(G.screen_w, G.screen_h);
    input_init(G.screen_w, G.screen_h);
    hud_init(G.screen_w, G.screen_h);

    for (int cx=-3; cx<=3; cx++) chunk_get_or_create(cx);
    player_init(&G.player, (float)SPAWN_BX, 60.f);
    for (int y=CHUNK_H-1; y>=2; y--) {
        if (world_get_block(SPAWN_BX,y)!=BLOCK_ID_AIR && world_get_block(SPAWN_BX,y-1)==BLOCK_ID_AIR) {
            G.player.box.y=(float)(y-2); break;
        }
    }

    inv_init(&G.inventory);
    inv_add(&G.inventory, BLOCK_ID_DIRT,   64);
    inv_add(&G.inventory, BLOCK_ID_STONE,  32);
    inv_add(&G.inventory, BLOCK_ID_WOOD,   16);
    inv_add(&G.inventory, BLOCK_ID_TORCH,  8);
    inv_add(&G.inventory, BLOCK_ID_SAND,   16);
    inv_add(&G.inventory, BLOCK_ID_COBBLE, 32);
    G.time_of_day = 0.3f;

    printf("[MC2D] Ready! F3=debug E=inventory ESC=pause\n");

#ifdef PLATFORM_WEB
    emscripten_set_main_loop(game_frame, TARGET_FPS, 1);
#else
    while (!WindowShouldClose()) game_frame();
#endif

    textures_unload();
    CloseWindow();
    return 0;
}
