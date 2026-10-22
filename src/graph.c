#include "graph.h"

#include <stddef.h>

#include "math.h"
#include "vec.h"
#include "alloc.h"
#include "mesh.h"

float graph_x_t(float t, float u)
{
	if (0)
	{
		return t + u;
	}

	//return (4 * math_cos(t) - math_sin(u) * math_cos(t));
	return (math_cos(t) * math_sin(u));
	//return t;
}

float graph_y_t(float t, float u)
{
	if (0)
	{
		return t + u;
	}

	//return (4 * math_sin(t) - math_sin(u) * math_sin(t));
	return (math_sin(t) * math_sin(u));
	//return u;
}

float graph_z_t(float t, float u)
{
	if (0)
	{
		return t + u;
	}

	return (math_cos(u));
	//return math_sin(t + u);
	//return math_sin(t) + math_cos(u);
	//return 0;
}

float graph_z(float x, float y)
{
	return x + y;
	//return 0;
}

void graph_merge(struct mesh *v) //merge similar verticies together //note: does not currently remove the vertices themselves, only changes the triangles
{
	char *dupes = alloc_calloc(v->vertex_count, sizeof(*dupes));

	for (size_t i = 0; i < v->vertex_count; i++)
	{
		for (size_t j = i + 1; j < v->vertex_count; j++)
		{
			if (dupes[j] == 0)
			{
				float v1[3] = {v->vertices[3 * i + 0], v->vertices[3 * i + 1], v->vertices[3 * i + 2]};
				float v2[3] = {v->vertices[3 * j + 0], v->vertices[3 * j + 1], v->vertices[3 * j + 2]};
				float v3[3];

				vec_subtract(v1, v2, v3);

				if (math_abs_float(vec_length(v3)) < 0.00001f)
				{
					dupes[j] = 1;

					for (size_t k = 0; k < v->index_count; k++)
					{
						for (int l = 0; l < 3; l++)
						{
							if ((size_t)v->indices[3 * k + l] == j)
							{
								v->indices[3 * k + l] = (unsigned int)i;
							}
						}
					}
				}
			}
		}
	}

	alloc_free(dupes);
}

void graph_remove(struct mesh *v) //Remove faces with two or more of the same vertex
{
	int index = 0;

	for (size_t i = 0; i < v->index_count; i++)
	{
		if (v->indices[3 * i + 0] != v->indices[3 * i + 1] && v->indices[3 * i + 0] != v->indices[3 * i + 2] && v->indices[3 * i + 1] != v->indices[3 * i + 2])
		{
			if (v->vertex_count > 0)
			{
				v->indices[3 * index + 0] = v->indices[3 * i + 0];
				v->indices[3 * index + 1] = v->indices[3 * i + 1];
				v->indices[3 * index + 2] = v->indices[3 * i + 2];
			}

			index++;
		}
	}

	v->index_count = index;

	if (v->vertex_count > 0)
	{
		v->indices = alloc_realloc(v->indices, 3 * v->index_count * (int)sizeof(*v->indices));
	}
}

void graph_generate_normals(struct mesh *v)
{
	v->normals = alloc_calloc(3 * v->vertex_count, sizeof(*v->normals)); //Can cause memory leak if v->norm has already been allocated

	for (size_t i = 0; i < v->index_count; i++)
	{
		float p1[3] = {v->vertices[3 * v->indices[3 * i + 0] + 0], v->vertices[3 * v->indices[3 * i + 0] + 1], v->vertices[3 * v->indices[3 * i + 0] + 2]};
		float p2[3] = {v->vertices[3 * v->indices[3 * i + 1] + 0], v->vertices[3 * v->indices[3 * i + 1] + 1], v->vertices[3 * v->indices[3 * i + 1] + 2]};
		float p3[3] = {v->vertices[3 * v->indices[3 * i + 2] + 0], v->vertices[3 * v->indices[3 * i + 2] + 1], v->vertices[3 * v->indices[3 * i + 2] + 2]};

		float nf[3];

		vec_normal(p1, p2, p3, nf, 1);

		for (int j = 0; j < 3; j++)
		{
			for (int k = 0; k < 3; k++)
			{
				v->normals[3 * v->indices[3 * i + j] + k] += nf[k];
			}
		}
	}

	for (size_t i = 0; i < v->vertex_count; i++)
	{
		float nf[3] = {v->normals[3 * i], v->normals[3 * i + 1], v->normals[3 * i + 2]};

		if (vec_length(nf) != 0)
		{
			vec_normalize(nf, nf); //for blender, 1 must be negative, or winding order must be flipped

			v->normals[3 * i + 0] = nf[0];
			v->normals[3 * i + 1] = nf[1];
			v->normals[3 * i + 2] = nf[2];
		}
	}
}

void graph_test(struct mesh *v)
{
	//Rotate Model
	//float rot[12] = {1,0,0,0,0,1,0,0,0,0,1,0};

	//float theta_x = - PI / 2 - 0.3f;

	//rot[5] = math_cos(theta_x);
	//rot[6] = -math_sin(theta_x);
	//rot[9] = math_sin(theta_x);
	//rot[10] = math_cos(theta_x);

	//v->vert = vec_mult2(rot, v->vert, v->v_s);


	//Change Size of Model
	for (size_t i = 0; i < v->vertex_count; i++)
	{
		v->vertices[3 * i + 0] = v->vertices[3 * i + 0] * 0.5f;
		v->vertices[3 * i + 1] = v->vertices[3 * i + 1] * 0.5f;
		v->vertices[3 * i + 2] = v->vertices[3 * i + 2] * 0.5f;
		//v->vert[3 * i + 0] *= -1;
	}

	//v->dir = 0;

	//for (size_t i = 0; i < v->ind; i++)
	//{
	//	//int hold = v->indices[3 * i + 1];
	//	//v->indices[3 * i + 1] = v->indices[3 * i + 2];
	//	//v->indices[3 * i + 2] = hold;

	//	int hold = v->indices[3 * i + 0];
	//	v->indices[3 * i + 0] = v->indices[3 * i + 2];
	//	v->indices[3 * i + 2] = hold;
	//}
}

void graph_wave(struct mesh *v, int frames, float speed, float scale, unsigned char animation)
{
	if (animation == 1)
	{
		for (size_t i = 0; i < v->vertex_count; i++)
		{
			v->vertices[3 * i + 1] = math_cos(scale * (v->vertices[3 * i + 0] + (frames / speed)));
		}
	}
	else if (animation == 2)
	{
		for (size_t i = 0; i < v->vertex_count; i++)
		{
			v->vertices[3 * i + 1] = math_cos(scale * (v->vertices[3 * i + 2] + (frames / speed)));
		}
	}
	else if (animation == 3)
	{
		for (size_t i = 0; i < v->vertex_count; i++)
		{
			v->vertices[3 * i + 1] = math_cos(scale * (v->vertices[3 * i + 0] + (frames / speed))) + math_cos(scale * (v->vertices[3 * i + 2] + (frames / speed)));
		}
	}
}

void graph_create(struct mesh *v, const float *alt, float sea, size_t w, size_t h)
{
	mesh_destroy(v);
	//mesh_create(v, w * h, 0, 0, 2 * (w - 1) * (h - 1), 1);
	mesh_create(v, 2 * (w - 1) * (h - 1), w * h, 0, 0, "terrain");

	//float scale = 10;
	float scale = 60;

	for (size_t i = 0; i < w * h; i++)
	{
		size_t x = i % w;
		size_t y = i / w;
		float z = alt[i];

		if (z <= sea)
		{
			//z = z - 0.1f;
			z = scale * (z - sea) + (sea - 0.1f);
		}

		v->vertices[3 * i + 0] = (x - (w / 2.0f)) * 0.003f;
		v->vertices[3 * i + 1] = 5 * scale * z * 0.003f;
		v->vertices[3 * i + 2] = (y - (h / 2.0f)) * 0.003f;
	}

	for (size_t i = 0; i < (w - 1) * (h - 1); i++)
	{
		size_t x = i % (w - 1);
		size_t y = i / (w - 1);

		size_t v1 = (x + 1) + y * w;
		size_t v2 = (x + 1) + (y + 1) * w;
		size_t v3 = v2 + 1;
		size_t v4 = v1 + 1;

		v->indices[3 * (2 * i) + 0] = (unsigned int)v3 - 1;
		v->indices[3 * (2 * i) + 1] = (unsigned int)v2 - 1;
		v->indices[3 * (2 * i) + 2] = (unsigned int)v1 - 1;

		v->indices[3 * (2 * i) + 3] = (unsigned int)v1 - 1;
		v->indices[3 * (2 * i) + 4] = (unsigned int)v4 - 1;
		v->indices[3 * (2 * i) + 5] = (unsigned int)v3 - 1;
	}

	graph_generate_normals(v);
	//mesh_save(v);
}

void graph_create_2(struct mesh *v, float x1, float x2, float y1, float y2, float x_del, float y_del, int dir, int cull, char *mod, unsigned char normals)
{
	size_t x_size = (size_t)((x2 - x1) / x_del);
	size_t y_size = (size_t)((y2 - y1) / y_del);

	//mesh_create(v, (x_size + 1) * (y_size + 1), 1, 0, 2 * x_size * y_size, dir);
	mesh_create(v, 2 * x_size * y_size, (x_size + 1) * (y_size + 1), 1, 0, mod);

	for (size_t i = 0; i < (x_size + 1) * (y_size + 1); i++)
	{
		float x = x1 + x_del * (float)(i % (x_size + 1));
		float y = y1 + y_del * (float)(i / (x_size + 1));

		if (cull)
		{
			v->vertices[3 * i + 0] = graph_x_t(x, y);
			v->vertices[3 * i + 1] = graph_z_t(x, y);
			v->vertices[3 * i + 2] = graph_y_t(x, y);
		}
		else
		{
			v->vertices[3 * i + 0] = x;
			v->vertices[3 * i + 1] = graph_z(x, y);
			v->vertices[3 * i + 2] = y;
		}

		if (i % 2 == 0)
		{
			v->texture_coordinates[2 * i + 0] = 1 - 0.94f;
			v->texture_coordinates[2 * i + 1] = 1 - 0.81f;
		}
		else
		{
			v->texture_coordinates[2 * i + 0] = 1 - 0.69f;
			v->texture_coordinates[2 * i + 1] = 1 - 0.81f;
		}
	}

	for (size_t i = 0; i < x_size * y_size; i++)
	{
		size_t x = i % (x_size);
		size_t y = i / (x_size);

		size_t v1 = (x + 1) + y * (x_size + 1);
		size_t v2 = (x + 1) + (y + 1) * (x_size + 1);
		size_t v3 = v2 + 1;
		size_t v4 = v1 + 1;

		if (dir)
		{
			v->indices[3 * (2 * i) + 0] = (unsigned int)v3 - 1;
			v->indices[3 * (2 * i) + 1] = (unsigned int)v2 - 1;
			v->indices[3 * (2 * i) + 2] = (unsigned int)v1 - 1;

			v->indices[3 * (2 * i) + 3] = (unsigned int)v1 - 1;
			v->indices[3 * (2 * i) + 4] = (unsigned int)v4 - 1;
			v->indices[3 * (2 * i) + 5] = (unsigned int)v3 - 1;
		}
		else
		{
			v->indices[3 * (2 * i) + 0] = (unsigned int)v1 - 1;
			v->indices[3 * (2 * i) + 1] = (unsigned int)v2 - 1;
			v->indices[3 * (2 * i) + 2] = (unsigned int)v3 - 1;

			v->indices[3 * (2 * i) + 3] = (unsigned int)v3 - 1;
			v->indices[3 * (2 * i) + 4] = (unsigned int)v4 - 1;
			v->indices[3 * (2 * i) + 5] = (unsigned int)v1 - 1;
		}
	}

	graph_merge(v);
	graph_remove(v);

	if (normals)
	{
		graph_generate_normals(v);
	}
}

void graph_create_square(struct mesh *v, float x1, float x2, float y1, float y2, float x_del, float y_del, int dir, char *mod)
{
	size_t x_size = (size_t)((x2 - x1) / x_del);
	size_t y_size = (size_t)((y2 - y1) / y_del);

	mesh_create(v, 2 * x_size * y_size, (x_size + 1) * (y_size + 1), 1, 0, mod);

	for (size_t i = 0; i < (x_size + 1) * (y_size + 1); i++)
	{
		float x = x1 + x_del * (float)(i % (x_size + 1));
		float y = y1 + y_del * (float)(i / (x_size + 1));

		v->vertices[3 * i + 0] = x;
		v->vertices[3 * i + 1] = y;
		v->vertices[3 * i + 2] = 0;

		if (i == 0)
		{
			v->texture_coordinates[2 * i + 0] = 0;
			v->texture_coordinates[2 * i + 1] = 0;
		}
		else if (i == 1)
		{
			v->texture_coordinates[2 * i + 0] = 1;
			v->texture_coordinates[2 * i + 1] = 0;
		}
		else if (i == 2)
		{
			v->texture_coordinates[2 * i + 0] = 0;
			v->texture_coordinates[2 * i + 1] = 1;
		}
		else if (i == 3)
		{
			v->texture_coordinates[2 * i + 0] = 1;
			v->texture_coordinates[2 * i + 1] = 1;
		}
		else
		{
			v->texture_coordinates[2 * i + 0] = 0.5f;
			v->texture_coordinates[2 * i + 1] = 0.5f;
		}
	}

	for (size_t i = 0; i < x_size * y_size; i++)
	{
		size_t x = i % (x_size);
		size_t y = i / (x_size);

		size_t v1 = (x + 1) + y * (x_size + 1);
		size_t v2 = (x + 1) + (y + 1) * (x_size + 1);
		size_t v3 = v2 + 1;
		size_t v4 = v1 + 1;

		if (dir)
		{
			v->indices[3 * (2 * i) + 0] = (unsigned int)v3 - 1;
			v->indices[3 * (2 * i) + 1] = (unsigned int)v2 - 1;
			v->indices[3 * (2 * i) + 2] = (unsigned int)v1 - 1;

			v->indices[3 * (2 * i) + 3] = (unsigned int)v1 - 1;
			v->indices[3 * (2 * i) + 4] = (unsigned int)v4 - 1;
			v->indices[3 * (2 * i) + 5] = (unsigned int)v3 - 1;
		}
		else
		{
			v->indices[3 * (2 * i) + 0] = (unsigned int)v1 - 1;
			v->indices[3 * (2 * i) + 1] = (unsigned int)v2 - 1;
			v->indices[3 * (2 * i) + 2] = (unsigned int)v3 - 1;

			v->indices[3 * (2 * i) + 3] = (unsigned int)v3 - 1;
			v->indices[3 * (2 * i) + 4] = (unsigned int)v4 - 1;
			v->indices[3 * (2 * i) + 5] = (unsigned int)v1 - 1;
		}
	}

	graph_generate_normals(v);
}