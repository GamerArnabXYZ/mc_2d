#include "TextureAtlas.h"
#include "../core/SDL_incl.h"
#include <cstdio>

TextureAtlas::TextureAtlas() : m_tex(nullptr), m_atlasW(0), m_atlasH(0) {}
TextureAtlas::~TextureAtlas() { free(); }

bool TextureAtlas::load(SDL_Renderer* renderer, const std::string& path) {
    SDL_Surface* surf = IMG_Load(path.c_str());
    if (!surf) {
        // Fallback: generate a colored grid atlas programmatically
        // Each cell = solid color representing the block type
        int sz = ATLAS_COLS * ATLAS_BLOCK_SIZE;
        surf = SDL_CreateRGBSurface(0, sz, sz, 32, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
        if (!surf) return false;

        // Fill each cell with a distinct color (RGBA)
        static const Uint32 COLORS[] = {
            0x00000000, // AIR (transparent)
            0x3A7A1CFF, // GRASS
            0x7A5230FF, // DIRT
            0x8A8A8AFF, // STONE
            0xE8D28AFF, // SAND
            0x9A9090FF, // GRAVEL
            0x8B6914FF, // WOOD
            0x2A8A1CFF, // LEAVES
            0x2255AAFF, // WATER
            0x5A5A5AFF, // COAL ORE
            0x7A7A7AFF, // IRON ORE
            0xE8C820FF, // GOLD ORE
            0x00D4D4FF, // DIAMOND ORE
            0xC8A060FF, // PLANKS
            0x888888FF, // COBBLESTONE
            0x444444FF, // BEDROCK
            0xCCEEFFFF, // GLASS
        };
        int numColors = sizeof(COLORS) / sizeof(COLORS[0]);
        SDL_LockSurface(surf);
        Uint32* px = (Uint32*)surf->pixels;
        for (int row = 0; row < ATLAS_COLS; row++) {
            for (int col = 0; col < ATLAS_COLS; col++) {
                int idx = row * ATLAS_COLS + col;
                Uint32 color = (idx < numColors) ? COLORS[idx] : 0xFF00FFFF;
                for (int py = 0; py < ATLAS_BLOCK_SIZE; py++) {
                    for (int bx = 0; bx < ATLAS_BLOCK_SIZE; bx++) {
                        px[(row * ATLAS_BLOCK_SIZE + py) * sz + col * ATLAS_BLOCK_SIZE + bx] = color;
                    }
                }
            }
        }
        SDL_UnlockSurface(surf);
        m_atlasW = sz;
        m_atlasH = sz;
    } else {
        m_atlasW = surf->w;
        m_atlasH = surf->h;
    }

    m_tex = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_FreeSurface(surf);
    return m_tex != nullptr;
}

void TextureAtlas::free() {
    if (m_tex) { SDL_DestroyTexture(m_tex); m_tex = nullptr; }
}

SDL_Rect TextureAtlas::getRectAt(int col, int row) const {
    return { col * ATLAS_BLOCK_SIZE, row * ATLAS_BLOCK_SIZE,
             ATLAS_BLOCK_SIZE, ATLAS_BLOCK_SIZE };
}

SDL_Rect TextureAtlas::getRect(uint8_t blockID, bool topFace) const {
    if (blockID == 0 || blockID >= BLOCK_COUNT) return {0, 0, ATLAS_BLOCK_SIZE, ATLAS_BLOCK_SIZE};
    const BlockDef& def = BLOCK_DEFS[blockID];
    if (topFace)
        return getRectAt(def.atlasXTop, def.atlasYTop);
    return getRectAt(def.atlasX, def.atlasY);
}
