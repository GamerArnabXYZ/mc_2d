#pragma once
#include <SDL2/SDL.h>
#include "Timer.h"
#include "../world/World.h"
#include "../player/Player.h"
#include "../renderer/Renderer.h"
#include "../renderer/Camera.h"
#include "../input/InputManager.h"
#include "../save/SaveManager.h"
#include "../ui/CraftingUI.h"

class Game {
public:
    Game();
    ~Game();

    bool init();
    void run();
    void shutdown();
    void tickFrame(); // emscripten per-frame callback

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

    GameState     m_state;
    bool          m_running;
    float         m_saveTimer;

    // Actual render size (may differ from WINDOW_W/H on mobile)
    int           m_screenW, m_screenH;

    void tickHome(float dt);
    void tickPlaying(float dt);
    void renderHome();
    void processInput();
    void update(float dt);
    void autoSave(float dt);
    void getScreenSize();
};
