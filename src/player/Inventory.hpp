#pragma once

#include <array>
#include <cstdint>

#include "world/Block.hpp"

namespace mc2d {

struct ItemStack {
  BlockType block;
  std::uint16_t count;
};

class Inventory {
 public:
  static constexpr int Slots = 9;

  Inventory();
  bool add(BlockType block, std::uint16_t count = 1);
  bool consumeSelected();
  const ItemStack& selected() const;
  const std::array<ItemStack, Slots>& slots() const;
  void setSelection(int index);
  int selection() const;

 private:
  std::array<ItemStack, Slots> m_slots;
  int m_selected;
};

}  // namespace mc2d
