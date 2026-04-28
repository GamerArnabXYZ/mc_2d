#include "Player.h"
#include <cmath>

Player::Player()
    : m_x(CHUNK_W * BLOCK_SIZE * (WORLD_CHUNKS / 2) + BLOCK_SIZE * 4.0f)
    , m_y((CHUNK_H - SURFACE_BASE - 4) * (float)BLOCK_SIZE)
    , m_velX(0), m_velY(0)
    , m_onGround(false), m_inWater(false)
    , m_breaking(false)
    , m_breakBX(0), m_breakBY(0)
    , m_breakTimer(0.0f)
{}

// ─── Solid check: test a point against world blocks ────────────────────────────
bool Player::solidAt(float px, float py, const World& w) const {
    int bx = (int)floorf(px / BLOCK_SIZE);
    int by = (int)floorf(py / BLOCK_SIZE);
    uint8_t id = w.getBlock(bx, by);
    return blockIsSolid(id);
}

// ─── X-axis collision ──────────────────────────────────────────────────────────
void Player::resolveX(float dx, const World& w) {
    float nx = m_x + dx;
    // Test 4 points on player's leading edge
    float testY[4] = { m_y + 1, m_y + PLAYER_H * 0.33f, m_y + PLAYER_H * 0.66f, m_y + PLAYER_H - 1 };
    float testX     = (dx > 0) ? (nx + PLAYER_W) : nx;

    bool blocked = false;
    for (int i = 0; i < 4; i++) {
        if (solidAt(testX, testY[i], w)) { blocked = true; break; }
    }

    if (!blocked) {
        m_x = nx;
    } else {
        m_velX = 0;
        if (dx > 0) {
            int bx = (int)floorf((m_x + PLAYER_W) / BLOCK_SIZE);
            m_x = (float)(bx * BLOCK_SIZE) - PLAYER_W - 0.1f;
        } else {
            int bx = (int)floorf(m_x / BLOCK_SIZE);
            m_x = (float)((bx + 1) * BLOCK_SIZE) + 0.1f;
        }
    }
}

// ─── Y-axis collision ──────────────────────────────────────────────────────────
void Player::resolveY(float dy, const World& w) {
    float ny   = m_y + dy;
    bool  down = (dy > 0);
    float testX[3] = { m_x + 1, m_x + PLAYER_W * 0.5f, m_x + PLAYER_W - 1 };
    float testY    = down ? (ny + PLAYER_H) : ny;

    bool blocked = false;
    for (int i = 0; i < 3; i++) {
        if (solidAt(testX[i], testY, w)) { blocked = true; break; }
    }

    if (!blocked) {
        m_y = ny;
        if (down) m_onGround = false;
    } else {
        m_velY = 0;
        if (down) {
            int by = (int)floorf((m_y + PLAYER_H) / BLOCK_SIZE);
            m_y = (float)(by * BLOCK_SIZE) - PLAYER_H - 0.1f;
            m_onGround = true;
        } else {
            int by = (int)floorf(m_y / BLOCK_SIZE);
            m_y = (float)((by + 1) * BLOCK_SIZE) + 0.1f;
        }
    }
}

// ─── Main update ──────────────────────────────────────────────────────────────
void Player::update(float dt, World& world) {
    // Check water
    int cx  = (int)floorf((m_x + PLAYER_W * 0.5f) / BLOCK_SIZE);
    int cy  = (int)floorf((m_y + PLAYER_H * 0.5f) / BLOCK_SIZE);
    m_inWater = (world.getBlock(cx, cy) == BLOCK_WATER);

    // ── Gravity ───────────────────────────────────────────────────────────────
    float gravMult = m_inWater ? 0.25f : 1.0f;
    m_velY += GRAVITY * gravMult * dt;
    if (m_velY > MAX_FALL_SPEED) m_velY = MAX_FALL_SPEED;

    m_onGround = false;

    // ── Move X ────────────────────────────────────────────────────────────────
    resolveX(m_velX * dt, world);

    // ── Move Y ────────────────────────────────────────────────────────────────
    resolveY(m_velY * dt, world);

    // Friction in water
    if (m_inWater) {
        m_velX *= 0.85f;
        m_velY *= 0.85f;
    }

    // ── Block breaking ────────────────────────────────────────────────────────
    if (m_breaking) {
        uint8_t id = world.getBlock(m_breakBX, m_breakBY);
        if (id == BLOCK_AIR) {
            m_breaking = false; // block already gone (e.g. water)
        } else {
            float hardness = BLOCK_DEFS[id].hardness;
            m_breakTimer += dt;
            if (hardness <= 0.0f || m_breakTimer >= hardness) {
                // Break the block, give drop
                uint8_t drop = BLOCK_DEFS[id].dropID;
                if (drop == 255) drop = id; // self-drop
                if (drop != 0)
                    m_inv.addItem(drop, 1);
                world.setBlock(m_breakBX, m_breakBY, BLOCK_AIR);
                m_breaking   = false;
                m_breakTimer = 0.0f;
            }
        }
    }
}

// ─── Input handlers ───────────────────────────────────────────────────────────
void Player::jump() {
    if (m_onGround) {
        m_velY = JUMP_FORCE;
        m_onGround = false;
    } else if (m_inWater) {
        m_velY = JUMP_FORCE * 0.6f; // swim up
    }
}

void Player::startBreak(int bx, int by) {
    if (m_breakBX != bx || m_breakBY != by) {
        m_breakTimer = 0.0f; // reset on new target
    }
    m_breaking = true;
    m_breakBX  = bx;
    m_breakBY  = by;
}

void Player::stopBreak() {
    m_breaking   = false;
    m_breakTimer = 0.0f;
}

void Player::placeBlock(int bx, int by, World& world) {
    ItemStack& held = m_inv.heldItem();
    if (held.empty()) return;
    if (world.getBlock(bx, by) != BLOCK_AIR) return;

    // Don't place inside player
    float blkPx = (float)(bx * BLOCK_SIZE);
    float blkPy = (float)(by * BLOCK_SIZE);
    if (blkPx < m_x + PLAYER_W && blkPx + BLOCK_SIZE > m_x &&
        blkPy < m_y + PLAYER_H && blkPy + BLOCK_SIZE > m_y)
        return;

    world.setBlock(bx, by, held.id);
    m_inv.removeHeld(1);
}
