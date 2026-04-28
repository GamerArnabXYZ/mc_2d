#include "Game.h"
#include "Config.h"
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <cmath>
#include <cstdio>

#ifdef __EMSCRIPTEN__
  #include <emscripten.h>
  // Emscripten needs a static loop callback
  static Game* g_game = nullptr;
  static void emLoop() { if (g_game) g_game->run(); }
#endif

// ─── Constructor / Destructor ─────────────────────────────────────────────────
Game::Game()
    : m_win(nullptr)
    , m_world(WORLD_SEED_DEFAULT)
    , m_running(false)
    , m_saveTimer(0.0f)
{}

Game::~Game() { shutdown(); }

// ─── Init ─────────────────────────────────────────────────────────────────────
bool Game::init() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_TIMER) != 0) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return false;
    }

    IMG_Init(IMG_INIT_PNG);
    TTF_Init();

    Uint32 flags = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE;
#ifdef PLATFORM_ANDROID
    flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
#endif

    m_win = SDL_CreateWindow(
        WINDOW_TITLE,
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_W, WINDOW_H, flags
    );
    if (!m_win) {
        SDL_Log("Window creation failed: %s", SDL_GetError());
        return false;
    }

    if (!m_renderer.init(m_win)) {
        SDL_Log("Renderer init failed");
        return false;
    }

    // Generate initial world around spawn
    m_world.ensureChunksAround(WORLD_CHUNKS / 2);

    // Load save if exists
    if (m_save.saveExists()) {
        m_save.loadWorld(m_world);
        m_save.loadPlayer(m_player);
    }

    m_running = true;
    SDL_Log("CraftSDL initialized OK");
    return true;
}

// ─── Main Loop ────────────────────────────────────────────────────────────────
void Game::run() {
#ifdef __EMSCRIPTEN__
    // Emscripten: set loop callback and return immediately
    g_game = this;
    emscripten_set_main_loop(emLoop, 0, 1);
    return;
#endif

    while (m_running) {
        m_timer.tick();
        float dt = m_timer.getDelta();

        if (!m_input.processEvents()) {
            m_running = false;
            break;
        }

        processInput();
        update(dt);
        m_renderer.render(m_world, m_player, m_camera, m_timer.getFPS(), &m_craftUI, &m_input);
        autoSave(dt);
    }
}

// ─── Input → Game Commands ────────────────────────────────────────────────────
void Game::processInput() {
    // Movement
    m_player.setMoveX(m_input.moveX());

    // Jump
    if (m_input.jumpPressed()) m_player.jump();

    // Inventory
    if (m_input.openInventory()) m_player.toggleInventory();

    // Slot selection
    int scroll = m_input.slotScroll();
    if (scroll != 0) m_player.scrollSlot(scroll);
    int direct = m_input.hotbarDirect();
    if (direct >= 0) m_player.selectSlot(direct);

    // ── Block targeting: screen → world → block ───────────────────────────────
    int tsx = m_input.targetScreenX();
    int tsy = m_input.targetScreenY();
    int tbx = m_camera.screenToBlockX(tsx);
    int tby = m_camera.screenToBlockY(tsy);

    // Check reach from player center
    float pcx = m_player.x() + PLAYER_W * 0.5f;
    float pcy = m_player.y() + PLAYER_H * 0.5f;
    float tpx = (float)(tbx * BLOCK_SIZE) + BLOCK_SIZE * 0.5f;
    float tpy = (float)(tby * BLOCK_SIZE) + BLOCK_SIZE * 0.5f;
    float dist = sqrtf((tpx - pcx) * (tpx - pcx) + (tpy - pcy) * (tpy - pcy));

    if (m_input.isBreaking() && dist <= BREAK_REACH) {
        m_player.startBreak(tbx, tby);
    } else {
        m_player.stopBreak();
    }

    if (m_input.placedThisFrame() && dist <= PLACE_REACH) {
        m_player.placeBlock(tbx, tby, m_world);
    }
}

// ─── Update ───────────────────────────────────────────────────────────────────
void Game::update(float dt) {
    // Ensure chunks around player
    m_world.ensureChunksAround(m_player.chunkX());

    // Update world (day-night cycle)
    m_world.update(dt);

    // Update player physics
    m_player.update(dt, m_world);

    // Update camera → follow player center
    float camTargX = m_player.x() + PLAYER_W * 0.5f;
    float camTargY = m_player.y() + PLAYER_H * 0.5f;
    m_camera.update(camTargX, camTargY, dt);
}

// ─── Auto-save ────────────────────────────────────────────────────────────────
void Game::autoSave(float dt) {
    m_saveTimer += dt;
    if (m_saveTimer >= 60.0f) {
        m_saveTimer = 0.0f;
        m_save.saveWorld(m_world);
        m_save.savePlayer(m_player);
        SDL_Log("Auto-saved.");
    }
}

// ─── Shutdown ─────────────────────────────────────────────────────────────────
void Game::shutdown() {
    if (m_running) {
        m_save.saveWorld(m_world);
        m_save.savePlayer(m_player);
    }
    m_renderer.shutdown();
    if (m_win) { SDL_DestroyWindow(m_win); m_win = nullptr; }
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}
