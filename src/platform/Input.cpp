#include "platform/Input.hpp"

namespace mc2d {

void Input::poll(InputState& state) {
  state.jumpPressed = false;
  state.placePressed = false;
  state.breakPressed = false;
  state.wheelY = 0;

  SDL_Event ev;
  while (SDL_PollEvent(&ev)) {
    if (ev.type == SDL_QUIT) state.quit = true;
    if (ev.type == SDL_KEYDOWN && ev.key.keysym.sym == SDLK_SPACE) state.jumpPressed = true;
    if (ev.type == SDL_MOUSEBUTTONDOWN && ev.button.button == SDL_BUTTON_LEFT) state.breakPressed = true;
    if (ev.type == SDL_MOUSEBUTTONDOWN && ev.button.button == SDL_BUTTON_RIGHT) state.placePressed = true;
    if (ev.type == SDL_MOUSEWHEEL) state.wheelY = ev.wheel.y;
  }

  int mx = 0;
  int my = 0;
  const auto buttons = SDL_GetMouseState(&mx, &my);
  state.mouseX = mx;
  state.mouseY = my;

  const Uint8* k = SDL_GetKeyboardState(nullptr);
  const float left = (k[SDL_SCANCODE_A] || k[SDL_SCANCODE_LEFT]) ? -1.0f : 0.0f;
  const float right = (k[SDL_SCANCODE_D] || k[SDL_SCANCODE_RIGHT]) ? 1.0f : 0.0f;
  state.moveAxis = left + right;
  if (buttons & SDL_BUTTON(SDL_BUTTON_LEFT)) state.breakPressed = true;
}

}  // namespace mc2d
