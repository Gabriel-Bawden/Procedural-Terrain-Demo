#ifndef MATH_H
#define MATH_H

#include <stddef.h>

//Mathematical Constants
#define PI 3.1415926f
#define PI_2 (PI / 2.0f)
#define E 2.71828182846f

int math_abs_int(int a); //Should handle min int

float math_abs_float(float a);

int math_min_int(int a, int b);

size_t math_min_size_t(size_t a, size_t b);

float math_min_float(float a, float b);

int math_max_int(int a, int b);

size_t math_max_size_t(size_t a, size_t b);

float math_max_float(float a, float b);

int math_mod(int a, int b);

int math_pow_int(int a, unsigned int b);

int math_floor(float a);

int math_divide_floor(int a, int b);

int math_modulo_euclidean(int a, int b);

float math_round(float a);

float math_cos(float a);

float math_sin(float x);

float math_tan(float x);

float math_sqrt(float number);

float math_atan(float x);

float math_atan2(float y, float x);

float math_pow(double a, double b);

#endif