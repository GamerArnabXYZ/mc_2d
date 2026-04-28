#pragma once

#include <SDL.h>

#include "player/Inventory.hpp"

namespace mc2d {

class Hud {
 public:
  explicit Hud(SDL_Renderer* renderer);
  void render(const Inventory& inventory);

 private:
  SDL_Renderer* m_renderer;
};

}  // namespace mc2d
