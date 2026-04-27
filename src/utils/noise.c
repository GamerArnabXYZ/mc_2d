#include "noise.h"
#include <math.h>
#include <string.h>

/* ─── Permutation table ──────────────────────────────────────── */
static uint8_t perm[512];
static float   grad1[16] = {1,-1, 1,-1, 1,-1, 1,-1, 1,-1, 1,-1, 1,-1, 1,-1};
static float   grad2x[8] = {1,-1, 1,-1, 0, 0, 0, 0};
static float   grad2y[8] = {0, 0, 0, 0, 1,-1, 1,-1};

static float fade(float t) {
    return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
}
static float lerp_f(float a, float b, float t) {
    return a + t * (b - a);
}

void noise_init(uint32_t seed) {
    /* Seeded Fisher-Yates shuffle on 0..255 */
    uint8_t base[256];
    for (int i = 0; i < 256; i++) base[i] = (uint8_t)i;
    uint32_t s = seed;
    for (int i = 255; i > 0; i--) {
        s = s * 1664525u + 1013904223u; /* LCG */
        int j = (int)(s >> 24) % (i + 1);
        uint8_t tmp = base[i]; base[i] = base[j]; base[j] = tmp;
    }
    for (int i = 0; i < 256; i++) perm[i] = perm[i + 256] = base[i];
}

/* ─── 1D Perlin ──────────────────────────────────────────────── */
float noise1d(float x) {
    int   ix = (int)floorf(x) & 255;
    float fx = x - floorf(x);
    float u  = fade(fx);
    float g0 = grad1[perm[ix    ] & 15] * fx;
    float g1 = grad1[perm[ix + 1] & 15] * (fx - 1.0f);
    return lerp_f(g0, g1, u);
}

/* ─── 2D Perlin ──────────────────────────────────────────────── */
float noise2d(float x, float y) {
    int   ix = (int)floorf(x) & 255;
    int   iy = (int)floorf(y) & 255;
    float fx = x - floorf(x);
    float fy = y - floorf(y);
    float u  = fade(fx);
    float v  = fade(fy);

    int aa = perm[perm[ix  ] + iy  ] & 7;
    int ba = perm[perm[ix+1] + iy  ] & 7;
    int ab = perm[perm[ix  ] + iy+1] & 7;
    int bb = perm[perm[ix+1] + iy+1] & 7;

    float v00 = grad2x[aa]*fx         + grad2y[aa]*fy;
    float v10 = grad2x[ba]*(fx-1.0f) + grad2y[ba]*fy;
    float v01 = grad2x[ab]*fx         + grad2y[ab]*(fy-1.0f);
    float v11 = grad2x[bb]*(fx-1.0f) + grad2y[bb]*(fy-1.0f);

    float x1 = lerp_f(v00, v10, u);
    float x2 = lerp_f(v01, v11, u);
    return lerp_f(x1, x2, v);
}

/* ─── Octave wrappers ────────────────────────────────────────── */
float octave_noise1d(float x, int octaves, float persistence, float lacunarity) {
    float val = 0.0f, amp = 1.0f, freq = 1.0f, max = 0.0f;
    for (int i = 0; i < octaves; i++) {
        val  += noise1d(x * freq) * amp;
        max  += amp;
        amp  *= persistence;
        freq *= lacunarity;
    }
    return val / max;
}

float octave_noise2d(float x, float y, int octaves, float persistence, float lacunarity) {
    float val = 0.0f, amp = 1.0f, freq = 1.0f, max = 0.0f;
    for (int i = 0; i < octaves; i++) {
        val  += noise2d(x * freq, y * freq) * amp;
        max  += amp;
        amp  *= persistence;
        freq *= lacunarity;
    }
    return val / max;
}
