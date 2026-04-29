#pragma once
#include "../world/World.h"
#include "../player/Player.h"
#include <string>

// ─── SaveManager ──────────────────────────────────────────────────────────────
// Handles world chunk saves + player position/inventory persistence
class SaveManager {
public:
    explicit SaveManager(const std::string& saveDir = SAVE_DIR);

    bool saveWorld  (World& world);
    bool loadWorld  (World& world);
    bool savePlayer (const Player& p);
    bool loadPlayer (Player& p);

    bool saveExists() const;

private:
    std::string m_dir;
    void ensureDir() const;
};
