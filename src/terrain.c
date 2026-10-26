#include "terrain.h"

#include <stddef.h>

#include "math.h"
#include "vec.h"
#include "str.h"
#include "alloc.h"
#include "os.h"
#include "mesh.h"
#include "texture.h"
#include "render.h"
#include "perlin.h"

void terrain_create(struct terrain *terrain, size_t w, size_t h, int seed)
{
	terrain->w = w;
	terrain->h = h;

	terrain->seed = seed;

	terrain->scale = 0.001f;

	terrain->altitude = alloc_calloc(terrain->w * terrain->h, sizeof(*terrain->altitude));
	terrain->water = alloc_calloc(terrain->w * terrain->h, sizeof(*terrain->water));
	terrain->sediment = alloc_calloc(terrain->w * terrain->h, sizeof(*terrain->sediment));
	terrain->water_outflow = alloc_calloc(4 * terrain->w * terrain->h, sizeof(*terrain->water_outflow));
	terrain->sediment_outflow = alloc_calloc(4 * terrain->w * terrain->h, sizeof(*terrain->sediment_outflow));
	terrain->velocity = alloc_calloc(2 * terrain->w * terrain->h, sizeof(*terrain->velocity));

	terrain->change_map = alloc_calloc(terrain->w * terrain->h, sizeof(*terrain->change_map));

	terrain->dt = 0.1f;
	terrain->k_flow = 14.4886f;
	terrain->k_capacity = 1.5f;
	terrain->k_dissolution = 0.001f;
	terrain->k_deposition = 0.001f;
	terrain->k_evap = 0.000568f;
	terrain->rain_drops = 3000;

	terrain->erosion_flag = 0;

	texture_create(&terrain->textures[0], terrain->w, terrain->h, "heightmap_color");
	texture_create(&terrain->textures[1], terrain->w, terrain->h, "heightmap_grayscale");
	texture_create(&terrain->textures[2], terrain->w, terrain->h, "water");
	texture_create(&terrain->textures[3], terrain->w, terrain->h, "velocity");
	texture_create(&terrain->textures[4], terrain->w, terrain->h, "sediment");
	texture_create(&terrain->textures[5], terrain->w, terrain->h, "heightmap_net_change");
	texture_create(&terrain->textures[6], terrain->w, terrain->h, "watershed");

	mesh_create(&terrain->mesh, 2 * (terrain->w - 1) * (terrain->h - 1), terrain->w * terrain->h, 0, 0, "terrain");
}

void terrain_destroy(struct terrain *terrain)
{
	alloc_free(terrain->altitude);
	alloc_free(terrain->water);
	alloc_free(terrain->sediment);
	alloc_free(terrain->water_outflow);
	alloc_free(terrain->sediment_outflow);
	alloc_free(terrain->velocity);

	alloc_free(terrain->change_map);

	for (size_t i = 0; i < sizeof(terrain->textures) / sizeof(*terrain->textures); i++)
	{
		load_texture_destroy(&terrain->textures[i]);
	}

	mesh_destroy(&terrain->mesh);
}

static void terrain_insert_sort(const float *a, int *b, int ind, int set_val_in)
{
	int set_val = set_val_in;

	for (int i = ind; i > 0; i--)
	{
		if (a[set_val] >= a[b[i - 1]])
		{
			for (int j = i; j <= ind; j++)
			{
				int hold = b[j];
				b[j] = set_val;
				set_val = hold;
			}

			break;
		}
	}
}

static void terrain_get_neighbors(size_t *neighbors, unsigned char *neighbors_exist, size_t index, size_t w, size_t h)
{
	neighbors[0] = index + 1 + w; // x + 1, y + 1
	neighbors[1] = index + 1; // x + 1
	neighbors[2] = index + 1 - w; // x + 1, y - 1
	neighbors[3] = index - w; // y - 1
	neighbors[4] = index - 1 - w; // x - 1, y - 1
	neighbors[5] = index - 1; // x - 1
	neighbors[6] = index - 1 + w; // x - 1, y + 1
	neighbors[7] = index + w; // y + 1

	neighbors_exist[0] = 1;
	neighbors_exist[1] = 1;
	neighbors_exist[2] = 1;
	neighbors_exist[3] = 1;
	neighbors_exist[4] = 1;
	neighbors_exist[5] = 1;
	neighbors_exist[6] = 1;
	neighbors_exist[7] = 1;

	size_t x = index % w;
	size_t y = index / w;

	if (x == 0)
	{
		neighbors_exist[4] = 0;
		neighbors_exist[5] = 0;
		neighbors_exist[6] = 0;
	}
	else if (x == w - 1)
	{
		neighbors_exist[0] = 0;
		neighbors_exist[1] = 0;
		neighbors_exist[2] = 0;
	}
	if (y == 0)
	{
		neighbors_exist[2] = 0;
		neighbors_exist[3] = 0;
		neighbors_exist[4] = 0;
	}
	else if (y == h - 1)
	{
		neighbors_exist[0] = 0;
		neighbors_exist[6] = 0;
		neighbors_exist[7] = 0;
	}
}

void terrain_river(const float *alt, int *water, size_t w, size_t h, float sea)
{
	int *idx = alloc_malloc(w * h * sizeof(*idx));

	for (size_t i = 0; i < w * h; i++)
	{
		water[i] = 1;
		idx[i] = (int)i;
	}

	//sort elevation from high to low
	vec_quicksort(idx, alt, 0, (int)(w * h - 1));

	for (size_t i = 0; i < w * h; i++)
	{
		size_t in = idx[i];

		float alt_cur = alt[idx[i]];

		if (alt_cur <= sea)
		{
			break;
		}

		size_t neighbors[8];
		unsigned char neighbors_exist[8];
		terrain_get_neighbors(neighbors, neighbors_exist, in, w, h);

		float low = 2147483647.0f; //Max int
		size_t index = 0;

		unsigned char riv = 1;

		for (size_t j = 0; j < 8; j++)
		{
			if (neighbors_exist[j])
			{
				float alt1 = alt[neighbors[j]];

				if (alt1 <= sea)
				{
					riv = 0;

					break;
				}
				else if (alt1 < low)
				{
					low = alt1;
					index = j;
				}
			}
		}

		if (riv && low < alt_cur)
		{
			water[neighbors[index]] += water[idx[i]];
		}
		else if (riv)
		{
			//water[neighbors[ind]] = -1;
		}
	}

	alloc_free(idx);
}

void terrain_watershed(struct texture *watershed, float *alt, float sea)
{
	size_t w = watershed->w;
	size_t h = watershed->h;

	int *color = alloc_calloc(w * h, sizeof(*color));

	for (size_t i = 0; i < w * h; i++)
	{
		size_t in = i;

		unsigned char flow = alt[i] > sea;

		unsigned char meets_ocean = 1;

		while (flow)
		{
			float alt_cur = alt[in];

			size_t neighbors[8];
			unsigned char neighbors_exist[8];
			terrain_get_neighbors(neighbors, neighbors_exist, in, w, h);

			float low = 2147483647.0f; //Max int
			size_t index = 0;

			unsigned char riv = 1;

			for (size_t j = 0; j < 8; j++)
			{
				if (neighbors_exist[j])
				{
					float alt1 = alt[neighbors[j]];

					if (alt1 <= sea)
					{
						riv = 0;
						flow = 0;

						break;
					}
					else if (alt1 < low)
					{
						low = alt1;
						index = j;
					}
				}
			}

			if (riv && low < alt_cur)
			{
				in = neighbors[index];
			}
			else if (riv)
			{
				in = neighbors[index];

				flow = 0;
				meets_ocean = 0;
			}
		}

		int c = color[in];

		if (c == 0 && meets_ocean)
		{
			color[in] = ((((utl_rand() % 255) + 1) << 0) & (255 << 0)) + ((((utl_rand() % 255) + 1) << 8) & (255 << 8)) + ((((utl_rand() % 255) + 1) << 16) & (255 << 16));
			c = color[in];
		}
		else if (c == 0)
		{
			color[in] = ((255 << 0) & (255 << 0)) + ((0 << 8) & (255 << 8)) + ((0 << 16) & (255 << 16));
			c = color[in];
		}

		if (alt[i] > sea)
		{
			render_DrawPixel(watershed->pixels, w, i % w, i / w, (c >> 0) & 255, (c >> 8) & 255, (c >> 16) & 255);
		}
		else
		{
			render_DrawPixel(watershed->pixels, w, i % w, i / w, 0, 0, 150);
		}
	}

	alloc_free(color);
}

void terrain_alt_from_texture(struct terrain *terrain, struct texture *texture, float sea)
{
	for (size_t i = 0; i < math_min_size_t(terrain->w, texture->w) * math_min_size_t(terrain->h, texture->h); i++)
	{
		const size_t offset = 4 * i;

		int b = texture->pixels[offset + 0];
		int g = texture->pixels[offset + 1];
		int r = texture->pixels[offset + 2];

		if (b == 255 && g == 0 && r == 0)
		{
			terrain->altitude[i] = sea;
		}
		else if (b == 0 && g == 0 && r == 0)
		{
			terrain->altitude[i] = sea;
		}
		else if (b == 0 && g == 255 && r == 0)
		{
			terrain->altitude[i] = sea + 0.25f;
		}
		else if (b == 0 && g == 60 && r == 130)
		{
			terrain->altitude[i] = sea + 0.3f;
		}
	}
}

void terrain_draw_rivers(struct terrain *terrain, float sea)
{
	int *water = alloc_calloc(terrain->w * terrain->h, sizeof(*water));

	terrain_river(terrain->altitude, water, terrain->w, terrain->h, sea);

	for (size_t i = 0; i < terrain->w; i++)
	{
		for (size_t j = 0; j < terrain->h; j++)
		{
			int low = 100; //100
			int high = 1000; //1000

			if (water[i + terrain->w * j] >= low)
			{
				float level = (high - water[i + terrain->w * j]) / (float)(high - low);

				int color = math_max_int(0, (int)(155 + level * 100));

				render_DrawPixel(terrain->textures[0].pixels, terrain->w, i, j, 0, 0, (unsigned char)color);
			}
		}
	}

	alloc_free(water);
}

void terrain_draw(struct terrain *terrain, float sea)
{
	for (size_t i = 0; i < terrain->w * terrain->h; i++)
	{
		size_t x = i % terrain->w;
		size_t y = i / terrain->w;

		//Color heightmap
		float z = terrain->altitude[i];

		float max_height = 3.0f;

		float c1 = 0.01f;
		float c2 = 0.4f;
		float c3 = 0.6f;
		float c4 = 0.8f;
		float c5 = 0.9f;
		float c6 = 1.0f;

		unsigned char r;
		unsigned char g;
		unsigned char b;

		if (z <= sea)
		{
			float m = (sea - z) / (sea - 0);
			m = 1 - m;
			m = (m + 1) / 2;

			r = (unsigned char)(math_min_float((math_max_float(20 * m, 0)), 255));
			g = (unsigned char)(math_min_float((math_max_float(40 * m, 0)), 255));
			b = (unsigned char)(math_min_float((math_max_float(220 * m, 0)), 255));
		}
		else if (z < sea + c1)
		{
			r = 238;
			g = 214;
			b = 175;
		}
		else if (z < sea + c2)
		{
			float m = (sea + c2 - z) / (c2 - c1);
			m = 1 - m;
			m = (m + 1) / 2;

			r = (unsigned char)(math_min_float((math_max_float(34 * m, 0)), 255));
			g = (unsigned char)(math_min_float((math_max_float(139 * m, 0)), 255));
			b = (unsigned char)(math_min_float((math_max_float(34 * m, 0)), 255));
		}
		else if (z < sea + c3)
		{
			float m = (sea + c3 - z) / (c3 - c2);
			m = (m + 1) / 2;

			r = (unsigned char)(math_min_float((math_max_float(60 * m, 0)), 255));
			g = (unsigned char)(math_min_float((math_max_float(139 * m, 0)), 255));
			b = (unsigned char)(math_min_float((math_max_float(34 * m, 0)), 255));
		}
		else if (z < sea + c4)
		{
			float m = (sea + c4 - z) / (c4 - c3);
			m = (m + 1) / 2;

			r = (unsigned char)(math_min_float((math_max_float(35 * m, 0)), 255));
			g = (unsigned char)(math_min_float((math_max_float(65 * m, 0)), 255));
			b = (unsigned char)(math_min_float((math_max_float(20 * m, 0)), 255));
		}
		else if (z < sea + c5)
		{
			float m = (sea + c5 - z) / (c5 - c4);
			m = (m + 1) / 2;

			r = (unsigned char)(math_min_float((math_max_float(25 * m, 0)), 255));
			g = (unsigned char)(math_min_float((math_max_float(30 * m, 0)), 255));
			b = (unsigned char)(math_min_float((math_max_float(10 * m, 0)), 255));
		}
		else if (z < sea + c6)
		{
			float m = (sea + c6 - z) / (c6 - c5);
			m = (m + 1) / 2;

			r = (unsigned char)(math_min_float((math_max_float(20 * m, 0)), 255));
			g = (unsigned char)(math_min_float((math_max_float(15 * m, 0)), 255));
			b = (unsigned char)(math_min_float((math_max_float(10 * m, 0)), 255));
		}
		else if (z < sea + max_height)
		{
			float m = (sea + max_height - z) / (max_height - c6);
			m = 1 - m;
			m = (m + 0.5f) / 1.5f;

			r = (unsigned char)(math_min_float((math_max_float(255 * m, 0)), 255));
			g = (unsigned char)(math_min_float((math_max_float(255 * m, 0)), 255));
			b = (unsigned char)(math_min_float((math_max_float(255 * m, 0)), 255));
		}
		else
		{
			r = 255;
			g = 255;
			b = 255;
		}

		render_DrawPixel(terrain->textures[0].pixels, terrain->w, x, y, r, g, b);

		//Grayscale heightmap
		r = (unsigned char)(math_min_float((math_max_float(255 * z, 0)), 255));
		g = (unsigned char)(math_min_float((math_max_float(255 * z, 0)), 255));
		b = (unsigned char)(math_min_float((math_max_float(255 * z, 0)), 255));

		render_DrawPixel(terrain->textures[1].pixels, terrain->w, x, y, r, g, b);

		//Water
		float water_amount = 3 * terrain->water[i];

		r = 0;
		g = (unsigned char)(math_min_float((math_max_float(water_amount * 30, 0)), 255));
		b = (unsigned char)(math_min_float((math_max_float(water_amount * 120, 0)), 255));

		render_DrawPixel(terrain->textures[2].pixels, terrain->w, x, y, r, g, b);

		//Velocity
		float velocity_magnitude = 5 * math_sqrt(terrain->velocity[2 * i + 0] * terrain->velocity[2 * i + 0] + terrain->velocity[2 * i + 1] * terrain->velocity[2 * i + 1]);

		r = (unsigned char)(math_min_float((math_max_float(velocity_magnitude * 120, 0)), 255));
		g = (unsigned char)(math_min_float((math_max_float(velocity_magnitude * 30, 0)), 255));
		b = 0;

		render_DrawPixel(terrain->textures[3].pixels, terrain->w, x, y, r, g, b);

		//Sediment
		float sediment_amount = 10 * terrain->sediment[i];

		r = (unsigned char)(math_min_float((math_max_float(sediment_amount * 120, 0)), 255));
		g = (unsigned char)(math_min_float((math_max_float(sediment_amount * 120, 0)), 255));
		b = 0;

		render_DrawPixel(terrain->textures[4].pixels, terrain->w, x, y, r, g, b);

		//Net change
		float change = terrain->change_map[i];

		if (change >= 0)
		{
			r = 0;
			g = (unsigned char)(math_min_float((math_max_float(255 * change * 10, 0)), 255));
			b = 0;
		}
		else
		{
			r = (unsigned char)(math_min_float((math_max_float(255 * -change * 10, 0)), 255));
			g = 0;
			b = 0;
		}

		render_DrawPixel(terrain->textures[5].pixels, terrain->w, x, y, r, g, b);
	}
}

void terrain_generate(struct terrain *terrain)
{
	perlin_create(&terrain->perlin, terrain->seed);

	for (size_t i = 0; i < sizeof(terrain->textures) / sizeof(*terrain->textures); i++)
	{
		memset(terrain->textures[i].pixels, 255, 4 * terrain->textures[i].w * terrain->textures[i].h * sizeof(*terrain->textures[i].pixels));
	}

	memset(terrain->altitude, 0, terrain->w * terrain->h * sizeof(*terrain->altitude));
	memset(terrain->water, 0, terrain->w * terrain->h * sizeof(*terrain->water));
	memset(terrain->sediment, 0, terrain->w * terrain->h * sizeof(*terrain->sediment));
	memset(terrain->water_outflow, 0, 4 * terrain->w * terrain->h * sizeof(*terrain->water_outflow));
	memset(terrain->sediment_outflow, 0, 4 * terrain->w * terrain->h * sizeof(*terrain->sediment_outflow));
	memset(terrain->velocity, 0, 2 * terrain->w * terrain->h * sizeof(*terrain->velocity));
	memset(terrain->change_map, 0, terrain->w * terrain->h * sizeof(*terrain->change_map));

	//float min = 0.0f;
	//float max = 0.0f;

	for (size_t i = 0; i < terrain->w; i++)
	{
		for (size_t j = 0; j < terrain->h; j++)
		{
			float p1 = perlin_val(&terrain->perlin, (float)i, (float)j, 8, terrain->scale, 2.0f, 0.5f);

			terrain->altitude[i + terrain->w * j] = p1;

			//if (alt[i + w * j] > max) max = alt[i + w * j];
			//if (alt[i + w * j] < min) min = alt[i + w * j];

			//
			//float p2 = perlin_val(&perlin, (float)(i + terrain->w), (float)(j + terrain->h), 8, terrain->scale * 2.5f, 2.0f, 0.5f);

			//if (p2 > 0.6)
			//{
			//	terrain->altitude[i + terrain->w * j] += 3 * p2;
			//}
			//
		}
	}

	//for (int i = 0; i < w * h; i++)
	//{
	//	alt[i] = (alt[i] - min) / (max - min);
	//}
}

static void terrain_get_nearest(size_t *neighbors, unsigned char *neighbors_exist, size_t index, size_t w, size_t h)
{
	neighbors[0] = index - 1; // x - 1
	neighbors[1] = index + 1; // x + 1
	neighbors[2] = index - w; // y - 1
	neighbors[3] = index + w; // y + 1

	neighbors_exist[0] = 1;
	neighbors_exist[1] = 1;
	neighbors_exist[2] = 1;
	neighbors_exist[3] = 1;

	size_t x = index % w;
	size_t y = index / w;

	if (x == 0)
	{
		neighbors_exist[0] = 0;
	}
	else if (x == w - 1)
	{
		neighbors_exist[1] = 0;
	}
	if (y == 0)
	{
		neighbors_exist[2] = 0;
	}
	else if (y == h - 1)
	{
		neighbors_exist[3] = 0;
	}
}

static float terrain_calc_flux(float *alt, float *water, size_t current, size_t neighbor, unsigned char neighbor_exists)
{
	if (neighbor_exists)
	{
		return ((alt[current] + water[current]) - (alt[neighbor] + water[neighbor]));
	}
	else
	{
		return 0;
	}
}

void terrain_water_flow2(struct terrain *terrain)
{
	for (unsigned int i = 0; i < terrain->rain_drops; i++)
	{
		int rad = 10;
		int rad_2 = rad * rad;

		int x = utl_rand() % terrain->w;
		int y = utl_rand() % terrain->h;

		for (int j = math_max_int(0, x - rad); j < math_min_int((int)(terrain->w - 1), x + rad); j++)
		{
			for (int k = math_max_int(0, y - rad); k < math_min_int((int)(terrain->h - 1), y + rad); k++)
			{
				float d = (float)((j - x) * (j - x) + (k - y) * (k - y));

				if (d <= rad_2)
				{
					float a = (0.00005f * terrain->dt) * (rad_2 - d);
					terrain->water[j + terrain->w * k] += a;
				}
			}
		}
	}

	//for (size_t i = 0; i < terrain->w * terrain->h; i++)
	//{
	//	if (utl_rand() % 10)
	//	{
	//		terrain->water[i] += terrain->dt * (terrain->rain_drops / 300000.0f);
	//	}
	//}

	for (size_t i = 0; i < terrain->w * terrain->h; i++)
	{
		size_t neighbors[4];
		unsigned char neighbors_exist[4];
		terrain_get_nearest(neighbors, neighbors_exist, i, terrain->w, terrain->h);

		float fl = math_max_float(0, terrain->water_outflow[4 * i + 0] + terrain->dt * terrain->k_flow * terrain_calc_flux(terrain->altitude, terrain->water, i, neighbors[0], neighbors_exist[0]));
		float fr = math_max_float(0, terrain->water_outflow[4 * i + 1] + terrain->dt * terrain->k_flow * terrain_calc_flux(terrain->altitude, terrain->water, i, neighbors[1], neighbors_exist[1]));
		float fb = math_max_float(0, terrain->water_outflow[4 * i + 2] + terrain->dt * terrain->k_flow * terrain_calc_flux(terrain->altitude, terrain->water, i, neighbors[2], neighbors_exist[2]));
		float ft = math_max_float(0, terrain->water_outflow[4 * i + 3] + terrain->dt * terrain->k_flow * terrain_calc_flux(terrain->altitude, terrain->water, i, neighbors[3], neighbors_exist[3]));

		float total_flow = fl + fr + fb + ft;
		float scale = 0;

		if (total_flow)
		{
			scale = math_min_float(1, terrain->water[i] / (terrain->dt * (fl + fr + fb + ft)));
		}

		terrain->water_outflow[4 * i + 0] = scale * fl;
		terrain->water_outflow[4 * i + 1] = scale * fr;
		terrain->water_outflow[4 * i + 2] = scale * fb;
		terrain->water_outflow[4 * i + 3] = scale * ft;
	}

	for (size_t i = 0; i < terrain->w * terrain->h; i++)
	{
		size_t neighbors[4];
		unsigned char neighbors_exist[4];
		terrain_get_nearest(neighbors, neighbors_exist, i, terrain->w, terrain->h);

		float fl_out = terrain->water_outflow[4 * i + 0];
		float fr_out = terrain->water_outflow[4 * i + 1];
		float fb_out = terrain->water_outflow[4 * i + 2];
		float ft_out = terrain->water_outflow[4 * i + 3];

		float fl_in = 0;
		float fr_in = 0;
		float fb_in = 0;
		float ft_in = 0;

		if (neighbors_exist[0])
		{
			fl_in = terrain->water_outflow[4 * neighbors[0] + 1];
		}
		if (neighbors_exist[1])
		{
			fr_in = terrain->water_outflow[4 * neighbors[1] + 0];
		}
		if (neighbors_exist[2])
		{
			fb_in = terrain->water_outflow[4 * neighbors[2] + 3];
		}
		if (neighbors_exist[3])
		{
			ft_in = terrain->water_outflow[4 * neighbors[3] + 2];
		}

		terrain->water[i] += terrain->dt * ((fl_in + fr_in + fb_in + ft_in) - (fl_out + fr_out + fb_out + ft_out));

		//Update Velocity
		terrain->velocity[2 * i + 0] = (fl_in - fl_out + fr_out - fr_in) / 2;
		terrain->velocity[2 * i + 1] = (fb_in - fb_out + ft_out - ft_in) / 2;
	}

	for (size_t i = 0; i < terrain->w * terrain->h; i++)
	{
		size_t neighbors[4];
		unsigned char neighbors_exist[4];
		terrain_get_nearest(neighbors, neighbors_exist, i, terrain->w, terrain->h);

		float max_neighbor = -1000;
		float min_neighbor = 1000;

		for (size_t j = 0; j < 4; j++)
		{
			if (neighbors_exist[j])
			{
				float alt_cur = terrain->altitude[neighbors[j]];

				if (alt_cur > max_neighbor)
				{
					max_neighbor = alt_cur;
				}
				if (alt_cur < min_neighbor)
				{
					min_neighbor = alt_cur;
				}
			}
		}

		float max_diff = terrain->altitude[i] - max_neighbor;
		float min_diff = terrain->altitude[i] - min_neighbor;

		float hxl = 0;
		float hxr = 0;
		float hyb = 0;
		float hyt = 0;

		if (!neighbors_exist[0])
		{
			hxr = terrain->altitude[neighbors[1]];
			hxl = -hxr;
		}
		else if (!neighbors_exist[1])
		{
			hxl = terrain->altitude[neighbors[0]];
			hxr = -hxl;
		}
		else
		{
			hxl = terrain->altitude[neighbors[0]];
			hxr = terrain->altitude[neighbors[1]];
		}
		if (!neighbors_exist[2])
		{
			hyt = terrain->altitude[neighbors[3]];
			hyb = -hyt;
		}
		else if (!neighbors_exist[3])
		{
			hyb = terrain->altitude[neighbors[2]];
			hyt = -hyb;
		}
		else
		{
			hyb = terrain->altitude[neighbors[2]];
			hyt = terrain->altitude[neighbors[3]];
		}

		float dx = (hxr - hxl) / 2;
		float dy = (hyt - hyb) / 2;
		float squared = dx * dx + dy * dy;
		float sin_angle = math_sqrt(squared / (1 + squared));

		float velocity_magnitude = math_sqrt(terrain->velocity[2 * i + 0] * terrain->velocity[2 * i + 0] + terrain->velocity[2 * i + 1] * terrain->velocity[2 * i + 1]);

		sin_angle = math_min_float(sin_angle, 0.7f);
		sin_angle = math_max_float(sin_angle, 0.1f);

		float capacity = terrain->k_capacity * sin_angle * velocity_magnitude; //sin_angle / 0.0008f | 0.0008f is roughly the median sin_angle

		float sed = terrain->sediment[i];

		//if (capacity > sed)
		//{
		//	float dissolve = terrain->k_dissolving * (capacity - sed);

		//	terrain->altitude[i] -= dissolve;
		//	terrain->sediment[i] += dissolve;

		//	//Net Change Map
		//	terrain->change_map[i] -= dissolve;
		//}
		//else
		//{
		//	float deposit = terrain->k_deposition * (sed - capacity);

		//	terrain->altitude[i] += deposit;
		//	terrain->sediment[i] -= deposit;

		//	//Net Change Map
		//	terrain->change_map[i] += deposit;
		//}

		//if (min_diff < 0)
		//{
		//	//float deposit = math_min_float(100 * terrain->k_deposition * (sed), -max_diff + 0.01f);
		//	float deposit = math_min_float(sed, -max_diff + 0.01f);

		//	terrain->altitude[i] += deposit;
		//	terrain->sediment[i] -= deposit;

		//	//Net Change Map
		//	terrain->change_map[i] += deposit;
		//}
		//else 
		if (capacity > sed)
		{
			if (min_diff > 0)
			{
				float dissolve = math_min_float(terrain->k_dissolution * (capacity - sed), min_diff);
				//float dissolve = k_dissolving * (capacity - sed);

				terrain->altitude[i] -= dissolve;
				terrain->sediment[i] += dissolve;

				//Net Change Map
				terrain->change_map[i] -= dissolve;
			}
		}
		else
		{
			if (max_diff < 0)
			{
				float deposit = math_min_float(terrain->k_deposition * (sed - capacity), -max_diff + 0.01f);
				//float deposit = k_deposition * (sed - capacity);

				terrain->altitude[i] += deposit;
				terrain->sediment[i] -= deposit;

				//Net Change Map
				terrain->change_map[i] += deposit;



				//
				//float deposit = math_min_float(0.5f * sed, -max_diff + 0.01f);
				////float deposit = k_deposition * (sed - capacity);

				//terrain->altitude[i] += deposit;
				//terrain->sediment[i] -= deposit;

				////Net Change Map
				//terrain->change_map[i] += deposit;
			}
		}
	}

	for (size_t i = 0; i < terrain->w * terrain->h; i++)
	{
		size_t x = i % terrain->w;
		size_t y = i / terrain->w;

		float xp = (float)x - terrain->velocity[2 * i + 0] * terrain->dt;
		float yp = (float)y - terrain->velocity[2 * i + 1] * terrain->dt;
		int x1 = (int)(xp);
		int y1 = (int)(yp);
		int x2 = x1 + 1;
		int y2 = y1 + 1;

		float sediment_amount = 0;

		if ((x1 >= 0 && x1 < (int)terrain->w && y1 >= 0 && y1 < (int)terrain->h) && (x2 >= 0 && x2 < (int)terrain->w && y2 >= 0 && y2 < (int)terrain->h))
		{
			float s11 = terrain->sediment[x1 + terrain->w * y1];
			float s21 = terrain->sediment[x2 + terrain->w * y1];
			float s12 = terrain->sediment[x1 + terrain->w * y2];
			float s22 = terrain->sediment[x2 + terrain->w * y2];

			sediment_amount = s11 * (x2 - xp) * (y2 - yp) / ((x2 - x1) * (y2 - y1))
				+ s21 * (xp - x1) * (y2 - yp) / ((x2 - x1) * (y2 - y1))
				+ s12 * (x2 - xp) * (yp - y1) / ((x2 - x1) * (y2 - y1))
				+ s22 * (xp - x1) * (yp - y1) / ((x2 - x1) * (y2 - y1));
		}
		else
		{
			sediment_amount = terrain->sediment[i];
		}

		terrain->sediment[i] = sediment_amount;

		//terrain->water[i] *= (1 - terrain->k_evap * terrain->dt);
		terrain->water[i] = math_max_float(0, terrain->water[i] - terrain->k_evap * terrain->dt);
	}
}

void terrain_water_flow(struct terrain *terrain)
{
	for (unsigned int i = 0; i < terrain->rain_drops; i++)
	{
		int rad = 10;
		int rad_2 = rad * rad;

		int x = utl_rand() % terrain->w;
		int y = utl_rand() % terrain->h;

		for (int j = math_max_int(0, x - rad); j < math_min_int((int)(terrain->w - 1), x + rad); j++)
		{
			for (int k = math_max_int(0, y - rad); k < math_min_int((int)(terrain->h - 1), y + rad); k++)
			{
				float d = (float)((j - x) * (j - x) + (k - y) * (k - y));

				if (d <= rad_2)
				{
					float a = (0.00005f * terrain->dt) * (rad_2 - d);
					terrain->water[j + terrain->w * k] += a;
				}
			}
		}
	}

	//Must be greater than 0 due to float rounding issues. The issue arises when k_evap is 0 and the build is in release
	float zero = 0.000001f;

	for (size_t i = 0; i < terrain->w * terrain->h; i++)
	{
		size_t neighbors[4];
		unsigned char neighbors_exist[4];
		terrain_get_nearest(neighbors, neighbors_exist, i, terrain->w, terrain->h);

		float water_outflow_left = math_max_float(0, terrain->water_outflow[4 * i + 0] + terrain->dt * terrain->k_flow * terrain_calc_flux(terrain->altitude, terrain->water, i, neighbors[0], neighbors_exist[0]));
		float water_outflow_right = math_max_float(0, terrain->water_outflow[4 * i + 1] + terrain->dt * terrain->k_flow * terrain_calc_flux(terrain->altitude, terrain->water, i, neighbors[1], neighbors_exist[1]));
		float water_outflow_bottom = math_max_float(0, terrain->water_outflow[4 * i + 2] + terrain->dt * terrain->k_flow * terrain_calc_flux(terrain->altitude, terrain->water, i, neighbors[2], neighbors_exist[2]));
		float water_outflow_top = math_max_float(0, terrain->water_outflow[4 * i + 3] + terrain->dt * terrain->k_flow * terrain_calc_flux(terrain->altitude, terrain->water, i, neighbors[3], neighbors_exist[3]));

		float flux_total_unscaled = water_outflow_left + water_outflow_right + water_outflow_bottom + water_outflow_top;
		float scale = 0;

		if (flux_total_unscaled > zero)
		{
			scale = math_min_float(1.0f, terrain->water[i] / (terrain->dt * flux_total_unscaled));
		}

		terrain->water_outflow[4 * i + 0] = scale * water_outflow_left;
		terrain->water_outflow[4 * i + 1] = scale * water_outflow_right;
		terrain->water_outflow[4 * i + 2] = scale * water_outflow_bottom;
		terrain->water_outflow[4 * i + 3] = scale * water_outflow_top;

		float flux_total_scaled = terrain->water_outflow[4 * i + 0] + terrain->water_outflow[4 * i + 1] + terrain->water_outflow[4 * i + 2] + terrain->water_outflow[4 * i + 3];

		if (terrain->water[i] > zero && flux_total_scaled > zero)
		{
			float perc_water_leaving = math_min_float(1.0f, (terrain->dt * flux_total_scaled) / terrain->water[i]);
			float sediment_leaving = perc_water_leaving * terrain->sediment[i];

			terrain->sediment_outflow[4 * i + 0] = (terrain->water_outflow[4 * i + 0] / flux_total_scaled) * sediment_leaving;
			terrain->sediment_outflow[4 * i + 1] = (terrain->water_outflow[4 * i + 1] / flux_total_scaled) * sediment_leaving;
			terrain->sediment_outflow[4 * i + 2] = (terrain->water_outflow[4 * i + 2] / flux_total_scaled) * sediment_leaving;
			terrain->sediment_outflow[4 * i + 3] = (terrain->water_outflow[4 * i + 3] / flux_total_scaled) * sediment_leaving;
		}
		else
		{
			terrain->sediment_outflow[4 * i + 0] = 0;
			terrain->sediment_outflow[4 * i + 1] = 0;
			terrain->sediment_outflow[4 * i + 2] = 0;
			terrain->sediment_outflow[4 * i + 3] = 0;
		}
	}

	for (size_t i = 0; i < terrain->w * terrain->h; i++)
	{
		size_t neighbors[4];
		unsigned char neighbors_exist[4];
		terrain_get_nearest(neighbors, neighbors_exist, i, terrain->w, terrain->h);

		float water_outflow_left = terrain->water_outflow[4 * i + 0];
		float water_outflow_right = terrain->water_outflow[4 * i + 1];
		float water_outflow_bottom = terrain->water_outflow[4 * i + 2];
		float water_outflow_top = terrain->water_outflow[4 * i + 3];

		float water_inflow_left = 0;
		float water_inflow_right = 0;
		float water_inflow_bottom = 0;
		float water_inflow_top = 0;

		float sediment_outflow_left = terrain->sediment_outflow[4 * i + 0];
		float sediment_outflow_right = terrain->sediment_outflow[4 * i + 1];
		float sediment_outflow_bottom = terrain->sediment_outflow[4 * i + 2];
		float sediment_outflow_top = terrain->sediment_outflow[4 * i + 3];

		float sediment_inflow_left = 0;
		float sediment_inflow_right = 0;
		float sediment_inflow_bottom = 0;
		float sediment_inflow_top = 0;

		if (neighbors_exist[0])
		{
			water_inflow_left = terrain->water_outflow[4 * neighbors[0] + 1];
			sediment_inflow_left = terrain->sediment_outflow[4 * neighbors[0] + 1];
		}
		if (neighbors_exist[1])
		{
			water_inflow_right = terrain->water_outflow[4 * neighbors[1] + 0];
			sediment_inflow_right = terrain->sediment_outflow[4 * neighbors[1] + 0];
		}
		if (neighbors_exist[2])
		{
			water_inflow_bottom = terrain->water_outflow[4 * neighbors[2] + 3];
			sediment_inflow_bottom = terrain->sediment_outflow[4 * neighbors[2] + 3];
		}
		if (neighbors_exist[3])
		{
			water_inflow_top = terrain->water_outflow[4 * neighbors[3] + 2];
			sediment_inflow_top = terrain->sediment_outflow[4 * neighbors[3] + 2];
		}

		//Updating water and sediment
		terrain->water[i] += terrain->dt * ((water_inflow_left + water_inflow_right + water_inflow_bottom + water_inflow_top) - (water_outflow_left + water_outflow_right + water_outflow_bottom + water_outflow_top));
		terrain->sediment[i] += ((sediment_inflow_left + sediment_inflow_right + sediment_inflow_bottom + sediment_inflow_top) - (sediment_outflow_left + sediment_outflow_right + sediment_outflow_bottom + sediment_outflow_top));

		//Updating velocity
		terrain->velocity[2 * i + 0] = (water_inflow_left - water_outflow_left + water_outflow_right - water_inflow_right) / 2;
		terrain->velocity[2 * i + 1] = (water_inflow_bottom - water_outflow_bottom + water_outflow_top - water_inflow_top) / 2;
	}

	for (size_t i = 0; i < terrain->w * terrain->h; i++)
	{
		size_t neighbors[4];
		unsigned char neighbors_exist[4];
		terrain_get_nearest(neighbors, neighbors_exist, i, terrain->w, terrain->h);

		float altitude_neighbor_max = -1000;
		float altitude_neighbor_min = 1000;

		for (size_t j = 0; j < 4; j++)
		{
			if (neighbors_exist[j])
			{
				float altitude_neighbor = terrain->altitude[neighbors[j]];

				if (altitude_neighbor > altitude_neighbor_max)
				{
					altitude_neighbor_max = altitude_neighbor;
				}
				if (altitude_neighbor < altitude_neighbor_min)
				{
					altitude_neighbor_min = altitude_neighbor;
				}
			}
		}

		float max_diff = terrain->altitude[i] - altitude_neighbor_max;
		float min_diff = terrain->altitude[i] - altitude_neighbor_min;

		float velocity_magnitude = math_sqrt(terrain->velocity[2 * i + 0] * terrain->velocity[2 * i + 0] + terrain->velocity[2 * i + 1] * terrain->velocity[2 * i + 1]);

		//float depth_scale = 1;
		//float depth_max = 0.1f;

		//if (terrain->water[i] > depth_max)
		//{
		//	depth_scale = 1 / (50 * (terrain->water[i] - depth_max) + 1);
		//}

		//float capacity = terrain->k_capacity * velocity_magnitude * depth_scale;

		float capacity = terrain->k_capacity * velocity_magnitude;

		float sed = terrain->sediment[i];

		if (capacity > sed)
		{
			if (min_diff > 0)
			{
				float dissolve = math_min_float(terrain->k_dissolution * (capacity - sed), min_diff);
				//float dissolve = k_dissolving * (capacity - sed);

				terrain->altitude[i] -= dissolve;
				terrain->sediment[i] += dissolve;

				//Net Change Map
				terrain->change_map[i] -= dissolve;
			}
		}
		else
		{
			if (max_diff < 0)
			{
				float deposit = math_min_float(terrain->k_deposition * (sed - capacity), -max_diff + 0.01f);
				//float deposit = k_deposition * (sed - capacity);

				terrain->altitude[i] += deposit;
				terrain->sediment[i] -= deposit;

				//Removing extra water to avoid causing ripples
				terrain->water[i] = math_max_float(0, terrain->water[i] - deposit);

				//Net Change Map
				terrain->change_map[i] += deposit;
			}
		}
	}

	for (size_t i = 0; i < terrain->w * terrain->h; i++)
	{
		terrain->water[i] = math_max_float(0, terrain->water[i] - terrain->k_evap * terrain->dt);

		//Doesn't work well at lower rain amounts compared to drop method
		//terrain->water[i] += terrain->dt * 0.0012f; //0.0007f, 0.01f, 0.0012f




		//size_t x = i % terrain->w;
		//size_t y = i / terrain->w;

		//float ptest = perlin_val(&terrain->perlin, (float)(x + terrain->w), (float)(y + terrain->h), 8, terrain->scale * 2.5f, 2.0f, 0.5f);

		//if (ptest > 0.6)
		//{
		//	terrain->altitude[i] += ptest / 100000;
		//}

		//float ptest = perlin_val(&terrain->perlin, (float)(x), (float)(y), 8, terrain->scale, 2.0f, 0.5f);

		//if (ptest > 0.55)
		//{
		//	terrain->altitude[i] += ptest / 10000;
		//}
	}
}