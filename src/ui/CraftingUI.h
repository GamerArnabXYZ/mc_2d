#pragma once
#include <SDL2/SDL.h>
#include "../player/Inventory.h"
#include "../renderer/TextureAtlas.h"

// ─── CraftingUI ───────────────────────────────────────────────────────────────
// Renders full-screen inventory + 2×2 crafting grid when player opens inv.
// Touch-friendly: large slots, clear labels.
class CraftingUI {
public:
    CraftingUI();

    // Call every frame when inventory is open
    void render(SDL_Renderer* rend, TextureAtlas& atlas, const Inventory& inv);

    // Returns true if event was consumed (tap on a slot etc.)
    bool handleTouch(float fx, float fy, bool down, Inventory& inv);
    bool handleClick(int mx, int my, bool rightBtn, Inventory& inv);

private:
    // Layout constants (portrait-friendly, centered panel)
    static const int PANEL_W   = 320;
    static const int PANEL_H   = 420;
    static const int SLOT_SZ   = 44;
    static const int SLOT_PAD  = 6;

    // Computed in render based on WINDOW size
    int m_panelX, m_panelY;

    // Drag state: which slot is being dragged
    int  m_dragSlot;   // -1=none; 0..8=hotbar; 100+n=inv; 200+n=craft in; 300=craft out
    bool m_dragIsTouch;

    // ── Layout helpers ────────────────────────────────────────────────────────
    SDL_Rect hotbarSlotRect  (int i) const;
    SDL_Rect invSlotRect     (int row, int col) const;
    SDL_Rect craftInSlotRect (int i) const; // 0..3 (2×2 grid)
    SDL_Rect craftOutRect    ()      const;

    // ── Draw helpers ──────────────────────────────────────────────────────────
    void drawSlot    (SDL_Renderer* r, SDL_Rect rect, const ItemStack& item,
                      TextureAtlas& atlas, bool selected = false);
    void drawPanel   (SDL_Renderer* r);
    void drawLabel   (SDL_Renderer* r, const char* text, int x, int y);
    void fillRect    (SDL_Renderer* r, SDL_Rect rect, SDL_Color col);

    // Slot hit test: returns slot code or -1
    int slotAt(int px, int py, const Inventory& inv) const;

    // Swap two slots
    void swapSlots(int a, int b, Inventory& inv);
};
