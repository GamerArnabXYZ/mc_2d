#include "io/SaveSystem.hpp"

#include <fstream>

#include "player/Player.hpp"
#include "world/World.hpp"

namespace mc2d {

bool SaveSystem::save(const std::string& path, const World& world, const Player& player) {
  std::ofstream out(path, std::ios::binary);
  if (!out.is_open()) return false;

  out << player.x() << ' ' << player.y() << '\n';
  out << world.chunks().size() << '\n';
  for (const auto& [chunkX, chunk] : world.chunks()) {
    out << chunkX << '\n';
    for (int y = 0; y < 128; ++y) {
      for (int x = 0; x < 32; ++x) {
        out << static_cast<int>(chunk.get(x, y)) << ' ';
      }
      out << '\n';
    }
  }
  return true;
}

bool SaveSystem::load(const std::string&, World&, Player&) {
  // Phase-1 me minimal runtime stable rakhne ke liye load stub hai.
  return false;
}

}  // namespace mc2d
