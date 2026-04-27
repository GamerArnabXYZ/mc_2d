#include "input.h"
#include <stdlib.h>
#include <math.h>
#include <string.h>

/* ─── Helpers ────────────────────────────────────────────────── */
static bool circle_hit(float tx, float ty, float cx, float cy, float r) {
    float dx = tx - cx, dy = ty - cy;
    return (dx*dx + dy*dy) <= (r*r);
}

static bool rect_hit(float tx, float ty, float rx, float ry, float rw, float rh) {
    return tx >= rx && tx <= rx+rw && ty >= ry && ty <= ry+rh;
}

/* ─── Create / Destroy ───────────────────────────────────────── */
InputCtx* input_create(float sw, float sh) {
    InputCtx* ic      = (InputCtx*)calloc(1, sizeof(InputCtx));
    ic->sw            = sw;
    ic->sh            = sh;
    ic->joy_touch_id  = -1;
    ic->is_touch_device = (GetTouchPointCount() >= 0); /* always true on mobile */
    return ic;
}

void input_destroy(InputCtx* ic) { free(ic); }

void input_resize(InputCtx* ic, float sw, float sh) {
    ic->sw = sw;
    ic->sh = sh;
}

/* ─── Update ─────────────────────────────────────────────────── */
void input_update(InputCtx* ic) {
    InputState* s = &ic->state;
    float sw = ic->sw, sh = ic->sh;

    /* Reset per-frame */
    s->move_left  = false;
    s->move_right = false;
    s->jump       = false;
    s->dig        = false;
    s->place      = false;
    s->btn_jump   = false;
    s->btn_dig    = false;
    s->btn_place  = false;
    s->joy_x      = 0.0f;
    s->joy_y      = 0.0f;
    s->scroll_delta = 0;
    s->inventory  = false;
    s->pause      = false;

    /* ── Keyboard ─────────────────────────────────────────────── */
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))  s->move_left  = true;
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) s->move_right = true;
    if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP))
        s->jump = true;
    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))   s->dig   = true;
    if (IsMouseButtonDown(MOUSE_RIGHT_BUTTON))  s->place = true;
    if (IsKeyPressed(KEY_E))  s->inventory = true;
    if (IsKeyPressed(KEY_ESCAPE)) s->pause = true;

    /* Mouse scroll for hotbar */
    s->scroll_delta = (int)GetMouseWheelMove();

    /* Mouse cursor */
    Vector2 mp     = GetMousePosition();
    s->cursor_screen.x = mp.x;
    s->cursor_screen.y = mp.y;
    s->pointer_down    = IsMouseButtonDown(MOUSE_LEFT_BUTTON);

    /* ── Touch ────────────────────────────────────────────────── */
    int tc = GetTouchPointCount();
    if (tc <= 0) {
        ic->joy_touch_id = -1;
        return;
    }

    /* Button layout */
    float bjx = sw * BTN_JUMP_X_FRAC  - BTN_SIZE * 0.5f;
    float bjy = sh * BTN_JUMP_Y_FRAC  - BTN_SIZE * 0.5f;
    float bdx = sw * BTN_DIG_X_FRAC   - BTN_SIZE * 0.5f;
    float bdy = sh * BTN_DIG_Y_FRAC   - BTN_SIZE * 0.5f;
    float bpx = sw * BTN_PLACE_X_FRAC - BTN_SIZE * 0.5f;
    float bpy = sh * BTN_PLACE_Y_FRAC - BTN_SIZE * 0.5f;

    for (int i = 0; i < tc && i < 10; i++) {
        int   tid = GetTouchPointId(i);
        Vector2 tp = GetTouchPosition(i);
        float tx = tp.x, ty = tp.y;

        /* Joystick acquisition on left side */
        if (tx < sw * 0.5f) {
            if (ic->joy_touch_id == -1) {
                /* New joystick touch */
                ic->joy_touch_id = tid;
                ic->joy_base_x   = tx;
                ic->joy_base_y   = ty;
            }
            if (ic->joy_touch_id == tid) {
                float dx = tx - ic->joy_base_x;
                float dy = ty - ic->joy_base_y;
                float len = sqrtf(dx*dx + dy*dy);
                if (len > JOY_OUTER_RADIUS) {
                    dx = dx / len * JOY_OUTER_RADIUS;
                    dy = dy / len * JOY_OUTER_RADIUS;
                    len = JOY_OUTER_RADIUS;
                }
                s->joy_x = dx / JOY_OUTER_RADIUS;
                s->joy_y = dy / JOY_OUTER_RADIUS;
                if (s->joy_x < -0.3f) s->move_left  = true;
                if (s->joy_x >  0.3f) s->move_right = true;
            }
        }

        /* Right-side buttons */
        if (rect_hit(tx, ty, bjx, bjy, BTN_SIZE, BTN_SIZE)) {
            s->btn_jump = true;
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) s->jump = true;
            /* For touch, treat any touch on jump as jump press */
            s->jump = true;
        }
        if (rect_hit(tx, ty, bdx, bdy, BTN_SIZE, BTN_SIZE)) {
            s->btn_dig = true;
            s->dig     = true;
        }
        if (rect_hit(tx, ty, bpx, bpy, BTN_SIZE, BTN_SIZE)) {
            s->btn_place = true;
            s->place     = true;
        }

        /* Screen tap for cursor */
        if (tx > sw * 0.2f && tx < sw * 0.8f && ty < sh * 0.6f) {
            s->cursor_screen.x = tx;
            s->cursor_screen.y = ty;
            s->pointer_down    = true;
        }
    }

    /* Joystick release detection */
    bool joy_alive = false;
    for (int i = 0; i < tc; i++) {
        if (GetTouchPointId(i) == ic->joy_touch_id) { joy_alive = true; break; }
    }
    if (!joy_alive) {
        ic->joy_touch_id = -1;
        s->joy_x = 0.0f;
        s->joy_y = 0.0f;
    }
}

/* ─── HUD draw ───────────────────────────────────────────────── */
void input_draw_hud(const InputCtx* ic) {
    float sw = ic->sw, sh = ic->sh;

    /* Virtual joystick */
    float jcx = sw * JOY_CENTER_X_FRAC;
    float jcy = sh * JOY_CENTER_Y_FRAC;

    /* Outer ring */
    DrawCircleLines((int)jcx, (int)jcy, JOY_OUTER_RADIUS,
                    (Color){255,255,255,80});
    /* Inner knob */
    float kx = jcx + ic->state.joy_x * JOY_OUTER_RADIUS;
    float ky = jcy + ic->state.joy_y * JOY_OUTER_RADIUS;
    DrawCircle((int)kx, (int)ky, JOY_INNER_RADIUS,
               (Color){255,255,255,120});
    DrawCircleLines((int)kx, (int)ky, JOY_INNER_RADIUS,
                    (Color){255,255,255,200});

    /* Action buttons */
    Color c_jump  = ic->state.btn_jump  ? (Color){255,220,50,220} : (Color){255,255,255,100};
    Color c_dig   = ic->state.btn_dig   ? (Color){255,80,80,220}  : (Color){255,255,255,100};
    Color c_place = ic->state.btn_place ? (Color){80,200,100,220} : (Color){255,255,255,100};

    float bjx = sw * BTN_JUMP_X_FRAC  - BTN_SIZE * 0.5f;
    float bjy = sh * BTN_JUMP_Y_FRAC  - BTN_SIZE * 0.5f;
    float bdx = sw * BTN_DIG_X_FRAC   - BTN_SIZE * 0.5f;
    float bdy = sh * BTN_DIG_Y_FRAC   - BTN_SIZE * 0.5f;
    float bpx = sw * BTN_PLACE_X_FRAC - BTN_SIZE * 0.5f;
    float bpy = sh * BTN_PLACE_Y_FRAC - BTN_SIZE * 0.5f;

    /* Rounded rectangles for MD3 feel */
    DrawRectangleRounded((Rectangle){bjx,bjy,BTN_SIZE,BTN_SIZE}, 0.4f, 8, c_jump);
    DrawText("↑", (int)(bjx + BTN_SIZE*0.35f), (int)(bjy + BTN_SIZE*0.25f), 22, WHITE);

    DrawRectangleRounded((Rectangle){bdx,bdy,BTN_SIZE,BTN_SIZE}, 0.4f, 8, c_dig);
    DrawText("⛏", (int)(bdx + BTN_SIZE*0.3f), (int)(bdy + BTN_SIZE*0.25f), 18, WHITE);

    DrawRectangleRounded((Rectangle){bpx,bpy,BTN_SIZE,BTN_SIZE}, 0.4f, 8, c_place);
    DrawText("[+]", (int)(bpx + BTN_SIZE*0.2f), (int)(bpy + BTN_SIZE*0.3f), 16, WHITE);
}
