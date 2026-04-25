#include "core/mc_types.h"
#include "core/mc_world.h"
#include "core/mc_texture.h"
#include "core/mc_input.h"
#include "core/mc_camera.h"
#include "core/mc_physics.h"
#include "core/mc_player.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Screen settings
#define SCREEN_WIDTH    800
#define SCREEN_HEIGHT   600
#define TARGET_FPS      60
#define GAME_TITLE      "MC Clone - Infinite 2D World"

// Global game state
static struct {
    MC_World world;
    MC_Player player;
    MC_Camera camera;
    MC_TextureCache textures;
    MC_Input input;
    MC_PhysicsWorld physics;
    bool running;
    f32 delta_time;
} g_game = {0};

// Initialize all game systems
static bool init_systems(void) {
    printf("[INFO] Initializing game systems...\n");

    // Initialize Raylib window
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, GAME_TITLE);
    SetTargetFPS(TARGET_FPS);

    // Initialize input handler
    mc_input_init(&g_game.input);

    // Initialize texture system with embedded fallbacks
    if (!mc_texture_init(&g_game.textures)) {
        printf("[ERROR] Texture system initialization failed\n");
        return false;
    }
    printf("[OK] Textures loaded with fallback support\n");

    // Initialize procedural world
    if (!mc_world_init(&g_game.world, MC_SEED)) {
        printf("[ERROR] World initialization failed\n");
        return false;
    }
    printf("[OK] Infinite world initialized (seed: %d)\n", MC_SEED);

    // Initialize player at spawn
    mc_player_init(&g_game.player);

    // Find ground level at spawn position
    i32 spawn_x = 128;
    i32 spawn_y = mc_world_get_height(&g_game.world, spawn_x);
    g_game.player.position.x = (f32)spawn_x * MC_TILE_SIZE;
    g_game.player.position.y = (f32)(spawn_y - 2) * MC_TILE_SIZE;
    g_game.player.bounds.x = g_game.player.position.x;
    g_game.player.bounds.y = g_game.player.position.y;

    // Initialize camera
    mc_camera_init(&g_game.camera, SCREEN_WIDTH, SCREEN_HEIGHT);
    mc_camera_set_position(&g_game.camera, g_game.player.position.x, g_game.player.position.y);

    // Initialize physics
    mc_physics_init(&g_game.physics);

    g_game.running = true;
    printf("[SUCCESS] All systems initialized\n\n");

    return true;
}

// Main update loop
static void update(void) {
    // Process input
    mc_input_update(&g_game.input);

    // Player movement
    f32 move_dir = mc_input_horizontal(&g_game.input);
    g_game.player.velocity.x = move_dir * g_game.player.move_speed;

    if (move_dir > 0) g_game.player.facing = 1;
    else if (move_dir < 0) g_game.player.facing = -1;

    // Jumping
    if (g_game.input.state.jump && g_game.player.on_ground) {
        g_game.player.velocity.y = -g_game.player.jump_force;
        g_game.player.on_ground = false;
    }

    // Physics simulation
    mc_physics_step(&g_game.physics, &g_game.player, g_game.delta_time);

    // Update world chunks
    mc_world_update(&g_game.world, g_game.player.position.x, g_game.player.position.y);

    // Camera follow
    mc_camera_follow(&g_game.camera, &g_game.player, g_game.delta_time);
}

// Main render loop
static void render(void) {
    // Sky background
    ClearBackground((Color){ 135, 206, 235, 255 });

    // Apply camera transform
    mc_camera_apply(&g_game.camera);

    // Render world tiles
    mc_world_render(&g_game.world, &g_game.camera, &g_game.textures);

    // Render player
    mc_player_render(&g_game.player, &g_game.textures, &g_game.camera);

    // End camera transform
    mc_camera_reset(&g_game.camera);

    // HUD - Health bar
    DrawRectangle(10, 10, 160, 24, (Color){ 0, 0, 0, 180 });
    f32 health_pct = (f32)g_game.player.health / (f32)g_game.player.max_health;
    DrawRectangle(12, 12, (i32)(156 * health_pct), 20, (Color){ 200, 50, 50, 255 });
    DrawText("❤", 15, 12, 16, WHITE);

    // FPS counter
    DrawFPS(SCREEN_WIDTH - 80, 10);

    // Position debug
    #ifdef DEBUG
    DrawText(TextFormat("X: %.0f  Y: %.0f",
        g_game.player.position.x, g_game.player.position.y), 10, 40, 14, WHITE);
    DrawText(TextFormat("Chunks: %d", g_game.world.chunk_count), 10, 58, 14, WHITE);
    DrawText(TextFormat("V: %.0f, %.0f",
        g_game.player.velocity.x, g_game.player.velocity.y), 10, 76, 14, WHITE);
    #endif
}

// Cleanup
static void cleanup(void) {
    printf("\n[INFO] Shutting down...\n");

    mc_world_shutdown(&g_game.world);
    mc_texture_shutdown(&g_game.textures);
    CloseWindow();

    printf("[SUCCESS] Cleanup complete\n");
}

// Game loop function for Emscripten
#ifdef __EMSCRIPTEN__
static void emscripten_loop(void) {
    if (!g_game.running) return;

    g_game.delta_time = GetFrameTime();
    update();
    render();

    if (WindowShouldClose()) {
        g_game.running = false;
    }
}
#endif

// Main entry point
int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    printf("╔══════════════════════════════════════╗\n");
    printf("║   MC CLONE - INFINITE 2D WORLD       ║\n");
    printf("║   C99 + Raylib | Zero-Crash Engine   ║\n");
    printf("╚══════════════════════════════════════╝\n\n");

    if (!init_systems()) {
        printf("[FATAL] System initialization failed!\n");
        return 1;
    }

    #ifdef __EMSCRIPTEN__
        printf("[INFO] Running on Web/Emscripten platform\n");
        emscripten_set_main_loop(emscripten_loop, TARGET_FPS, 1);
    #else
        printf("[INFO] Running on Desktop/Android platform\n");
        printf("[INFO] Controls: A/D or Arrow Keys to move, Space to jump\n\n");

        while (g_game.running && !WindowShouldClose()) {
            g_game.delta_time = GetFrameTime();
            update();
            render();
        }
    #endif

    cleanup();
    return 0;
}