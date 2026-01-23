#ifndef PERLIN_H
#define PERLIN_H
#include "vstd/vtypes.h"


void SetNewSeed(i64 seed);
void GeneratePerlinNoise(void* buff, int w, int h, f32 freq, f32 amp, f32 contrast);
static f32 Perlin(f32 x, f32 y);
static f32 Interpolate(f32 a0, f32 a1, f32 w);
static f32 DotGridGradient(int ix, int iy, f32 x, f32 y);
static vec2 RandomGradient(int ix, int iy);
#endif