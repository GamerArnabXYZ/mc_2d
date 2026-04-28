#include "ui/Hud.hpp"

#include "core/Config.hpp"

namespace mc2d {

Hud::Hud(SDL_Renderer* renderer) : m_renderer(renderer) {}

void Hud::render(const Inventory& inventory) {
  constexpr float slotSize = 52.0f;
  const float totalW = slotSize * Inventory::Slots;
  const float startX = (Config::ScreenWidth - totalW) * 0.5f;
  const float y = Config::ScreenHeight - slotSize - 12.0f;

  for (int i = 0; i < Inventory::Slots; ++i) {
    SDL_FRect r{startX + i * slotSize, y, slotSize - 4.0f, slotSize - 4.0f};
    const bool selected = (i == inventory.selection());
    SDL_SetRenderDrawColor(m_renderer, selected ? 255 : 70, selected ? 220 : 70, 80, 255);
    SDL_RenderFillRectF(m_renderer, &r);
  }
}

}  // namespace mc2d
