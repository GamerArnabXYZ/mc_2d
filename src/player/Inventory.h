#pragma once
#include "../core/Config.h"
#include "../world/Block.h"
#include <cstdint>
#include <cstring>

// ─── ItemStack ────────────────────────────────────────────────────────────────
struct ItemStack {
    uint8_t  id;    // block/item ID (0 = empty)
    uint8_t  count;
    ItemStack() : id(0), count(0) {}
    ItemStack(uint8_t i, uint8_t c) : id(i), count(c) {}
    bool empty() const { return id == 0 || count == 0; }
};

// ─── CraftingRecipe ───────────────────────────────────────────────────────────
struct CraftingRecipe {
    uint8_t in[4];  // 2x2 inputs (0=any/wildcard)
    uint8_t out;
    uint8_t count;
};

// ─── Inventory ────────────────────────────────────────────────────────────────
class Inventory {
public:
    Inventory();

    // ── Access ────────────────────────────────────────────────────────────────
    ItemStack&       hotbarSlot(int i)         { return m_hotbar[i]; }
    const ItemStack& hotbarSlot(int i) const   { return m_hotbar[i]; }
    ItemStack&       invSlot   (int i)         { return m_inv[i]; }
    const ItemStack& invSlot   (int i) const   { return m_inv[i]; }
    ItemStack& heldItem  ()        { return m_hotbar[m_selected]; }
    int        selected  () const  { return m_selected; }
    bool       isOpenConst() const  { return m_open; }
    void       setSelected(int i)  { m_selected = i % HOTBAR_SLOTS; }
    void       scrollSelected(int d);

    // ── Add/Remove ────────────────────────────────────────────────────────────
    bool addItem (uint8_t id, int count = 1); // returns false if full
    void removeHeld(int count = 1);
    bool hasItem (uint8_t id, int count = 1) const;
    void removeItem(uint8_t id, int count = 1);

    // ── Crafting (2x2 grid) ───────────────────────────────────────────────────
    // grid[0..3] = 2x2 input
    void       setCraftInput(int slot, uint8_t id);
    uint8_t    getCraftInput(int slot) const { return m_craftIn[slot]; }
    ItemStack  getCraftOutput() const;
    bool       doCraft();    // consume inputs, add output to inv

    // ── Open/close state ──────────────────────────────────────────────────────
    bool isOpen() const     { return m_open; }
    void setOpen(bool o)    { m_open = o; }

private:
    ItemStack m_hotbar[HOTBAR_SLOTS];
    ItemStack m_inv[INV_ROWS * INV_COLS];
    int       m_selected;
    uint8_t   m_craftIn[4];
    bool      m_open;

    static const CraftingRecipe RECIPES[];
    static const int            RECIPE_COUNT;
};
