#include "Inventory.h"
#include <cstring>

// ─── Crafting Recipes (2×2, 0=wildcard/empty) ─────────────────────────────────
// Layout: [0][1]
//         [2][3]
const CraftingRecipe Inventory::RECIPES[] = {
    // Planks from wood (any single wood in grid)
    {{BLOCK_WOOD, 0, 0, 0},    BLOCK_PLANKS,     4},
    // Sticks from 2 planks (vertical)
    {{BLOCK_PLANKS, 0, BLOCK_PLANKS, 0}, ITEM_STICK, 4},
    // Crafting table: 4 planks
    {{BLOCK_PLANKS, BLOCK_PLANKS, BLOCK_PLANKS, BLOCK_PLANKS}, BLOCK_CRAFTING, 1},
    // Glass from sand + dirt as fuel (simplified, 2 sand = 1 glass)
    {{BLOCK_SAND, BLOCK_SAND, 0, 0}, BLOCK_GLASS, 1},
    // Cobblestone back to stone (smelting simplified)
    {{BLOCK_COBBLESTONE, BLOCK_COBBLESTONE, BLOCK_COBBLESTONE, BLOCK_COBBLESTONE}, BLOCK_STONE, 4},
    // Chest: 2x2 planks = 1 chest
    {{BLOCK_PLANKS, BLOCK_PLANKS, BLOCK_PLANKS, BLOCK_PLANKS}, BLOCK_CHEST, 1},
};
const int Inventory::RECIPE_COUNT = sizeof(RECIPES) / sizeof(RECIPES[0]);

// ─── Constructor ──────────────────────────────────────────────────────────────
Inventory::Inventory() : m_selected(0), m_open(false) {
    memset(m_craftIn, 0, sizeof(m_craftIn));
    // Give player starter items
    m_hotbar[0] = ItemStack(BLOCK_GRASS,    64);
    m_hotbar[1] = ItemStack(BLOCK_DIRT,     32);
    m_hotbar[2] = ItemStack(BLOCK_STONE,    32);
    m_hotbar[3] = ItemStack(BLOCK_PLANKS,   16);
    m_hotbar[4] = ItemStack(BLOCK_COBBLESTONE, 8);
    m_hotbar[5] = ItemStack(BLOCK_GLASS,    4);
    m_hotbar[6] = ItemStack(BLOCK_TORCH,    8);
    m_hotbar[7] = ItemStack(BLOCK_WOOD,     16);
}

// ─── Scroll ───────────────────────────────────────────────────────────────────
void Inventory::scrollSelected(int d) {
    m_selected = (m_selected + d + HOTBAR_SLOTS) % HOTBAR_SLOTS;
}

// ─── Add item ─────────────────────────────────────────────────────────────────
bool Inventory::addItem(uint8_t id, int count) {
    // Try stack on existing hotbar slot
    for (int i = 0; i < HOTBAR_SLOTS; i++) {
        if (m_hotbar[i].id == id && m_hotbar[i].count < MAX_STACK) {
            int space = MAX_STACK - m_hotbar[i].count;
            int add   = (count <= space) ? count : space;
            m_hotbar[i].count += add;
            count -= add;
            if (count == 0) return true;
        }
    }
    // Try empty hotbar slot
    for (int i = 0; i < HOTBAR_SLOTS; i++) {
        if (m_hotbar[i].empty()) {
            int add = (count <= MAX_STACK) ? count : MAX_STACK;
            m_hotbar[i] = ItemStack(id, (uint8_t)add);
            count -= add;
            if (count == 0) return true;
        }
    }
    // Try inventory
    for (int i = 0; i < INV_ROWS * INV_COLS; i++) {
        if (m_inv[i].id == id && m_inv[i].count < MAX_STACK) {
            int space = MAX_STACK - m_inv[i].count;
            int add   = (count <= space) ? count : space;
            m_inv[i].count += add;
            count -= add;
            if (count == 0) return true;
        }
    }
    for (int i = 0; i < INV_ROWS * INV_COLS; i++) {
        if (m_inv[i].empty()) {
            int add = (count <= MAX_STACK) ? count : MAX_STACK;
            m_inv[i] = ItemStack(id, (uint8_t)add);
            count -= add;
            if (count == 0) return true;
        }
    }
    return (count == 0);
}

// ─── Remove held ──────────────────────────────────────────────────────────────
void Inventory::removeHeld(int count) {
    ItemStack& s = m_hotbar[m_selected];
    if (s.count <= count) { s.id = 0; s.count = 0; }
    else s.count -= count;
}

bool Inventory::hasItem(uint8_t id, int count) const {
    int total = 0;
    for (int i = 0; i < HOTBAR_SLOTS; i++)
        if (m_hotbar[i].id == id) total += m_hotbar[i].count;
    for (int i = 0; i < INV_ROWS * INV_COLS; i++)
        if (m_inv[i].id == id) total += m_inv[i].count;
    return total >= count;
}

void Inventory::removeItem(uint8_t id, int count) {
    // Remove from hotbar first, then inventory
    for (int i = 0; i < HOTBAR_SLOTS && count > 0; i++) {
        if (m_hotbar[i].id == id) {
            int take = MIN(count, m_hotbar[i].count);
            m_hotbar[i].count -= take;
            if (m_hotbar[i].count == 0) m_hotbar[i].id = 0;
            count -= take;
        }
    }
    for (int i = 0; i < INV_ROWS * INV_COLS && count > 0; i++) {
        if (m_inv[i].id == id) {
            int take = MIN(count, m_inv[i].count);
            m_inv[i].count -= take;
            if (m_inv[i].count == 0) m_inv[i].id = 0;
            count -= take;
        }
    }
}

// ─── Crafting input ───────────────────────────────────────────────────────────
void Inventory::setCraftInput(int slot, uint8_t id) {
    if (slot >= 0 && slot < 4) m_craftIn[slot] = id;
}

ItemStack Inventory::getCraftOutput() const {
    for (int r = 0; r < RECIPE_COUNT; r++) {
        const CraftingRecipe& recipe = RECIPES[r];
        bool match = true;
        for (int i = 0; i < 4; i++) {
            // 0 in recipe = wildcard (anything including air)
            if (recipe.in[i] != 0 && recipe.in[i] != m_craftIn[i]) {
                match = false;
                break;
            }
            if (recipe.in[i] == 0 && m_craftIn[i] != 0 && recipe.in[i] != 0) {
                match = false;
                break;
            }
        }
        if (match && recipe.out != 255)
            return ItemStack(recipe.out, recipe.count);
    }
    return ItemStack();
}

bool Inventory::doCraft() {
    ItemStack out = getCraftOutput();
    if (out.empty()) return false;
    // Consume one of each non-zero input
    for (int i = 0; i < 4; i++) {
        if (m_craftIn[i] != 0) {
            removeItem(m_craftIn[i], 1);
            m_craftIn[i] = 0;
        }
    }
    addItem(out.id, out.count);
    return true;
}
