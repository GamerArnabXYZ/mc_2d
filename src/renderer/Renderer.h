#pragma once
#include <SDL2/SDL.h>
#if __has_include(<SDL2/SDL_ttf.h>)
  #include <SDL2/SDL_ttf.h>
#elif __has_include(<SDL_ttf.h>)
  #include <SDL_ttf.h>
#endif
#include "TextureAtlas.h"
#include "Camera.h"
#include "../world/World.h"
#include "../player/Player.h"
#include "../core/Config.h"
#include "../ui/CraftingUI.h"
#include "../input/InputManager.h"
#include <string>

class Renderer {
public:
    Renderer();
    ~Renderer();

    bool init(SDL_Window* win);
    void shutdown();

    void render(const World& world, const Player& player, const Camera& cam,
                float fps, CraftingUI* craftUI = nullptr,
                const InputManager* input = nullptr);

    SDL_Renderer* sdl()      { return m_rend; }
    TextureAtlas& getAtlas() { return m_atlas; }

private:
    SDL_Renderer* m_rend;
    TextureAtlas  m_atlas;
    TTF_Font*     m_font;

    void renderSky          (uint8_t amb);
    void renderChunks       (const World& world, const Camera& cam, uint8_t amb);
    void renderBlock        (int sx, int sy, uint8_t id, uint8_t amb, bool topExposed);
    void renderBreakOverlay (const Player& p, const Camera& cam, const World& w);
    void renderPlayer       (const Player& p, const Camera& cam, uint8_t amb);
    void renderHUD          (const Player& p, float fps, const InputManager* input);
    void renderHotbar       (const Player& p);
    void renderTouchOverlay (const InputManager& inp);

    void drawRect  (int x,int y,int w,int h, SDL_Color col, bool fill=true);
    void drawText  (const std::string& txt, int x,int y, SDL_Color col);
    void drawItem  (uint8_t id, int x, int y, int size);
};
