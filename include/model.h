#ifndef MODEL_H
#define MODEL_H

#include "mesh.h"
#include "texture.h"

struct model
{
	struct mesh *mesh;
	struct mesh_material base_material;

	float translation[16];
	float rotation_x[16];
	float rotation_y[16];
	float rotation_z[16];

	float rotation[16];
	float transformation[16];

	float x;
	float y;
	float z;

	float theta_x;
	float theta_y;
	float theta_z;

	float scale;
};

void model_create(struct model *v, struct mesh *mesh, struct texture *texture);

void model_change_color(struct model *v, unsigned char r, unsigned char g, unsigned char b);

void model_update(struct model *v);

void model_set_x(struct model *v, float x);

void model_set_y(struct model *v, float y);

void model_set_z(struct model *v, float z);

void model_set_theta_x(struct model *v, float theta_x);

void model_set_theta_y(struct model *v, float theta_y);

void model_set_theta_z(struct model *v, float theta_z);

void model_set_scale(struct model *v, float scale);

#endif