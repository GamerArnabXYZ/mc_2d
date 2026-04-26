// hud.c - HUD Rendering: MD3 Design, Touch-Perfect Sizing
#include <stdint.h>
#include "hud.h"
#include "../renderer/textures.h"
#include "../world/blocks.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

HUD g_hud = {0};

// ============================================================
// Init & Layout
// ============================================================
void hud_init(int screen_w, int screen_h) {
    memset(&g_hud, 0, sizeof(HUD));
    g_hud.show_debug = false;
    hud_layout(screen_w, screen_h);
}

void hud_layout(int screen_w, int screen_h) {
    g_hud.screen_w  = screen_w;
    g_hud.screen_h  = screen_h;

    // Slot size: 52px on large screens, 44px on small (MD3 min 48dp)
    g_hud.slot_size = (screen_w >= 720) ? 52 : 44;
    g_hud.slot_pad  = 4;

    int total_w = INV_HOTBAR_SIZE * (g_hud.slot_size + g_hud.slot_pad) - g_hud.slot_pad;
    g_hud.hotbar_x = (screen_w - total_w) / 2;
    g_hud.hotbar_y = screen_h - g_hud.slot_size - 14;

    // Inventory panel — centered, 9×4 grid
    int cols = 9, rows = 4; // 1 hotbar row + 3 main rows
    g_hud.inv_panel_w = cols * (g_hud.slot_size + g_hud.slot_pad) + 24;
    g_hud.inv_panel_h = rows * (g_hud.slot_size + g_hud.slot_pad) + 48;
    g_hud.inv_panel_x = (screen_w  - g_hud.inv_panel_w) / 2;
    g_hud.inv_panel_y = (screen_h - g_hud.inv_panel_h) / 2;
}

void hud_update(bool inv_key_pressed) {
    if (inv_key_pressed) g_hud.inv_open = !g_hud.inv_open;
}

void hud_set_highlight(int32_t bx, int32_t by, bool valid) {
    g_hud.highlight_valid = valid;
    g_hud.highlight_bx = bx;
    g_hud.highlight_by = by;
}

// ============================================================
// Rounded rect helper (fills + optional border)
// ============================================================
static void draw_panel(float x, float y, float w, float h,
                        float roundness, Color fill, Color border, float bthick) {
    Rectangle r = {x, y, w, h};
    DrawRectangleRounded(r, roundness, 8, fill);
    if (bthick > 0)
        DrawRectangleRoundedLines(r, roundness, 8, border);
    (void)bthick;
}

// ============================================================
// Hotbar
// ============================================================
void hud_draw_hotbar(const Inventory* inv) {
    int ss  = g_hud.slot_size;
    int sp  = g_hud.slot_pad;
    int hx  = g_hud.hotbar_x;
    int hy  = g_hud.hotbar_y;

    // Background pill
    int total_w = INV_HOTBAR_SIZE * (ss + sp) - sp;
    draw_panel((float)hx - 6, (float)hy - 6,
               (float)total_w + 12, (float)ss + 12,
               0.3f, MD3_SURFACE, MD3_OUTLINE, 1);

    for (int i = 0; i < INV_HOTBAR_SIZE; i++) {
        ItemSlot s  = inv->slots[i];
        float sx    = (float)(hx + i * (ss + sp));
        float sy    = (float)hy;
        bool  sel   = (i == inv->selected);

        // Slot bg
        Color slot_bg = sel ? (Color){50, 50, 70, 220} : (Color){35, 35, 45, 180};
        draw_panel(sx, sy, (float)ss, (float)ss, 0.2f, slot_bg,
                   sel ? MD3_PRIMARY : MD3_OUTLINE, 1.5f);

        // Selection glow ring
        if (sel) {
            DrawRectangleRoundedLines((Rectangle){sx-2, sy-2, (float)ss+4, (float)ss+4},
                                     0.25f, 8, (Color){100,200,255,200});
        }

        // Block icon
        if (s.block_id != 0) {
            const BlockDef* bd = block_get(s.block_id);
            tex_draw_tile(bd->tex_side,
                          sx + 4, sy + 4, (float)(ss - 8), WHITE);

            // Count badge
            if (s.count > 1) {
                char cnt[4];
                snprintf(cnt, sizeof(cnt), "%d", s.count);
                int tw = MeasureText(cnt, 12);
                // Shadow
                DrawText(cnt, (int)(sx + ss - tw - 3) + 1,
                         (int)(sy + ss - 15) + 1, 12, (Color){0,0,0,180});
                DrawText(cnt, (int)(sx + ss - tw - 3),
                         (int)(sy + ss - 15), 12, MD3_ON_SURFACE);
            }
        }

        // Slot number 1-9
        char num[2] = {'1' + (char)i, '\0'};
        DrawText(num, (int)(sx + 4), (int)(sy + 3), 10,
                 (Color){180,180,200, sel ? 255 : 120});
    }
}

// ============================================================
// Health Bar — heart icons + segmented bar
// ============================================================
void hud_draw_health(const Player* p) {
    int sw = g_hud.screen_w, sh = g_hud.screen_h;
    int bar_x = 16;
    int bar_y = sh - g_hud.slot_size - 60;
    int max_hearts = p->max_health / 2; // 2 HP per heart
    int full_h = p->health / 2;
    int half_h = p->health % 2;

    int heart_size = 16;
    int gap        = 3;

    for (int i = 0; i < max_hearts; i++) {
        int hx = bar_x + i * (heart_size + gap);
        int hy = bar_y;

        // Empty heart outline
        DrawText("♡", hx, hy, heart_size, (Color){80, 30, 30, 200});

        if (i < full_h) {
            // Full heart
            DrawText("♥", hx, hy, heart_size, (Color){220, 50, 50, 255});
        } else if (i == full_h && half_h) {
            // Half heart
            BeginScissorMode(hx, hy, heart_size/2, heart_size);
            DrawText("♥", hx, hy, heart_size, (Color){220, 50, 50, 255});
            EndScissorMode();
        }
    }
    (void)sw;
}

// ============================================================
// Mining Progress Bar
// ============================================================
void hud_draw_mining_progress(const Player* p) {
    if (!p->is_mining || p->mine_progress <= 0) return;

    int sw = g_hud.screen_w, sh = g_hud.screen_h;
    int bar_w = 160, bar_h = 10;
    int bx    = (sw - bar_w) / 2;
    int by    = sh - g_hud.slot_size - 30;

    // Background
    draw_panel((float)bx - 2, (float)by - 2,
               (float)bar_w + 4, (float)bar_h + 4,
               0.5f, (Color){20,20,25,200}, MD3_OUTLINE, 1);

    // Fill — orange to red based on progress
    float prog = p->mine_progress;
    Color fill = {
        (uint8_t)(255),
        (uint8_t)(160 * (1.0f - prog)),
        30, 230
    };
    DrawRectangleRounded(
        (Rectangle){(float)bx, (float)by, (float)(bar_w * prog), (float)bar_h},
        0.5f, 6, fill
    );

    // Label
    const BlockDef* bd = block_get(p->mine_x >= 0 ?
        (uint8_t)0 : 0); // placeholder — caller passes actual block
    (void)bd;
    DrawText("Mining...", bx + bar_w + 6, by - 1, 12,
             (Color){200,200,200,180});
}

// ============================================================
// Day/Night Clock — sun icon + time arc
// ============================================================
void hud_draw_daytime(float time_of_day) {
    int sw = g_hud.screen_w;
    int cx = sw - 36, cy = 36;
    int r  = 20;

    // Clock circle background
    DrawCircle(cx, cy, (float)(r + 3), (Color){20,20,30,180});
    DrawCircleLines(cx, cy, (float)(r + 3), MD3_OUTLINE);

    // Arc fill — how much of day has passed
    // DrawRing not available in all raylib — use circle sectors
    float angle = time_of_day * 360.0f - 90.0f; // Start from top
    Color arc_col = (time_of_day < 0.5f) ?
        (Color){255, 220, 80, 200} :   // Day = yellow
        (Color){ 80, 100, 200, 200};    // Night = blue

    DrawCircleSector((Vector2){(float)cx,(float)cy}, (float)r,
                     -90.0f, -90.0f + time_of_day * 360.0f,
                     20, arc_col);
    DrawCircleLines(cx, cy, (float)r, (Color){255,255,255,60});

    // Sun/moon icon
    const char* icon = (time_of_day > 0.25f && time_of_day < 0.75f) ? "☀" : "☽";
    int iw = MeasureText(icon, 14);
    DrawText(icon, cx - iw/2, cy - 7, 14,
             (time_of_day > 0.25f && time_of_day < 0.75f) ?
             (Color){255,240,100,255} : (Color){200,210,255,255});
    (void)angle;
}

// ============================================================
// Crosshair — only on desktop/non-touch
// ============================================================
void hud_draw_crosshair(int screen_w, int screen_h) {
    int cx = screen_w / 2, cy = screen_h / 2;
    int sz = 10, thick = 2;

    // Shadow
    DrawRectangle(cx - sz + 1, cy - thick/2 + 1, sz*2, thick, (Color){0,0,0,120});
    DrawRectangle(cx - thick/2 + 1, cy - sz + 1, thick, sz*2, (Color){0,0,0,120});

    // White cross
    DrawRectangle(cx - sz, cy - thick/2, sz*2, thick, (Color){255,255,255,200});
    DrawRectangle(cx - thick/2, cy - sz, thick, sz*2, (Color){255,255,255,200});
}

// ============================================================
// Debug overlay
// ============================================================
void hud_draw_debug(const Player* p, int chunk_count) {
    if (!g_hud.show_debug) return;

    const char* state_names[] = {"IDLE","RUN","JUMP","FALL","SWIM","MINE"};
    int st = (int)p->state;
    if (st < 0 || st > 5) st = 0;

    char buf[256];
    snprintf(buf, sizeof(buf),
        "Pos: %.2f, %.2f\n"
        "Vel: %.2f, %.2f\n"
        "State: %s\n"
        "Ground: %s | Water: %s\n"
        "Chunks: %d\n"
        "FPS: %d",
        p->box.x, p->box.y,
        p->vel_x, p->vel_y,
        state_names[st],
        p->on_ground ? "YES" : "no",
        p->in_water  ? "YES" : "no",
        chunk_count,
        GetFPS()
    );

    // Panel bg
    draw_panel(6, 6, 200, 120, 0.1f,
               (Color){10,10,20,180}, (Color){60,60,80,160}, 1);

    // Draw each line
    const char* line = buf;
    int ly = 14;
    char tmp[64];
    int ti = 0;
    while (*line) {
        if (*line == '\n' || *(line+1) == '\0') {
            if (*(line+1) == '\0' && *line != '\n') tmp[ti++] = *line;
            tmp[ti] = '\0';
            DrawText(tmp, 12, ly, 13, (Color){180, 220, 180, 220});
            ly += 17;
            ti = 0;
        } else {
            tmp[ti++] = *line;
        }
        line++;
    }
}

// ============================================================
// Inventory Panel (full 9×4 grid)
// ============================================================
void hud_draw_inventory_panel(const Inventory* inv) {
    if (!g_hud.inv_open) return;

    int px  = g_hud.inv_panel_x;
    int py  = g_hud.inv_panel_y;
    int pw  = g_hud.inv_panel_w;
    int ph  = g_hud.inv_panel_h;
    int ss  = g_hud.slot_size;
    int sp  = g_hud.slot_pad;

    // Dim background
    DrawRectangle(0, 0, g_hud.screen_w, g_hud.screen_h, (Color){0,0,0,100});

    // Panel
    draw_panel((float)px, (float)py, (float)pw, (float)ph,
               0.08f, MD3_SURFACE, MD3_OUTLINE, 1.5f);

    // Title
    DrawText("Inventory", px + 12, py + 10, 18, MD3_PRIMARY);

    // Draw slots (hotbar row at bottom, main above)
    // Row 0-2 = main inventory (slots 9-35)
    // Row 3   = hotbar (slots 0-8)
    for (int row = 0; row < 4; row++) {
        int y_off = py + 36 + row * (ss + sp);
        int is_hotbar_row = (row == 3);

        if (is_hotbar_row) {
            y_off += 8; // Extra gap before hotbar row
            // Separator line
            DrawLine(px + 8, y_off - 6, px + pw - 8, y_off - 6,
                     MD3_OUTLINE);
        }

        for (int col = 0; col < 9; col++) {
            int slot_idx = is_hotbar_row ? col : (9 + row * 9 + col);
            ItemSlot s   = inv->slots[slot_idx];
            bool sel     = (slot_idx == inv->selected);

            float sx = (float)(px + 12 + col * (ss + sp));
            float sy = (float)y_off;

            Color bg = sel
                ? (Color){50,60,80,220}
                : (Color){35,35,48,200};
            draw_panel(sx, sy, (float)ss, (float)ss, 0.15f, bg,
                       sel ? MD3_PRIMARY : MD3_OUTLINE, 1);

            if (s.block_id != 0) {
                const BlockDef* bd = block_get(s.block_id);
                tex_draw_tile(bd->tex_side, sx+4, sy+4, (float)(ss-8), WHITE);

                // Count
                if (s.count > 1) {
                    char cnt[4];
                    snprintf(cnt, sizeof(cnt), "%d", s.count);
                    int tw = MeasureText(cnt, 11);
                    DrawText(cnt, (int)(sx+ss-tw-3)+1, (int)(sy+ss-14)+1,
                             11, (Color){0,0,0,160});
                    DrawText(cnt, (int)(sx+ss-tw-3), (int)(sy+ss-14),
                             11, MD3_ON_SURFACE);
                }

                // Tooltip on hover (desktop)
                Vector2 mouse = GetMousePosition();
                if (mouse.x >= sx && mouse.x < sx+ss &&
                    mouse.y >= sy && mouse.y < sy+ss) {
                    const BlockDef* bdef = block_get(s.block_id);
                    int tw2 = MeasureText(bdef->name, 13) + 12;
                    float tx = sx + ss/2 - tw2/2;
                    float ty2 = sy - 24;
                    draw_panel(tx-2, ty2-2, (float)tw2+4, 20,
                               0.3f, (Color){20,20,30,230},
                               MD3_OUTLINE, 1);
                    DrawText(bdef->name, (int)(tx+4), (int)(ty2+3),
                             13, MD3_ON_SURFACE);
                }
            }
        }
    }

    // Close hint
    DrawText("[E] Close", px + pw - 80, py + ph - 24, 13,
             (Color){150,150,170,200});
}

// ============================================================
// Main draw call — call every frame
// ============================================================
void hud_draw(const Player* p, const Inventory* inv, float time_of_day) {
    hud_draw_hotbar(inv);
    hud_draw_health(p);
    hud_draw_mining_progress(p);
    hud_draw_daytime(time_of_day);
    hud_draw_inventory_panel(inv);

    // Debug toggle with F3
    if (IsKeyPressed(KEY_F3)) g_hud.show_debug = !g_hud.show_debug;
    // chunk count passed as 0 here — main.c passes real value
    hud_draw_debug(p, 0);
}
