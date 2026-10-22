#ifndef LIGHT_DIR_H
#define LIGHT_DIR_H

#include "mesh.h"
#include "camera.h"

struct light_dir
{
	float direction[3];
	float direction_transformed[3];

	float theta_x;
	float theta_y;
	float theta_z;

	float rotation_x[16];
	float rotation_y[16];
	float rotation_z[16];

	float rotation[16];

	float Ia[3];
	float I[3];

	struct camera cam;
};

void light_dir_create(struct light_dir *v, float dir_x, float dir_y, float dir_z, float Ia_r, float Ia_g, float Ia_b, float I_r, float I_g, float I_b);

void light_dir_update(struct light_dir *v, struct camera *cam, unsigned char shadow);

void light_dir_set_theta_x(struct light_dir *v, float theta_x);

void light_dir_set_theta_y(struct light_dir *v, float theta_y);

void light_dir_set_theta_z(struct light_dir *v, float theta_z);

void light_dir_nospec(struct light_dir *v, const struct mesh_material *mat, const float *normal, float *color, float shadow);

void light_dir_spec(struct light_dir *v, const struct mesh_material *mat, const float *normal, const float *position, float *color, float shadow);

#endif