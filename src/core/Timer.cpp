#include "Timer.h"
#include "../core/Config.h"

Timer::Timer()
    : m_prev(SDL_GetPerformanceCounter())
    , m_freq(SDL_GetPerformanceFrequency())
    , m_delta(0.0f)
    , m_fps(0.0f)
    , m_fpsAccum(0.0f)
    , m_fpsFrames(0)
{}

void Timer::tick() {
    Uint64 now   = SDL_GetPerformanceCounter();
    float  raw   = (float)(now - m_prev) / (float)m_freq;
    m_prev       = now;

    // Cap delta to avoid physics explosion after lag spike
    m_delta = (raw > 0.05f) ? 0.05f : raw;

    // FPS counter (update every second)
    m_fpsAccum += m_delta;
    m_fpsFrames++;
    if (m_fpsAccum >= 1.0f) {
        m_fps       = (float)m_fpsFrames / m_fpsAccum;
        m_fpsAccum  = 0.0f;
        m_fpsFrames = 0;
    }
}

float   Timer::getDelta()   const { return m_delta; }
float   Timer::getFPS()     const { return m_fps; }
Uint64  Timer::getTotalMs() const {
    return (SDL_GetPerformanceCounter() * 1000ULL) / m_freq;
}
