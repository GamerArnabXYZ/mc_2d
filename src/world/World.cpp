#include "world/World.hpp"

#include <cmath>

#include "core/Config.hpp"

namespace mc2d {

World::World(std::uint32_t seed) : m_generator(seed) {}

Chunk& World::getOrCreateChunk(int chunkX) {
  auto it = m_chunks.find(chunkX);
  if (it != m_chunks.end()) {
    return it->second;
  }
  Chunk chunk(chunkX);
  m_generator.generateChunk(chunk);
  auto [insertedIt, _] = m_chunks.emplace(chunkX, chunk);
  return insertedIt->second;
}

const Chunk* World::findChunk(int chunkX) const {
  auto it = m_chunks.find(chunkX);
  if (it == m_chunks.end()) return nullptr;
  return &it->second;
}

BlockType World::getBlock(int worldX, int worldY) {
  const int chunkX = static_cast<int>(std::floor(static_cast<float>(worldX) / Config::ChunkWidth));
  const int localX = ((worldX % Config::ChunkWidth) + Config::ChunkWidth) % Config::ChunkWidth;
  return getOrCreateChunk(chunkX).get(localX, worldY);
}

BlockType World::getBlock(int worldX, int worldY) const {
  const int chunkX = static_cast<int>(std::floor(static_cast<float>(worldX) / Config::ChunkWidth));
  const int localX = ((worldX % Config::ChunkWidth) + Config::ChunkWidth) % Config::ChunkWidth;
  const Chunk* chunk = findChunk(chunkX);
  if (!chunk) return BlockType::Air;
  return chunk->get(localX, worldY);
}

void World::setBlock(int worldX, int worldY, BlockType type) {
  const int chunkX = static_cast<int>(std::floor(static_cast<float>(worldX) / Config::ChunkWidth));
  const int localX = ((worldX % Config::ChunkWidth) + Config::ChunkWidth) % Config::ChunkWidth;
  getOrCreateChunk(chunkX).set(localX, worldY, type);
}

const std::unordered_map<int, Chunk>& World::chunks() const { return m_chunks; }

void World::ensureChunksAround(int playerWorldX) {
  const int center = static_cast<int>(std::floor(static_cast<float>(playerWorldX) / Config::ChunkWidth));
  for (int i = -Config::RenderDistanceChunks; i <= Config::RenderDistanceChunks; ++i) {
    getOrCreateChunk(center + i);
  }
}

void World::clear() { m_chunks.clear(); }

}  // namespace mc2d
