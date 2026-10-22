#include "vec.h"

#include <stddef.h>

#include "math.h"

//Vector Operations
float vec_dot(const float *a, const float *b)
{
	return (a[0] * b[0] + a[1] * b[1] + a[2] * b[2]);
}

float vec_length(const float *a)
{
	return math_sqrt(a[0] * a[0] + a[1] * a[1] + a[2] * a[2]);
}

void vec_add(const float *a, const float *b, float *c)
{
	c[0] = a[0] + b[0];
	c[1] = a[1] + b[1];
	c[2] = a[2] + b[2];
}

void vec_subtract(const float *a, const float *b, float *c)
{
	c[0] = a[0] - b[0];
	c[1] = a[1] - b[1];
	c[2] = a[2] - b[2];
}

void vec_multiply(const float *a, const float *b, float *c)
{
	c[0] = a[0] * b[0];
	c[1] = a[1] * b[1];
	c[2] = a[2] * b[2];
}

void vec_scale(float c, const float *a, float *b)
{
	b[0] = c * a[0];
	b[1] = c * a[1];
	b[2] = c * a[2];
}

void vec_equate(const float *a, float *b)
{
	b[0] = a[0];
	b[1] = a[1];
	b[2] = a[2];
}

void vec_cross(const float *a, const float *b, float *c)
{
	c[0] = a[1] * b[2] - a[2] * b[1];
	c[1] = -a[0] * b[2] + a[2] * b[0];
	c[2] = a[0] * b[1] - a[1] * b[0];
}

float vec_normalize(const float *a, float *b)
{
	float sum1 = math_sqrt(a[0] * a[0] + a[1] * a[1] + a[2] * a[2]);
	float sum = 1 / sum1;

	b[0] = sum * a[0];
	b[1] = sum * a[1];
	b[2] = sum * a[2];

	return sum1;
}

void vec_normal(const float *a, const float *b, const float *c, float *d, int norm)
{
	float s1[3] = {0};
	float s2[3] = {0};
	float s3[3] = {0};

	vec_subtract(c, a, s1);
	vec_subtract(b, a, s2);

	vec_cross(s1, s2, s3);

	if (norm) 
	{
		vec_normalize(s3, s3);
	}

	d[0] = s3[0];
	d[1] = s3[1];
	d[2] = s3[2];
}

//Matrix Operations
void vec_mult_4x4_to_vec(const float *a, const float *b, float *c)
{
	c[0] = a[0] * b[0] + a[1] * b[1] + a[2] * b[2] + a[3];
	c[1] = a[4] * b[0] + a[5] * b[1] + a[6] * b[2] + a[7];
	c[2] = a[8] * b[0] + a[9] * b[1] + a[10] * b[2] + a[11];
}

void vec_mult_4x4(const float *a, const float *b, float *c)
{
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			c[i * 4 + j] = (a[4 * i + 0] * b[4 * 0 + j]) + (a[4 * i + 1] * b[4 * 1 + j]) + (a[4 * i + 2] * b[4 * 2 + j]) + (a[4 * i + 3] * b[4 * 3 + j]);
		}
	}
}

void vec_mult(const float* a, size_t a_m, size_t a_n, const float *b, size_t b_m, size_t b_n, float *c)
{
	if (a_n == b_m)
	{
		for (size_t i = 0; i < a_m; i++)
		{
			for (size_t j = 0; j < b_n; j++)
			{
				float sum = 0;

				for (size_t k = 0; k < a_n; k++)
				{
					sum += a[i * a_n + k] * b[k * b_n + j];
				}

				c[i * b_n + j] = sum;
			}
		}
	}
}

void vec_plane_create(struct vec_plane *v, float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3, float x4, float y4, float z4)
{
	v->p1[0] = x1;
	v->p1[1] = y1;
	v->p1[2] = z1;

	v->p2[0] = x2;
	v->p2[1] = y2;
	v->p2[2] = z2;

	v->p3[0] = x3;
	v->p3[1] = y3;
	v->p3[2] = z3;

	v->p4[0] = x4;
	v->p4[1] = y4;
	v->p4[2] = z4;

	float a1[3];
	float a2[3];
	float a3[3];

	vec_subtract(v->p2, v->p1, a1);
	vec_subtract(v->p3, v->p1, a2);
	vec_cross(a1, a2, a3);

	vec_normalize(a3, v->n);

	v->d = -vec_dot(v->n, v->p1);
}

//Algorithms
void vec_quicksort(int *a, const float *b, int left, int right) //sort array from greatest to least
{
	if (left < right)
	{
		int i = left;
		int j = right;
		float pivot = b[a[(i + j) / 2]];

		while (i <= j)
		{
			while (b[a[i]] > pivot)
			{
				i++;
			}
			while (b[a[j]] < pivot)
			{
				j--;
			}

			if (i <= j)
			{
				int temp = a[i];
				a[i] = a[j];
				a[j] = temp;
				i++;
				j--;
			}
		}

		vec_quicksort(a, b, left, j);
		vec_quicksort(a, b, i, right);
	}
}