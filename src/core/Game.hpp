#pragma once

#include <SDL.h>

#include "core/Timer.hpp"
#include "game/GameState.hpp"
#include "platform/Input.hpp"
#include "platform/TouchControls.hpp"
#include "player/Player.hpp"
#include "render/Camera.hpp"
#include "render/Renderer.hpp"
#include "ui/Hud.hpp"
#include "world/World.hpp"

namespace mc2d {

class Game {
 public:
  Game();
  ~Game();
  bool init();
  void run();

 private:
  void handleBlockActions(const InputState& input);

  SDL_Window* m_window;
  SDL_Renderer* m_sdlRenderer;
  Timer m_timer;
  Input m_input;
  TouchControls m_touch;
  World m_world;
  Player m_player;
  Camera m_camera;
  GameState m_state;
  Renderer* m_renderer;
  Hud* m_hud;
};

}  // namespace mc2d
