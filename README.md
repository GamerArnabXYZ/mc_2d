# MC2D — 2D Minecraft Clone Engine
**ArnabLabZ Studio** | Pure C99 + Raylib 4.5

## Features
- 16×256 chunk world, Perlin noise worldgen, 6 biomes
- 26 block types with procedural texture atlas (no image files needed)
- AABB physics with 4× sub-stepping (no tunneling)
- LRU chunk cache (64 chunks max)
- Unified keyboard + touch input with virtual joystick (60px targets)
- Material Design 3 style UI
- Android SDK 21–34, ARM64/ARMv7/x86_64
- WebAssembly via Emscripten 3.1.64

## Quick Build

### Desktop (Linux)
```bash
sudo apt install libraylib-dev
make
./mc2d
```

### Web (WASM)
```bash
source /path/to/emsdk/emsdk_env.sh
make web
# Open web/index.html in browser
```

### Android APK
Push to main/master branch → GitHub Actions auto-builds APK.
Download from Actions > Artifacts > MC2D-APK.

## Controls
| Action       | Keyboard         | Touch            |
|-------------|-----------------|-----------------|
| Move        | A / D            | Left joystick    |
| Jump        | Space            | ↑ button         |
| Dig         | Left click       | ⛏ button         |
| Place       | Right click      | [+] button       |
| Inventory   | E                | (coming soon)    |
| Pause       | Escape           | —                |
| Hotbar      | Scroll wheel     | —                |
| Debug       | F3               | —                |

## Architecture
```
src/
  main.c              Game loop, state machine
  mc2d_types.h        All shared types & constants
  world/
    worldgen.c/h      Chunk gen, Perlin noise, LRU cache, block props
  physics/
    physics.c/h       AABB sweep, 4× sub-step, gravity
  renderer/
    renderer.c/h      Raylib draw, procedural atlas, camera
  input/
    input.c/h         Keyboard + touch joystick unified
  player/
    player.c/h        Movement, dig/place, inventory, hotbar
  ui/
    ui.c/h            MD3 menus, pause, game over
  utils/
    noise.c/h         Pure-C Perlin noise (seeded)
android/              Gradle + CMake Android project
web/                  Emscripten shell
.github/workflows/    Full CI/CD pipeline
```
