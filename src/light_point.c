#include "light_point.h"

#include "math.h"
#include "vec.h"
#include "mesh.h"
#include "model.h"

void light_point_create(struct light_point *v, float Ia_r, float Ia_g, float Ia_b, float I_r, float I_g, float I_b, float q1, float q2, float q3, struct mesh *mod)
{
	v->x = 0;
	v->y = 0;
	v->z = 0;

	v->position[0] = v->x;
	v->position[1] = v->y;
	v->position[2] = v->z;

	v->position_transformed[0] = v->x;
	v->position_transformed[1] = v->y;
	v->position_transformed[2] = v->z;

	v->Ia[0] = Ia_r;
	v->Ia[1] = Ia_g;
	v->Ia[2] = Ia_b;

	v->I[0] = I_r;
	v->I[1] = I_g;
	v->I[2] = I_b;

	v->quadratic[0] = q1;
	v->quadratic[1] = q2;
	v->quadratic[2] = q3;

	model_create(&v->light_model, mod, NULL);
	model_set_scale(&v->light_model, 0.25f);
	v->light_model.base_material.ka[0] = 0.8f;
	v->light_model.base_material.ka[1] = 0.8f;
	v->light_model.base_material.ka[2] = 0.0f;
	v->light_model.base_material.kd[0] = 0.8f;
	v->light_model.base_material.kd[1] = 0.8f;
	v->light_model.base_material.kd[2] = 0.0f;
}

void light_point_update(struct light_point *v, const float *transformation)
{
	vec_mult_4x4_to_vec(transformation, v->position, v->position_transformed);

	model_update(&v->light_model);
}

void light_point_set_x(struct light_point *v, float x)
{
	v->x = x;

	v->position[0] = x;

	model_set_x(&v->light_model, v->x);
}

void light_point_set_y(struct light_point *v, float y)
{
	v->y = y;

	v->position[1] = y;

	model_set_y(&v->light_model, v->y);
}

void light_point_set_z(struct light_point *v, float z)
{
	v->z = z;

	v->position[2] = z;

	model_set_z(&v->light_model, v->z);
}

void light_point_nospec(struct light_point *v, const struct mesh_material *mat, const float *normal, const float *position, float *color)
{
	float position_normalized[3];
	vec_normalize(position, position_normalized);

	float dir[3];
	vec_subtract(position, v->position_transformed, dir);

	float d1 = vec_normalize(dir, dir);
	float d = 1 / ((v->quadratic[0] * d1 + v->quadratic[1]) * d1 + v->quadratic[2]);

	float diff = math_max_float(0, -vec_dot(normal, dir));

	diff *= d;

	for (int i = 0; i < 3; i++)
	{
		color[i] = math_min_float(1, (mat->ka[i] * v->Ia[i] + mat->kd[i] * v->I[i] * diff));
	}
}

void light_point_spec(struct light_point *v, const struct mesh_material *mat, const float *normal, const float *position, float *color)
{
	float position_normalized[3];
	vec_normalize(position, position_normalized);

	float dir[3];
	vec_subtract(position, v->position_transformed, dir);

	float d1 = vec_normalize(dir, dir);
	float d = 1 / ((v->quadratic[0] * d1 + v->quadratic[1]) * d1 + v->quadratic[2]);

	float an = vec_dot(normal, dir);

	float diff;
	float spec;

	if (an > 0)
	{
		diff = 0;
		spec = 0;
	}
	else
	{
		float reflect[3];
		vec_scale(2 * an, normal, reflect);
		vec_subtract(dir, reflect, reflect);

		diff = -an;
		spec = math_pow(math_max_float(0, -vec_dot(reflect, position_normalized)), mat->ns);
	}

	diff *= d;

	for (int i = 0; i < 3; i++)
	{
		color[i] = math_min_float(1, (mat->ka[i] * v->Ia[i] + mat->kd[i] * v->I[i] * diff + mat->ks[i] * v->I[i] * diff * spec));
	}
}