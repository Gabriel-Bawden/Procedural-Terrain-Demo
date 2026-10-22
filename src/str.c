#include "str.h"

#include <limits.h>
#include <stddef.h>

#include "math.h"
#include "os.h"

//Used to test if code works with 8 byte integers
//#define int long long

//Note: this fails because shorts overflow from the large numbers of pixels in the textures
//#define int short

//String Functions
size_t str_len(const char *src) //Returns string length
{
	const char *str;

	for (str = src; *str; str++);

	ptrdiff_t diff = str - src;

	return (size_t)diff; //First value not included in this count which cancels out null terminating character
}

void str_cat(char *dst, const char *src, size_t size) //Concatenate string. dst must be set to {0} if empty. Size is the lenght of the dst array.
{
	if (dst && src && size)
	{
		size_t dst_len = str_len(dst);
		size_t src_len = str_len(src);

		if (dst_len + src_len + 1 > size)
		{
			src_len = size - dst_len - 1;
		}

		dst += dst_len; //Sets dst pointer to terminating null position

		memcpy(dst, src, src_len);

		dst[src_len] = '\0';
	}
}

unsigned char str_equal(const char *a, const char *b) //Check if two strings are equal
{
	while (*a && (*a == *b))
	{
		a++;
		b++;
	}

	//return !(int)(*a - *b);
	return !(*(const unsigned char *)a - *(const unsigned char *)b);
}

//WARNING: WILL NOT WORK FOR UTF-8 STRINGS
void str_reverse(char *a)
{
	if (str_len(a))
	{
		size_t left = 0;
		size_t right = str_len(a) - 1;

		while (left < right)
		{
			char temp = a[left];
			a[left] = a[right];
			a[right] = temp;
			left++;
			right--;
		}
	}
}

//Return value: indicates whether function succeeded
int str_int_to_str(char *str, int size, int n) //Needs array of length 12
{
	//Since INT_MIN is at least -32,767
	if (size < 7)
	{
		return 0;
	}

	//Handling INT_MIN values since -INT_MIN is undefined
	if (n == INT_MIN)
	{
		n++;
	}

	int neg = 0;

	if (n < 0)
	{
		neg = 1;
		n *= -1;
	}

	int i = 0;

	do
	{
		str[i] = n % 10 + '0';
		n /= 10;
		i++;
	} while (n > 0 && i < size - 2);

	//Error: str does not have enough space
	if (n)
	{
		return 0;
	}

	if (neg)
	{
		str[i] = '-';
		i++;
	}

	str[i] = '\0';

	str_reverse(str);

	return 1;
}

int str_str_to_int(const char *str)
{
	int n = 0;
	int neg = 0;

	if (str[0] == '-')
	{
		neg = 1;
	}

	for (int i = neg; str[i] != '\0'; i++)
	{
		int digit = str[i] - '0';

		//Check whether digit is a number
		if (digit >= 0 && digit <= 9)
		{
			//Check for overflow
			if (n <= (INT_MAX - digit) / 10)
			{
				n = 10 * n + digit;
			}
			else
			{
				n = INT_MAX;

				break;
			}
		}
		else
		{
			return 0;
		}
	}

	if (neg)
	{
		n *= -1;
	}

	return n;
}

float str_str_to_float(const char *str) //Convert string to float //use with 10 character limit
{
	int n = 0;
	int neg = 1; //1 if not negative, -1 if negative
	int dec = -1; //-1 if no decimal, if decimal, carries index value of decmial within the string
	size_t len = str_len(str);

	if (str[0] == '-')
	{
		neg = -1;
	}

	//double mult = pow(10, (len - (neg < 0) - 1));

	long mult = 1;

	for (size_t i = 0; i < len - (neg < 0) - 1; i++)
	{
		mult *= 10;
	}

	for (size_t i = neg < 0; i < len; i++)
	{
		if (str[i] == '.')
		{
			dec = (int)i;
		}
		else
		{
			n += mult * (int)(str[i] - '0');

			mult /= 10;
		}
	}

	float dec_mult = 1;

	if (dec > 0)
	{
		dec_mult = math_pow(10, (float)(len - dec));
	}

	return n * neg / dec_mult;
}

void str_float_to_str(char *str, int str_size, float n) //Convert float to string //Max of 34 characters
{
	int n1 = (int)n;
	float diff = math_abs_float(n - n1);
	int n2 = (int)(diff * 1000000000);

	char str1[15] = {0};
	char str2[15] = {0};
	char str3[15] = {0};

	int zero_count = 0;

	if (diff != 0.0f)
	{
		while ((diff * 10) < 1.0f)
		{
			str2[zero_count] = '0';
			zero_count++;
			diff *= 10;
		}

		str2[zero_count] = '\0';
	}

	str_int_to_str(str1, sizeof(str1), n1);
	str_int_to_str(str3, sizeof(str2), n2);

	for (size_t i = str_len(str3) - 1; i > 0; i--)
	{
		if (str3[i] == '0')
		{
			str3[i] = '\0';
		}
		else
		{
			break;
		}
	}

	str[0] = '\0';
	str_cat(str, str1, str_size);
	str_cat(str, ".", str_size);
	str_cat(str, str2, str_size);
	str_cat(str, str3, str_size);
}

//Reading
float utl_binary_to_float(unsigned char *p)
{
	if (sizeof(float) == BINARY_FLOAT_SIZE)
	{
		unsigned long value_unsigned =
			((unsigned long)p[0] << 0) |
			((unsigned long)p[1] << 8) |
			((unsigned long)p[2] << 16) |
			((unsigned long)p[3] << 24);

		float value;
		memcpy(&value, &value_unsigned, sizeof(value));

		return value;
	}
	else
	{
		return 0;
	}
}

unsigned int utl_binary_to_uint16(unsigned char *p)
{
	unsigned int value;

	value =
		((unsigned int)p[0] << 0) |
		((unsigned int)p[1] << 8);

	return value;
}

long utl_binary_to_int32(unsigned char *p)
{
	unsigned long value =
		((unsigned long)p[0] << 0) |
		((unsigned long)p[1] << 8) |
		((unsigned long)p[2] << 16) |
		((unsigned long)p[3] << 24);

	if (value & 0x80000000UL) //Check if leftmost bit is 1 (negative)
	{
		return -((long)((~value & 0xFFFFFFFFUL) + 1)); //Convert to signed
	}
	else
	{
		return (long)value;
	}
}

unsigned long utl_binary_to_uint32(unsigned char *p)
{
	unsigned long value;

	value =
		((unsigned long)p[0] << 0) |
		((unsigned long)p[1] << 8) |
		((unsigned long)p[2] << 16) |
		((unsigned long)p[3] << 24);

	return value;
}

//Writing
void utl_float_to_binary(unsigned char *p, float value)
{
	if (sizeof(float) == BINARY_FLOAT_SIZE)
	{
		unsigned long value_unsigned;
		memcpy(&value_unsigned, &value, sizeof(value));

		p[0] = (value_unsigned >> 0) & 0xFF;
		p[1] = (value_unsigned >> 8) & 0xFF;
		p[2] = (value_unsigned >> 16) & 0xFF;
		p[3] = (value_unsigned >> 24) & 0xFF;
	}
}

void utl_uint16_to_binary(unsigned char *p, unsigned int value)
{
	//Could add range check

	p[0] = (value >> 0) & 0xFF;
	p[1] = (value >> 8) & 0xFF;
}

void utl_int32_to_binary(unsigned char *p, long value)
{
	if (value < (-2147483647L - 1L) || value > 2147483647L)
	{
		return;
	}

	unsigned long value_unsigned;

	if (value < 0)
	{
		value_unsigned = (unsigned long)(~(-value) + 1);
	}
	else
	{
		value_unsigned = (unsigned long)value;
	}

	p[0] = (value_unsigned >> 0) & 0xFF;
	p[1] = (value_unsigned >> 8) & 0xFF;
	p[2] = (value_unsigned >> 16) & 0xFF;
	p[3] = (value_unsigned >> 24) & 0xFF;
}

void utl_uint32_to_binary(unsigned char *p, unsigned long value)
{
	//Could add range check

	p[0] = (value >> 0) & 0xFF;
	p[1] = (value >> 8) & 0xFF;
	p[2] = (value >> 16) & 0xFF;
	p[3] = (value >> 24) & 0xFF;
}

//Random
static unsigned long int utl_next = 1;

int utl_rand(void) // RAND_MAX assumed to be 32767
{
	utl_next = utl_next * 1103515245 + 12345;
	return (unsigned int)(utl_next / 65536) % 32768;
}

void utl_srand(unsigned int seed)
{
	utl_next = seed;
}