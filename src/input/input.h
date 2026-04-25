#pragma once
// input.h - Unified Input System
// Handles: Keyboard/Mouse (Desktop), Touch (Android), Virtual Joystick (Web/Mobile)
// Material Design 3 touch targets: minimum 48x48dp

#include "raylib.h"
#include <stdbool.h>

// ============================================================
// Virtual Joystick (MD3 compliant)
// ============================================================
#define JOYSTICK_RADIUS       60.0f   // Outer circle radius (px)
#define JOYSTICK_KNOB_RADIUS  25.0f   // Inner knob radius
#define JOYSTICK_DEAD_ZONE    0.15f   // Normalized dead zone

typedef struct {
    Vector2 center;         // Fixed center position
    Vector2 knob;           // Current knob position
    Vector2 normalized;     // Output: -1..+1 in X and Y
    bool    active;         // Finger is touching
    int     touch_id;       // Which touch finger (-1 = none)
} VirtualJoystick;

// ============================================================
// Action Buttons (right side)
// ============================================================
typedef struct {
    Vector2 pos;            // Center position
    float   radius;
    bool    pressed;
    bool    held;
    bool    just_pressed;
    char    label[8];
} TouchButton;

#define BTN_JUMP     0
#define BTN_MINE     1
#define BTN_PLACE    2
#define BTN_INV      3
#define BTN_COUNT    4

// ============================================================
// Unified Input State — read each frame
// ============================================================
typedef struct {
    // Movement: -1 to +1
    float   move_x;     // Horizontal
    float   move_y;     // Vertical (for swimming)

    // Actions
    bool    jump;
    bool    jump_held;
    bool    mine;       // Left click / touch mine button
    bool    place;      // Right click / touch place button
    bool    inventory;  // Tab / inventory button
    bool    pause;      // Esc / pause

    // Block interaction: screen position of touch/click
    bool    interact_valid;
    Vector2 interact_screen; // Screen coords of mine/place action

    // Hotbar scroll
    int     hotbar_delta; // +1 or -1

    // Virtual joystick (for rendering)
    VirtualJoystick joy;
    TouchButton     buttons[BTN_COUNT];

    // Platform flags
    bool    is_touch;    // Mobile/web touch mode
    bool    is_keyboard; // Has physical keyboard
} InputState;

extern InputState g_input;

void input_init(int screen_w, int screen_h);
void input_update(int screen_w, int screen_h);  // Call every frame
void input_draw_virtual(void);                   // Render joystick + buttons

// Layout helper — repositions controls on resize
void input_layout(int screen_w, int screen_h);

// Convert screen coords to world block coords
Vector2 input_screen_to_world(Vector2 screen, Vector2 camera_offset, float block_size);
