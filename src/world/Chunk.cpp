#include "world/Chunk.hpp"

namespace mc2d {

Chunk::Chunk(int chunkX) : m_chunkX(chunkX) {
  m_blocks.fill(BlockType::Air);
}

BlockType Chunk::get(int x, int y) const {
  if (x < 0 || x >= Config::ChunkWidth || y < 0 || y >= Config::ChunkHeight) {
    return BlockType::Air;
  }
  return m_blocks[y * Config::ChunkWidth + x];
}

void Chunk::set(int x, int y, BlockType t) {
  if (x < 0 || x >= Config::ChunkWidth || y < 0 || y >= Config::ChunkHeight) {
    return;
  }
  m_blocks[y * Config::ChunkWidth + x] = t;
}

int Chunk::chunkX() const { return m_chunkX; }

}  // namespace mc2d
