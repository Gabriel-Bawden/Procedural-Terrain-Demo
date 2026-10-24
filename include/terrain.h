#ifndef TERRAIN_H
#define TERRAIN_H

#include <stddef.h>

#include "mesh.h"
#include "texture.h"
#include "perlin.h"

struct terrain
{
	float *altitude;
	float *water;
	float *sediment;
	float *water_outflow;
	float *sediment_outflow;
	float *velocity;

	float *change_map;

	size_t w;
	size_t h;

	int seed;

	float scale;

	float dt;
	float k_flow;
	float k_capacity;
	float k_dissolution;
	float k_deposition;
	float k_evap;

	unsigned int rain_drops;

	unsigned char erosion_flag;

	struct texture textures[7];
	struct mesh mesh;

	struct perlin perlin;
};

void terrain_create(struct terrain *terrain, size_t w, size_t h, int seed);

void terrain_destroy(struct terrain *terrain);

void terrain_river(const float *alt, int *water, size_t w, size_t h, float sea);

void terrain_watershed(struct texture *watershed, float *alt, float sea);

void terrain_alt_from_texture(struct terrain *terrain, struct texture *texture, float sea);

void terrain_draw_rivers(struct terrain *terrain, float sea);

void terrain_draw(struct terrain *terrain, float sea);

void terrain_generate(struct terrain *terrain);

void terrain_water_flow2(struct terrain *terrain);

void terrain_water_flow(struct terrain *terrain);

#endif