#pragma once

#include <cstdint>

namespace mc2d {

enum class BlockType : std::uint8_t {
  Air = 0,
  Grass,
  Dirt,
  Stone,
  Wood,
  Leaves,
  CoalOre,
  IronOre
};

inline bool isSolid(BlockType type) { return type != BlockType::Air && type != BlockType::Leaves; }

}  // namespace mc2d
