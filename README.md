# mc_2d

C++ + SDL2 based 2D sandbox (Minecraft-inspired) with single codebase targetting Android, WebAssembly, and Desktop.

## Phase Roadmap
- **Phase 1 (current):** core engine loop, chunks, procedural terrain, mining/placing, collision, gravity, HUD hotbar, touch/keyboard input, CI build pipelines, save+load parser baseline.
- Phase 2: texture atlas renderer, crafting recipes, proper save/load, chunk mesh cache.
- Phase 3: biomes, advanced mobs, multiplayer-ready net abstraction.

## Build (Desktop)
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
./build/mc2d
```
> SDL2 agar system me missing ho to CMake automatically SDL2 fetch karega (`MC2D_FETCH_SDL2=ON` default).

## Build (Web)
```bash
emcmake cmake -S . -B build-web -DCMAKE_BUILD_TYPE=Release
cmake --build build-web -j
```

## Android
Android wrapper is in `android/` with native glue notes. CI uses NDK-based CMake build step for fast APK pipeline extension.
