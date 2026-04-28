#pragma once

#include <unordered_map>

#include "world/Chunk.hpp"
#include "world/WorldGenerator.hpp"

namespace mc2d {

class World {
 public:
  explicit World(std::uint32_t seed);
  BlockType getBlock(int worldX, int worldY);
  BlockType getBlock(int worldX, int worldY) const;
  void setBlock(int worldX, int worldY, BlockType type);
  const std::unordered_map<int, Chunk>& chunks() const;
  void ensureChunksAround(int playerWorldX);
  void clear();

 private:
  Chunk& getOrCreateChunk(int chunkX);
  const Chunk* findChunk(int chunkX) const;

  WorldGenerator m_generator;
  std::unordered_map<int, Chunk> m_chunks;
};

}  // namespace mc2d
