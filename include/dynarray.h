#ifndef DYNARRAY_H
#define DYNARRAY_H

#include <stddef.h>

struct dynarray
{
	size_t size_array; //number of elements in the array
	size_t size_element; //size in bytes of each element
	size_t current; //current element
	void (*destroy)(void *); //function pointer to object destructor
	unsigned char *array;
};

void dynarray_create(struct dynarray *v, size_t size_element, void (*destroy)(void *));

void *dynarray_get(struct dynarray *v, size_t index);

void dynarray_destroy_element(struct dynarray *v, size_t index);

void dynarray_clear(struct dynarray *v);

void dynarray_destroy(void *ptr);

void dynarray_size_check(struct dynarray *v);

void dynarray_set(struct dynarray *v, size_t index, void *value);

void dynarray_push(struct dynarray *v, void *value);

void dynarray_pop(struct dynarray *v, void *pop);

void dynarray_insert(struct dynarray *v, size_t index, void *value);

void dynarray_remove(struct dynarray *v, size_t index);

size_t dynarray_len(struct dynarray *v);

#endif