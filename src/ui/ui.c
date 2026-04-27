#include "ui.h"
#include <stdlib.h>
#include <math.h>
#include <stdio.h>

UICtx* ui_create(void) {
    UICtx* u = (UICtx*)calloc(1, sizeof(UICtx));
    u->fade_alpha = 0.0f;
    return u;
}
void ui_destroy(UICtx* u) { free(u); }

/* ─── MD3 button ─────────────────────────────────────────────── */
bool ui_button(const char* label, float x, float y, float w, float h,
               Color bg, Color text_col) {
    Rectangle r = {x, y, w, h};
    Vector2   mp = GetMousePosition();
    bool hover  = CheckCollisionPointRec(mp, r);
    bool click  = false;

    /* Touch support */
    int tc = GetTouchPointCount();
    for (int i = 0; i < tc; i++) {
        Vector2 tp = GetTouchPosition(i);
        if (CheckCollisionPointRec(tp, r)) {
            hover = true;
            click = true;
        }
    }
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && hover) click = true;

    Color draw_bg = hover
        ? (Color){(unsigned char)fminf(bg.r+40,255),
                  (unsigned char)fminf(bg.g+40,255),
                  (unsigned char)fminf(bg.b+40,255), bg.a}
        : bg;

    DrawRectangleRounded(r, 0.35f, 8, draw_bg);
    DrawRectangleRoundedLines(r, 0.35f, 8, 1.5f,
                              (Color){255,255,255,80});

    int fs  = (int)(h * 0.4f);
    if (fs < 12) fs = 12;
    int tw  = MeasureText(label, fs);
    DrawText(label, (int)(x + w*0.5f - tw*0.5f),
             (int)(y + h*0.5f - fs*0.5f), fs, text_col);
    return click;
}

/* ─── Pause menu ─────────────────────────────────────────────── */
GameStateEnum ui_draw_pause(UICtx* u, int sw, int sh) {
    (void)u;
    /* Dim background */
    DrawRectangle(0, 0, sw, sh, (Color){0,0,0,140});

    /* Card */
    float cw = (float)(sw > 400 ? 320 : sw * 0.8f);
    float ch = 280.0f;
    float cx = (sw - cw) * 0.5f;
    float cy = (sh - ch) * 0.5f;

    DrawRectangleRounded((Rectangle){cx, cy, cw, ch}, 0.12f, 8,
                         (Color){30,30,40,230});
    DrawRectangleRoundedLines((Rectangle){cx,cy,cw,ch}, 0.12f, 8, 2.0f,
                               (Color){255,255,255,60});

    int title_fs = (int)(cw * 0.12f);
    DrawText("PAUSED",
             (int)(cx + cw*0.5f - MeasureText("PAUSED", title_fs)*0.5f),
             (int)(cy + 28), title_fs, WHITE);

    float btn_w = cw * 0.7f;
    float btn_h = 50.0f;
    float btn_x = cx + (cw - btn_w) * 0.5f;

    if (ui_button("RESUME", btn_x, cy + 90, btn_w, btn_h,
                  (Color){60,140,80,255}, WHITE))
        return GAMESTATE_PLAYING;

    if (ui_button("MAIN MENU", btn_x, cy + 155, btn_w, btn_h,
                  (Color){100,60,160,255}, WHITE))
        return GAMESTATE_MENU;

    if (ui_button("EXIT", btn_x, cy + 215, btn_w, btn_h,
                  (Color){160,50,50,255}, WHITE))
        CloseWindow();

    return GAMESTATE_PAUSED;
}

/* ─── Main menu ──────────────────────────────────────────────── */
void ui_draw_main_menu(UICtx* u, int sw, int sh, GameStateEnum* out_state) {
    (void)u;
    /* Animated gradient background */
    DrawRectangleGradientV(0, 0, sw, sh,
                           (Color){15,20,40,255},
                           (Color){30,15,50,255});

    /* Title */
    const char* title = "MC2D";
    int tfs = (int)(sw * 0.15f);
    if (tfs > 96) tfs = 96;
    int tw = MeasureText(title, tfs);
    DrawText(title, (sw - tw)/2, sh/6, tfs, (Color){255,220,50,255});

    const char* sub = "ArnabLabZ Studio";
    int sfs = (int)(tfs * 0.25f);
    int sw2 = MeasureText(sub, sfs);
    DrawText(sub, (sw-sw2)/2, sh/6 + tfs + 8, sfs, (Color){180,180,255,200});

    float btn_w = (float)(sw > 400 ? 280 : sw * 0.6f);
    float btn_h = 60.0f;
    float btn_x = (sw - btn_w) * 0.5f;

    if (ui_button("PLAY", btn_x, sh*0.45f, btn_w, btn_h,
                  (Color){50,160,90,255}, WHITE))
        *out_state = GAMESTATE_PLAYING;

    if (ui_button("EXIT", btn_x, sh*0.45f + 80, btn_w, btn_h,
                  (Color){140,50,50,255}, WHITE))
        CloseWindow();

    /* Version */
    char ver[32];
    snprintf(ver, sizeof(ver), "v%d.%d", MC2D_VERSION_MAJOR, MC2D_VERSION_MINOR);
    DrawText(ver, 8, sh - 22, 14, (Color){150,150,150,180});
}

/* ─── Game over ──────────────────────────────────────────────── */
void ui_draw_gameover(UICtx* u, int sw, int sh, float respawn_t, GameStateEnum* out_state) {
    (void)u; (void)out_state;
    DrawRectangle(0, 0, sw, sh, (Color){80,0,0,120});

    const char* msg = "YOU DIED";
    int fs  = (int)(sw * 0.1f);
    if (fs > 72) fs = 72;
    int tw  = MeasureText(msg, fs);
    DrawText(msg, (sw-tw)/2, sh/3, fs, (Color){255,60,60,255});

    char rtxt[64];
    snprintf(rtxt, sizeof(rtxt), "Respawning in %.1fs...", respawn_t > 0 ? respawn_t : 0.0f);
    int rfs = 20;
    int rtw = MeasureText(rtxt, rfs);
    DrawText(rtxt, (sw-rtw)/2, sh/3 + fs + 20, rfs, (Color){255,200,200,220});
}
