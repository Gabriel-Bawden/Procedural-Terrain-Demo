#include "texture.h"

#include <stddef.h>

#include "str.h"
#include "alloc.h"
#include "os.h"
#include "dynarray.h"

unsigned char texture_create_from_file(struct texture *v, const char *file_path)
{
	v->name = NULL;

	struct os_file file;

	if (os_file_create(&file, file_path, 'r'))
	{
		size_t last_index = str_len(file_path) - 1;

		if (last_index > 4)
		{
			for (size_t i = last_index - 4; i < last_index - 3; i--)
			{
				if (file_path[i] == '\\')
				{
					v->name = alloc_calloc(last_index - i - 3, sizeof(char)); //One larger than amount copied for NULL 
					memcpy(v->name, &file_path[i + 1], (last_index - i - 4) * sizeof(char)); //Subtract 4 for the .mdl

					break;
				}
			}
		}

		size_t size = os_file_size(&file);
		unsigned char *buffer = alloc_malloc(size);
		os_file_read(&file, buffer, size);
		os_file_destroy(&file);

		if (buffer)
		{
			size_t offset = (size_t)utl_binary_to_uint32(&buffer[10]);
			v->w = (size_t)utl_binary_to_uint32(&buffer[18]);
			v->h = (size_t)utl_binary_to_uint32(&buffer[22]);
			v->bitdepth = utl_binary_to_uint16(&buffer[28]);

			v->pixels = alloc_calloc(v->w * v->h * 4, sizeof(*v->pixels));
			v->bitdepth /= 8;

			size_t padd = 4 - (v->w * (size_t)v->bitdepth) % 4; //About padding: https://stackoverflow.com/questions/2601365/padding-in-24-bits-rgb-bitmap
			padd *= padd < 4;

			size_t k = 0;

			for (size_t i = 0; i < (v->w * v->h); i++)
			{
				size_t x = (i + v->w) % v->w;
				size_t y = (i / v->w);

				size_t ind = 4 * (x + v->w * y);

				if (padd != 0 && x == 0 && y != 0)
				{
					k += padd;
				}

				size_t pos = i * (size_t)v->bitdepth + offset + k;

				if (v->bitdepth == 4)
				{
					v->pixels[ind + 0] = buffer[pos + 0];
					v->pixels[ind + 1] = buffer[pos + 1];
					v->pixels[ind + 2] = buffer[pos + 2];
					v->pixels[ind + 3] = buffer[pos + 3];
				}
				else if (v->bitdepth == 3)
				{
					v->pixels[ind + 0] = buffer[pos + 0];
					v->pixels[ind + 1] = buffer[pos + 1];
					v->pixels[ind + 2] = buffer[pos + 2];
					v->pixels[ind + 3] = 255;
				}
			}
		}
		else
		{
			//error_print("Error: texture not found\n");
		}

		alloc_free(buffer);

		return 1;
	}
	else
	{
		return 0;
	}
}

void texture_create(struct texture *v, size_t w, size_t h, char *name)
{
	if (name)
	{
		size_t size = (str_len(name) + 1) * sizeof(*v->name);
		v->name = alloc_malloc(size);
		memcpy(v->name, name, size);
	}
	else
	{
		v->name = NULL;
	}

	v->w = w;
	v->h = h;

	v->pixels = alloc_malloc(v->w * v->h * 4 * (int)sizeof(*v->pixels));
	memset(v->pixels, 255, v->w * v->h * 4 * sizeof(*v->pixels));
	v->bitdepth = 4;
}

void load_texture_destroy(void *ptr)
{
	struct texture *v = ptr;

	alloc_free(v->name);
	alloc_free(v->pixels); //freeing unallocated pointer is undefined behavior
}

void texture_load_directory(const char *directory_path, struct dynarray *textures)
{
	struct os_file_directory file_directory;

	if (os_file_directory_create(&file_directory, directory_path))
	{
		while (os_file_directory_next(&file_directory))
		{
			char path[MAXIMUM_PATH] = {0};
			str_cat(path, directory_path, sizeof(path));
			str_cat(path, "\\", sizeof(path));
			str_cat(path, file_directory.file_name, sizeof(path));

			if (file_directory.type == 1)
			{
				texture_load_directory(path, textures);
			}
			else if ((str_len(path) > str_len(".bmp")) && str_equal(&path[str_len(path) - str_len(".bmp")], ".bmp"))
			{
				struct texture *load_texture_new = alloc_calloc(1, sizeof(*load_texture_new));

				if (texture_create_from_file(load_texture_new, path))
				{
					dynarray_push(textures, &load_texture_new);
				}
			}
		}

		os_file_directory_destroy(&file_directory);
	}
}

void texture_save(struct texture *v)
{
	char file_path[MAXIMUM_PATH] = {0};
	str_cat(file_path, os_file_get_directory(), sizeof(file_path));
	str_cat(file_path, "textures\\", sizeof(file_path));
	str_cat(file_path, v->name, sizeof(file_path));
	str_cat(file_path, ".bmp", sizeof(file_path));

	struct os_file file;

	if (os_file_create(&file, file_path, 'w'))
	{
		v->bitdepth = 4;

		size_t header_size = 54;
		size_t size = header_size + v->w * v->h * v->bitdepth;

		unsigned char* buffer = alloc_calloc(size, sizeof(*buffer));

		//BMP Signature
		unsigned int bmp_signature = 19778;
		utl_uint16_to_binary(&buffer[0], bmp_signature);

		//File Size
		utl_uint32_to_binary(&buffer[2], (unsigned long)size);

		//Header Offset
		utl_uint32_to_binary(&buffer[10], (unsigned long)header_size);

		//DIP Header Size
		unsigned long dip_header_size = 40;
		utl_uint32_to_binary(&buffer[14], dip_header_size);

		//Width
		utl_uint32_to_binary(&buffer[18], (unsigned long)v->w);

		//Height
		utl_uint32_to_binary(&buffer[22], (unsigned long)v->h);

		//Number of Color Planes
		unsigned int color_planes = 1;
		utl_uint16_to_binary(&buffer[26], color_planes);

		//Bit Depth
		unsigned int bit_depth = v->bitdepth * 8;
		utl_uint16_to_binary(&buffer[28], bit_depth);

		//X Resolution
		unsigned long x_resolution = 3780;
		utl_uint32_to_binary(&buffer[38], x_resolution);

		//Y Resolution
		unsigned long y_resolution = 3780;
		utl_uint32_to_binary(&buffer[42], y_resolution);

		memcpy(buffer + header_size * sizeof(*buffer), v->pixels, v->w * v->h * v->bitdepth * sizeof(*v->pixels));

		os_file_write(&file, buffer, size * sizeof(*buffer));
		os_file_destroy(&file);

		alloc_free(buffer);
	}
}

struct texture *texture_find(struct dynarray *textures, const char *name)
{
	if (dynarray_len(textures))
	{
		struct texture *texture = *((struct texture **)dynarray_get(textures, 0));

		for (size_t i = 0; i < dynarray_len(textures); i++)
		{
			struct texture *temp = *((struct texture**)dynarray_get(textures, i));

			if (str_equal(temp->name, name))
			{
				texture = temp;
			}
		}

		return texture;
	}
	else
	{
		return NULL;
	}
}