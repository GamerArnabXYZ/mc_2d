#include "core/Game.h"
#include "core/SDL_incl.h"

// ─── main ─────────────────────────────────────────────────────────────────────
// On Android SDL2 redefines main via SDL_main.h automatically.
// On Emscripten we use emscripten_set_main_loop inside Game::run().
// On Desktop it's a standard int main().
int main(int argc, char* argv[]) {
    (void)argc; (void)argv;

    Game game;

    if (!game.init()) {
        SDL_Log("Game init failed. Exiting.");
        return 1;
    }

    game.run();
    // game.shutdown() called in destructor
    return 0;
}
