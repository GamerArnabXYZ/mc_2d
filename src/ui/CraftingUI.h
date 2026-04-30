#pragma once
#include <SDL2/SDL.h>
#include "../player/Inventory.h"
#include "../renderer/TextureAtlas.h"

class CraftingUI {
public:
    CraftingUI();

    void render(SDL_Renderer* rend, TextureAtlas& atlas, Inventory& inv);
    bool handleClick(int mx, int my, bool rightBtn, Inventory& inv);

private:
    int m_panelX, m_panelY;

    SDL_Rect hotbarSlotRect (int i) const;
    SDL_Rect invSlotRect    (int row, int col) const;
    SDL_Rect craftInSlotRect(int i) const;
    SDL_Rect craftOutRect   () const;
    SDL_Rect closeRect      () const;

    int  slotAt   (int px, int py) const;
    void drawSlot (SDL_Renderer* r, SDL_Rect rect, const ItemStack& item,
                   TextureAtlas& atlas, bool selected);
    void drawBlockText(SDL_Renderer* r, const char* txt, int x, int y, SDL_Color col);
};
