#include "core/Game.hpp"

int main(int, char**) {
  mc2d::Game game;
  if (!game.init()) {
    return 1;
  }
  game.run();
  return 0;
}
