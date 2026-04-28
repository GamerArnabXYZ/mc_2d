#include "core/Game.hpp"

#include <cmath>

#include "core/Config.hpp"

namespace mc2d {

Game::Game()
    : m_window(nullptr),
      m_sdlRenderer(nullptr),
      m_world(Config::WorldSeed),
      m_renderer(nullptr),
      m_hud(nullptr) {}

Game::~Game() {
  delete m_hud;
  delete m_renderer;
  if (m_sdlRenderer) SDL_DestroyRenderer(m_sdlRenderer);
  if (m_window) SDL_DestroyWindow(m_window);
  SDL_Quit();
}

bool Game::init() {
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_TIMER) != 0) {
    return false;
  }

  m_window = SDL_CreateWindow("mc_2d", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, Config::ScreenWidth,
                              Config::ScreenHeight, SDL_WINDOW_SHOWN);
  if (!m_window) return false;

  m_sdlRenderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if (!m_sdlRenderer) return false;

  m_renderer = new Renderer(m_sdlRenderer);
  m_hud = new Hud(m_sdlRenderer);
  return true;
}

void Game::handleBlockActions(const InputState& input) {
  const int wx = static_cast<int>(std::floor(m_player.x() + 1.0f));
  const int wy = static_cast<int>(std::floor(m_player.y() + 1.0f));
  if (input.breakPressed) {
    BlockType broken = m_world.getBlock(wx, wy);
    if (broken != BlockType::Air) {
      m_world.setBlock(wx, wy, BlockType::Air);
      m_player.inventory().add(broken);
    }
  }

  if (input.placePressed) {
    const auto selected = m_player.inventory().selected();
    if (selected.count > 0 && selected.block != BlockType::Air && m_world.getBlock(wx + 1, wy) == BlockType::Air) {
      if (m_player.inventory().consumeSelected()) {
        m_world.setBlock(wx + 1, wy, selected.block);
      }
    }
  }
}

void Game::run() {
  InputState input;
  while (!input.quit) {
    m_timer.tick();
    m_input.poll(input);
    m_touch.apply(input, Config::ScreenWidth, Config::ScreenHeight);

    if (input.wheelY != 0) {
      const int next = (m_player.inventory().selection() + (input.wheelY > 0 ? 1 : -1) + Inventory::Slots) % Inventory::Slots;
      m_player.inventory().setSelection(next);
    }

    m_player.update(m_timer.deltaSeconds(), m_world, input.moveAxis, input.jumpPressed);
    handleBlockActions(input);
    m_world.ensureChunksAround(static_cast<int>(m_player.x()));
    m_camera.follow(m_player.x(), m_player.y());

    m_state.dayNightTimer += m_timer.deltaSeconds() * 0.1f;

    m_renderer->render(m_world, m_player, m_camera, m_state.dayNightTimer);
    m_hud->render(m_player.inventory());
    SDL_RenderPresent(m_sdlRenderer);
  }
}

}  // namespace mc2d
