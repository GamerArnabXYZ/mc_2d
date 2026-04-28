#pragma once

#include <cstdint>

namespace mc2d {

class Timer {
 public:
  Timer();
  void tick();
  float deltaSeconds() const;
  std::uint32_t frameTimeMs() const;

 private:
  std::uint64_t m_prevCounter;
  float m_deltaSeconds;
  std::uint32_t m_frameMs;
};

}  // namespace mc2d
