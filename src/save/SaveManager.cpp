#include "SaveManager.h"
#include <cstdio>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef _WIN32
  #include <direct.h>
  #define MKDIR(p) _mkdir(p)
#else
  #include <unistd.h>
  #define MKDIR(p) mkdir(p, 0755)
#endif

SaveManager::SaveManager(const std::string& saveDir) : m_dir(saveDir) {}

void SaveManager::ensureDir() const {
    MKDIR(m_dir.c_str());
}

bool SaveManager::saveExists() const {
    std::string path = m_dir + "/player.bin";
    FILE* f = fopen(path.c_str(), "rb");
    if (f) { fclose(f); return true; }
    return false;
}

bool SaveManager::saveWorld(World& world) {
    ensureDir();
    world.saveAll(m_dir);
    return true;
}

bool SaveManager::loadWorld(World& world) {
    world.loadAll(m_dir);
    return true;
}

// ─── Player state: position + hotbar ─────────────────────────────────────────
struct PlayerSave {
    float   x, y;
    uint8_t hotbarID   [HOTBAR_SLOTS];
    uint8_t hotbarCount[HOTBAR_SLOTS];
    int     selected;
};

bool SaveManager::savePlayer(const Player& p) {
    ensureDir();
    std::string path = m_dir + "/player.bin";
    FILE* f = fopen(path.c_str(), "wb");
    if (!f) return false;

    PlayerSave ps;
    ps.x        = p.x();
    ps.y        = p.y();
    ps.selected = const_cast<Player&>(p).inventory().selected();
    for (int i = 0; i < HOTBAR_SLOTS; i++) {
        ItemStack& s    = const_cast<Player&>(p).inventory().hotbarSlot(i);
        ps.hotbarID[i]  = s.id;
        ps.hotbarCount[i] = s.count;
    }
    fwrite(&ps, sizeof(ps), 1, f);
    fclose(f);
    return true;
}

bool SaveManager::loadPlayer(Player& p) {
    std::string path = m_dir + "/player.bin";
    FILE* f = fopen(path.c_str(), "rb");
    if (!f) return false;

    PlayerSave ps;
    fread(&ps, sizeof(ps), 1, f);
    fclose(f);

    // We can't directly set player position through current API,
    // so we expose a setter via friend or direct struct access.
    p.setPosition(ps.x, ps.y);
    p.inventory().setSelected(ps.selected);
    for (int i = 0; i < HOTBAR_SLOTS; i++) {
        p.inventory().hotbarSlot(i) = ItemStack(ps.hotbarID[i], ps.hotbarCount[i]);
    }
    return true;
}
