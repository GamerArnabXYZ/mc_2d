# MC2D Engine — Architecture Guide
**ArnabLabZ Studio | C99 + Raylib**

---

## 📁 Project Structure

```
mc2d-engine/
├── src/
│   ├── main.c                    ← Game loop, init, render orchestration
│   ├── world/
│   │   ├── blocks.h/.c           ← Block registry + JSON loader
│   │   ├── chunk.h/.c            ← 16×16 pool-based chunk manager
│   │   └── worldgen.h/.c        ← Perlin noise world generation
│   ├── player/
│   │   ├── player.h/.c           ← AABB physics + state machine
│   │   └── inventory.h/.c       ← Stack-based inventory
│   ├── renderer/
│   │   └── textures.h/.c        ← Resilient atlas loader
│   └── input/
│       └── input.h/.c           ← Unified keyboard+touch input
├── assets/
│   └── blocks.json              ← Block definitions (optional)
├── Makefile                     ← Build for all platforms
├── build-android/               ← Gradle + CMake Android project
└── .github/workflows/build.yml  ← CI/CD pipeline
```

---

## 🧠 Memory Architecture

### Block Storage — Column-Major Layout
```c
uint8_t blocks[CHUNK_W][CHUNK_H]; // [x][y] = [16][256]
// 1 chunk = 16 × 256 × 1 byte = 4 KB
// 64 chunks (max pool) = 256 KB total
```
**Kyun column-major?** Side-scroller mein zyada tar vertical access hota hai
(gravity, mining depth). Column-major `[x][y]` cache-friendly access deta hai.

### Chunk Pool — Zero malloc per frame
```
CHUNK_POOL_MAX = 64 chunks
Memory = 64 × 4KB = 256KB (fixed, stack-allocated)

LRU Eviction: jab pool full ho, sabse purana (last_accessed frame counter)
chunk evict hota hai. Koi dynamic allocation nahi.
```

### Block Definition
```c
typedef struct {
    uint8_t id;          // 1 byte
    char name[32];       // 32 bytes
    uint8_t flags;       // 1 byte (bitfield: solid|liquid|transparent...)
    float hardness;      // 4 bytes
    uint8_t drop_id;     // 1 byte
    uint8_t tex_*;       // 3 bytes (atlas tile indices)
    uint8_t light_emit;  // 1 byte
    uint8_t friction;    // 1 byte
} BlockDef;              // Total: ~44 bytes per block type
// 64 block types = 2.8 KB — fits in L1 cache!
```

---

## 🌍 World Generation Flow

```
player_cx (chunk X)
    ↓
chunk_update_active()  — decides which chunks load/unload
    ↓
chunk_get_or_create(cx)
    ↓
worldgen_generate_chunk(chunk)
    ↓
  for each column x (0..15):
    1. perlin2d() → surface_height
    2. for each y (0..255):
       - y==0         → BEDROCK
       - y < surface-depth → STONE (with cave check)
       - y < surface-1     → DIRT
       - y == surface-1    → GRASS (or SAND near water)
       - y > surface, y < water_level → WATER
    3. Tree spawn (10% chance on GRASS)
```

### Perlin Noise — Pure C Implementation
```c
// Octave Perlin (no external lib)
float perlin2d(x, y, freq, depth, seed)
// depth=4 octaves for terrain → smooth hills
// depth=3 octaves for caves → irregular tunnels
// depth=2 octaves for ores → small clusters

// Seeded permutation table (Fisher-Yates shuffle)
// Different seed → completely different world
```

---

## ⚡ Physics — AABB Collision

### Player AABB Dimensions
```
Width:  0.6 blocks (fits through 1-block gap)
Height: 1.8 blocks (standard Minecraft player)
```

### Collision Resolution (Sweep + Binary Search)
```
Each axis resolved separately:
1. Move X → check overlap → binary search exact contact point
2. Move Y → check overlap → binary search exact contact point

9-point AABB check per step:
  [TL]  [TC]  [TR]
  [ML]        [MR]   ← Midpoints for thin walls
  [BL]  [BC]  [BR]

Sub-stepping for fast movement:
  if velocity > 0.4 blocks/frame → split into 4 sub-steps
  → Prevents tunneling through thin blocks
```

### Gravity & Physics Constants
```c
GRAVITY       = 20.0f   // blocks/s² (2x real gravity for snappy feel)
JUMP_FORCE    = 9.0f    // upward impulse
MAX_FALL_SPEED= 30.0f   // terminal velocity
GROUND_FRICTION= 0.75f  // per frame multiplier
AIR_RESISTANCE = 0.92f
WATER_GRAVITY  = 0.3x   // reduced in water
```

---

## 🎭 Player State Machine

```
            ┌─────────────────────────────────┐
            ↓                                 │
         [IDLE] ──move──→ [RUN] ──jump──→ [JUMP]
            │                                 │
         [MINE] ←──mining                 fall↓
            │                             [FALL]
         [SWIM] ←──in_water──────────────────┘
```

State transition logic `player_update_state()`:
- `in_water` → SWIM (overrides all)
- `!on_ground && vel_y < 0` → JUMP
- `!on_ground && vel_y > 0` → FALL  
- `is_mining` → MINE
- `|vel_x| > 0.1` → RUN
- else → IDLE

---

## 🖼️ Texture System — Zero-Crash Design

```
textures_init("assets/atlas.png")
    ↓
FileExists(path)?
  YES → LoadImage() → success? → GPU texture ✅
  NO  → generate fallback atlas (procedural)
                ↓
    16×16 tile generators (pure C math):
    - Dirt: noise pattern, brown tones
    - Grass: green top, dirt sides
    - Stone: gray with variation
    - Water: sine-wave blue (semi-transparent)
    - Torch: pixel art flame
    - Missing: magenta checkerboard
                ↓
    Pack into 256×256 atlas → GPU upload
    → Game runs with ZERO asset files ✅
```

### Atlas Layout
```
Atlas = 16 tiles wide × N tiles tall
Each tile = 16×16 RGBA pixels

Tile index → UV coords:
  tx = (index % 16) * 16
  ty = (index / 16) * 16
  rect = {tx, ty, 16, 16}

DrawTexturePro(atlas, src_rect, dst_rect, ...) → pixel-perfect rendering
```

---

## 🕹️ Input System — Platform Unified

```
Platform Detection at runtime:
  PLATFORM_ANDROID → is_touch = true
  PLATFORM_WEB     → is_touch = true (initially)
  Desktop          → is_touch = (touchpoints > 0)

Both paths write to same InputState struct:
  g_input.move_x    → player movement
  g_input.jump      → player jump
  g_input.mine      → start mining
  g_input.place     → place block
  g_input.interact_screen → where to mine/place
```

### Virtual Joystick Math
```c
// Joystick normalized output (-1 to +1):
dx = touch.x - joystick.center.x
dy = touch.y - joystick.center.y
dist = sqrt(dx² + dy²)
max_dist = JOYSTICK_RADIUS - KNOB_RADIUS  // = 35px

if dist > max_dist:
  dx = dx/dist * max_dist  // Clamp to circle
  dy = dy/dist * max_dist

normalized.x = dx / max_dist  // Range: -1..+1
normalized.y = dy / max_dist

Dead zone: if |normalized.x| < 0.15 → 0
```

### MD3 Touch Targets
```
Button radius = 30px → diameter = 60px
MD3 minimum = 48dp → 60px > 48dp ✅
Margin from edges = 20px ✅
```

---

## 🔨 Mining System — Vector2 Grid Math

### Screen → World Block Conversion
```c
// Camera offset = top-left corner of visible area in pixels
world_x = (screen_x + camera.target.x) / BLOCK_SIZE
world_y = (screen_y + camera.target.y) / BLOCK_SIZE

// Block coords = floor of world coords
block_x = (int32_t)floor(world_x)
block_y = (int32_t)floor(world_y)
```

### Mining Progress
```c
// Rate = 1/hardness blocks/second
rate = 1.0f / block.hardness  // e.g. dirt=2.0/s, stone=0.67/s
progress += rate * dt
if progress >= 1.0:
    world_set_block(bx, by, AIR)
    inventory_add(drop_id, 1)
```

### Reach Check
```c
dist = sqrt((bx+0.5 - player_cx)² + (by+0.5 - player_cy)²)
if dist > MINE_REACH (5.0 blocks): ignore input
```

---

## 🚀 CI/CD Pipeline Flow

```
git push main / git tag v1.0.0
        ↓
GitHub Actions triggers 3 parallel jobs:

Job 1: build-web (Ubuntu)
  ├── Install Emscripten SDK (cached)
  ├── Build Raylib for WASM (cached)
  ├── emcc compile → index.html + index.wasm + index.data
  └── Deploy to gh-pages branch → auto live at GitHub Pages

Job 2: build-android (Ubuntu)  
  ├── Setup Java 17 + Android SDK
  ├── Install NDK 26.3
  ├── Build Raylib for armeabi-v7a, arm64-v8a, x86_64
  ├── Generate release keystore (CI use)
  ├── Gradle assembleRelease
  │   └── splits.abi → 4 APKs automatically:
  │       • MC2D-1.0.0-armeabi-v7a-release.apk  (32-bit ARM)
  │       • MC2D-1.0.0-arm64-v8a-release.apk    (64-bit ARM) ← Most devices
  │       • MC2D-1.0.0-x86_64-release.apk       (Emulator)
  │       • MC2D-1.0.0-universal-release.apk     (All ABIs bundled)
  └── Upload as artifacts (30 day retention)

Job 3: create-release (only on tag)
  ├── Download APK artifacts
  └── Create GitHub Release with all 4 APKs attached
```

---

## 🛠️ Local Build (Termux)

```bash
# Install deps in Termux
pkg install clang make

# Get Raylib (pre-built for Android available)
pkg install raylib

# Build and run
make native && ./mc2d

# Web build (needs emsdk)
source ~/emsdk/emsdk_env.sh
make web
make web-serve  # localhost:8080
```

---

## 📊 Performance Budget

| System | Memory | CPU/frame |
|--------|--------|-----------|
| Chunk pool (64 chunks) | 256 KB | ~0ms |
| Block registry (64 types) | ~3 KB | O(n) lookup |
| Texture atlas | ~1 MB VRAM | 1 drawcall/frame |
| Player physics | <1 KB | ~0.1ms |
| World render | 0 (direct) | O(visible blocks) |
| Input state | <1 KB | ~0ms |
| **Total RAM** | **~5 MB** | |

Target: **60 FPS** on Android ARMv7 (2015+ devices)

---

*Built with ❤️ by ArnabLabZ Studio — GamerArnabXYZ*
