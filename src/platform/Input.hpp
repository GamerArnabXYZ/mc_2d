#pragma once

#include <SDL.h>

namespace mc2d {

struct InputState {
  bool quit = false;
  bool jumpPressed = false;
  bool placePressed = false;
  bool breakPressed = false;
  float moveAxis = 0.0f;
  int wheelY = 0;
  int mouseX = 0;
  int mouseY = 0;
};

class Input {
 public:
  void poll(InputState& state);
};

}  // namespace mc2d
