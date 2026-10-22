#include "os.h"

#include <stddef.h>

//This is a separate file as whole program optimization must be disabled here

#if defined(_WIN64)

#pragma function(memset)
void *__cdecl memset(void *dst, int value, size_t size)
{
	unsigned char *p = dst;
	unsigned char c = (unsigned char)value;

	size_t pattern = c;
	pattern |= pattern << 8;
	pattern |= pattern << 16;
	pattern |= pattern << 32;

	//Must first align to 8 bytes
	while (size && ((size_t)p % 8))
	{
		*p = c;
		p++;
		size--;
	}

	//Setting 8 bytes at a time
	while (size >= 8)
	{
		*(size_t *)p = pattern;
		p += 8;
		size -= 8;
	}

	//Setting the leftover bytes
	while (size)
	{
		*p = c;
		p++;
		size--;
	}

	return dst;
}

#else

#pragma function(memset)
void *__cdecl memset(void *dst, int value, size_t size)
{
	unsigned char *p = dst;
	unsigned char c = (unsigned char)value;

	while (size)
	{
		*p = c;
		p++;
		size--;
	}

	return dst;
}

#endif

//#pragma function(memset)
//void *__cdecl memset(void *dst, int value, size_t size)
//{
//	unsigned char *dst_char = dst;
//
//	while (size)
//	{
//		*dst_char = (unsigned char)value;
//		dst_char++;
//		size--;
//	}
//
//	return dst;
//}

#pragma function(memcpy)
void *__cdecl memcpy(void *dst, const void *src, size_t size)
{
	unsigned char *dst_char = dst;
	const unsigned char *src_char = src;

	while (size)
	{
		*dst_char = *src_char;
		dst_char++;
		src_char++;
		size--;
	}

	return dst;
}

#pragma function(memmove)
void *__cdecl memmove(void *dst, const void *src, size_t size)
{
	unsigned char *dst_char = dst;
	const unsigned char *src_char = src;

	if (dst <= src)
	{
		while (size--)
		{
			*dst_char++ = *src_char++;
		}
	}
	else if (dst > src)
	{
		dst_char += size - 1;
		src_char += size - 1;
		while (size--)
		{
			*dst_char-- = *src_char--;
		}
	}
	return (dst);
}