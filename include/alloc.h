#ifndef ALLOC_H
#define ALLOC_H

#include <stddef.h>

void *alloc_malloc(size_t size);

void *alloc_calloc(size_t count, size_t size);

void *alloc_realloc(void *ptr, size_t size);

void alloc_free(void *ptr);

#endif