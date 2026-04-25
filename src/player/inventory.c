// inventory.c
#include "inventory.h"
#include <string.h>

void inv_init(Inventory* inv) {
    memset(inv, 0, sizeof(Inventory));
    inv->selected = 0;
}

bool inv_add(Inventory* inv, uint8_t block_id, int count) {
    // First try stack onto existing slot
    for (int i = 0; i < INV_TOTAL; i++) {
        if (inv->slots[i].block_id == block_id && inv->slots[i].count < STACK_MAX) {
            int space = STACK_MAX - inv->slots[i].count;
            int add   = (count < space) ? count : space;
            inv->slots[i].count += (uint8_t)add;
            count -= add;
            if (count <= 0) return true;
        }
    }
    // Then find empty slot
    for (int i = 0; i < INV_TOTAL && count > 0; i++) {
        if (inv->slots[i].block_id == 0) {
            int add = (count > STACK_MAX) ? STACK_MAX : count;
            inv->slots[i].block_id = block_id;
            inv->slots[i].count    = (uint8_t)add;
            count -= add;
        }
    }
    return (count <= 0);
}

bool inv_remove(Inventory* inv, int slot, int count) {
    if (slot < 0 || slot >= INV_TOTAL) return false;
    if (inv->slots[slot].count < count) return false;
    inv->slots[slot].count -= (uint8_t)count;
    if (inv->slots[slot].count == 0) inv->slots[slot].block_id = 0;
    return true;
}

ItemSlot inv_get_selected(const Inventory* inv) {
    return inv->slots[inv->selected];
}

void inv_select(Inventory* inv, int slot) {
    if (slot >= 0 && slot < INV_HOTBAR_SIZE) inv->selected = slot;
}

void inv_swap(Inventory* inv, int a, int b) {
    if (a < 0 || a >= INV_TOTAL || b < 0 || b >= INV_TOTAL) return;
    ItemSlot tmp       = inv->slots[a];
    inv->slots[a]      = inv->slots[b];
    inv->slots[b]      = tmp;
}
