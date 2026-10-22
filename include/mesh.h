#ifndef MESH_H
#define MESH_H

#include <stddef.h>

#include "dynarray.h"
#include "texture.h"

struct mesh_material
{
	size_t start_index;
	float ka[3];
	float kd[3];
	float ks[3];
	unsigned int illum; //Could be made unsigned char
	float ns;
	struct texture *map_kd;
	struct texture *map_bump;
};

struct mesh
{
	char *name;

	size_t index_count;
	size_t vertex_count;
	size_t material_count;
	size_t index_capacity;
	size_t vertex_capacity;
	size_t material_capacity;
	unsigned int *indices;
	float *vertices;
	float *texture_coordinates;
	float *normals;
	struct mesh_material *materials;
};

unsigned char mesh_create_from_file(struct mesh *v, const char *file_path, struct dynarray *textures);

void mesh_create(struct mesh *v, size_t index_count, size_t vertex_count, unsigned char has_text, unsigned char has_norm, char *mod);

unsigned int mesh_add_vertex(struct mesh *v, float x, float y, float z, float tu, float tv);

void mesh_add_index(struct mesh *v, unsigned int index1, unsigned int index2, unsigned int index3);

void mesh_destroy(void *ptr);

void mesh_load_directory(const char *directory_path, struct dynarray *models, struct dynarray *textures);

void mesh_save(struct mesh *v);

struct mesh *mesh_find(struct dynarray *meshes, const char *name);

#endif