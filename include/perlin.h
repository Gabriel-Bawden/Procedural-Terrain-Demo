#ifndef PERLIN_H
#define PERLIN_H

struct perlin
{
	int seed;
	size_t p[512];
};

float perlin_noise(struct perlin *perlin, float x, float y);

void perlin_create(struct perlin *perlin, int seed);

float perlin_val(struct perlin *perlin, float x, float y, int octaves, float scale, float lacunarity, float persistence);

#endif