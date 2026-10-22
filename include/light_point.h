#ifndef LIGHT_POINT_H
#define LIGHT_POINT_H

#include "mesh.h"
#include "model.h"

struct light_point
{
	float position[3];
	float position_transformed[3];

	float x;
	float y;
	float z;

	float Ia[3];
	float I[3];

	float quadratic[3];

	struct model light_model;
};

void light_point_create(struct light_point *v, float Ia_r, float Ia_g, float Ia_b, float I_r, float I_g, float I_b, float q1, float q2, float q3, struct mesh *mod);

void light_point_update(struct light_point *v, const float *transformation);

void light_point_set_x(struct light_point *v, float x);

void light_point_set_y(struct light_point *v, float y);

void light_point_set_z(struct light_point *v, float z);

void light_point_nospec(struct light_point *v, const struct mesh_material *mat, const float *normal, const float *position, float *color);

void light_point_spec(struct light_point *v, const struct mesh_material *mat, const float *normal, const float *position, float *color);

#endif