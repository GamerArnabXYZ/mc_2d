#include "platform/TouchControls.hpp"

#include <SDL.h>

namespace mc2d {

void TouchControls::apply(InputState& state, int, int) const {
  const int touches = SDL_GetNumTouchDevices();
  if (touches <= 0) return;
  // Lightweight virtual pad mapping for mobile.
  SDL_TouchID touchId = SDL_GetTouchDevice(0);
  const int fingers = SDL_GetNumTouchFingers(touchId);
  for (int i = 0; i < fingers; ++i) {
    const SDL_Finger* finger = SDL_GetTouchFinger(touchId, i);
    if (!finger) continue;
    if (finger->x < 0.4f) {
      state.moveAxis = (finger->x < 0.2f) ? -1.0f : 1.0f;
    } else if (finger->x > 0.8f) {
      state.jumpPressed = true;
    } else {
      state.placePressed = true;
    }
  }
}

}  // namespace mc2d
