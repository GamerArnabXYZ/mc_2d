#include "Renderer.h"
#include <cstdio>
#include <cmath>

Renderer::Renderer() : m_rend(nullptr), m_font(nullptr) {}
Renderer::~Renderer() { shutdown(); }

bool Renderer::init(SDL_Window* win) {
    m_rend = SDL_CreateRenderer(win, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!m_rend) return false;
    SDL_SetRenderDrawBlendMode(m_rend, SDL_BLENDMODE_BLEND);
    m_atlas.load(m_rend, "assets/textures/atlas.png");
    if (TTF_WasInit())
        m_font = TTF_OpenFont("assets/fonts/font.ttf", 11);
    return true;
}

void Renderer::shutdown() {
    m_atlas.free();
    if (m_font)  { TTF_CloseFont(m_font); m_font = nullptr; }
    for (auto& pair : m_textCache) {
        SDL_DestroyTexture(pair.second);
    }
    m_textCache.clear();
    if (m_rend)  { SDL_DestroyRenderer(m_rend); m_rend = nullptr; }
}

// ─── Block color palette (nice colors) ───────────────────────────────────────
static SDL_Color blockColor(uint8_t id) {
    switch(id) {
        case BLOCK_GRASS:       return {80,  150, 50,  255};
        case BLOCK_DIRT:        return {130, 90,  55,  255};
        case BLOCK_STONE:       return {130, 130, 130, 255};
        case BLOCK_SAND:        return {220, 200, 130, 255};
        case BLOCK_GRAVEL:      return {150, 140, 130, 255};
        case BLOCK_WOOD:        return {140, 100, 50,  255};
        case BLOCK_LEAVES:      return {45,  130, 35,  200};
        case BLOCK_WATER:       return {40,  100, 200, 160};
        case BLOCK_COAL_ORE:    return {80,  80,  80,  255};
        case BLOCK_IRON_ORE:    return {150, 110, 80,  255};
        case BLOCK_GOLD_ORE:    return {200, 170, 30,  255};
        case BLOCK_DIAMOND_ORE: return {30,  210, 210, 255};
        case BLOCK_PLANKS:      return {190, 145, 80,  255};
        case BLOCK_COBBLESTONE: return {110, 110, 110, 255};
        case BLOCK_BEDROCK:     return {40,  40,  40,  255};
        case BLOCK_GLASS:       return {180, 220, 255, 100};
        case BLOCK_TORCH:       return {255, 200, 50,  255};
        case BLOCK_CRAFTING:    return {160, 110, 60,  255};
        case BLOCK_CHEST:       return {180, 130, 50,  255};
        case BLOCK_SNOW:        return {230, 240, 255, 255};
        case BLOCK_ICE:         return {150, 200, 240, 200};
        case BLOCK_CACTUS:      return {50,  160, 50,  255};
        default:                return {200, 50,  200, 255};
    }
}

// ─── Top face (lighter) and side shade ───────────────────────────────────────
static SDL_Color lighten(SDL_Color c, float f) {
    return {(uint8_t)CLAMP(c.r*f,0,255),(uint8_t)CLAMP(c.g*f,0,255),
            (uint8_t)CLAMP(c.b*f,0,255),c.a};
}
static SDL_Color dimBy(SDL_Color c, uint8_t ambient) {
    float t = ambient / 255.0f;
    return {(uint8_t)(c.r*t),(uint8_t)(c.g*t),(uint8_t)(c.b*t),c.a};
}

// ─── Render ───────────────────────────────────────────────────────────────────
void Renderer::render(const World& world, const Player& player,
                      const Camera& cam, float fps,
                      CraftingUI* craftUI, const InputManager* input) {
    uint8_t amb = world.getAmbientLight();

    renderSky(amb);
    renderChunks(world, cam, amb);
    renderBreakOverlay(player, cam, world);
    renderPlayer(player, cam, amb);
    renderHUD(player, fps, input);

    if (craftUI && player.inventory().isOpen())
        craftUI->render(this, player.inventory());

    SDL_RenderPresent(m_rend);
}

void Renderer::renderSky(uint8_t amb) {
    float t = amb / 255.0f;
    SDL_SetRenderDrawColor(m_rend,
        (uint8_t)(8  + t*100), (uint8_t)(12 + t*160), (uint8_t)(30 + t*200), 255);
    SDL_RenderClear(m_rend);

    // Simple sun/moon indicator
    if (t > 0.4f) {
        // Sun
        SDL_SetRenderDrawColor(m_rend, 255, 230, 80, (uint8_t)((t-0.4f)*420));
        SDL_Rect sun = {WINDOW_W - 60, 20, 36, 36};
        SDL_RenderFillRect(m_rend, &sun);
    } else {
        // Moon
        SDL_SetRenderDrawColor(m_rend, 200, 200, 220, 160);
        SDL_Rect moon = {WINDOW_W - 60, 20, 28, 28};
        SDL_RenderFillRect(m_rend, &moon);
    }
}

void Renderer::renderChunks(const World& world, const Camera& cam, uint8_t amb) {
    int visW = WINDOW_W / BLOCK_SIZE + 3;
    int visH = WINDOW_H / BLOCK_SIZE + 3;
    int camBX = (int)floorf(cam.x() / BLOCK_SIZE) - visW/2;
    int camBY = (int)floorf(cam.y() / BLOCK_SIZE) - visH/2;

    for (int dy = 0; dy < visH; dy++) {
        int by = camBY + dy;
        if (by < 0 || by >= CHUNK_H) continue;
        for (int dx = 0; dx < visW; dx++) {
            int bx = camBX + dx;
            uint8_t id = world.getBlock(bx, by);
            if (id == BLOCK_AIR) continue;

            // Simple occlusion: skip if all 4 neighbors solid+opaque
            bool nS = !blockIsTransparent(world.getBlock(bx, by-1)) && blockIsSolid(world.getBlock(bx,by-1));
            bool sS = !blockIsTransparent(world.getBlock(bx, by+1)) && blockIsSolid(world.getBlock(bx,by+1));
            bool lS = !blockIsTransparent(world.getBlock(bx-1,by))  && blockIsSolid(world.getBlock(bx-1,by));
            bool rS = !blockIsTransparent(world.getBlock(bx+1,by))  && blockIsSolid(world.getBlock(bx+1,by));
            if (nS && sS && lS && rS) continue;

            int sx = cam.worldToScreenX((float)(bx * BLOCK_SIZE));
            int sy = cam.worldToScreenY((float)(by * BLOCK_SIZE));
            renderBlock(sx, sy, id, amb, world.getBlock(bx, by-1) == BLOCK_AIR);
        }
    }
}

void Renderer::renderBlock(int sx, int sy, uint8_t id, uint8_t amb, bool topExposed) {
    SDL_Rect dst = {sx, sy, BLOCK_SIZE, BLOCK_SIZE};
    
    if (m_atlas.isLoaded()) {
        SDL_SetTextureColorMod(m_atlas.texture(), amb, amb, amb);
        
        // Main face
        SDL_Rect src = m_atlas.getRect(id, false);
        SDL_RenderCopy(m_rend, m_atlas.texture(), &src, &dst);

        // Top face highlight (only if top is exposed to air)
        if (topExposed && BLOCK_SIZE >= 16) {
            SDL_Rect srcTop = m_atlas.getRect(id, true);
            SDL_Rect dstTop = {sx, sy, BLOCK_SIZE, BLOCK_SIZE/5};
            srcTop.h /= 5; // Use top 1/5th of the top texture
            SDL_RenderCopy(m_rend, m_atlas.texture(), &srcTop, &dstTop);
        }
    } else {
        SDL_Color base = dimBy(blockColor(id), amb);
        SDL_SetRenderDrawColor(m_rend, base.r, base.g, base.b, base.a);
        SDL_RenderFillRect(m_rend, &dst);

        if (topExposed && BLOCK_SIZE >= 16) {
            SDL_Color top = dimBy(lighten(blockColor(id), 1.35f), amb);
            SDL_Rect topBar = {sx, sy, BLOCK_SIZE, BLOCK_SIZE/5};
            SDL_SetRenderDrawColor(m_rend, top.r, top.g, top.b, top.a);
            SDL_RenderFillRect(m_rend, &topBar);
        }
    }

    // Grid line (subtle border)
    SDL_SetRenderDrawColor(m_rend, 0, 0, 0, 25);
    SDL_RenderDrawRect(m_rend, &dst);
}

void Renderer::renderBreakOverlay(const Player& p, const Camera& cam, const World& w) {
    if (!p.isBreaking()) return;
    int bx = p.breakTargetX(), by = p.breakTargetY();
    uint8_t id = w.getBlock(bx, by);
    if (id == BLOCK_AIR) return;

    float hard = BLOCK_DEFS[id].hardness;
    if (hard <= 0) hard = 0.3f;
    float prog = p.breakProgress() / hard;
    prog = CLAMP(prog, 0, 1);

    int sx = cam.worldToScreenX((float)(bx * BLOCK_SIZE));
    int sy = cam.worldToScreenY((float)(by * BLOCK_SIZE));
    SDL_Rect dst = {sx, sy, BLOCK_SIZE, BLOCK_SIZE};

    // Crack overlay: darken proportionally
    uint8_t alpha = (uint8_t)(prog * 160);
    SDL_SetRenderDrawColor(m_rend, 0, 0, 0, alpha);
    SDL_RenderFillRect(m_rend, &dst);

    // White border pulse
    SDL_SetRenderDrawColor(m_rend, 255, 255, 255, 180);
    SDL_RenderDrawRect(m_rend, &dst);

    // Progress bar at top of block
    SDL_Rect bar    = {sx, sy - 6, BLOCK_SIZE, 4};
    SDL_Rect fill   = {sx, sy - 6, (int)(BLOCK_SIZE * prog), 4};
    SDL_SetRenderDrawColor(m_rend, 60, 60, 60, 200);
    SDL_RenderFillRect(m_rend, &bar);
    SDL_SetRenderDrawColor(m_rend, 255, 160, 30, 255);
    SDL_RenderFillRect(m_rend, &fill);
}

void Renderer::renderPlayer(const Player& p, const Camera& cam, uint8_t amb) {
    int sx = cam.worldToScreenX(p.x());
    int sy = cam.worldToScreenY(p.y());
    float t = amb / 255.0f;

    // Shadow on ground
    SDL_SetRenderDrawColor(m_rend, 0, 0, 0, 60);
    SDL_Rect shadow = {sx + 2, sy + PLAYER_H - 2, PLAYER_W - 4, 4};
    SDL_RenderFillRect(m_rend, &shadow);

    // Legs
    SDL_SetRenderDrawColor(m_rend, (uint8_t)(40*t), (uint8_t)(80*t), (uint8_t)(160*t), 255);
    SDL_Rect legL = {sx+4,         sy+PLAYER_H-14, PLAYER_W/2-5, 14};
    SDL_Rect legR = {sx+PLAYER_W/2+1, sy+PLAYER_H-14, PLAYER_W/2-5, 14};
    SDL_RenderFillRect(m_rend, &legL);
    SDL_RenderFillRect(m_rend, &legR);

    // Body
    SDL_SetRenderDrawColor(m_rend, (uint8_t)(60*t), (uint8_t)(120*t), (uint8_t)(210*t), 255);
    SDL_Rect body = {sx+3, sy+12, PLAYER_W-6, PLAYER_H-26};
    SDL_RenderFillRect(m_rend, &body);

    // Head
    SDL_SetRenderDrawColor(m_rend, (uint8_t)(220*t), (uint8_t)(180*t), (uint8_t)(130*t), 255);
    SDL_Rect head = {sx+4, sy, PLAYER_W-8, 13};
    SDL_RenderFillRect(m_rend, &head);

    // Eyes
    SDL_SetRenderDrawColor(m_rend, 30, 30, 80, 255);
    SDL_Rect eye1 = {sx+6, sy+3, 3, 3};
    SDL_Rect eye2 = {sx+PLAYER_W-9, sy+3, 3, 3};
    SDL_RenderFillRect(m_rend, &eye1);
    SDL_RenderFillRect(m_rend, &eye2);

    // Held item in hand
    const ItemStack& held = const_cast<Player&>(p).inventory().heldItem();
    if (!held.empty()) {
        drawItem(held.id, sx + PLAYER_W, sy + PLAYER_H/2, 12);
    }
}

void Renderer::renderHUD(const Player& p, float fps, const InputManager* input) {
    // Crosshair
    int cx = WINDOW_W/2, cy = WINDOW_H/2;
    SDL_SetRenderDrawColor(m_rend, 255, 255, 255, 200);
    SDL_Rect ch1 = {cx-10, cy-1, 20, 2};
    SDL_Rect ch2 = {cx-1, cy-10, 2, 20};
    SDL_RenderFillRect(m_rend, &ch1);
    SDL_RenderFillRect(m_rend, &ch2);
    SDL_SetRenderDrawColor(m_rend, 0, 0, 0, 100);
    SDL_Rect chs = {cx-11, cy-2, 22, 4};
    SDL_RenderDrawRect(m_rend, &chs);

    // Hotbar
    renderHotbar(p);

    // FPS (top-left)
    char buf[32];
    snprintf(buf, sizeof(buf), "FPS:%.0f", fps);
    drawText(buf, 6, 6, {255,255,100,255});

    // Touch joystick overlay
    if (input && input->joyActive()) {
        renderTouchOverlay(*input);
    }

    // Touch control hints (semi-transparent, shown always on mobile)
#if defined(PLATFORM_ANDROID) || defined(PLATFORM_WEB)
    if (!input || !input->joyActive()) {
        // Hint circles when joystick not active
        SDL_SetRenderDrawColor(m_rend, 255,255,255,25);
        // Left joystick area hint
        SDL_Rect joyHint = {10, WINDOW_H-140, 120, 120};
        SDL_RenderDrawRect(m_rend, &joyHint);
        // Right action area hint
        SDL_Rect actHint = {WINDOW_W/2+10, WINDOW_H-80, WINDOW_W/2-20, 60};
        SDL_RenderDrawRect(m_rend, &actHint);
    }
#endif

    // Inventory button (top right)
    SDL_SetRenderDrawColor(m_rend, 80, 80, 80, 180);
    SDL_Rect invBtn = {WINDOW_W-70, 10, 60, 60};
    SDL_RenderFillRect(m_rend, &invBtn);
    SDL_SetRenderDrawColor(m_rend, 200, 200, 200, 255);
    SDL_RenderDrawRect(m_rend, &invBtn);
    drawText("INV", WINDOW_W-60, 34, {255,255,255,255});
}

void Renderer::renderHotbar(const Player& p) {
    const int SZ  = 46;
    const int PAD = 5;
    const int BAR_W = HOTBAR_SLOTS * (SZ + PAD) - PAD;
    const int BX = (WINDOW_W - BAR_W) / 2;
    const int BY = WINDOW_H - SZ - 8;

    // Background bar
    SDL_SetRenderDrawColor(m_rend, 0, 0, 0, 120);
    SDL_Rect bar = {BX-6, BY-4, BAR_W+12, SZ+8};
    SDL_RenderFillRect(m_rend, &bar);

    for (int i = 0; i < HOTBAR_SLOTS; i++) {
        int sx = BX + i*(SZ+PAD);
        bool sel = (i == p.inventory().selected());

        // Slot bg
        SDL_Color bg = sel ? SDL_Color{180,160,60,230} : SDL_Color{70,70,70,200};
        SDL_SetRenderDrawColor(m_rend, bg.r, bg.g, bg.b, bg.a);
        SDL_Rect slot = {sx, BY, SZ, SZ};
        SDL_RenderFillRect(m_rend, &slot);
        SDL_SetRenderDrawColor(m_rend, sel?255:120, sel?255:120, sel?0:120, 255);
        SDL_RenderDrawRect(m_rend, &slot);

        const ItemStack& item = p.inventory().hotbarSlot(i);
        if (!item.empty()) {
            drawItem(item.id, sx+5, BY+5, SZ-10);
            // Count
            if (item.count > 1) {
                char cnt[8]; snprintf(cnt,sizeof(cnt),"%d",item.count);
                drawText(cnt, sx+SZ-20, BY+SZ-15, {255,255,255,255});
            }
        }
    }
}

void Renderer::renderTouchOverlay(const InputManager& inp) {
    if (!inp.joyActive()) return;
    float bx=inp.joyBaseX(), by=inp.joyBaseY();
    float sx=inp.joyStickX(), sy=inp.joyStickY();
    int R=(int)TOUCH_JOYSTICK_R;

    // Outer ring
    SDL_SetRenderDrawColor(m_rend,255,255,255,35);
    SDL_Rect outer={( int)bx-R,(int)by-R,R*2,R*2};
    SDL_RenderFillRect(m_rend,&outer);
    SDL_SetRenderDrawColor(m_rend,255,255,255,100);
    SDL_RenderDrawRect(m_rend,&outer);

    // Inner stick nub
    int nr=20;
    SDL_Rect nub={(int)sx-nr,(int)sy-nr,nr*2,nr*2};
    SDL_SetRenderDrawColor(m_rend,255,255,255,150);
    SDL_RenderFillRect(m_rend,&nub);
    SDL_SetRenderDrawColor(m_rend,220,220,220,220);
    SDL_RenderDrawRect(m_rend,&nub);
}

void Renderer::drawRect(int x,int y,int w,int h,SDL_Color c,bool fill){
    SDL_SetRenderDrawColor(m_rend,c.r,c.g,c.b,c.a);
    SDL_Rect r={x,y,w,h};
    if(fill) SDL_RenderFillRect(m_rend,&r);
    else SDL_RenderDrawRect(m_rend,&r);
}

void Renderer::drawText(const std::string& txt,int x,int y,SDL_Color col){
    if (!m_font) {
        SDL_SetRenderDrawColor(m_rend,col.r,col.g,col.b,col.a);
        SDL_Rect r={x,y,(int)txt.size()*6,8};
        SDL_RenderFillRect(m_rend,&r);
        return;
    }

    uint32_t colKey = (col.r << 24) | (col.g << 16) | (col.b << 8) | col.a;
    TextCacheKey key = {txt, colKey};
    
    SDL_Texture* tex = nullptr;
    auto it = m_textCache.find(key);
    if (it != m_textCache.end()) {
        tex = it->second;
    } else {
        SDL_Surface* s = TTF_RenderText_Blended(m_font, txt.c_str(), col);
        if (!s) return;
        tex = SDL_CreateTextureFromSurface(m_rend, s);
        SDL_FreeSurface(s);
        if (!tex) return;
        m_textCache[key] = tex;
        
        // Simple cache pruning if it gets too large
        if (m_textCache.size() > 100) {
            auto kill = m_textCache.begin();
            SDL_DestroyTexture(kill->second);
            m_textCache.erase(kill);
        }
    }

    int tw, th;
    SDL_QueryTexture(tex, nullptr, nullptr, &tw, &th);
    SDL_Rect dst = {x, y, tw, th};
    SDL_RenderCopy(m_rend, tex, nullptr, &dst);
}

void Renderer::drawItem(uint8_t id,int x,int y,int size){
    if (m_atlas.isLoaded()) {
        SDL_SetTextureColorMod(m_atlas.texture(), 255, 255, 255);
        SDL_Rect src = m_atlas.getRect(id, false);
        SDL_Rect dst = {x, y, size, size};
        SDL_RenderCopy(m_rend, m_atlas.texture(), &src, &dst);
    } else {
        SDL_Color c = blockColor(id);
        SDL_SetRenderDrawColor(m_rend, c.r, c.g, c.b, c.a);
        SDL_Rect r = {x, y, size, size};
        SDL_RenderFillRect(m_rend, &r);
    }
}
