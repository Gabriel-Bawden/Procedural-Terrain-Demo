#include "model.h"

#include "math.h"
#include "vec.h"
#include "mesh.h"
#include "texture.h"

void model_create(struct model *v, struct mesh *mesh, struct texture *texture)
{
	v->mesh = mesh;

	v->x = 0;
	v->y = 0;
	v->z = 0;

	v->theta_x = 0;
	v->theta_y = 0;
	v->theta_z = 0;

	v->scale = 1;

	v->base_material.start_index = 0;
	v->base_material.ka[0] = 0.8f;
	v->base_material.ka[1] = 0.8f;
	v->base_material.ka[2] = 0.8f;
	v->base_material.kd[0] = 0.8f;
	v->base_material.kd[1] = 0.8f;
	v->base_material.kd[2] = 0.8f;
	v->base_material.ks[0] = 0.2f;
	v->base_material.ks[1] = 0.2f;
	v->base_material.ks[2] = 0.2f;
	v->base_material.illum = 0;
	v->base_material.ns = 20;
	v->base_material.map_kd = texture;
	v->base_material.map_bump = NULL;

	for (int i = 0; i < 16; i++)
	{
		float val = 0;

		if (i % 5 == 0)
		{
			val = 1;
		}

		v->translation[i] = val;
		v->rotation_x[i] = val;
		v->rotation_y[i] = val;
		v->rotation_z[i] = val;

		v->rotation[i] = val;
		v->transformation[i] = val;
	}
}

void model_change_color(struct model *v, unsigned char r, unsigned char g, unsigned char b)
{
	v->base_material.ka[0] = r / 255.0f;
	v->base_material.ka[1] = g / 255.0f;
	v->base_material.ka[2] = b / 255.0f;
	v->base_material.kd[0] = r / 255.0f;
	v->base_material.kd[1] = g / 255.0f;
	v->base_material.kd[2] = b / 255.0f;
}

void model_update(struct model *v)
{
	float rotation[16];
	vec_mult_4x4(v->rotation_x, v->rotation_z, rotation);
	vec_mult_4x4(v->rotation_y, rotation, v->rotation);
	vec_mult_4x4(v->translation, v->rotation, v->transformation);
}

void model_set_x(struct model *v, float x)
{
	v->x = x;

	v->translation[3] = v->x;
}

void model_set_y(struct model *v, float y)
{
	v->y = y;

	v->translation[7] = v->y;
}

void model_set_z(struct model *v, float z)
{
	v->z = z;

	v->translation[11] = v->z;
}

void model_set_theta_x(struct model *v, float theta_x)
{
	v->theta_x = theta_x;

	v->rotation_x[5] = math_cos(v->theta_x);
	v->rotation_x[6] = math_sin(v->theta_x);
	v->rotation_x[9] = -math_sin(v->theta_x);
	v->rotation_x[10] = math_cos(v->theta_x);
}

void model_set_theta_y(struct model *v, float theta_y)
{
	v->theta_y = theta_y;

	v->rotation_y[0] = math_cos(v->theta_y);
	v->rotation_y[2] = -math_sin(v->theta_y);
	v->rotation_y[8] = math_sin(v->theta_y);
	v->rotation_y[10] = math_cos(v->theta_y);
}

void model_set_theta_z(struct model *v, float theta_z)
{
	v->theta_z = theta_z;

	v->rotation_z[0] = math_cos(v->theta_z);
	v->rotation_z[1] = math_sin(v->theta_z);
	v->rotation_z[4] = -math_sin(v->theta_z);
	v->rotation_z[5] = math_cos(v->theta_z);
}

void model_set_scale(struct model *v, float scale)
{
	v->scale = scale;

	v->translation[0] = v->scale;
	v->translation[5] = v->scale;
	v->translation[10] = v->scale;
}