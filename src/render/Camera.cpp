#include "render/Camera.hpp"

#include "core/Config.hpp"

namespace mc2d {

Camera::Camera() : m_x(0.0f), m_y(0.0f) {}

void Camera::follow(float targetX, float targetY) {
  m_x = targetX - (Config::ScreenWidth / static_cast<float>(Config::TileSize)) * 0.5f;
  m_y = targetY - (Config::ScreenHeight / static_cast<float>(Config::TileSize)) * 0.5f;
}

SDL_FPoint Camera::worldToScreen(float x, float y) const {
  return SDL_FPoint{(x - m_x) * Config::TileSize, Config::ScreenHeight - ((y - m_y) * Config::TileSize)};
}

float Camera::x() const { return m_x; }
float Camera::y() const { return m_y; }

}  // namespace mc2d
