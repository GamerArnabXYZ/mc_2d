#pragma once
#include "Inventory.h"
#include "../world/World.h"
#include "../core/Config.h"
#include <SDL2/SDL.h>

class Player {
public:
    Player();

    // Call once after first world chunk generated
    void spawnOnSurface(const World& world);

    void update(float dt, World& world);

    // Input
    void setMoveX(float vx)    { m_velX = vx; }
    void jump();
    void startBreak(int bx, int by);
    void stopBreak();
    void placeBlock(int bx, int by, World& world);
    void selectSlot(int i)     { m_inv.setSelected(i); }
    void scrollSlot(int d)     { m_inv.scrollSelected(d); }
    void toggleInventory()     { m_inv.setOpen(!m_inv.isOpen()); }
    void setPosition(float x, float y) { m_x = x; m_y = y; }

    // Getters
    float x()          const { return m_x; }
    float y()          const { return m_y; }
    bool  onGround()   const { return m_onGround; }
    bool  inWater()    const { return m_inWater; }
    bool  isSpawned()  const { return m_spawned; }

    Inventory&       inventory()       { return m_inv; }
    const Inventory& inventory() const { return m_inv; }

    int   chunkX() const {
        return World::blockToChunk((int)floorf(m_x / BLOCK_SIZE));
    }

    // Break state
    float breakProgress() const { return m_breakTimer; }
    int   breakTargetX()  const { return m_breakBX; }
    int   breakTargetY()  const { return m_breakBY; }
    bool  isBreaking()    const { return m_breaking; }

private:
    float     m_x, m_y, m_velX, m_velY;
    bool      m_onGround, m_inWater, m_spawned;
    bool      m_breaking;
    int       m_breakBX, m_breakBY;
    float     m_breakTimer;
    Inventory m_inv;

    bool solidAt(float px, float py, const World& w) const;
    void resolveX(float dx, const World& w);
    void resolveY(float dy, const World& w);
};
