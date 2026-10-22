#include "mesh.h"

#include <stddef.h>

#include "str.h"
#include "alloc.h"
#include "os.h"
#include "dynarray.h"
#include "texture.h"

unsigned char mesh_create_from_file(struct mesh *v, const char *file_path, struct dynarray *textures)
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

		if (buffer && size > 6 * BINARY_INT_SIZE)
		{
			v->indices = NULL;
			v->vertices = NULL;
			v->texture_coordinates = NULL;
			v->normals = NULL;
			v->materials = NULL;

			size_t index = 0;

			v->index_count = (size_t)utl_binary_to_uint32(buffer + index);
			index += BINARY_INT_SIZE;

			v->vertex_count = (size_t)utl_binary_to_uint32(buffer + index);
			index += BINARY_INT_SIZE;

			unsigned int has_texture = (unsigned int)utl_binary_to_uint32(buffer + index);
			index += BINARY_INT_SIZE;

			unsigned int has_normals = (unsigned int)utl_binary_to_uint32(buffer + index);
			index += BINARY_INT_SIZE;

			v->material_count = (size_t)utl_binary_to_uint32(buffer + index);
			index += BINARY_INT_SIZE;

			size_t len = (5 + 3 * v->index_count + (3 + 2 * (size_t)has_texture + 3 * (size_t)has_normals) * v->vertex_count) * BINARY_INT_SIZE;
			size_t materials_len = v->material_count * (2 * BINARY_INT_SIZE + 10 * BINARY_FLOAT_SIZE + 50 * sizeof(char));

			if (size != len + materials_len)
			{
				//error_print("Error: model format not valid\n");
			}

			if (v->index_count && v->vertex_count)
			{
				v->index_capacity = v->index_count;
				v->vertex_capacity = v->vertex_count;

				v->indices = alloc_malloc(3 * v->index_capacity * sizeof(*v->indices));
				v->vertices = alloc_malloc(3 * v->vertex_capacity * sizeof(*v->vertices));

				for (size_t i = 0; i < 3 * v->index_count; i++)
				{
					v->indices[i] = (unsigned int)utl_binary_to_uint32(buffer + index);
					index += BINARY_INT_SIZE;
				}

				for (size_t i = 0; i < 3 * v->vertex_count; i++)
				{
					v->vertices[i] = utl_binary_to_float(buffer + index);
					index += BINARY_FLOAT_SIZE;
				}

				if (has_texture)
				{
					v->texture_coordinates = alloc_malloc(2 * v->vertex_capacity * sizeof(*v->texture_coordinates));

					for (size_t i = 0; i < 2 * v->vertex_count; i++)
					{
						v->texture_coordinates[i] = utl_binary_to_float(buffer + index);
						index += BINARY_FLOAT_SIZE;
					}
				}

				if (has_normals)
				{
					v->normals = alloc_malloc(3 * v->vertex_capacity * sizeof(*v->normals));

					for (size_t i = 0; i < 3 * v->vertex_count; i++)
					{
						v->normals[i] = utl_binary_to_float(buffer + index);
						index += BINARY_FLOAT_SIZE;
					}
				}

				if (v->material_count)
				{
					//Beware of struct padding: structs can have padding between members, so the material struct cannot simply be copied from the binary file

					v->material_capacity = v->material_count;

					v->materials = alloc_malloc(v->material_capacity * sizeof(*v->materials));

					for (size_t i = 0; i < v->material_count; i++)
					{
						v->materials[i].start_index = (size_t)utl_binary_to_uint32(buffer + index);
						index += BINARY_INT_SIZE;

						for (size_t j = 0; j < 3; j++)
						{
							v->materials[i].ka[j] = utl_binary_to_float(buffer + index);
							index += BINARY_FLOAT_SIZE;
						}

						for (size_t j = 0; j < 3; j++)
						{
							v->materials[i].kd[j] = utl_binary_to_float(buffer + index);
							index += BINARY_FLOAT_SIZE;
						}

						for (size_t j = 0; j < 3; j++)
						{
							v->materials[i].ks[j] = utl_binary_to_float(buffer + index);
							index += BINARY_FLOAT_SIZE;
						}

						v->materials[i].illum = (unsigned int)utl_binary_to_uint32(buffer + index);
						index += BINARY_INT_SIZE;

						v->materials[i].ns = utl_binary_to_float(buffer + index);
						index += BINARY_FLOAT_SIZE;

						char map_kd[50] = {0};

						for (size_t j = 0; j < sizeof(map_kd); j++)
						{
							map_kd[j] = buffer[index];
							index += sizeof(*buffer);
						}

						map_kd[sizeof(map_kd) - 1] = '\0';

						if (str_len(map_kd))
						{
							v->materials[i].map_kd = texture_find(textures, map_kd);
						}
						else
						{
							v->materials[i].map_kd = NULL;
						}

						char map_bump[50] = {0};

						for (size_t j = 0; j < sizeof(map_bump); j++)
						{
							map_bump[j] = buffer[index];
							index += sizeof(*buffer);
						}

						map_bump[sizeof(map_bump) - 1] = '\0';

						if (str_len(map_bump))
						{
							v->materials[i].map_bump = texture_find(textures, map_bump);
						}
						else
						{
							v->materials[i].map_bump = NULL;
						}
					}
				}
			}

			//Could add code to loop through all indices and make sure they don't exceed the max vertex
		}
		else
		{
			// error_print("Error: model not found\n");
		}

		alloc_free(buffer);

		return 1;
	}
	else
	{
		return 0;
	}
}

void mesh_create(struct mesh *v, size_t index_count, size_t vertex_count, unsigned char has_texture, unsigned char has_normals, char *mod)
{
	v->index_count = index_count;
	v->vertex_count = vertex_count;
	v->material_count = 0;
	v->indices = NULL;
	v->vertices = NULL;
	v->texture_coordinates = NULL;
	v->normals = NULL;
	v->materials = NULL;

	if (mod)
	{
		size_t size = (str_len(mod) + 1) * (int)sizeof(*v->name);
		v->name = alloc_malloc(size);
		memcpy(v->name, mod, size);
	}
	else
	{
		v->name = NULL;
	}

	if (v->index_count && v->vertex_count)
	{
		v->index_capacity = v->index_count;
		v->vertex_capacity = v->vertex_count;

		v->indices = alloc_calloc(3 * v->index_capacity, sizeof(*v->indices));
		v->vertices = alloc_calloc(3 * v->vertex_capacity, sizeof(*v->vertices));

		if (has_texture)
		{
			v->texture_coordinates = alloc_calloc(2 * v->vertex_capacity, sizeof(*v->texture_coordinates));
		}

		if (has_normals)
		{
			v->normals = alloc_calloc(3 * v->vertex_capacity, sizeof(*v->normals));
		}
	}
}

unsigned int mesh_add_vertex(struct mesh *v, float x, float y, float z, float tu, float tv)
{
	if (v->vertex_count >= v->vertex_capacity)
	{
		v->vertex_capacity = 2 * v->vertex_count;

		if (v->vertex_capacity == 0)
		{
			v->vertex_capacity = 1;
		}

		v->vertices = alloc_realloc(v->vertices, 3 * v->vertex_capacity * sizeof(*v->vertices));
		v->texture_coordinates = alloc_realloc(v->texture_coordinates, 2 * v->vertex_capacity * sizeof(*v->texture_coordinates));
	}

	v->vertices[3 * v->vertex_count + 0] = x;
	v->vertices[3 * v->vertex_count + 1] = y;
	v->vertices[3 * v->vertex_count + 2] = z;

	v->texture_coordinates[2 * v->vertex_count + 0] = tu;
	v->texture_coordinates[2 * v->vertex_count + 1] = tv;

	v->vertex_count++;

	return (unsigned int)(v->vertex_count - 1);
}

void mesh_add_index(struct mesh *v, unsigned int index1, unsigned int index2, unsigned int index3)
{
	if (v->index_count >= v->index_capacity)
	{
		v->index_capacity = 2 * v->index_count;

		if (v->index_capacity == 0)
		{
			v->index_capacity = 1;
		}

		v->indices = alloc_realloc(v->indices, 3 * v->index_capacity * sizeof(*v->indices));
	}

	v->indices[3 * v->index_count + 0] = index1;
	v->indices[3 * v->index_count + 1] = index2;
	v->indices[3 * v->index_count + 2] = index3;

	v->index_count++;
}

void mesh_destroy(void *ptr)
{
	struct mesh *v = ptr;

	alloc_free(v->name);
	alloc_free(v->vertices); //freeing unallocated pointer is undefined behavior, must be at least NULL
	alloc_free(v->texture_coordinates);
	alloc_free(v->normals);
	alloc_free(v->indices);
	alloc_free(v->materials);
}

void mesh_load_directory(const char *directory_path, struct dynarray *models, struct dynarray *textures)
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
				mesh_load_directory(path, models, textures);
			}
			else if ((str_len(path) > str_len(".mdl")) && str_equal(&path[str_len(path) - str_len(".mdl")], ".mdl"))
			{
				struct mesh *mesh_new = alloc_calloc(1, sizeof(*mesh_new));

				if (mesh_create_from_file(mesh_new, path, textures))
				{
					dynarray_push(models, &mesh_new);
				}
			}
		}

		os_file_directory_destroy(&file_directory);
	}
}

void mesh_save(struct mesh *v)
{
	char file_path[MAXIMUM_PATH] = {0};
	str_cat(file_path, os_file_get_directory(), sizeof(file_path));
	str_cat(file_path, "models\\", sizeof(file_path));
	str_cat(file_path, v->name, sizeof(file_path));
	str_cat(file_path, ".mdl", sizeof(file_path));

	struct os_file file;

	if (os_file_create(&file, file_path, 'w'))
	{
		unsigned int has_texture = (v->texture_coordinates != NULL);
		unsigned int has_normals = (v->normals != NULL);

		size_t len = (5 + 3 * v->index_count + (3 + 2 * (size_t)has_texture + 3 * (size_t)has_normals) * v->vertex_count) * BINARY_INT_SIZE;
		size_t materials_len = v->material_count * (2 * BINARY_INT_SIZE + 10 * BINARY_FLOAT_SIZE + 50 * (int)sizeof(char));

		unsigned char* buffer = alloc_malloc(len + materials_len);

		size_t index = 0;

		utl_uint32_to_binary(buffer + index, (unsigned long)v->index_count);
		index += BINARY_INT_SIZE;

		utl_uint32_to_binary(buffer + index, (unsigned long)v->vertex_count);
		index += BINARY_INT_SIZE;

		utl_uint32_to_binary(buffer + index, (unsigned long)has_texture);
		index += BINARY_INT_SIZE;

		utl_uint32_to_binary(buffer + index, (unsigned long)has_normals);
		index += BINARY_INT_SIZE;

		utl_uint32_to_binary(buffer + index, (unsigned long)v->material_count);
		index += BINARY_INT_SIZE;

		if (v->vertex_count)
		{
			for (size_t i = 0; i < 3 * v->index_count; i++)
			{
				utl_uint32_to_binary(buffer + index, (unsigned long)v->indices[i]);
				index += BINARY_INT_SIZE;
			}

			for (size_t i = 0; i < 3 * v->vertex_count; i++)
			{
				utl_float_to_binary(buffer + index, v->vertices[i]);
				index += BINARY_FLOAT_SIZE;
			}
		}

		if (has_texture)
		{
			for (size_t i = 0; i < 2 * v->vertex_count; i++)
			{
				utl_float_to_binary(buffer + index, v->texture_coordinates[i]);
				index += BINARY_FLOAT_SIZE;
			}
		}

		if (has_normals)
		{
			for (size_t i = 0; i < 3 * v->vertex_count; i++)
			{
				utl_float_to_binary(buffer + index, v->normals[i]);
				index += BINARY_FLOAT_SIZE;
			}
		}

		if (v->material_count)
		{
			for (size_t i = 0; i < v->material_count; i++)
			{
				utl_uint32_to_binary(buffer + index, (unsigned long)v->materials[i].start_index);
				index += BINARY_INT_SIZE;

				for (size_t j = 0; j < 3; j++)
				{
					utl_float_to_binary(buffer + index, v->materials[i].ka[j]);
					index += BINARY_FLOAT_SIZE;
				}

				for (size_t j = 0; j < 3; j++)
				{
					utl_float_to_binary(buffer + index, v->materials[i].kd[j]);
					index += BINARY_FLOAT_SIZE;
				}

				for (size_t j = 0; j < 3; j++)
				{
					utl_float_to_binary(buffer + index, v->materials[i].ks[j]);
					index += BINARY_FLOAT_SIZE;
				}

				utl_uint32_to_binary(buffer + index, (unsigned long)v->materials[i].illum);
				index += BINARY_INT_SIZE;

				utl_float_to_binary(buffer + index, v->materials[i].ns);
				index += BINARY_FLOAT_SIZE;

				for (size_t j = 0; j < sizeof(v->materials[i].map_kd->name); j++)
				{
					buffer[index] = v->materials[i].map_kd->name[j]; //Should be making sure map_kd is not blank
					index += sizeof(*buffer);
				}

				buffer[index - 1] = '\0';

				for (size_t j = 0; j < sizeof(v->materials[i].map_bump->name); j++)
				{
					buffer[index] = v->materials[i].map_bump->name[j]; //Should be making sure map_bump is not blank
					index += sizeof(*buffer);
				}

				buffer[index - 1] = '\0';
			}
		}

		os_file_write(&file, buffer, len + materials_len);
		os_file_destroy(&file);

		alloc_free(buffer);
	}
}

struct mesh *mesh_find(struct dynarray *meshes, const char *name)
{
	if (dynarray_len(meshes))
	{
		struct mesh *mesh = *((struct mesh **)dynarray_get(meshes, 0));

		for (size_t i = 0; i < dynarray_len(meshes); i++)
		{
			struct mesh *temp = *((struct mesh **)dynarray_get(meshes, i));

			if (str_equal(temp->name, name))
			{
				mesh = temp;
			}
		}

		return mesh;
	}
	else
	{
		return NULL;
	}
}