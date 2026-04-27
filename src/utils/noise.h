#ifndef MC2D_NOISE_H
#define MC2D_NOISE_H

#include <stdint.h>

/* Pure-C Perlin noise — no external deps */
void  noise_init(uint32_t seed);
float noise1d(float x);
float noise2d(float x, float y);
float octave_noise1d(float x, int octaves, float persistence, float lacunarity);
float octave_noise2d(float x, float y, int octaves, float persistence, float lacunarity);

#endif /* MC2D_NOISE_H */
