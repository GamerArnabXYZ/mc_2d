#include "CraftingUI.h"
#include "../core/Config.h"
#include <cstdio>
#include <cstring>

CraftingUI::CraftingUI()
    : m_panelX(0), m_panelY(0)
    , m_dragSlot(-1), m_dragIsTouch(false)
{}

// ─── Layout ───────────────────────────────────────────────────────────────────
// Panel centered on screen
SDL_Rect CraftingUI::hotbarSlotRect(int i) const {
    int x = m_panelX + 10 + i * (SLOT_SZ + SLOT_PAD);
    int y = m_panelY + PANEL_H - SLOT_SZ - 10;
    return {x, y, SLOT_SZ, SLOT_SZ};
}

SDL_Rect CraftingUI::invSlotRect(int row, int col) const {
    int x = m_panelX + 10 + col * (SLOT_SZ + SLOT_PAD);
    int y = m_panelY + 160 + row * (SLOT_SZ + SLOT_PAD);
    return {x, y, SLOT_SZ, SLOT_SZ};
}

// 2×2 crafting grid (right side of panel)
SDL_Rect CraftingUI::craftInSlotRect(int i) const {
    int col = i % 2, row = i / 2;
    int x   = m_panelX + PANEL_W - 10 - (2 * (SLOT_SZ + SLOT_PAD)) + col * (SLOT_SZ + SLOT_PAD);
    int y   = m_panelY + 36 + row * (SLOT_SZ + SLOT_PAD);
    return {x, y, SLOT_SZ, SLOT_SZ};
}

SDL_Rect CraftingUI::craftOutRect() const {
    int x = m_panelX + PANEL_W - 10 - SLOT_SZ - 4;
    int y = m_panelY + 36 + SLOT_SZ + SLOT_PAD + 4;
    return {x, y, SLOT_SZ + 8, SLOT_SZ + 8};
}

// ─── Slot hit-test ────────────────────────────────────────────────────────────
// Returns: 0..8=hotbar, 100+n=inv(0..26), 200+n=craftIn(0..3), 300=craftOut, -1=none
int CraftingUI::slotAt(int px, int py, const Inventory& inv) const {
    for (int i = 0; i < HOTBAR_SLOTS; i++) {
        SDL_Rect r = hotbarSlotRect(i);
        if (px >= r.x && px < r.x+r.w && py >= r.y && py < r.y+r.h) return i;
    }
    for (int row = 0; row < INV_ROWS; row++) {
        for (int col = 0; col < INV_COLS; col++) {
            SDL_Rect r = invSlotRect(row, col);
            if (px >= r.x && px < r.x+r.w && py >= r.y && py < r.y+r.h)
                return 100 + row * INV_COLS + col;
        }
    }
    for (int i = 0; i < 4; i++) {
        SDL_Rect r = craftInSlotRect(i);
        if (px >= r.x && px < r.x+r.w && py >= r.y && py < r.y+r.h) return 200 + i;
    }
    {
        SDL_Rect r = craftOutRect();
        if (px >= r.x && px < r.x+r.w && py >= r.y && py < r.y+r.h) return 300;
    }
    return -1;
}

// ─── Render ───────────────────────────────────────────────────────────────────
void CraftingUI::render(SDL_Renderer* rend, TextureAtlas& atlas, const Inventory& inv) {
    // Center panel
    m_panelX = (WINDOW_W - PANEL_W) / 2;
    m_panelY = (WINDOW_H - PANEL_H) / 2;

    // Dim background
    SDL_SetRenderDrawColor(rend, 0, 0, 0, 160);
    SDL_Rect full = {0, 0, WINDOW_W, WINDOW_H};
    SDL_RenderFillRect(rend, &full);

    // Panel background
    drawPanel(rend);

    // ── Section labels ────────────────────────────────────────────────────────
    drawLabel(rend, "INVENTORY", m_panelX + 10, m_panelY + 144);
    drawLabel(rend, "CRAFT",     m_panelX + PANEL_W - 120, m_panelY + 24);

    // ── Inventory slots (3×9) ─────────────────────────────────────────────────
    for (int row = 0; row < INV_ROWS; row++) {
        for (int col = 0; col < INV_COLS; col++) {
            SDL_Rect r = invSlotRect(row, col);
            drawSlot(rend, r, inv.invSlot(row * INV_COLS + col), atlas);
        }
    }

    // ── Hotbar slots ──────────────────────────────────────────────────────────
    for (int i = 0; i < HOTBAR_SLOTS; i++) {
        SDL_Rect r = hotbarSlotRect(i);
        drawSlot(rend, r, inv.hotbarSlot(i), atlas, i == inv.selected());
    }

    // ── Crafting input 2×2 ────────────────────────────────────────────────────
    for (int i = 0; i < 4; i++) {
        SDL_Rect r = craftInSlotRect(i);
        ItemStack craftItem(inv.getCraftInput(i), (inv.getCraftInput(i) != 0) ? 1 : 0);
        drawSlot(rend, r, craftItem, atlas);
    }

    // Arrow "→" between craft grid and output
    {
        SDL_Rect ar = craftInSlotRect(1);
        drawLabel(rend, "->", ar.x + SLOT_SZ + 2, ar.y + SLOT_SZ / 2 - 6);
    }

    // ── Craft output slot ─────────────────────────────────────────────────────
    ItemStack output = inv.getCraftOutput();
    SDL_Rect outR = craftOutRect();
    // Highlight output if something available
    if (!output.empty()) {
        SDL_SetRenderDrawColor(rend, 60, 200, 60, 80);
        SDL_RenderFillRect(rend, &outR);
    }
    drawSlot(rend, outR, output, atlas);

    // ── Close hint ────────────────────────────────────────────────────────────
    drawLabel(rend, "[E] or tap outside to close", m_panelX + 60, m_panelY + PANEL_H - 18);
}

// ─── Touch handling ───────────────────────────────────────────────────────────
bool CraftingUI::handleTouch(float fx, float fy, bool down, Inventory& inv) {
    int px = (int)fx, py = (int)fy;

    // Click outside panel → close
    if (down && (px < m_panelX || px > m_panelX + PANEL_W ||
                 py < m_panelY || py > m_panelY + PANEL_H)) {
        inv.setOpen(false);
        return true;
    }

    if (down) {
        int slot = slotAt(px, py, inv);
        if (slot == 300) {
            // Craft output: do craft
            inv.doCraft();
            return true;
        }
        if (slot >= 200 && slot < 204) {
            // Toggle craft input: cycle through blocks in hotbar
            int ci = slot - 200;
            uint8_t cur = inv.getCraftInput(ci);
            // Find next non-empty hotbar item after current
            uint8_t next = 0;
            for (int i = 0; i < HOTBAR_SLOTS; i++) {
                if (inv.hotbarSlot(i).id > cur) { next = inv.hotbarSlot(i).id; break; }
            }
            inv.setCraftInput(ci, next);
            return true;
        }
    }
    return false; // didn't consume
}

bool CraftingUI::handleClick(int mx, int my, bool rightBtn, Inventory& inv) {
    // Right click on craft output = craft
    if (rightBtn) {
        SDL_Rect outR = craftOutRect();
        if (mx >= outR.x && mx < outR.x+outR.w && my >= outR.y && my < outR.y+outR.h) {
            inv.doCraft();
            return true;
        }
    }
    return handleTouch((float)mx, (float)my, true, inv);
}

// ─── Draw helpers ─────────────────────────────────────────────────────────────
void CraftingUI::fillRect(SDL_Renderer* r, SDL_Rect rect, SDL_Color col) {
    SDL_SetRenderDrawColor(r, col.r, col.g, col.b, col.a);
    SDL_RenderFillRect(r, &rect);
}

void CraftingUI::drawPanel(SDL_Renderer* r) {
    // Shadow
    SDL_Rect shadow = {m_panelX + 4, m_panelY + 4, PANEL_W, PANEL_H};
    fillRect(r, shadow, {0, 0, 0, 120});

    // Panel body
    SDL_Rect panel = {m_panelX, m_panelY, PANEL_W, PANEL_H};
    fillRect(r, panel, {40, 35, 30, 240});

    // Border
    SDL_SetRenderDrawColor(r, 160, 130, 80, 255);
    SDL_RenderDrawRect(r, &panel);

    // Title bar
    SDL_Rect title = {m_panelX, m_panelY, PANEL_W, 22};
    fillRect(r, title, {60, 50, 35, 255});
    drawLabel(r, "  INVENTORY & CRAFTING", m_panelX + 4, m_panelY + 4);
}

void CraftingUI::drawLabel(SDL_Renderer* r, const char* text, int x, int y) {
    // Simple pixel-art style: draw colored rect per character (no font dep)
    // Each "char" = 5px wide block
    SDL_SetRenderDrawColor(r, 220, 200, 140, 255);
    int cx = x;
    for (const char* c = text; *c; c++, cx += 5) {
        if (*c == ' ') continue;
        SDL_Rect dot = {cx, y, 4, 8};
        SDL_RenderFillRect(r, &dot);
    }
}

void CraftingUI::drawSlot(SDL_Renderer* r, SDL_Rect rect,
                           const ItemStack& item, TextureAtlas& atlas,
                           bool selected) {
    // Slot background
    SDL_Color bg = selected
        ? SDL_Color{180, 160, 80, 220}
        : SDL_Color{55, 50, 42, 210};
    fillRect(r, rect, bg);

    // Slot border
    SDL_SetRenderDrawColor(r, 100, 90, 70, 255);
    SDL_RenderDrawRect(r, &rect);

    if (item.empty()) return;

    // Item icon (inner rect)
    SDL_Rect inner = {rect.x + 5, rect.y + 5, rect.w - 10, rect.h - 10};
    if (atlas.isLoaded()) {
        SDL_Rect src = atlas.getRect(item.id, false);
        SDL_SetTextureColorMod(atlas.texture(), 255, 255, 255);
        SDL_RenderCopy(r, atlas.texture(), &src, &inner);
    } else {
        // Fallback color square
        SDL_SetRenderDrawColor(r, 100, 180, 100, 255);
        SDL_RenderFillRect(r, &inner);
    }

    // Count badge (bottom-right)
    if (item.count > 1) {
        char buf[8];
        snprintf(buf, sizeof(buf), "%d", item.count);
        // Draw tiny number as pixel blocks
        SDL_SetRenderDrawColor(r, 255, 255, 80, 255);
        SDL_Rect badge = {rect.x + rect.w - 14, rect.y + rect.h - 10, 12, 8};
        fillRect(r, badge, {0, 0, 0, 160});
        // We just draw a small colored mark; real text needs TTF
        SDL_SetRenderDrawColor(r, 255, 255, 80, 255);
        SDL_RenderDrawRect(r, &badge);
    }
}
