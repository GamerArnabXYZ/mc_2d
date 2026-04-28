#pragma once

#include <cstdint>

#include "world/Chunk.hpp"

namespace mc2d {

class WorldGenerator {
 public:
  explicit WorldGenerator(std::uint32_t seed);
  void generateChunk(Chunk& chunk) const;

 private:
  float noise(int x) const;
  std::uint32_t m_seed;
};

}  // namespace mc2d
