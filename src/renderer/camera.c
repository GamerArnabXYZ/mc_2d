// camera.c - 2D Camera: Smooth Follow + Screen Shake
#include <stdint.h>
#include "camera.h"
#include <math.h>
#include <string.h>
#include <stdlib.h>

GameCamera g_camera = {0};

// Simple LCG for shake randomness (no srand dependency)
static uint32_t s_rng = 0xCAFEBABE;
static float rng_f(void) {
    s_rng = s_rng * 1664525u + 1013904223u;
    return (float)(s_rng >> 16) / 65535.0f * 2.0f - 1.0f; // -1..+1
}

void camera_init(int screen_w, int screen_h) {
    memset(&g_camera, 0, sizeof(GameCamera));
    g_camera.zoom          = 1.0f;
    g_camera.follow_speed  = 8.0f;
    g_camera.screen_w      = screen_w;
    g_camera.screen_h      = screen_h;
}

void camera_update(float dt, float target_wx, float target_wy, float block_size) {
    // Target = center player on screen
    float tx = target_wx * block_size - g_camera.screen_w  * 0.5f;
    float ty = target_wy * block_size - g_camera.screen_h  * 0.55f; // Slightly above center

    // Lerp smooth follow
    float t = 1.0f - expf(-g_camera.follow_speed * dt); // Exponential decay
    g_camera.position.x += (tx - g_camera.position.x) * t;
    g_camera.position.y += (ty - g_camera.position.y) * t;

    // Hard clamp to world bounds if set
    if (g_camera.bound_right > g_camera.bound_left) {
        float max_x = g_camera.bound_right * block_size - g_camera.screen_w;
        if (g_camera.position.x < g_camera.bound_left * block_size) g_camera.position.x = g_camera.bound_left * block_size;
        if (g_camera.position.x > max_x) g_camera.position.x = max_x;
    }

    // Screen shake
    if (g_camera.shake_duration > 0.0f) {
        g_camera.shake_duration -= dt;
        float power = g_camera.shake_duration * g_camera.shake_intensity;
        g_camera.shake_offset.x = rng_f() * power;
        g_camera.shake_offset.y = rng_f() * power;
        if (g_camera.shake_duration < 0.0f) {
            g_camera.shake_duration = 0.0f;
            g_camera.shake_offset   = (Vector2){0, 0};
        }
    }
}

void camera_shake(float duration, float intensity) {
    // Additive shake — don't reset if already shaking harder
    if (intensity > g_camera.shake_intensity || g_camera.shake_duration <= 0) {
        g_camera.shake_duration  = duration;
        g_camera.shake_intensity = intensity;
    }
}

void camera_resize(int screen_w, int screen_h) {
    g_camera.screen_w = screen_w;
    g_camera.screen_h = screen_h;
}

Vector2 camera_world_to_screen(float wx, float wy, float block_size) {
    return (Vector2){
        wx * block_size - g_camera.position.x + g_camera.shake_offset.x,
        wy * block_size - g_camera.position.y + g_camera.shake_offset.y
    };
}

Vector2 camera_screen_to_world(float sx, float sy, float block_size) {
    return (Vector2){
        (sx + g_camera.position.x - g_camera.shake_offset.x) / block_size,
        (sy + g_camera.position.y - g_camera.shake_offset.y) / block_size
    };
}

Camera2D camera_get_raylib(float block_size) {
    (void)block_size;
    return (Camera2D){
        .offset   = (Vector2){0, 0},
        .target   = (Vector2){
            g_camera.position.x - g_camera.shake_offset.x,
            g_camera.position.y - g_camera.shake_offset.y
        },
        .rotation = 0.0f,
        .zoom     = g_camera.zoom
    };
}
