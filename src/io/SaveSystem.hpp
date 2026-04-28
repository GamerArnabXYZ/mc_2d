#pragma once

#include <string>

namespace mc2d {

class Player;
class World;

class SaveSystem {
 public:
  static bool save(const std::string& path, const World& world, const Player& player);
  static bool load(const std::string& path, World& world, Player& player);
};

}  // namespace mc2d
