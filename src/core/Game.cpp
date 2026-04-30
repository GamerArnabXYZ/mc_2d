#include "Game.h"
#include "Config.h"
#if __has_include(<SDL2/SDL_image.h>)
  #include <SDL2/SDL_image.h>
#elif __has_include(<SDL_image.h>)
  #include <SDL_image.h>
#endif
#if __has_include(<SDL2/SDL_ttf.h>)
  #include <SDL2/SDL_ttf.h>
#elif __has_include(<SDL_ttf.h>)
  #include <SDL_ttf.h>
#endif
#include <cmath>
#include <cstdio>

#ifdef __EMSCRIPTEN__
  #include <emscripten.h>
  static Game* g_game = nullptr;
  static void emTick() { if (g_game) g_game->tickFrame(); }
#endif

Game::Game()
    : m_win(nullptr)
    , m_world(WORLD_SEED_DEFAULT)
    , m_state(GameState::HOME)
    , m_running(false)
    , m_saveTimer(0)
    , m_screenW(WINDOW_W), m_screenH(WINDOW_H)
{}

Game::~Game() { shutdown(); }

// ─── Get actual window drawable size ─────────────────────────────────────────
void Game::getScreenSize() {
    SDL_GL_GetDrawableSize(m_win, &m_screenW, &m_screenH);
    if (m_screenW <= 0) SDL_GetWindowSize(m_win, &m_screenW, &m_screenH);
    m_input.setScreenSize(m_screenW, m_screenH);
    SDL_Log("Screen size: %d x %d", m_screenW, m_screenH);
}

// ─── Init ─────────────────────────────────────────────────────────────────────
bool Game::init() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_TIMER) != 0) return false;
    IMG_Init(IMG_INIT_PNG);
    TTF_Init();

    Uint32 flags = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI;
#ifdef PLATFORM_ANDROID
    flags = SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_ALLOW_HIGHDPI;
#endif

    m_win = SDL_CreateWindow(WINDOW_TITLE,
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_W, WINDOW_H, flags);
    if (!m_win) return false;

    if (!m_renderer.init(m_win)) return false;

    getScreenSize();

    // Set logical render size so everything scales perfectly on any screen
    SDL_RenderSetLogicalSize(m_renderer.sdl(), WINDOW_W, WINDOW_H);

    // Generate world
    m_world.ensureChunksAround(WORLD_CHUNKS / 2);

    if (m_save.saveExists()) {
        m_save.loadWorld(m_world);
        m_save.loadPlayer(m_player);
    }

    m_running = true;
    SDL_Log("Game init OK — showing home screen");
    return true;
}

// ─── run() ────────────────────────────────────────────────────────────────────
void Game::run() {
#ifdef __EMSCRIPTEN__
    g_game = this;
    emscripten_set_main_loop(emTick, 0, 1);
#else
    while (m_running) {
        tickFrame();
    }
#endif
}

// ─── tickFrame: one frame ─────────────────────────────────────────────────────
void Game::tickFrame() {
    m_timer.tick();
    float dt = m_timer.getDelta();

    if (!m_input.processEvents(m_state)) {
        m_running = false;
#ifdef __EMSCRIPTEN__
        emscripten_cancel_main_loop();
#endif
        return;
    }

    switch (m_state) {
        case GameState::HOME:    tickHome(dt);    break;
        case GameState::PLAYING: tickPlaying(dt); break;
        default: break;
    }
}

// ─── Home screen tick ─────────────────────────────────────────────────────────
void Game::tickHome(float dt) {
    (void)dt;
    renderHome();
    if (m_input.startPressed()) {
        m_state = GameState::PLAYING;
        // Spawn player now
        m_world.ensureChunksAround(WORLD_CHUNKS / 2);
        m_player.spawnOnSurface(m_world);
    }
}

// ─── Playing tick ─────────────────────────────────────────────────────────────
void Game::tickPlaying(float dt) {
    // Handle inv tap separately
    if (m_player.inventory().isOpen()) {
        int tx, ty;
        if (m_input.invTapThisFrame(tx, ty)) {
            m_craftUI.handleClick(tx, ty, false, m_player.inventory());
        }
        // Also handle 'E' or inv button to close
        if (m_input.openInventory()) {
            m_player.inventory().setOpen(false);
        }
    } else {
        processInput();
        update(dt);
    }

    // Render
    m_renderer.render(m_world, m_player, m_camera,
                      m_timer.getFPS(), &m_craftUI, &m_input);
    autoSave(dt);
}

// ─── Home screen render ───────────────────────────────────────────────────────
void Game::renderHome() {
    SDL_Renderer* rend = m_renderer.sdl();

    // Background gradient (deep night sky)
    SDL_SetRenderDrawColor(rend, 8, 12, 35, 255);
    SDL_RenderClear(rend);

    // Stars
    srand(42);
    SDL_SetRenderDrawColor(rend, 255, 255, 255, 160);
    for (int i = 0; i < 80; i++) {
        int sx = rand() % WINDOW_W;
        int sy = rand() % (WINDOW_H / 2);
        SDL_Rect star = {sx, sy, (i%3==0)?2:1, (i%3==0)?2:1};
        SDL_RenderFillRect(rend, &star);
    }

    // Ground silhouette
    SDL_SetRenderDrawColor(rend, 30, 80, 30, 255);
    SDL_Rect ground = {0, WINDOW_H*3/4, WINDOW_W, WINDOW_H/4};
    SDL_RenderFillRect(rend, &ground);

    // Block decorations (mini blocks on ground)
    struct { int x; uint8_t id; } decos[] = {
        {20, BLOCK_WOOD},{21, BLOCK_WOOD},{20, BLOCK_LEAVES},
        {19, BLOCK_LEAVES},{21, BLOCK_LEAVES},
        {WINDOW_W-60, BLOCK_STONE},{WINDOW_W-59, BLOCK_STONE},
        {WINDOW_W-60, BLOCK_COAL_ORE},
    };
    int bs = 28;
    int groundY = WINDOW_H*3/4 - bs;
    for (auto& d : decos) {
        SDL_Color c;
        switch(d.id) {
            case BLOCK_WOOD:    c={140,100,50,255}; break;
            case BLOCK_LEAVES:  c={45,130,35,200};  break;
            case BLOCK_STONE:   c={130,130,130,255};break;
            case BLOCK_COAL_ORE:c={70,70,70,255};   break;
            default:            c={100,100,100,255};
        }
        SDL_SetRenderDrawColor(rend, c.r,c.g,c.b,c.a);
        SDL_Rect br = {d.x*bs, groundY, bs, bs};
        SDL_RenderFillRect(rend, &br);
        SDL_SetRenderDrawColor(rend,0,0,0,40);
        SDL_RenderDrawRect(rend, &br);
    }

    // Title: "CRAFT SDL"
    // Big block letters using rectangles
    int ty = WINDOW_H / 5;
    // Title background pill
    SDL_SetRenderDrawColor(rend, 0, 0, 0, 140);
    SDL_Rect titleBg = {WINDOW_W/2 - 160, ty - 16, 320, 80};
    SDL_RenderFillRect(rend, &titleBg);

    // ⛏️ pickaxe icon (simple cross)
    SDL_SetRenderDrawColor(rend, 255, 200, 50, 255);
    SDL_Rect pkH = {WINDOW_W/2 - 30, ty + 10, 60, 10};
    SDL_Rect pkV = {WINDOW_W/2 - 5,  ty - 5,  10, 40};
    SDL_RenderFillRect(rend, &pkH);
    SDL_RenderFillRect(rend, &pkV);

    // Title text blocks (pixel font simulation)
    // "CraftSDL" using colored block strips
    SDL_SetRenderDrawColor(rend, 80, 200, 80, 255);
    SDL_Rect tLine1 = {WINDOW_W/2 - 100, ty + 55, 200, 6};
    SDL_RenderFillRect(rend, &tLine1);
    SDL_SetRenderDrawColor(rend, 60, 160, 60, 255);
    SDL_Rect tLine2 = {WINDOW_W/2 - 80, ty + 63, 160, 4};
    SDL_RenderFillRect(rend, &tLine2);

    // Subtitle box
    int subY = ty + 90;
    SDL_SetRenderDrawColor(rend, 0,0,0,120);
    SDL_Rect subBg = {WINDOW_W/2-130, subY-8, 260, 30};
    SDL_RenderFillRect(rend, &subBg);
    SDL_SetRenderDrawColor(rend, 100,220,100,200);
    SDL_RenderDrawRect(rend, &subBg);

    // "2D SURVIVAL SANDBOX" text strip
    SDL_SetRenderDrawColor(rend, 180, 255, 180, 200);
    SDL_Rect stxt = {WINDOW_W/2 - 90, subY, 180, 14};
    SDL_RenderFillRect(rend, &stxt);

    // START button
    int btnY = WINDOW_H / 2 + 20;
    float pulse = 0.5f + 0.5f * sinf(SDL_GetTicks() * 0.003f);
    uint8_t btnAlpha = (uint8_t)(180 + 75 * pulse);

    SDL_SetRenderDrawColor(rend, 50, 160, 50, btnAlpha);
    SDL_Rect btn = {WINDOW_W/2 - 120, btnY, 240, 64};
    SDL_RenderFillRect(rend, &btn);
    SDL_SetRenderDrawColor(rend, 100, 255, 100, 255);
    SDL_RenderDrawRect(rend, &btn);
    // Inner highlight
    SDL_SetRenderDrawColor(rend, 150, 255, 150, 60);
    SDL_Rect btnHL = {WINDOW_W/2-118, btnY+2, 236, 20};
    SDL_RenderFillRect(rend, &btnHL);

    // "PLAY" text in button (pixel blocks)
    SDL_SetRenderDrawColor(rend, 255, 255, 255, 255);
    int px = WINDOW_W/2 - 30, py = btnY + 22;
    // P
    SDL_Rect p1[]={{px,py,4,20},{px,py,14,4},{px,py+8,14,4},{px+10,py,4,12}};
    for (auto& r:p1) SDL_RenderFillRect(rend,&r);
    px+=18;
    // L
    SDL_Rect p2[]={{px,py,4,20},{px,py+16,14,4}};
    for (auto& r:p2) SDL_RenderFillRect(rend,&r);
    px+=18;
    // A
    SDL_Rect p3[]={{px+5,py,4,4},{px,py+4,4,16},{px+10,py+4,4,16},{px,py+8,14,4}};
    for (auto& r:p3) SDL_RenderFillRect(rend,&r);
    px+=18;
    // Y
    SDL_Rect p4[]={{px,py,4,8},{px+10,py,4,8},{px+5,py+8,4,12}};
    for (auto& r:p4) SDL_RenderFillRect(rend,&r);

    // Bottom hint
    SDL_SetRenderDrawColor(rend, 150, 150, 150, 160);
    SDL_Rect hint = {WINDOW_W/2-110, WINDOW_H-55, 220, 16};
    SDL_RenderFillRect(rend, &hint);

    // Version
    SDL_SetRenderDrawColor(rend, 80, 80, 80, 200);
    SDL_Rect ver = {4, WINDOW_H-18, 80, 12};
    SDL_RenderFillRect(rend, &ver);

    SDL_RenderPresent(rend);
}

// ─── processInput ─────────────────────────────────────────────────────────────
void Game::processInput() {
    m_player.setMoveX(m_input.moveX());
    if (m_input.jumpPressed()) m_player.jump();
    if (m_input.openInventory()) m_player.toggleInventory();

    int scroll = m_input.slotScroll();
    if (scroll) m_player.scrollSlot(scroll);
    int direct = m_input.hotbarDirect();
    if (direct >= 0) m_player.selectSlot(direct);

    // Block targeting: screen → world → block
    int tsx = m_input.targetScreenX();
    int tsy = m_input.targetScreenY();
    int tbx = m_camera.screenToBlockX(tsx);
    int tby = m_camera.screenToBlockY(tsy);

    // Distance in blocks
    float pcx = m_player.x() + PLAYER_W * 0.5f;
    float pcy = m_player.y() + PLAYER_H * 0.5f;
    float tpx = tbx * BLOCK_SIZE + BLOCK_SIZE * 0.5f;
    float tpy = tby * BLOCK_SIZE + BLOCK_SIZE * 0.5f;
    float dist = sqrtf((tpx-pcx)*(tpx-pcx)+(tpy-pcy)*(tpy-pcy)) / BLOCK_SIZE;

    if (m_input.isBreaking() && dist <= BREAK_REACH)
        m_player.startBreak(tbx, tby);
    else
        m_player.stopBreak();

    if (m_input.placedThisFrame() && dist <= PLACE_REACH)
        m_player.placeBlock(tbx, tby, m_world);
}

// ─── update ───────────────────────────────────────────────────────────────────
void Game::update(float dt) {
    m_world.ensureChunksAround(m_player.chunkX());
    m_world.update(dt);
    if (!m_player.isSpawned()) m_player.spawnOnSurface(m_world);
    m_player.update(dt, m_world);

    float cx = m_player.x() + PLAYER_W * 0.5f;
    float cy = m_player.y() + PLAYER_H * 0.5f;
    m_camera.update(cx, cy, dt);
}

void Game::autoSave(float dt) {
    m_saveTimer += dt;
    if (m_saveTimer >= 60.0f) {
        m_saveTimer = 0;
        m_save.saveWorld(m_world);
        m_save.savePlayer(m_player);
    }
}

void Game::shutdown() {
    if (m_running && m_state == GameState::PLAYING) {
        m_save.saveWorld(m_world);
        m_save.savePlayer(m_player);
    }
    m_running = false;
    m_renderer.shutdown();
    if (m_win) { SDL_DestroyWindow(m_win); m_win = nullptr; }
    TTF_Quit(); IMG_Quit(); SDL_Quit();
}
