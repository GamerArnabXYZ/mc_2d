#include "io/SaveSystem.hpp"

#include <fstream>

#include "core/Config.hpp"
#include "player/Player.hpp"
#include "world/Block.hpp"
#include "world/Chunk.hpp"
#include "world/World.hpp"

namespace mc2d {

bool SaveSystem::save(const std::string& path, const World& world, const Player& player) {
  std::ofstream out(path, std::ios::binary);
  if (!out.is_open()) return false;

  out << player.x() << ' ' << player.y() << '\n';
  out << world.chunks().size() << '\n';
  for (const auto& [chunkX, chunk] : world.chunks()) {
    out << chunkX << '\n';
    for (int y = 0; y < Config::ChunkHeight; ++y) {
      for (int x = 0; x < Config::ChunkWidth; ++x) {
        out << static_cast<int>(chunk.get(x, y)) << ' ';
      }
      out << '\n';
    }
  }
  return true;
}

bool SaveSystem::load(const std::string& path, World& world, Player&) {
  std::ifstream in(path, std::ios::binary);
  if (!in.is_open()) return false;

  float px = 0.0f;
  float py = 0.0f;
  std::size_t chunkCount = 0;
  in >> px >> py;
  in >> chunkCount;
  if (in.fail()) return false;
  (void)px;
  (void)py;

  world.clear();
  for (std::size_t i = 0; i < chunkCount; ++i) {
    int chunkX = 0;
    in >> chunkX;
    if (in.fail()) return false;
    for (int y = 0; y < Config::ChunkHeight; ++y) {
      for (int x = 0; x < Config::ChunkWidth; ++x) {
        int raw = 0;
        in >> raw;
        if (in.fail()) return false;
        world.setBlock(chunkX * Config::ChunkWidth + x, y, static_cast<BlockType>(raw));
      }
    }
  }
  return true;
}

}  // namespace mc2d
