#include "light_dir.h"

#include "math.h"
#include "vec.h"
#include "mesh.h"
#include "camera.h"

void light_dir_create(struct light_dir *v, float dir_x, float dir_y, float dir_z, float Ia_r, float Ia_g, float Ia_b, float I_r, float I_g, float I_b)
{
	v->direction[0] = dir_x;
	v->direction[1] = dir_y;
	v->direction[2] = dir_z;

	v->Ia[0] = Ia_r;
	v->Ia[1] = Ia_g;
	v->Ia[2] = Ia_b;

	v->I[0] = I_r;
	v->I[1] = I_g;
	v->I[2] = I_b;

	v->theta_y = 0;

	vec_normalize(v->direction, v->direction);
	vec_equate(v->direction, v->direction_transformed);

	for (int i = 0; i < 16; i++)
	{
		float val = 0;

		if (i % 5 == 0)
		{
			val = 1;
		}

		v->rotation_x[i] = val;
		v->rotation_y[i] = val;
		v->rotation_z[i] = val;

		v->rotation[i] = val;
	}

	camera_create(&v->cam, 1024, 1024, -110, 110, 60, CAMERA_ORTHOGRAPHIC);
}

void light_dir_update(struct light_dir *v, struct camera *cam, unsigned char shadow)
{
	float rotation[16];
	vec_mult_4x4(v->rotation_x, v->rotation_z, rotation);
	vec_mult_4x4(v->rotation_y, rotation, v->rotation);

	float rotation_combined[16];
	vec_mult_4x4(cam->rotation, v->rotation, rotation_combined);

	vec_mult_4x4_to_vec(rotation_combined, v->direction, v->direction_transformed);

	float direction_rotated[3];
	vec_mult_4x4_to_vec(v->rotation, v->direction, direction_rotated);

	if (shadow)
	{
		camera_test(&v->cam, direction_rotated, cam);
	}
}

void light_dir_set_theta_x(struct light_dir *v, float theta_x)
{
	v->theta_x = theta_x;

	v->rotation_x[5] = math_cos(v->theta_x);
	v->rotation_x[6] = math_sin(v->theta_x);
	v->rotation_x[9] = -math_sin(v->theta_x);
	v->rotation_x[10] = math_cos(v->theta_x);
}

void light_dir_set_theta_y(struct light_dir *v, float theta_y)
{
	v->theta_y = theta_y;

	v->rotation_y[0] = math_cos(v->theta_y);
	v->rotation_y[2] = -math_sin(v->theta_y);
	v->rotation_y[8] = math_sin(v->theta_y);
	v->rotation_y[10] = math_cos(v->theta_y);
}

void light_dir_set_theta_z(struct light_dir *v, float theta_z)
{
	v->theta_z = theta_z;

	v->rotation_z[0] = math_cos(v->theta_z);
	v->rotation_z[1] = math_sin(v->theta_z);
	v->rotation_z[4] = -math_sin(v->theta_z);
	v->rotation_z[5] = math_cos(v->theta_z);
}

void light_dir_nospec(struct light_dir *v, const struct mesh_material *mat, const float *normal, float *color, float shadow)
{
	float diff = math_max_float(0, -vec_dot(normal, v->direction_transformed));

	diff *= shadow;

	for (int i = 0; i < 3; i++)
	{
		color[i] = math_min_float(1, (mat->ka[i] * v->Ia[i] + mat->kd[i] * v->I[i] * diff));
	}
}

void light_dir_spec(struct light_dir *v, const struct mesh_material *mat, const float *normal, const float *position, float *color, float shadow)
{
	float position_normalized[3];
	vec_normalize(position, position_normalized);

	float an = vec_dot(normal, v->direction_transformed);

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
		vec_subtract(v->direction_transformed, reflect, reflect);

		diff = -an;
		spec = math_pow(math_max_float(0, -vec_dot(reflect, position_normalized)), mat->ns);
	}

	diff *= shadow;

	for (int i = 0; i < 3; i++)
	{
		color[i] = math_min_float(1, (mat->ka[i] * v->Ia[i] + mat->kd[i] * v->I[i] * diff + mat->ks[i] * v->I[i] * diff * spec));
	}
}