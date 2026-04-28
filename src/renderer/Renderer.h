#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "TextureAtlas.h"
#include "Camera.h"
#include "../world/World.h"
#include "../player/Player.h"
#include "../core/Config.h"
#include "../ui/CraftingUI.h"
#include "../input/InputManager.h"
#include <string>

// ─── Renderer ─────────────────────────────────────────────────────────────────
class Renderer {
public:
    Renderer();
    ~Renderer();

    bool init(SDL_Window* win);
    void shutdown();

    // ── Main render pass ──────────────────────────────────────────────────────
    void render(const World& world, const Player& player, const Camera& cam,
                float fps, CraftingUI* craftUI = nullptr,
                const InputManager* input = nullptr);

    SDL_Renderer* sdl()      { return m_rend; }
    TextureAtlas& getAtlas() { return m_atlas; }

private:
    SDL_Renderer* m_rend;
    TextureAtlas  m_atlas;
    TTF_Font*     m_font;   // small bitmap font for HUD text

    // ── Sub-renders ───────────────────────────────────────────────────────────
    void renderSky    (uint8_t ambient);
    void renderChunks (const World& world, const Camera& cam, uint8_t ambient);
    void renderBlock  (int screenX, int screenY, uint8_t blockID, uint8_t ambient);
    void renderPlayer (const Player& p, const Camera& cam, uint8_t ambient);
    void renderBreak  (const Player& p, const Camera& cam);
    void renderHUD    (const Player& p, float fps);
    void renderHotbar (const Player& p);

    // ── Draw helpers ──────────────────────────────────────────────────────────
    void drawRect     (int x, int y, int w, int h, SDL_Color col, bool fill = true);
    void drawText     (const std::string& txt, int x, int y, SDL_Color col);
    void drawItem        (uint8_t id, int x, int y, int size);
    void renderTouchOverlay(const InputManager& input);
    void renderInventory   (CraftingUI& ui, Inventory& inv);
};
