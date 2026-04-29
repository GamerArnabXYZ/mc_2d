#pragma once
#include "SDL_incl.h"
#include "../core/Timer.h"
#include "../world/World.h"
#include "../player/Player.h"
#include "../renderer/Renderer.h"
#include "../renderer/Camera.h"
#include "../input/InputManager.h"
#include "../save/SaveManager.h"
#include "../ui/CraftingUI.h"

// ─── Game ─────────────────────────────────────────────────────────────────────
class Game {
public:
    Game();
    ~Game();

    bool init();
    void run();      // desktop: blocking loop; web: sets emscripten callback
    void shutdown();

    // Called once per frame — used by emscripten callback
    void tickFrame();

private:
    SDL_Window*   m_win;
    Timer         m_timer;
    World         m_world;
    Player        m_player;
    Renderer      m_renderer;
    Camera        m_camera;
    InputManager  m_input;
    SaveManager   m_save;
    CraftingUI    m_craftUI;

    bool          m_running;
    float         m_saveTimer;

    void update(float dt);
    void processInput();
    void autoSave(float dt);
};
