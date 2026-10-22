#ifndef TEXTURE_H
#define TEXTURE_H

#include <stddef.h>

#include "dynarray.h"

struct texture
{
	char *name;

	size_t w;
	size_t h;
	unsigned char *pixels;
	unsigned int bitdepth;
};

unsigned char texture_create_from_file(struct texture *v, const char *file_path);

void texture_create(struct texture *v, size_t w, size_t h, char *name);

void load_texture_destroy(void *ptr);

void texture_load_directory(const char *directory_path, struct dynarray *textures);

void texture_save(struct texture *v);

struct texture *texture_find(struct dynarray *textures, const char *name);

#endif