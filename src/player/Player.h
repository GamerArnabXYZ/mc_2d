#pragma once
#include "Inventory.h"
#include "../world/World.h"
#include "../core/Config.h"

// ─── Player ───────────────────────────────────────────────────────────────────
class Player {
public:
    Player();

    // ── Update ────────────────────────────────────────────────────────────────
    void update(float dt, World& world);

    // ── Input setters (called by InputManager) ────────────────────────────────
    void setMoveX(float vx)    { m_velX = vx; }   // -MOVE_SPEED..MOVE_SPEED
    void jump();
    void startBreak(int bx, int by);               // start breaking a block
    void stopBreak();
    void placeBlock(int bx, int by, World& world); // place held block
    void selectSlot(int i)     { m_inv.setSelected(i); }
    void scrollSlot(int d)     { m_inv.scrollSelected(d); }
    void toggleInventory()     { m_inv.setOpen(!m_inv.isOpen()); }

    // ── Getters ───────────────────────────────────────────────────────────────
    float      x()         const { return m_x; }
    void       setPosition(float x, float y) { m_x = x; m_y = y; }
    float      y()         const { return m_y; }
    bool       onGround()  const { return m_onGround; }
    bool       inWater()   const { return m_inWater; }
    Inventory&       inventory()       { return m_inv; }
    const Inventory& inventory() const { return m_inv; }
    int        chunkX()    const { return World::blockToChunk((int)(m_x / BLOCK_SIZE)); }

    // Break progress 0..1 for current target block
    float      breakProgress() const { return m_breakTimer; }
    int        breakTargetX()  const { return m_breakBX; }
    int        breakTargetY()  const { return m_breakBY; }
    bool       isBreaking()    const { return m_breaking; }

private:
    float     m_x, m_y;       // top-left pixel position
    float     m_velX, m_velY;
    bool      m_onGround;
    bool      m_inWater;

    // Break state
    bool      m_breaking;
    int       m_breakBX, m_breakBY;
    float     m_breakTimer;   // 0..hardness seconds

    Inventory m_inv;

    // ── Collision helpers ─────────────────────────────────────────────────────
    bool solidAt(float px, float py, const World& w) const;
    void resolveX(float dx, const World& w);
    void resolveY(float dy, const World& w);
};
