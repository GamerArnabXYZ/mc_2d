#pragma once
#include <stdint.h>
// hud.h - HUD: Hotbar, Health, Mining Progress, Day/Night, Crosshair
// Material Design 3 aesthetics — touch-perfect sizing

#include "raylib.h"
#include "../player/inventory.h"
#include "../player/player.h"
#include <stdbool.h>

// MD3 color tokens
#define MD3_PRIMARY      (Color){100, 200, 255, 255}
#define MD3_SURFACE      (Color){ 28,  28,  35, 200}
#define MD3_SURFACE_VAR  (Color){ 45,  45,  55, 180}
#define MD3_OUTLINE      (Color){100, 100, 120, 160}
#define MD3_ON_SURFACE   (Color){220, 220, 230, 255}
#define MD3_ERROR        (Color){220,  60,  60, 255}
#define MD3_SUCCESS      (Color){ 80, 200, 100, 255}

typedef struct {
    // Hotbar geometry (computed in hud_layout)
    int   hotbar_x, hotbar_y;
    int   slot_size;          // px per slot (MD3: min 48)
    int   slot_pad;           // gap between slots

    // Inventory panel (when open)
    bool  inv_open;
    int   inv_panel_x, inv_panel_y;
    int   inv_panel_w, inv_panel_h;

    // Block highlight (world pos)
    bool    highlight_valid;
    int32_t highlight_bx, highlight_by;

    // FPS visibility
    bool show_debug;

    int screen_w, screen_h;
} HUD;

extern HUD g_hud;

void hud_init(int screen_w, int screen_h);
void hud_layout(int screen_w, int screen_h);   // Call on resize
void hud_update(bool inv_key_pressed);

void hud_draw(const Player* p, const Inventory* inv, float time_of_day);

// Individual draw calls (used internally, exposed for custom rendering)
void hud_draw_hotbar(const Inventory* inv);
void hud_draw_health(const Player* p);
void hud_draw_mining_progress(const Player* p);
void hud_draw_daytime(float time_of_day);
void hud_draw_crosshair(int screen_w, int screen_h);
void hud_draw_debug(const Player* p, int chunk_count);
void hud_draw_inventory_panel(const Inventory* inv);
void hud_set_highlight(int32_t bx, int32_t by, bool valid);
