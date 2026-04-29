#include "Player.h"
#include <cmath>
#include <cstdio>

Player::Player()
    : m_x(0), m_y(0)  // Set properly in init() after world generates
    , m_velX(0), m_velY(0)
    , m_onGround(false), m_inWater(false)
    , m_breaking(false), m_breakBX(0), m_breakBY(0)
    , m_breakTimer(0.0f)
    , m_spawned(false)
{}

// ─── Find surface and spawn player ────────────────────────────────────────────
void Player::spawnOnSurface(const World& world) {
    // Start at center of loaded world, scan down from top to find surface
    int spawnChunk = WORLD_CHUNKS / 2;
    int spawnBX    = spawnChunk * CHUNK_W + CHUNK_W / 2;

    // Scan from top downward to find first solid block
    int surfY = SURFACE_AVG; // fallback
    for (int by = 5; by < CHUNK_H - 2; by++) {
        uint8_t blk = world.getBlock(spawnBX, by);
        if (blockIsSolid(blk) && !BLOCK_DEFS[blk].liquid) {
            surfY = by - 1; // spawn just above solid
            break;
        }
    }

    m_x = (float)(spawnBX * BLOCK_SIZE) - PLAYER_W * 0.5f;
    m_y = (float)(surfY * BLOCK_SIZE) - PLAYER_H - 2.0f;
    m_velX = 0; m_velY = 0;
    m_spawned = true;
    SDL_Log("Player spawned at block (%d, %d), px (%.0f, %.0f)", spawnBX, surfY, m_x, m_y);
}

// ─── Solid check ──────────────────────────────────────────────────────────────
bool Player::solidAt(float px, float py, const World& w) const {
    int bx = (int)floorf(px / BLOCK_SIZE);
    int by = (int)floorf(py / BLOCK_SIZE);
    uint8_t id = w.getBlock(bx, by);
    return blockIsSolid(id) && !BLOCK_DEFS[id].liquid;
}

// ─── X-axis collision ─────────────────────────────────────────────────────────
void Player::resolveX(float dx, const World& w) {
    float nx   = m_x + dx;
    float testX = (dx > 0) ? (nx + PLAYER_W - 0.5f) : (nx + 0.5f);
    float ys[3] = { m_y + 2, m_y + PLAYER_H * 0.5f, m_y + PLAYER_H - 2 };

    bool blocked = false;
    for (int i = 0; i < 3; i++)
        if (solidAt(testX, ys[i], w)) { blocked = true; break; }

    if (!blocked) {
        m_x = nx;
    } else {
        m_velX = 0;
        if (dx > 0) {
            m_x = floorf((m_x + PLAYER_W) / BLOCK_SIZE) * BLOCK_SIZE - PLAYER_W - 0.5f;
        } else {
            m_x = floorf(m_x / BLOCK_SIZE) * BLOCK_SIZE + BLOCK_SIZE + 0.5f;
        }
    }
}

// ─── Y-axis collision ─────────────────────────────────────────────────────────
void Player::resolveY(float dy, const World& w) {
    float ny   = m_y + dy;
    bool  down = (dy > 0);
    float testY = down ? (ny + PLAYER_H - 0.5f) : (ny + 0.5f);
    float xs[3] = { m_x + 2, m_x + PLAYER_W * 0.5f, m_x + PLAYER_W - 2 };

    bool blocked = false;
    for (int i = 0; i < 3; i++)
        if (solidAt(xs[i], testY, w)) { blocked = true; break; }

    if (!blocked) {
        m_y = ny;
        if (down) m_onGround = false;
    } else {
        m_velY = 0;
        if (down) {
            m_y = floorf((m_y + PLAYER_H) / BLOCK_SIZE) * BLOCK_SIZE - PLAYER_H - 0.5f;
            m_onGround = true;
        } else {
            m_y = floorf(m_y / BLOCK_SIZE) * BLOCK_SIZE + BLOCK_SIZE + 0.5f;
        }
    }
}

// ─── Update ───────────────────────────────────────────────────────────────────
void Player::update(float dt, World& world) {
    if (!m_spawned) {
        spawnOnSurface(world);
        return;
    }

    // Water check (center of player)
    int cx = (int)floorf((m_x + PLAYER_W*0.5f) / BLOCK_SIZE);
    int cy = (int)floorf((m_y + PLAYER_H*0.6f) / BLOCK_SIZE);
    m_inWater = (world.getBlock(cx, cy) == BLOCK_WATER);

    m_onGround = false;

    // Gravity
    float gravScale = m_inWater ? 0.2f : 1.0f;
    m_velY += GRAVITY * gravScale * dt;
    if (m_velY > MAX_FALL_SPEED) m_velY = MAX_FALL_SPEED;

    // Water drag
    if (m_inWater) { m_velX *= 0.82f; m_velY *= 0.82f; }

    // Move
    resolveX(m_velX * dt, world);
    resolveY(m_velY * dt, world);

    // Block breaking
    if (m_breaking) {
        uint8_t id = world.getBlock(m_breakBX, m_breakBY);
        if (id == BLOCK_AIR || id == BLOCK_BEDROCK) {
            m_breaking = false; m_breakTimer = 0;
        } else {
            float hard = BLOCK_DEFS[id].hardness;
            if (hard <= 0.0f) hard = 0.3f;
            m_breakTimer += dt;
            if (m_breakTimer >= hard) {
                uint8_t drop = BLOCK_DEFS[id].dropID;
                if (drop == 255) drop = id;
                if (drop != BLOCK_AIR) m_inv.addItem(drop, 1);
                world.setBlock(m_breakBX, m_breakBY, BLOCK_AIR);
                m_breaking = false; m_breakTimer = 0;
            }
        }
    }
}

void Player::jump() {
    if (m_onGround) {
        m_velY = JUMP_FORCE;
    } else if (m_inWater) {
        m_velY = JUMP_FORCE * 0.55f;
    }
}

void Player::startBreak(int bx, int by) {
    if (m_breakBX != bx || m_breakBY != by) m_breakTimer = 0;
    m_breaking = true; m_breakBX = bx; m_breakBY = by;
}

void Player::stopBreak() {
    m_breaking = false; m_breakTimer = 0;
}

void Player::placeBlock(int bx, int by, World& world) {
    ItemStack& held = m_inv.heldItem();
    if (held.empty()) return;
    if (world.getBlock(bx, by) != BLOCK_AIR) return;
    // Don't place inside player
    float bpx = (float)(bx * BLOCK_SIZE), bpy = (float)(by * BLOCK_SIZE);
    if (bpx + BLOCK_SIZE > m_x+2 && bpx < m_x+PLAYER_W-2 &&
        bpy + BLOCK_SIZE > m_y+2 && bpy < m_y+PLAYER_H-2) return;
    world.setBlock(bx, by, held.id);
    m_inv.removeHeld(1);
}
