#ifndef MC2D_INPUT_H
#define MC2D_INPUT_H

#include "../mc2d_types.h"
#include "raylib.h"

/* ─── Virtual joystick config ────────────────────────────────── */
#define JOY_CENTER_X_FRAC  0.12f   /* fraction of screen width  */
#define JOY_CENTER_Y_FRAC  0.75f   /* fraction of screen height */
#define JOY_OUTER_RADIUS   60.0f
#define JOY_INNER_RADIUS   25.0f

/* ─── Action button layout (right side) ─────────────────────── */
#define BTN_SIZE           60.0f   /* Material Design 3 touch target */
#define BTN_JUMP_X_FRAC    0.90f
#define BTN_JUMP_Y_FRAC    0.72f
#define BTN_DIG_X_FRAC     0.82f
#define BTN_DIG_Y_FRAC     0.85f
#define BTN_PLACE_X_FRAC   0.90f
#define BTN_PLACE_Y_FRAC   0.85f

typedef struct {
    /* Touch tracking */
    int   joy_touch_id;    /* -1 = inactive */
    float joy_base_x;
    float joy_base_y;
    bool  is_touch_device;

    /* Derived each frame */
    InputState state;

    /* Screen dims for layout */
    float sw, sh;
} InputCtx;

InputCtx* input_create(float sw, float sh);
void      input_destroy(InputCtx* ic);
void      input_update(InputCtx* ic);
void      input_draw_hud(const InputCtx* ic);   /* draws joystick + buttons */
void      input_resize(InputCtx* ic, float sw, float sh);

#endif /* MC2D_INPUT_H */
