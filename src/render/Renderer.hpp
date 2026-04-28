#pragma once

#include <SDL.h>

#include "render/Camera.hpp"
#include "world/World.hpp"

namespace mc2d {

class Player;

class Renderer {
 public:
  explicit Renderer(SDL_Renderer* sdlRenderer);
  void render(const World& world, const Player& player, const Camera& camera, float dayNightTime);

 private:
  SDL_Color colorForBlock(BlockType type) const;
  SDL_Renderer* m_renderer;
};

}  // namespace mc2d
