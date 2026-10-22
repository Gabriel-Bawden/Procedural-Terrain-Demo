#ifndef STR_H
#define STR_H

#include <stddef.h>

#define BINARY_INT_SIZE 4
#define BINARY_FLOAT_SIZE 4
#define BINARY_SHORT_SIZE 2
#define BINARY_CHAR_BIT_SIZE 8

//Used to test if code works with 8 byte integers
//#define int long long

//Note: this fails because shorts overflow from the large numbers of pixels in the textures
//#define int short

//String Functions
size_t str_len(const char *src); //Returns string length

void str_cat(char *dst, const char *src, size_t size); //Concatenate string. dst must be set to {0} if empty. Size is the lenght of the char array.

unsigned char str_equal(const char *a, const char *b); //Check if two strings are equal

void str_reverse(char *a);

//Return value: indicates whether function succeeded
int str_int_to_str(char *str, int size, int n); //Needs array of length 12

int str_str_to_int(const char *str);

float str_str_to_float(const char *str); //Convert string to float //use with 10 character limit

void str_float_to_str(char *str, int str_size, float n); //Convert float to string //Max of 34 characters

//Reading
float utl_binary_to_float(unsigned char *p);

unsigned int utl_binary_to_uint16(unsigned char *p);

long utl_binary_to_int32(unsigned char *p);

unsigned long utl_binary_to_uint32(unsigned char *p);

//Writing
void utl_float_to_binary(unsigned char *p, float value);

void utl_uint16_to_binary(unsigned char *p, unsigned int value);

void utl_int32_to_binary(unsigned char *p, long value);

void utl_uint32_to_binary(unsigned char *p, unsigned long value);

//Random
int utl_rand(void); //RAND_MAX assumed to be 32767

void utl_srand(unsigned int seed);

#endif