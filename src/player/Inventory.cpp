#include "player/Inventory.hpp"

namespace mc2d {

Inventory::Inventory() : m_selected(0) {
  m_slots.fill({BlockType::Air, 0});
  m_slots[0] = {BlockType::Dirt, 99};
  m_slots[1] = {BlockType::Stone, 99};
  m_slots[2] = {BlockType::Wood, 50};
}

bool Inventory::add(BlockType block, std::uint16_t count) {
  for (auto& slot : m_slots) {
    if (slot.block == block && slot.count < 999) {
      slot.count += count;
      return true;
    }
  }
  for (auto& slot : m_slots) {
    if (slot.count == 0) {
      slot = {block, count};
      return true;
    }
  }
  return false;
}

bool Inventory::consumeSelected() {
  auto& slot = m_slots[m_selected];
  if (slot.count == 0 || slot.block == BlockType::Air) return false;
  slot.count--;
  if (slot.count == 0) slot.block = BlockType::Air;
  return true;
}

const ItemStack& Inventory::selected() const { return m_slots[m_selected]; }
const std::array<ItemStack, Inventory::Slots>& Inventory::slots() const { return m_slots; }
void Inventory::setSelection(int index) {
  if (index >= 0 && index < Slots) m_selected = index;
}
int Inventory::selection() const { return m_selected; }

}  // namespace mc2d
