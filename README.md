# ⛏️ CraftSDL — 2D Minecraft-Inspired Sandbox

Cross-platform 2D sandbox game in C++ + SDL2. Single codebase → Android APK, WebAssembly, Desktop.

## Features
- 🌍 Procedural world with 4 biomes (plains, desert, forest, snow)
- 🪨 23 block types, ores, trees, cactus
- ⛏️ Block break/place with reach check
- 👤 Physics: gravity, jump, collision, water swimming
- 🎒 Hotbar (9 slots), inventory, 2×2 crafting
- 🌅 Day-night cycle with ambient lighting
- 💾 Auto-save every 60s (chunk binary saves)
- 📱 Touch joystick + action controls (mobile-first)
- ⌨️ Keyboard + mouse (desktop)
- 🎯 Chunk culling (only renders visible blocks)
- 🏃 60fps target, minimal RAM usage

## Build Locally

### Desktop (Linux)
```bash
sudo apt install libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev cmake build-essential
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
./build/CraftSDL
```

### WebAssembly
```bash
source /path/to/emsdk/emsdk_env.sh
emcmake cmake -B build-web -DCMAKE_BUILD_TYPE=Release
emmake cmake --build build-web -j$(nproc)
# Open build-web/CraftSDL.html in a browser (use local server)
python3 -m http.server -d build-web
```

### Android
```bash
cd android
./gradlew assembleRelease
# APK: android/app/build/outputs/apk/release/
```

## GitHub Actions (Auto-build)
Push to `main` → triggers:
- **build-android.yml** → APK artifact
- **build-web.yml** → WASM artifact + auto-deploys to `web` branch (GitHub Pages)

Push a tag `v1.x.x` → triggers:
- **release.yml** → builds both, creates GitHub Release with APK + web ZIP + source ZIP

## Controls

| Action | Mobile | Desktop |
|--------|--------|---------|
| Move | Left joystick | WASD / Arrows |
| Jump | Swipe joystick up | Space / W |
| Break block | Hold right side | Left click (hold) |
| Place block | Tap right side | Right click |
| Inventory | Top-right button | E |
| Select slot | — | 1-9 / Scroll |

## Project Structure
```
src/
  core/     Game loop, Timer, Config
  world/    Block definitions, Chunk, World, WorldGen
  player/   Physics, Inventory, Crafting
  renderer/ SDL2 render pipeline, TextureAtlas, Camera
  input/    Touch + keyboard/mouse unified input
  save/     Save/Load manager
android/    Android Gradle + NDK project
web/        Emscripten HTML shell
cmake/      Platform-specific CMake configs
.github/    CI/CD workflows
```
