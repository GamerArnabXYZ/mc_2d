#include "core/Timer.hpp"

#include <SDL.h>

namespace mc2d {

Timer::Timer() : m_prevCounter(SDL_GetPerformanceCounter()), m_deltaSeconds(0.016f), m_frameMs(16) {}

void Timer::tick() {
  const std::uint64_t now = SDL_GetPerformanceCounter();
  const std::uint64_t freq = SDL_GetPerformanceFrequency();
  const double delta = static_cast<double>(now - m_prevCounter) / static_cast<double>(freq);
  m_deltaSeconds = static_cast<float>(delta);
  m_frameMs = static_cast<std::uint32_t>(delta * 1000.0);
  m_prevCounter = now;
}

float Timer::deltaSeconds() const { return m_deltaSeconds; }
std::uint32_t Timer::frameTimeMs() const { return m_frameMs; }

}  // namespace mc2d
