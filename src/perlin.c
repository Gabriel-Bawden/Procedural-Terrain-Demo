#include "perlin.h"

#include "math.h"
#include "str.h"

static float perlin_dot(const float *a, const float *b)
{
	return (a[0] * b[0] + a[1] * b[1]);
}

static void perlin_const_vec(size_t a, float *b)
{
	int c = a % 4;

	if (c == 0)
	{
		b[0] = 0.707f;
		b[1] = 0.707f;
	}
	else if (c == 1)
	{
		b[0] = -0.707f;
		b[1] = 0.707f;
	}
	else if (c == 2)
	{
		b[0] = -0.707f;
		b[1] = -0.707f;
	}
	else if (c == 3)
	{
		b[0] = 0.707f;
		b[1] = -0.707f;
	}
}

static float perlin_lerp(float a1, float a2, float t)
{
	return (a1 + (a2 - a1) * t);
}

static float perlin_fade(float t)
{
	return (((6 * t - 15) * t + 10) * t * t * t);
}

static void perlin_shuffle(int seed, size_t *a)
{
	utl_srand(seed);

	for (size_t i = 256 - 1; i > 0; i--)
	{
		size_t j = ((size_t)(utl_rand())) % (i + 1);

		size_t hold = a[j];
		a[j] = a[i];
		a[i] = hold;
	}
}

float perlin_noise(struct perlin *perlin, float x, float y)
{
	x = math_abs_float(x);
	y = math_abs_float(y);

	int x0 = math_floor(x);
	int y0 = math_floor(y);

	float xf = x - x0;
	float yf = y - y0;

	//Vectors from corners to point
	float v_bl[2] = {xf, yf};
	float v_br[2] = {xf - 1, yf};
	float v_tl[2] = {xf, yf - 1};
	float v_tr[2] = {xf - 1, yf - 1};

	//Pseudo-Random Vectors
	int X = math_abs_int(x0 % 256);
	int Y = math_abs_int(y0 % 256);

	float r_bl[2];
	float r_br[2];
	float r_tl[2];
	float r_tr[2];
	perlin_const_vec(perlin->p[perlin->p[X] + Y], r_bl);
	perlin_const_vec(perlin->p[perlin->p[X + 1] + Y], r_br);
	perlin_const_vec(perlin->p[perlin->p[X] + Y + 1], r_tl);
	perlin_const_vec(perlin->p[perlin->p[X + 1] + Y + 1], r_tr);

	//Dot
	float a1 = perlin_dot(v_bl, r_bl);
	float a2 = perlin_dot(v_br, r_br);
	float a3 = perlin_dot(v_tl, r_tl);
	float a4 = perlin_dot(v_tr, r_tr);

	float u1 = perlin_fade(xf);
	float v1 = perlin_fade(yf);

	float ret = perlin_lerp(perlin_lerp(a1, a2, u1), perlin_lerp(a3, a4, u1), v1); // (-(1 / sqrt(2)), (1 / sqrt(2)))

	return (0.5f + (ret * (1 / math_sqrt(2)))); // (0, 1)
}

void perlin_create(struct perlin *perlin, int seed)
{
	perlin->seed = seed;

	size_t p1[256] = {0};

	for (size_t i = 0; i < 256; i++)
	{
		p1[i] = i;
	}

	perlin_shuffle(perlin->seed, p1);

	for (size_t i = 0; i < 512; i++)
	{
		perlin->p[i] = p1[(i + 256) % 256];
	}
}

float perlin_val(struct perlin *perlin, float x, float y, int octaves, float scale, float lacunarity, float persistence)
{
	float t = 0;

	float a = 1;
	float f = scale;

	float a_sum = 0;

	for (int i = 0; i < octaves; i++)
	{
		t += a * perlin_noise(perlin, x * f, y * f);

		a_sum += a;

		a = a * persistence;
		f = f * lacunarity;
	}

	t = t / a_sum;

	return t;
}