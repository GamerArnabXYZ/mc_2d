#pragma once

#include <SDL.h>

namespace mc2d {

class Camera {
 public:
  Camera();
  void follow(float targetX, float targetY);
  SDL_FPoint worldToScreen(float x, float y) const;
  float x() const;
  float y() const;

 private:
  float m_x;
  float m_y;
};

}  // namespace mc2d
