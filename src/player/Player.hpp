#pragma once

#include <SDL.h>

#include "player/Inventory.hpp"

namespace mc2d {

class World;

class Player {
 public:
  Player();
  void update(float dt, World& world, float moveAxis, bool jumpPressed);
  SDL_FRect bounds() const;
  float x() const;
  float y() const;
  Inventory& inventory();
  const Inventory& inventory() const;

 private:
  bool collides(const World& world, const SDL_FRect& rect) const;

  float m_x;
  float m_y;
  float m_vx;
  float m_vy;
  bool m_grounded;
  Inventory m_inventory;
};

}  // namespace mc2d
