#pragma once

#include <array>
#include <cstdint>

#include "core/Config.hpp"
#include "world/Block.hpp"

namespace mc2d {

class Chunk {
 public:
  explicit Chunk(int chunkX);
  BlockType get(int x, int y) const;
  void set(int x, int y, BlockType t);
  int chunkX() const;

 private:
  int m_chunkX;
  std::array<BlockType, Config::ChunkWidth * Config::ChunkHeight> m_blocks;
};

}  // namespace mc2d
