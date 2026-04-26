// input.c - Unified Input System Implementation
#include "input.h"
#include <string.h>
#include <math.h>

InputState g_input = {0};

// ============================================================
// Initialization
// ============================================================
void input_init(int screen_w, int screen_h) {
    memset(&g_input, 0, sizeof(InputState));
    g_input.joy.touch_id = -1;

    // Detect platform
#if defined(PLATFORM_ANDROID) || defined(PLATFORM_WEB)
    g_input.is_touch    = true;
    g_input.is_keyboard = false;
#else
    g_input.is_touch    = (GetTouchPointCount() > 0);
    g_input.is_keyboard = true;
#endif

    // Setup button labels
    strncpy(g_input.buttons[BTN_JUMP].label,  "↑",  7);
    strncpy(g_input.buttons[BTN_MINE].label,  "⛏",  7);
    strncpy(g_input.buttons[BTN_PLACE].label, "▪",  7);
    strncpy(g_input.buttons[BTN_INV].label,   "🎒", 7);

    input_layout(screen_w, screen_h);
}

// ============================================================
// Layout — positions controls based on screen size
// MD3: min 48px touch targets, 8px margins
// ============================================================
void input_layout(int screen_w, int screen_h) {
    float margin = 20.0f;
    float btn_r  = 30.0f; // button radius (60px diameter = MD3 compliant)

    // Joystick — bottom left
    g_input.joy.center = (Vector2){
        margin + JOYSTICK_RADIUS,
        (float)screen_h - margin - JOYSTICK_RADIUS
    };
    g_input.joy.knob = g_input.joy.center;

    // Action buttons — bottom right, stacked
    float bx = (float)screen_w - margin - btn_r;
    float by = (float)screen_h - margin - btn_r;

    g_input.buttons[BTN_JUMP].pos    = (Vector2){bx, by - (btn_r*2 + 12)*2};
    g_input.buttons[BTN_MINE].pos    = (Vector2){bx - btn_r*2 - 12, by};
    g_input.buttons[BTN_PLACE].pos   = (Vector2){bx, by};
    g_input.buttons[BTN_INV].pos     = (Vector2){bx - btn_r*2 - 12, by - (btn_r*2 + 12)};

    for (int i = 0; i < BTN_COUNT; i++) g_input.buttons[i].radius = btn_r;
}

// ============================================================
// Virtual Joystick update
// ============================================================
static void update_joystick(void) {
    VirtualJoystick* j = &g_input.joy;

    int touch_count = GetTouchPointCount();
    bool found = false;

    for (int t = 0; t < touch_count && t < 10; t++) {
        Vector2 tp = GetTouchPosition(t);
        int     id = GetTouchPointId(t);

        // Check if this touch owns joystick
        if (j->touch_id == id) {
            // Update knob
            float dx = tp.x - j->center.x;
            float dy = tp.y - j->center.y;
            float dist = sqrtf(dx*dx + dy*dy);
            float max_dist = JOYSTICK_RADIUS - JOYSTICK_KNOB_RADIUS;

            if (dist > max_dist) {
                dx = dx / dist * max_dist;
                dy = dy / dist * max_dist;
            }

            j->knob        = (Vector2){j->center.x + dx, j->center.y + dy};
            j->normalized  = (Vector2){dx / max_dist, dy / max_dist};
            j->active      = true;
            found          = true;
        } else if (j->touch_id == -1) {
            // Check if new touch is in joystick zone
            float dx = tp.x - j->center.x;
            float dy = tp.y - j->center.y;
            if (sqrtf(dx*dx + dy*dy) < JOYSTICK_RADIUS * 1.5f) {
                j->touch_id = id;
            }
        }
    }

    // Release joystick if finger lifted
    if (!found && j->touch_id != -1) {
        bool still_down = false;
        for (int t = 0; t < touch_count; t++) {
            if (GetTouchPointId(t) == j->touch_id) { still_down = true; break; }
        }
        if (!still_down) {
            j->touch_id   = -1;
            j->active     = false;
            j->knob       = j->center;
            j->normalized = (Vector2){0, 0};
        }
    }

    // Apply dead zone
    if (fabsf(j->normalized.x) < JOYSTICK_DEAD_ZONE) j->normalized.x = 0;
    if (fabsf(j->normalized.y) < JOYSTICK_DEAD_ZONE) j->normalized.y = 0;
}

// ============================================================
// Touch buttons update
// ============================================================
static void update_touch_buttons(void) {
    int touch_count = GetTouchPointCount();

    for (int b = 0; b < BTN_COUNT; b++) {
        TouchButton* btn = &g_input.buttons[b];
        bool was_held = btn->held;
        btn->held = false;
        btn->pressed = false;
        btn->just_pressed = false;

        for (int t = 0; t < touch_count && t < 10; t++) {
            Vector2 tp = GetTouchPosition(t);
            float dx = tp.x - btn->pos.x;
            float dy = tp.y - btn->pos.y;
            if (sqrtf(dx*dx + dy*dy) < btn->radius) {
                btn->held = true;
                btn->just_pressed = !was_held;
                btn->pressed = true;
            }
        }
    }
}

// ============================================================
// Main Update
// ============================================================
void input_update(int screen_w, int screen_h) {
    // Clear frame state
    g_input.move_x       = 0;
    g_input.move_y       = 0;
    g_input.jump         = false;
    g_input.jump_held    = false;
    g_input.mine         = false;
    g_input.place        = false;
    g_input.inventory    = false;
    g_input.pause        = false;
    g_input.interact_valid = false;
    g_input.hotbar_delta = 0;

    // Re-detect touch
    if (GetTouchPointCount() > 0) g_input.is_touch = true;

    // ---- TOUCH / MOBILE INPUT ----
    if (g_input.is_touch) {
        update_joystick();
        update_touch_buttons();

        g_input.move_x = g_input.joy.normalized.x;
        g_input.move_y = g_input.joy.normalized.y;

        if (g_input.buttons[BTN_JUMP].held)   { g_input.jump = true; g_input.jump_held = true; }
        if (g_input.buttons[BTN_MINE].held)   { g_input.mine = true; }
        if (g_input.buttons[BTN_PLACE].just_pressed) { g_input.place = true; }
        if (g_input.buttons[BTN_INV].just_pressed)   { g_input.inventory = !g_input.inventory; }

        // Mine target: use 2nd touch outside joystick zone
        int touch_count = GetTouchPointCount();
        for (int t = 0; t < touch_count; t++) {
            Vector2 tp = GetTouchPosition(t);
            float dx = tp.x - g_input.joy.center.x;
            float dy = tp.y - g_input.joy.center.y;
            if (sqrtf(dx*dx + dy*dy) > JOYSTICK_RADIUS * 2.0f) {
                // Not near joystick — use as targeting input
                g_input.interact_screen = tp;
                g_input.interact_valid  = true;
                break;
            }
        }
    }

    // ---- KEYBOARD / MOUSE INPUT (overlay — works alongside touch) ----
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))  g_input.move_x -= 1.0f;
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) g_input.move_x += 1.0f;
    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP))    g_input.move_y -= 1.0f;
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN))  g_input.move_y += 1.0f;

    if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_W)) g_input.jump = true;
    if (IsKeyDown(KEY_SPACE))   g_input.jump_held = true;

    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
        g_input.mine            = true;
        g_input.interact_screen = GetMousePosition();
        g_input.interact_valid  = true;
    }
    if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) {
        g_input.place           = true;
        g_input.interact_screen = GetMousePosition();
        g_input.interact_valid  = true;
    }

    if (IsKeyPressed(KEY_E) || IsKeyPressed(KEY_TAB)) g_input.inventory = !g_input.inventory;
    if (IsKeyPressed(KEY_ESCAPE)) g_input.pause = !g_input.pause;

    // Hotbar scroll
    float scroll = GetMouseWheelMove();
    if (scroll > 0) g_input.hotbar_delta = -1;
    if (scroll < 0) g_input.hotbar_delta =  1;

    // Number keys 1-9 for hotbar
    for (int k = KEY_ONE; k <= KEY_NINE; k++) {
        if (IsKeyPressed(k)) {
            // Will be handled by game loop
        }
    }

    // Clamp move
    if (g_input.move_x >  1.0f) g_input.move_x =  1.0f;
    if (g_input.move_x < -1.0f) g_input.move_x = -1.0f;
}

// ============================================================
// Render Virtual Controls — MD3 aesthetics
// ============================================================
void input_draw_virtual(void) {
    if (!g_input.is_touch) return;

    VirtualJoystick* j = &g_input.joy;

    // Joystick outer ring — semi-transparent
    DrawCircleV(j->center, JOYSTICK_RADIUS,     (Color){255,255,255, 40});
    DrawCircleLines((int)j->center.x, (int)j->center.y,
                    JOYSTICK_RADIUS, (Color){255,255,255,100});

    // Joystick knob — MD3 primary color
    Color knob_col = j->active ? (Color){100,200,255,200} : (Color){200,200,200,150};
    DrawCircleV(j->knob, JOYSTICK_KNOB_RADIUS, knob_col);

    // Action buttons
    for (int b = 0; b < BTN_COUNT; b++) {
        TouchButton* btn = &g_input.buttons[b];
        Color bg  = btn->held
            ? (Color){100,200,255,220}  // MD3 primary
            : (Color){30,30,30,160};    // MD3 surface
        Color rim = btn->held
            ? (Color){150,230,255,255}
            : (Color){180,180,180,120};

        DrawCircleV(btn->pos, btn->radius, bg);
        DrawCircleLines((int)btn->pos.x, (int)btn->pos.y, btn->radius, rim);
        DrawText(btn->label,
            (int)(btn->pos.x - btn->radius * 0.35f),
            (int)(btn->pos.y - btn->radius * 0.45f),
            (int)(btn->radius * 0.8f), WHITE);
    }
}

Vector2 input_screen_to_world(Vector2 screen, Vector2 camera_offset, float block_size) {
    return (Vector2){
        (screen.x + camera_offset.x) / block_size,
        (screen.y + camera_offset.y) / block_size
    };
}
