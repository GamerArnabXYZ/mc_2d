#pragma once

#include "platform/Input.hpp"

namespace mc2d {

class TouchControls {
 public:
  void apply(InputState& state, int screenW, int screenH) const;
};

}  // namespace mc2d
