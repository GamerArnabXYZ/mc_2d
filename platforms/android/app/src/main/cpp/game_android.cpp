#include <jni.h>
#include <android/log.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>

#define LOG_TAG "MCClone"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

#include "../../../include/core/mc_types.h"
#include "../../../include/core/mc_world.h"
#include "../../../include/core/mc_texture.h"
#include "../../../include/core/mc_input.h"
#include "../../../include/core/mc_player.h"
#include "../../../include/core/mc_physics.h"

// Android game state
static struct {
    MC_World world;
    MC_Player player;
    MC_TextureCache textures;
    MC_Input input;
    MC_PhysicsWorld physics;

    int screen_width;
    int screen_height;
    bool initialized;

    float joystick_x, joystick_y;
    bool jump_pressed, attack_pressed;

} g_game;

void game_init_android(void) {
    LOGI("Initializing game systems...");

    // Initialize world
    mc_world_init(&g_game.world, MC_SEED);

    // Initialize textures with fallbacks
    mc_texture_init(&g_game.textures);

    // Initialize player
    mc_player_init(&g_game.player);

    // Position player at spawn
    i32 spawn_x = 128;
    i32 spawn_y = mc_world_get_height(&g_game.world, spawn_x);
    g_game.player.position.x = (f32)spawn_x * MC_TILE_SIZE;
    g_game.player.position.y = (f32)(spawn_y - 2) * MC_TILE_SIZE;

    // Initialize physics
    mc_physics_init(&g_game.physics);

    // Initialize input
    mc_input_init(&g_game.input);

    g_game.initialized = true;
    g_game.joystick_x = 0;
    g_game.joystick_y = 0;

    LOGI("Game initialization complete");
}

void game_resize_android(int width, int height) {
    g_game.screen_width = width;
    g_game.screen_height = height;
    LOGI("Game resize: %dx%d", width, height);
}

void game_render_android(void) {
    if (!g_game.initialized) return;

    // Process input
    g_game.input.state.joystick_x = g_game.joystick_x;
    g_game.input.state.joystick_y = g_game.joystick_y;
    g_game.input.state.touch_jump = g_game.jump_pressed;
    g_game.input.state.attack = g_game.attack_pressed;

    // Update player
    f32 move_x = g_game.input.state.joystick_x;
    g_game.player.velocity.x = move_x * g_game.player.move_speed;

    if (move_x > 0) g_game.player.facing = 1;
    else if (move_x < 0) g_game.player.facing = -1;

    if (g_game.jump_pressed && g_game.player.on_ground) {
        g_game.player.velocity.y = -g_game.player.jump_force;
        g_game.player.on_ground = false;
    }

    // Apply physics
    mc_physics_step(&g_game.physics, &g_game.player, 1.0f / 60.0f);

    // Update world
    mc_world_update(&g_game.world,
                    g_game.player.position.x,
                    g_game.player.position.y);

    // Render world (simplified - full OpenGL rendering would be here)
    // In full implementation, use OpenGL ES 2.0 to render tiles
}

void game_set_input_android(float jx, float jy, int jump, int attack) {
    g_game.joystick_x = jx;
    g_game.joystick_y = jy;
    g_game.jump_pressed = jump != 0;
    g_game.attack_pressed = attack != 0;
}