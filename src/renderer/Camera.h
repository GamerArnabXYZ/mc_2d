#pragma once
#include "../core/Config.h"
#include <SDL2/SDL.h>
#include <cmath>

// ─── Camera ───────────────────────────────────────────────────────────────────
// Follows player with smooth lerp, converts world→screen coords
class Camera {
public:
    Camera();

    void update(float targetX, float targetY, float dt);

    // World pixel → screen pixel
    inline int worldToScreenX(float wx) const { return (int)(wx - m_x) + WINDOW_W / 2; }
    inline int worldToScreenY(float wy) const { return (int)(wy - m_y) + WINDOW_H / 2; }

    // Screen pixel → world pixel
    inline float screenToWorldX(int sx) const { return (float)(sx - WINDOW_W / 2) + m_x; }
    inline float screenToWorldY(int sy) const { return (float)(sy - WINDOW_H / 2) + m_y; }

    // Screen pixel → block coord
    inline int screenToBlockX(int sx) const {
        return (int)floorf(screenToWorldX(sx) / BLOCK_SIZE);
    }
    inline int screenToBlockY(int sy) const {
        return (int)floorf(screenToWorldY(sy) / BLOCK_SIZE);
    }

    float x() const { return m_x; }
    float y() const { return m_y; }

private:
    float m_x, m_y;          // camera center in world pixels
    static constexpr float LERP = 8.0f; // smoothness
};
