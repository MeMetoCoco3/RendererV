#include "perlin.h"
#include <sstream>
#include <fstream>

static auto A = 3284557443;

void SetNewSeed(i64 seed)
{
	A = seed;
}

void GeneratePerlinNoise(void* buff, int w, int h, f32 freq, f32 amp, f32 contrast)
{
	const int GRID_SIZE = 200;
	
	
	u8* Channel = (u8*)buff;

	for (int y = 0; y < h; y++)
	{
		for (int x = 0; x < w; x++)
		{
			f32 Val = 0;

			f32 Freq = freq;
			f32 Amp = amp;

			for (int i = 0; i < 12; i++)
			{
				Val += Perlin(x * Freq / GRID_SIZE, y * Freq / GRID_SIZE) * Amp;

				Freq *= 2;
				Amp /= 2;
			}

			// Contrast
			Val *= contrast;

			// Clipping
			if (Val > 1.0f)
				Val = 1.0f;
			else if (Val < -1.0f)
				Val = -1.0f;

			// Convert 1 to -1 into 255 to 0
			int Color = (int)(((Val + 1.0f) * 0.5f) * 255);

			// Set pixel color
			*Channel++ = (u8)Color;
			*Channel++ = (u8)Color;
			*Channel++ = (u8)Color;
			*Channel++ = 255;
		}
	}
}



static f32 Perlin(f32 x, f32 y)
{
	// Get Grid cell corners
	int x0 = (int)x;
	int y0 = (int)y;
	int x1 = x0 + 1;
	int y1 = y0 + 1;

	f32 sx = x - (f32)x0;
	f32 sy = y - (f32)y0;

	f32 n0 = DotGridGradient(x0, y0, x, y);
	f32 n1 = DotGridGradient(x1, y0, x, y);
	f32 ix0 = Interpolate(n0, n1, sx);


	n0 = DotGridGradient(x0, y1, x, y);
	n1 = DotGridGradient(x1, y1, x, y);
	f32 ix1 = Interpolate(n0, n1, sx);

	f32 value = Interpolate(ix0, ix1, sy);

	return value;
}

static f32 Interpolate(f32 a0, f32 a1, f32 w)
{
	return (a1 - a0) * (3.0f - w * 2.0f) * w * w + a0;
}


static f32 DotGridGradient(int ix, int iy, f32 x, f32 y)
{
	// Get Random Angle for intersection
	vec2 Gradient = RandomGradient(ix, iy);

	// We get distance from point to corner
	f32 dx = x - (float)ix;
	f32 dy = y - (float)iy;

	// Do dot product between delta and angle.
	return (dx * Gradient.x + dy * Gradient.y);

}


static vec2 RandomGradient(int ix, int iy)
{
	// No precomputed gradients mean this works for any number of grid coordinates
	const unsigned w = 8 * sizeof(unsigned);
	const unsigned s = w / 2;
	unsigned a = ix, b = iy;
	a *= A;

	b ^= a << s | a >> w - s;
	b *= 1999520717;

	a ^= b << s | b >> w - s;
	a *= 2048419325;
	float random = a * (3.14159265 / ~(~0u >> 1)); // in [0, 2*Pi]

	// Create the vector from the angle
	vec2 v;
	v.x = sin(random);
	v.y = cos(random);

	return v;
}