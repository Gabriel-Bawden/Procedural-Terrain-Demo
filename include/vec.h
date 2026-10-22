#ifndef VEC_H
#define VEC_H

#include <stddef.h>

//Vector Operations
float vec_dot(const float *a, const float *b);

float vec_length(const float *a);

void vec_add(const float *a, const float *b, float *c);

void vec_subtract(const float *a, const float *b, float *c);

void vec_multiply(const float *a, const float *b, float *c);

void vec_scale(float c, const float *a, float *b);

void vec_equate(const float *a, float *b);

void vec_cross(const float *a, const float *b, float *c);

float vec_normalize(const float *a, float *b);

void vec_normal(const float *a, const float *b, const float *c, float *d, int norm);

//Matrix Operations
void vec_mult_4x4_to_vec(const float *a, const float *b, float *c);

void vec_mult_4x4(const float *a, const float *b, float *c);

void vec_mult(const float *a, size_t a_m, size_t a_n, const float *b, size_t b_m, size_t b_n, float *c);

struct vec_plane
{
	float n[3];
	float d;

	float p1[3];
	float p2[3];
	float p3[3];
	float p4[3];
};

void vec_plane_create(struct vec_plane *v, float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3, float x4, float y4, float z4);

//Algorithms
void vec_quicksort(int *a, const float *b, int left, int right); //sort array from greatest to least

#endif