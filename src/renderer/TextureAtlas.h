#pragma once
#include "../core/SDL_incl.h"
#include "../world/Block.h"
#include "../core/Config.h"
#include <string>

// ─── TextureAtlas ─────────────────────────────────────────────────────────────
// Loads atlas.png, provides SDL_Rect for each block face
class TextureAtlas {
public:
    TextureAtlas();
    ~TextureAtlas();

    bool load(SDL_Renderer* renderer, const std::string& path);
    void free();

    // Get SDL_Rect for a block's side face (uses atlasX/Y from BlockDef)
    SDL_Rect getRect(uint8_t blockID, bool topFace = false) const;

    // Get rect directly by atlas grid position
    SDL_Rect getRectAt(int col, int row) const;

    SDL_Texture* texture() const { return m_tex; }
    bool isLoaded()        const { return m_tex != nullptr; }

private:
    SDL_Texture* m_tex;
    int          m_atlasW, m_atlasH; // actual texture dimensions
};
