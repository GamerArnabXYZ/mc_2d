#include "player/Player.hpp"

#include <algorithm>
#include <cmath>

#include "core/Config.hpp"
#include "world/World.hpp"

namespace mc2d {

Player::Player() : m_x(8.0f), m_y(80.0f), m_vx(0.0f), m_vy(0.0f), m_grounded(false) {}

SDL_FRect Player::bounds() const { return SDL_FRect{m_x, m_y, 0.8f, 1.8f}; }

bool Player::collides(const World& world, const SDL_FRect& rect) const {
  const int minX = static_cast<int>(std::floor(rect.x));
  const int maxX = static_cast<int>(std::floor(rect.x + rect.w));
  const int minY = static_cast<int>(std::floor(rect.y));
  const int maxY = static_cast<int>(std::floor(rect.y + rect.h));

  for (int x = minX; x <= maxX; ++x) {
    for (int y = minY; y <= maxY; ++y) {
      if (isSolid(world.getBlock(x, y))) {
        return true;
      }
    }
  }
  return false;
}

void Player::update(float dt, World& world, float moveAxis, bool jumpPressed) {
  m_vx = moveAxis * Config::WalkSpeed;
  m_vy -= Config::Gravity * dt;
  if (jumpPressed && m_grounded) {
    m_vy = Config::JumpVelocity;
    m_grounded = false;
  }

  SDL_FRect next = bounds();
  next.x += m_vx * dt;
  if (!collides(world, next)) {
    m_x = next.x;
  }

  next = bounds();
  next.y += m_vy * dt;
  if (!collides(world, next)) {
    m_y = next.y;
    m_grounded = false;
  } else {
    if (m_vy < 0) m_grounded = true;
    m_vy = 0.0f;
  }

  m_y = std::max(m_y, 0.0f);
}

float Player::x() const { return m_x; }
float Player::y() const { return m_y; }
Inventory& Player::inventory() { return m_inventory; }
const Inventory& Player::inventory() const { return m_inventory; }

}  // namespace mc2d
