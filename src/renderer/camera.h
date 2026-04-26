#pragma once
// camera.h - 2D Camera System
// Smooth follow, parallax-ready, screen shake, zoom

#include "raylib.h"
#include <stdbool.h>

typedef struct {
    Vector2 position;       // Top-left world position (pixels)
    Vector2 target_pos;     // Where camera wants to be
    float   zoom;
    float   follow_speed;   // Lerp factor (higher = snappier)

    // Screen shake
    float   shake_duration;
    float   shake_intensity;
    Vector2 shake_offset;

    // Bounds (world limits in blocks, 0 = unlimited)
    float   bound_left;
    float   bound_right;
    int     screen_w;
    int     screen_h;
} GameCamera;

extern GameCamera g_camera;

void    camera_init(int screen_w, int screen_h);
void    camera_update(float dt, float target_world_x, float target_world_y, float block_size);
void    camera_shake(float duration, float intensity);
void    camera_resize(int screen_w, int screen_h);

// Coordinate helpers
Vector2 camera_world_to_screen(float wx, float wy, float block_size);
Vector2 camera_screen_to_world(float sx, float sy, float block_size);

// Get raylib Camera2D compatible struct for BeginMode2D (optional)
Camera2D camera_get_raylib(float block_size);
