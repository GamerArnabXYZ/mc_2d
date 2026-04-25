/**
 * @file main.c
 * @brief MC Clone - 2D Side-Scrolling Infinite World Engine
 * @author MiniMax Agent
 *
 * Main entry point for the game engine.
 * Handles initialization, game loop, and platform-specific code.
 */

#include "core/mc_types.h"
#include "core/mc_world.h"
#include "core/mc_player.h"
#include "core/mc_camera.h"
#include "core/mc_texture.h"
#include "core/mc_input.h"
#include "core/mc_physics.h"
#include "core/mc_game.h"
#include <stdlib.h>
#include <stdio.h>

// Game configuration
#define SCREEN_WIDTH    800
#define SCREEN_HEIGHT   600
#define TARGET_FPS      60

// Global game state
typedef struct {
    MC_World world;
    MC_Player player;
    MC_Camera camera;
    MC_TextureCache textures;
    MC_Input input;
    MC_PhysicsWorld physics;
    bool running;
    f32 delta_time;
    u32 frame_count;
} MC_GameState;

static MC_GameState g_game = { 0 };

// Initialize game systems
static bool game_init(void) {
    // Initialize raylib
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "MC Clone - Infinite 2D World");
    SetTargetFPS(TARGET_FPS);

    // Initialize input system
    mc_input_init(&g_game.input);

    // Initialize texture system (with fallback)
    if (!mc_texture_init(&g_game.textures)) {
        printf("[ERROR] Failed to initialize texture system\n");
        return false;
    }

    // Initialize world with seed
    if (!mc_world_init(&g_game.world, MC_SEED)) {
        printf("[ERROR] Failed to initialize world\n");
        return false;
    }

    // Initialize player
    mc_player_init(&g_game.player);

    // Place player at spawn point
    i32 spawn_x = 128;  // World tile X
    i32 spawn_y = g_game.world.get_height(&g_game.world, spawn_x) - 2;
    g_game.player.position.x = (f32)spawn_x * MC_TILE_SIZE;
    g_game.player.position.y = (f32)spawn_y * MC_TILE_SIZE;
    g_game.player.bounds.x = g_game.player.position.x;
    g_game.player.bounds.y = g_game.player.position.y;

    // Initialize camera
    mc_camera_init(&g_game.camera, SCREEN_WIDTH, SCREEN_HEIGHT);

    // Initialize physics
    mc_physics_init(&g_game.physics);

    g_game.running = true;
    g_game.frame_count = 0;

    printf("[INFO] MC Clone initialized successfully\n");
    printf("[INFO] World seed: %d\n", MC_SEED);
    printf("[INFO] Target FPS: %d\n", TARGET_FPS);

    return true;
}

// Update game logic
static void game_update(void) {
    // Update input state
    mc_input_update(&g_game.input);

    // Handle player movement
    f32 move_x = mc_input_horizontal(&g_game.input);

    if (move_x < 0) {
        g_game.player.velocity.x = -g_game.player.move_speed;
        g_game.player.facing = -1;
    } else if (move_x > 0) {
        g_game.player.velocity.x = g_game.player.move_speed;
        g_game.player.facing = 1;
    } else {
        g_game.player.velocity.x = 0;
    }

    // Handle jumping
    if (g_game.input.state.jump && g_game.player.on_ground) {
        g_game.player.velocity.y = -g_game.player.jump_force;
        g_game.player.on_ground = false;
        g_game.player.is_jumping = true;
    }

    // Apply physics
    mc_physics_step(&g_game.physics, &g_game.player, g_game.delta_time);

    // Update world chunks based on player position
    mc_world_update(&g_game.world,
                    g_game.player.position.x,
                    g_game.player.position.y);

    // Update camera to follow player
    mc_camera_follow(&g_game.camera, &g_game.player, g_game.delta_time);

    // Handle pause
    if (IsKeyPressed(KEY_ESCAPE)) {
        // Toggle pause state
    }
}

// Render game
static void game_render(void) {
    // Clear screen with sky color
    ClearBackground((Color){ 135, 206, 235, 255 });  // Sky blue

    // Apply camera transform
    mc_camera_apply(&g_game.camera);

    // Render world tiles
    mc_world_render(&g_game.world, &g_game.camera, &g_game.textures);

    // Render player
    mc_player_render(&g_game.player, &g_game.textures, &g_game.camera);

    // Reset camera
    mc_camera_reset(&g_game.camera);

    // Render UI (HUD)
    DrawRectangle(10, 10, 150, 40, (Color){ 0, 0, 0, 128 });  // Health bar background
    DrawRectangle(12, 12, (i32)(146 * (f32)g_game.player.health / g_game.player.max_health), 36, RED);

    // Render debug info
    #ifdef DEBUG
    DrawText(TextFormat("Pos: %.1f, %.1f", g_game.player.position.x, g_game.player.position.y), 10, 60, 12, WHITE);
    DrawText(TextFormat("Chunks: %d", g_game.world.chunk_count), 10, 75, 12, WHITE);
    DrawText(TextFormat("FPS: %d", GetFPS()), 10, 90, 12, WHITE);
    #endif
}

// Shutdown game systems
static void game_shutdown(void) {
    mc_world_shutdown(&g_game.world);
    mc_texture_shutdown(&g_game.textures);
    CloseWindow();

    printf("[INFO] MC Clone shutdown complete\n");
}

// Main game loop (Desktop/Desktop/Web)
static void game_loop(void) {
    g_game.delta_time = GetFrameTime();

    // Update
    game_update();

    // Render
    game_render();

    // Check for exit
    if (WindowShouldClose()) {
        g_game.running = false;
    }
}

// Web-specific main
#ifdef __EMSCRIPTEN__
static void emscripten_main_loop(void) {
    if (g_game.running) {
        game_loop();
    }
}
#endif

// Main entry point
int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    printf("===========================================\n");
    printf("   MC Clone - Infinite 2D World Engine\n");
    printf("   C99 + Raylib | Zero-Crash Design\n");
    printf("===========================================\n\n");

    if (!game_init()) {
        printf("[FATAL] Game initialization failed\n");
        return 1;
    }

    #ifdef __EMSCRIPTEN__
        // Web: Use emscripten_set_main_loop
        emscripten_set_main_loop(emscripten_main_loop, TARGET_FPS, 1);
    #else
        // Desktop/Android: Standard game loop
        while (g_game.running) {
            game_loop();
        }
    #endif

    game_shutdown();

    return 0;
}