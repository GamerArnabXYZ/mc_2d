#include "world/WorldGenerator.hpp"

#include <cmath>

namespace mc2d {

WorldGenerator::WorldGenerator(std::uint32_t seed) : m_seed(seed) {}

float WorldGenerator::noise(int x) const {
  const float v = std::sin(static_cast<float>(x * 17 + static_cast<int>(m_seed % 997)) * 0.13f);
  return (v + 1.0f) * 0.5f;
}

void WorldGenerator::generateChunk(Chunk& chunk) const {
  const int baseX = chunk.chunkX() * Config::ChunkWidth;
  for (int localX = 0; localX < Config::ChunkWidth; ++localX) {
    const int worldX = baseX + localX;
    const int surface = 40 + static_cast<int>(noise(worldX) * 20.0f);
    for (int y = 0; y < Config::ChunkHeight; ++y) {
      if (y > surface) {
        chunk.set(localX, y, BlockType::Air);
      } else if (y == surface) {
        chunk.set(localX, y, BlockType::Grass);
      } else if (y > surface - 4) {
        chunk.set(localX, y, BlockType::Dirt);
      } else {
        const float oreNoise = noise(worldX + y * 3);
        if (oreNoise > 0.87f && y < surface - 8) {
          chunk.set(localX, y, BlockType::CoalOre);
        } else if (oreNoise > 0.95f && y < surface - 14) {
          chunk.set(localX, y, BlockType::IronOre);
        } else {
          chunk.set(localX, y, BlockType::Stone);
        }
      }
    }

    if ((worldX + static_cast<int>(m_seed)) % 29 == 0) {
      for (int ty = surface + 1; ty <= surface + 4 && ty < Config::ChunkHeight; ++ty) {
        chunk.set(localX, ty, BlockType::Wood);
      }
      for (int lx = localX - 2; lx <= localX + 2; ++lx) {
        for (int ly = surface + 3; ly <= surface + 6; ++ly) {
          if (ly < Config::ChunkHeight && lx >= 0 && lx < Config::ChunkWidth) {
            chunk.set(lx, ly, BlockType::Leaves);
          }
        }
      }
    }
  }
}

}  // namespace mc2d
