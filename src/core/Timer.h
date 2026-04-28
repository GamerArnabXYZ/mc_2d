#pragma once
#include <SDL2/SDL.h>

// ─── Timer ────────────────────────────────────────────────────────────────────
// Fixed-step accumulator pattern → stable 60fps physics on all devices
class Timer {
public:
    Timer();

    void     tick();           // call once per frame
    float    getDelta() const; // seconds since last frame (capped at 0.05s)
    float    getFPS()   const;
    Uint64   getTotalMs() const;

private:
    Uint64  m_prev;
    Uint64  m_freq;
    float   m_delta;
    float   m_fps;
    float   m_fpsAccum;
    int     m_fpsFrames;
};
