#ifndef FONT_H
#define FONT_H

#include <stddef.h>

#include "texture.h"

struct font
{
	size_t line_height;
	size_t base;
	size_t width;
	size_t height;
	size_t character_count;
	size_t mapping_count;

	int *characters;
	size_t *character_mappings;

	struct texture *texture;
};

void font_create(struct font *v, struct texture *texture);

void font_destroy(void *v);

#endif