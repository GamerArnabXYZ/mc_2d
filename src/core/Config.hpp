#pragma once

#include <cstdint>

namespace mc2d {

struct Config {
  static constexpr int ScreenWidth = 1280;
  static constexpr int ScreenHeight = 720;
  static constexpr int TileSize = 16;
  static constexpr int ChunkWidth = 32;
  static constexpr int ChunkHeight = 128;
  static constexpr int RenderDistanceChunks = 4;
  static constexpr int TargetFps = 60;
  static constexpr float Gravity = 28.0f;
  static constexpr float WalkSpeed = 6.0f;
  static constexpr float JumpVelocity = 10.5f;
  static constexpr std::uint32_t WorldSeed = 133742u;
};

}  // namespace mc2d
