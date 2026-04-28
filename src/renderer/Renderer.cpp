#include "Renderer.h"
#include <cstdio>
#include <cstring>
#include <cmath>

// ─── Constructor/Destructor ────────────────────────────────────────────────────
Renderer::Renderer() : m_rend(nullptr), m_font(nullptr) {}
Renderer::~Renderer() { shutdown(); }

bool Renderer::init(SDL_Window* win) {
    // Hardware accelerated renderer, VSync for battery saving on mobile
    m_rend = SDL_CreateRenderer(win, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!m_rend) return false;

    SDL_SetRenderDrawBlendMode(m_rend, SDL_BLENDMODE_BLEND);

    // Load texture atlas (falls back to generated colors if png missing)
    m_atlas.load(m_rend, "assets/textures/atlas.png");

    // Load font (optional, shows FPS etc)
    if (TTF_WasInit()) {
        m_font = TTF_OpenFont("assets/fonts/font.ttf", 12);
    }
    return true;
}

void Renderer::shutdown() {
    m_atlas.free();
    if (m_font) { TTF_CloseFont(m_font); m_font = nullptr; }
    if (m_rend) { SDL_DestroyRenderer(m_rend); m_rend = nullptr; }
}

// ─── Main Render ──────────────────────────────────────────────────────────────
void Renderer::render(const World& world, const Player& player,
                      const Camera& cam, float fps,
                      CraftingUI* craftUI, const InputManager* input) {
    uint8_t ambient = world.getAmbientLight();

    // 1. Sky
    renderSky(ambient);

    // 2. World chunks (with frustum culling)
    renderChunks(world, cam, ambient);

    // 3. Break overlay
    renderBreak(player, cam);

    // 4. Player sprite
    renderPlayer(player, cam, ambient);

    // 5. HUD (hotbar, FPS) — skip hotbar if inventory open
    renderHUD(player, fps);

    // 6. Touch joystick overlay (mobile only)
    if (input && input->joyActive()) {
        renderTouchOverlay(*input);
    }

    // 7. Inventory/crafting UI (rendered on top)
    if (craftUI && player.inventory().isOpenConst()) {
        craftUI->render(m_rend, m_atlas, const_cast<Player&>(player).inventory());
    }

    SDL_RenderPresent(m_rend);
}

// ─── Sky ──────────────────────────────────────────────────────────────────────
void Renderer::renderSky(uint8_t ambient) {
    // Lerp sky color: day=87,167,220  night=10,10,40
    float t = ambient / 255.0f;
    uint8_t r = (uint8_t)(10 + t * (87 - 10));
    uint8_t g = (uint8_t)(10 + t * (167 - 10));
    uint8_t b = (uint8_t)(40 + t * (220 - 40));
    SDL_SetRenderDrawColor(m_rend, r, g, b, 255);
    SDL_RenderClear(m_rend);
}

// ─── Chunk Rendering with culling ─────────────────────────────────────────────
void Renderer::renderChunks(const World& world, const Camera& cam, uint8_t ambient) {
    // Only draw blocks visible on screen (+1 block margin)
    int camBX = World::pixToBlock((int)cam.x()) - 1;
    int camBY = World::pixToBlock((int)cam.y()) - 1;
    int visW  = (WINDOW_W / BLOCK_SIZE) + 3;
    int visH  = (WINDOW_H / BLOCK_SIZE) + 3;

    for (int dy = 0; dy < visH; dy++) {
        int by = camBY + dy - visH / 2;
        if (by < 0 || by >= CHUNK_H) continue;

        for (int dx = 0; dx < visW; dx++) {
            int bx = camBX + dx - visW / 2;

            uint8_t id = world.getBlock(bx, by);
            if (id == BLOCK_AIR) continue;

            // Skip if all neighbors are solid (interior culling)
            // Simple: skip only if ALL 4 neighbors are solid & not transparent
            bool n = blockIsSolid(world.getBlock(bx, by - 1)) && !blockIsTransparent(world.getBlock(bx, by - 1));
            bool s = blockIsSolid(world.getBlock(bx, by + 1)) && !blockIsTransparent(world.getBlock(bx, by + 1));
            bool l = blockIsSolid(world.getBlock(bx - 1, by)) && !blockIsTransparent(world.getBlock(bx - 1, by));
            bool r = blockIsSolid(world.getBlock(bx + 1, by)) && !blockIsTransparent(world.getBlock(bx + 1, by));
            if (n && s && l && r) continue; // hidden block, skip

            int sx = cam.worldToScreenX((float)(bx * BLOCK_SIZE));
            int sy = cam.worldToScreenY((float)(by * BLOCK_SIZE));

            renderBlock(sx, sy, id, ambient);
        }
    }
}

// ─── Single Block ─────────────────────────────────────────────────────────────
void Renderer::renderBlock(int sx, int sy, uint8_t blockID, uint8_t ambient) {
    SDL_Rect dst = { sx, sy, BLOCK_SIZE, BLOCK_SIZE };

    if (m_atlas.isLoaded()) {
        SDL_Rect src = m_atlas.getRect(blockID, false);
        // Apply ambient lighting via color modulation
        SDL_SetTextureColorMod(m_atlas.texture(), ambient, ambient, ambient);
        SDL_RenderCopy(m_rend, m_atlas.texture(), &src, &dst);
    } else {
        // Fallback: solid colors matching block types
        static const SDL_Color FALLBACK[] = {
            {0,0,0,0},         // AIR
            {58,122,28,255},   // GRASS
            {122,82,48,255},   // DIRT
            {138,138,138,255}, // STONE
            {232,210,138,255}, // SAND
            {154,144,144,255}, // GRAVEL
            {139,105,20,255},  // WOOD
            {42,138,28,255},   // LEAVES
            {34,85,170,180},   // WATER
            {90,90,90,255},    // COAL ORE
            {122,122,122,255}, // IRON ORE
            {232,200,32,255},  // GOLD ORE
            {0,212,212,255},   // DIAMOND ORE
            {200,160,96,255},  // PLANKS
            {136,136,136,255}, // COBBLESTONE
            {68,68,68,255},    // BEDROCK
            {200,238,255,160}, // GLASS
        };
        if (blockID < sizeof(FALLBACK)/sizeof(FALLBACK[0])) {
            SDL_Color c = FALLBACK[blockID];
            // Dim by ambient
            c.r = (uint8_t)(c.r * ambient / 255);
            c.g = (uint8_t)(c.g * ambient / 255);
            c.b = (uint8_t)(c.b * ambient / 255);
            SDL_SetRenderDrawColor(m_rend, c.r, c.g, c.b, c.a);
            SDL_RenderFillRect(m_rend, &dst);
            // Dark border for depth
            SDL_SetRenderDrawColor(m_rend, 0, 0, 0, 60);
            SDL_RenderDrawRect(m_rend, &dst);
        }
    }
}

// ─── Break Overlay (crack progress) ───────────────────────────────────────────
void Renderer::renderBreak(const Player& p, const Camera& cam) {
    if (!p.isBreaking()) return;
    float prog = p.breakProgress();
    uint8_t id = 0; // we just show overlay, real hardness in player

    int sx = cam.worldToScreenX((float)(p.breakTargetX() * BLOCK_SIZE));
    int sy = cam.worldToScreenY((float)(p.breakTargetY() * BLOCK_SIZE));

    // Draw darkening overlay proportional to break progress
    // prog / hardness handled in Player; here prog = m_breakTimer
    SDL_Rect dst = {sx, sy, BLOCK_SIZE, BLOCK_SIZE};
    uint8_t alpha = (uint8_t)(prog * 180.0f);
    alpha = alpha > 200 ? 200 : alpha;
    SDL_SetRenderDrawColor(m_rend, 0, 0, 0, alpha);
    SDL_RenderFillRect(m_rend, &dst);

    // Highlight border
    SDL_SetRenderDrawColor(m_rend, 255, 255, 255, 120);
    SDL_RenderDrawRect(m_rend, &dst);
}

// ─── Player Sprite ────────────────────────────────────────────────────────────
void Renderer::renderPlayer(const Player& p, const Camera& cam, uint8_t ambient) {
    int sx = cam.worldToScreenX(p.x());
    int sy = cam.worldToScreenY(p.y());

    float t = ambient / 255.0f;
    uint8_t dim = (uint8_t)(ambient);

    // Body (torso + legs)
    SDL_SetRenderDrawColor(m_rend, (uint8_t)(64*t), (uint8_t)(128*t), (uint8_t)(220*t), 255); // blue shirt
    SDL_Rect body = {sx + 4, sy + 12, PLAYER_W - 8, 20};
    SDL_RenderFillRect(m_rend, &body);

    // Legs
    SDL_SetRenderDrawColor(m_rend, (uint8_t)(30*t), (uint8_t)(80*t), (uint8_t)(160*t), 255);
    SDL_Rect legL = {sx + 4, sy + 32, (PLAYER_W - 8) / 2 - 1, 12};
    SDL_Rect legR = {sx + PLAYER_W / 2 + 1, sy + 32, (PLAYER_W - 8) / 2 - 1, 12};
    SDL_RenderFillRect(m_rend, &legL);
    SDL_RenderFillRect(m_rend, &legR);

    // Head
    SDL_SetRenderDrawColor(m_rend, (uint8_t)(230*t), (uint8_t)(190*t), (uint8_t)(140*t), 255); // skin
    SDL_Rect head = {sx + 3, sy, PLAYER_W - 6, 12};
    SDL_RenderFillRect(m_rend, &head);

    // Eyes
    SDL_SetRenderDrawColor(m_rend, 0, 0, 0, 255);
    SDL_Rect eye1 = {sx + 6, sy + 3, 3, 3};
    SDL_Rect eye2 = {sx + 15, sy + 3, 3, 3};
    SDL_RenderFillRect(m_rend, &eye1);
    SDL_RenderFillRect(m_rend, &eye2);
}

// ─── HUD ──────────────────────────────────────────────────────────────────────
void Renderer::renderHUD(const Player& p, float fps) {
    renderHotbar(p);

    // FPS counter (top-left)
    char buf[32];
    snprintf(buf, sizeof(buf), "FPS: %.0f", fps);
    drawText(buf, 6, 6, {255, 255, 100, 255});

    // Crosshair center
    int cx = WINDOW_W / 2, cy = WINDOW_H / 2;
    SDL_SetRenderDrawColor(m_rend, 255, 255, 255, 160);
    SDL_Rect ch1 = {cx - 8, cy - 1, 16, 2};
    SDL_Rect ch2 = {cx - 1, cy - 8, 2, 16};
    SDL_RenderFillRect(m_rend, &ch1);
    SDL_RenderFillRect(m_rend, &ch2);
}

// ─── Hotbar ───────────────────────────────────────────────────────────────────
void Renderer::renderHotbar(const Player& p) {
    const int SLOT_SIZE  = 44;
    const int SLOT_PAD   = 4;
    const int BAR_W      = HOTBAR_SLOTS * (SLOT_SIZE + SLOT_PAD) - SLOT_PAD;
    const int BAR_X      = (WINDOW_W - BAR_W) / 2;
    const int BAR_Y      = WINDOW_H - SLOT_SIZE - 12;

    for (int i = 0; i < HOTBAR_SLOTS; i++) {
        int sx = BAR_X + i * (SLOT_SIZE + SLOT_PAD);
        bool selected = (i == p.inventory().selected());

        // Slot background
        SDL_Color bg = selected ? SDL_Color{200, 200, 200, 210} : SDL_Color{80, 80, 80, 180};
        drawRect(sx, BAR_Y, SLOT_SIZE, SLOT_SIZE, bg, true);
        drawRect(sx, BAR_Y, SLOT_SIZE, SLOT_SIZE, {0, 0, 0, 255}, false);

        // Item inside slot
        const ItemStack& item = p.inventory().hotbarSlot(i);
        if (!item.empty()) {
            drawItem(item.id, sx + 6, BAR_Y + 6, SLOT_SIZE - 12);
            // Count
            if (item.count > 1) {
                char cnt[8];
                snprintf(cnt, sizeof(cnt), "%d", item.count);
                drawText(cnt, sx + SLOT_SIZE - 16, BAR_Y + SLOT_SIZE - 14, {255, 255, 255, 255});
            }
        }
    }
}

// ─── Draw helpers ─────────────────────────────────────────────────────────────
void Renderer::drawRect(int x, int y, int w, int h, SDL_Color col, bool fill) {
    SDL_SetRenderDrawColor(m_rend, col.r, col.g, col.b, col.a);
    SDL_Rect r = {x, y, w, h};
    if (fill) SDL_RenderFillRect(m_rend, &r);
    else       SDL_RenderDrawRect(m_rend, &r);
}

void Renderer::drawText(const std::string& txt, int x, int y, SDL_Color col) {
    if (!m_font) {
        // Fallback: tiny colored dot if no font loaded
        SDL_SetRenderDrawColor(m_rend, col.r, col.g, col.b, col.a);
        SDL_Rect r = {x, y, (int)txt.size() * 6, 10};
        SDL_RenderFillRect(m_rend, &r);
        return;
    }
    SDL_Surface* surf = TTF_RenderText_Solid(m_font, txt.c_str(), col);
    if (!surf) return;
    SDL_Texture* tex = SDL_CreateTextureFromSurface(m_rend, surf);
    SDL_FreeSurface(surf);
    if (!tex) return;
    int tw, th;
    SDL_QueryTexture(tex, nullptr, nullptr, &tw, &th);
    SDL_Rect dst = {x, y, tw, th};
    SDL_RenderCopy(m_rend, tex, nullptr, &dst);
    SDL_DestroyTexture(tex);
}

void Renderer::drawItem(uint8_t id, int x, int y, int size) {
    if (id == 0) return;
    SDL_Rect dst = {x, y, size, size};
    if (m_atlas.isLoaded()) {
        SDL_Rect src = m_atlas.getRect(id, false);
        SDL_SetTextureColorMod(m_atlas.texture(), 255, 255, 255);
        SDL_RenderCopy(m_rend, m_atlas.texture(), &src, &dst);
    } else {
        // Fallback color box
        renderBlock(x, y, id, 255);
    }
}

// ─── Touch Joystick Overlay ───────────────────────────────────────────────────
// Renders virtual joystick base + stick nub on screen for touch devices
void Renderer::renderTouchOverlay(const InputManager& input) {
    if (!input.joyActive()) return;

    float bx = input.joyBaseX(),  by = input.joyBaseY();
    float sx = input.joyStickX(), sy = input.joyStickY();
    int   R  = (int)TOUCH_JOYSTICK_R;

    // Outer ring
    SDL_SetRenderDrawColor(m_rend, 255, 255, 255, 50);
    // Draw circle approximation with filled rect + ring
    SDL_Rect outer = {(int)bx - R, (int)by - R, R*2, R*2};
    SDL_SetRenderDrawColor(m_rend, 255, 255, 255, 40);
    SDL_RenderFillRect(m_rend, &outer);
    SDL_SetRenderDrawColor(m_rend, 255, 255, 255, 120);
    SDL_RenderDrawRect(m_rend, &outer);

    // Inner nub
    int nr = 18;
    SDL_Rect nub = {(int)sx - nr, (int)sy - nr, nr*2, nr*2};
    SDL_SetRenderDrawColor(m_rend, 255, 255, 255, 160);
    SDL_RenderFillRect(m_rend, &nub);
    SDL_SetRenderDrawColor(m_rend, 200, 200, 200, 220);
    SDL_RenderDrawRect(m_rend, &nub);
}
