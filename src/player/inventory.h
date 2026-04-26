#pragma once
// inventory.h - Slot-based Inventory System

#include "../world/blocks.h"
#include <stdint.h>
#include <stdbool.h>

#define INV_HOTBAR_SIZE  9
#define INV_MAIN_SIZE    27
#define INV_TOTAL        (INV_HOTBAR_SIZE + INV_MAIN_SIZE)
#define STACK_MAX        64

typedef struct {
    uint8_t  block_id;   // 0 = empty
    uint8_t  count;      // Stack count
} ItemSlot;

typedef struct {
    ItemSlot slots[INV_TOTAL]; // 0..8 = hotbar, 9..35 = main
    int      selected;         // Active hotbar slot 0..8
    bool     open;             // UI visible?
} Inventory;

void     inv_init(Inventory* inv);
bool     inv_add(Inventory* inv, uint8_t block_id, int count);   // Returns false if full
bool     inv_remove(Inventory* inv, int slot, int count);
ItemSlot inv_get_selected(const Inventory* inv);
void     inv_select(Inventory* inv, int slot);  // 0-8
void     inv_swap(Inventory* inv, int a, int b);
