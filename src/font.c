#include "font.h"

#include <stddef.h>

#include "str.h"
#include "alloc.h"
#include "os.h"
#include "texture.h"

void font_create(struct font *v, struct texture *texture)
{
	char path[MAXIMUM_PATH] = {0};
	str_cat(path, os_file_get_directory(), sizeof(path));
	str_cat(path, "fonts\\font.fnt", sizeof(path));

	struct os_file file;

	if (os_file_create(&file, path, 'r') && str_equal(texture->name, "font"))
	{
		size_t size = os_file_size(&file);
		unsigned char *buffer = alloc_malloc(size);
		os_file_read(&file, buffer, size);
		os_file_destroy(&file);

		size_t index = 0;

		v->line_height = (size_t)utl_binary_to_uint32(buffer + index);
		index += BINARY_INT_SIZE;

		v->base = (size_t)utl_binary_to_uint32(buffer + index);
		index += BINARY_INT_SIZE;

		v->width = (size_t)utl_binary_to_uint32(buffer + index);
		index += BINARY_INT_SIZE;

		v->height = (size_t)utl_binary_to_uint32(buffer + index);
		index += BINARY_INT_SIZE;

		v->character_count = (size_t)utl_binary_to_uint32(buffer + index);
		index += BINARY_INT_SIZE;

		v->mapping_count = (size_t)utl_binary_to_uint32(buffer + index);
		index += BINARY_INT_SIZE;

		v->characters = alloc_malloc(7 * v->character_count * sizeof(*v->characters));

		v->character_mappings = alloc_malloc(3 * v->mapping_count * sizeof(*v->character_mappings));

		for (size_t i = 0; i < 7 * v->character_count; i++)
		{
			v->characters[i] = (int)utl_binary_to_int32(buffer + index);
			index += BINARY_INT_SIZE;
		}

		for (size_t i = 0; i < 3 * v->mapping_count; i++)
		{
			v->character_mappings[i] = (size_t)utl_binary_to_uint32(buffer + index);
			index += BINARY_INT_SIZE;
		}

		v->texture = texture;

		alloc_free(buffer);
	}
	else
	{
		v->line_height = 13;
		v->base = 11;
		v->width = 15;
		v->height = 15;
		v->character_count = 1;
		v->mapping_count = 1;

		v->characters = alloc_malloc(7 * v->character_count * sizeof(*v->characters));

		v->character_mappings = alloc_malloc(3 * v->mapping_count * sizeof(*v->character_mappings));

		for (size_t i = 0; i < v->character_count; i++)
		{
			v->characters[7 * i + 0] = 0;
			v->characters[7 * i + 1] = 0;
			v->characters[7 * i + 2] = 5;
			v->characters[7 * i + 3] = 11;
			v->characters[7 * i + 4] = 0;
			v->characters[7 * i + 5] = 2;
			v->characters[7 * i + 6] = 6;
		}

		for (size_t i = 0; i < v->mapping_count; i++)
		{
			v->character_mappings[3 * i + 0] = 0;
			v->character_mappings[3 * i + 1] = 0;
			v->character_mappings[3 * i + 2] = 0;
		}

		v->texture = texture;
	}
}

void font_destroy(void *v)
{
	struct font *ptr = v;

	alloc_free(ptr->characters);
	alloc_free(ptr->character_mappings);
}