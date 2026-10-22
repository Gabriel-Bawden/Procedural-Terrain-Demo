#include "math.h"

#include <stddef.h>

int math_abs_int(int a) //Should handle min int
{
	if (a < 0)
	{
		a *= -1;
	}

	return a;
}

float math_abs_float(float a)
{
	if (a < 0)
	{
		a *= -1;
	}

	return a;
}

int math_min_int(int a, int b)
{
	if (a < b)
	{
		return a;
	}
	else
	{
		return b;
	}
}

size_t math_min_size_t(size_t a, size_t b)
{
	if (a < b)
	{
		return a;
	}
	else
	{
		return b;
	}
}

float math_min_float(float a, float b)
{
	if (a < b)
	{
		return a;
	}
	else
	{
		return b;
	}
}

int math_max_int(int a, int b)
{
	if (b < a)
	{
		return a;
	}
	else
	{
		return b;
	}
}

size_t math_max_size_t(size_t a, size_t b)
{
	if (b < a)
	{
		return a;
	}
	else
	{
		return b;
	}
}

float math_max_float(float a, float b)
{
	if (b < a)
	{
		return a;
	}
	else
	{
		return b;
	}
}

int math_mod(int a, int b)
{
	int r = a % b;

	if (r < 0)
	{
		r += b;
	}

	return r;
}

int math_pow_int(int a, unsigned int b)
{
	int c = 1;

	for (unsigned int i = 0; i < b; i++)
	{
		c *= a;
	}

	return c;
}

int math_floor(float a)
{
	int b = (int)a;

	if (a >= 0 || a == b)
	{
		return b;
	}
	else
	{
		return b - 1;
	}
}

int math_divide_floor(int a, int b)
{
	int q = a / b;
	int r = a % b;

	//-1 / 2 = 0, -2 / 2 = -1; these must match
	if (r != 0 && (a < 0) != (b < 0))
	{
		q--;
	}

	return q;
}

int math_modulo_euclidean(int a, int b)
{
	return (((a % b) + math_abs_int(b)) % math_abs_int(b));
}

float math_round(float a)
{
	int a_int = (int)a;
	float dec = a - a_int;

	if (a >= 0)
	{
		if (dec >= 0.5f)
		{
			return (float)(a_int + 1);
		}
		else
		{
			return (float)(a_int);
		}
	}
	else
	{
		if (dec <= -0.5f)
		{
			return (float)(a_int - 1);
		}
		else
		{
			return (float)(a_int);
		}
	}
}

float math_cos(float a)
{
	int mult = 1;

	if (a < 0)
	{
		a *= -1;
	}

	if (a > PI)
	{
		int div = (int)(a / PI);

		a = (a - (div * PI));

		if (div % 2)
		{
			mult = -1;
		}
	}

	float a1 = (a * a) / 2;
	float a2 = (a * a) / 12;
	float a3 = (a * a) / 30;
	float a4 = (a * a) / 56;
	float a5 = (a * a) / 90;
	float a6 = (a * a) / 132;
	float a7 = (a * a) / 182;

	return mult * (1 - a1 * (1 - (a2 * (1 - (a3 * (1 - (a4 * (1 - (a5 * (1 - (a6 * (1 - a7))))))))))));
}

float math_sin(float x)
{
	return math_cos(x - PI_2);
}

float math_tan(float x)
{
	return math_sin(x) / math_cos(x);
}

float math_sqrt(float number)
{
	//Fast inverse squareroot algorithm: https://en.wikipedia.org/wiki/Fast_inverse_square_root#Overview_of_the_code
	long i;
	float x2, y;
	const float threehalfs = 1.5f;

	x2 = number * 0.5f;
	y = number;
	i = *(long *)&y;
	i = 0x5f3759df - (i >> 1);
	y = *(float *)&i;
	y = y * (threehalfs - (x2 * y * y));   //Newton 1st iteration
	//y  = y * ( threehalfs - ( x2 * y * y ) );   //2nd iteration

	//Returning squareroot
	return 1 / y;
}

static float lookup_table_atan[102] =
{
	0.0f,		 0.0099996664f, 0.019997334f, 0.029991005f, 0.039978687f,
	0.049958397f, 0.059928156f,  0.069885999f, 0.079829983f, 0.089758173f,
	0.099668652f, 0.10955953f,   0.11942893f,  0.12927501f,  0.13909595f,
	0.14888994f,  0.15865526f,   0.16839015f,  0.17809294f,  0.18776195f,
	0.19739556f,  0.20699219f,   0.21655031f,  0.22606839f,  0.23554498f,
	0.24497867f,  0.25436807f,   0.26371184f,  0.27300870f,  0.28225741f,
	0.29145679f,  0.30060568f,   0.30970293f,  0.31874755f,  0.32773849f,
	0.33667481f,  0.34555557f,   0.35437992f,  0.36314702f,  0.37185606f,
	0.38050637f,  0.38909724f,   0.39762798f,  0.40609807f,  0.41450688f,
	0.42285392f,  0.43113875f,   0.43936089f,  0.44751999f,  0.45561564f,
	0.46364760f,  0.47161558f,   0.47951928f,  0.48735857f,  0.49513325f,
	0.50284320f,  0.51048833f,   0.51806855f,  0.52558380f,  0.53303409f,
	0.54041952f,  0.54774004f,   0.55499572f,  0.56218672f,  0.56931317f,
	0.57637525f,  0.58337301f,   0.59030676f,  0.59717667f,  0.60398299f,
	0.61072594f,  0.61740589f,   0.62402308f,  0.63057774f,  0.63707036f,
	0.64350110f,  0.64987046f,   0.65617871f,  0.66242629f,  0.66861355f,
	0.67474097f,  0.68080884f,   0.68681765f,  0.69276786f,  0.69865984f,
	0.70449406f,  0.71027100f,   0.71599114f,  0.72165483f,  0.72726268f,
	0.73281509f,  0.73831260f,   0.74375558f,  0.74914461f,  0.75448018f,
	0.75976276f,  0.76499283f,   0.77017093f,  0.77529752f,  0.78037310f,
	0.78539819f,  0.79037325f
};

float math_atan(float x)
{
	if (x > 0)
	{
		if (x <= 1)
		{
			int index = (int)math_round(x * 100);

			return lookup_table_atan[index];
		}
		else
		{
			//Using identity: arctan(x) = (pi / 2) - arctan(1/x)
			int index = (int)math_round((1 / x) * 100);

			return (PI_2 - lookup_table_atan[index]);
		}
	}
	else
	{
		if (x >= -1)
		{
			int index = (int)math_round(-x * 100);

			return -(lookup_table_atan[index]);
		}
		else
		{
			//Using identity: arctan(x) = (pi / 2) - arctan(1/x)
			int index = (int)math_round((1 / -x) * 100);

			return (lookup_table_atan[index] - PI_2);
		}
	}
}

float math_atan2(float y, float x)
{
	if (x > 0.0f)
	{
		return math_atan(y / x);
	}
	else if (x < 0.0f)
	{
		if (y >= 0.0f)
		{
			return math_atan(y / x) + PI;
		}
		else
		{
			return math_atan(y / x) - PI;
		}
	}
	else //x == 0
	{
		if (y > 0.0f)
		{
			return PI / 2.0f;
		}
		else if (y < 0.0f)
		{
			return -PI / 2.0f;
		}
		else
		{
			return 0.0f; //undefined; return 0
		}
	}
}

float math_pow(double a, double b)
{
	//Source: https://martin.ankerl.com/2012/01/25/optimized-approximative-pow-in-c-and-cpp/

	//Calculating approximation with fraction of the exponent
	int e = (int)b;

	union
	{
		double d;
		int x[2];
	}
	u = {a};

	u.x[1] = (int)((b - e) * (u.x[1] - 1072632447) + 1072632447);
	u.x[0] = 0;

	//Exponentiating by squaring with the exponent's integer part
	double r = 1.0;

	while (e)
	{
		if (e & 1)
		{
			r *= a;
		}

		a *= a;
		e >>= 1;
	}

	return (float)(r * u.d);
}