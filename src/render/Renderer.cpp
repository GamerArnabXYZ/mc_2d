#include "render/Renderer.hpp"

#include <algorithm>

#include "core/Config.hpp"
#include "player/Player.hpp"

namespace mc2d {

Renderer::Renderer(SDL_Renderer* sdlRenderer) : m_renderer(sdlRenderer) {}

SDL_Color Renderer::colorForBlock(BlockType type) const {
  switch (type) {
    case BlockType::Grass: return {92, 190, 72, 255};
    case BlockType::Dirt: return {123, 90, 56, 255};
    case BlockType::Stone: return {120, 120, 120, 255};
    case BlockType::Wood: return {115, 85, 45, 255};
    case BlockType::Leaves: return {68, 140, 75, 190};
    case BlockType::CoalOre: return {60, 60, 60, 255};
    case BlockType::IronOre: return {180, 150, 120, 255};
    default: return {0, 0, 0, 0};
  }
}

void Renderer::render(const World& world, const Player& player, const Camera& camera, float dayNightTime) {
  const float cycle = (std::sin(dayNightTime) + 1.0f) * 0.5f;
  const Uint8 sky = static_cast<Uint8>(40 + cycle * 130);
  SDL_SetRenderDrawColor(m_renderer, sky / 2, sky, 255, 255);
  SDL_RenderClear(m_renderer);

  const int minChunk = static_cast<int>(camera.x()) / Config::ChunkWidth - 1;
  const int maxChunk = static_cast<int>(camera.x()) / Config::ChunkWidth + Config::RenderDistanceChunks + 2;

  for (const auto& [chunkX, chunk] : world.chunks()) {
    if (chunkX < minChunk || chunkX > maxChunk) continue;
    for (int x = 0; x < Config::ChunkWidth; ++x) {
      for (int y = 0; y < Config::ChunkHeight; ++y) {
        const BlockType type = chunk.get(x, y);
        if (type == BlockType::Air) continue;
        const SDL_Color c = colorForBlock(type);
        SDL_SetRenderDrawColor(m_renderer, c.r, c.g, c.b, c.a);
        const int worldX = chunkX * Config::ChunkWidth + x;
        const SDL_FPoint sp = camera.worldToScreen(static_cast<float>(worldX), static_cast<float>(y));
        SDL_FRect r{sp.x, sp.y - Config::TileSize, static_cast<float>(Config::TileSize), static_cast<float>(Config::TileSize)};
        if (r.x + r.w < 0 || r.x > Config::ScreenWidth || r.y + r.h < 0 || r.y > Config::ScreenHeight) continue;
        SDL_RenderFillRectF(m_renderer, &r);
      }
    }
  }

  SDL_SetRenderDrawColor(m_renderer, 230, 210, 160, 255);
  const SDL_FRect pb = player.bounds();
  const SDL_FPoint psp = camera.worldToScreen(pb.x, pb.y + pb.h);
  SDL_FRect pr{psp.x, psp.y, pb.w * Config::TileSize, pb.h * Config::TileSize};
  SDL_RenderFillRectF(m_renderer, &pr);
}

}  // namespace mc2d
