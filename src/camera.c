#include "camera.h"

#include <stddef.h>

#include "math.h"
#include "vec.h"
#include "alloc.h"
#include "os.h"

void camera_create_frustum(struct camera *v)
{
	float right_near;
	float right_far;

	float top_near;
	float top_far;

	if (v->type == CAMERA_PERSPECTIVE)
	{
		right_near = (v->w / 2.0f) * (v->near_plane / v->l);
		right_far = (v->far_plane * right_near) / v->near_plane;

		top_near = (v->h / 2.0f) * (v->near_plane / v->l);
		top_far = (v->far_plane * top_near) / v->near_plane;
	}
	else
	{
		//right_near = (v->w / 2.0f) / (v->fov);
		//right_far = right_near;

		//top_near = (v->h / 2.0f) / (v->fov);
		//top_far = top_near;

		right_near = v->fov / 2.0f;
		right_far = right_near;

		top_near = v->fov / 2.0f;
		top_far = top_near;
	}

	vec_plane_create(&v->planes[0], -right_near, -top_near, v->near_plane, right_near, -top_near, v->near_plane, -right_near, top_near, v->near_plane, right_near, top_near, v->near_plane);
	vec_plane_create(&v->planes[1], -right_far, -top_far, v->far_plane, -right_far, top_far, v->far_plane, right_far, -top_far, v->far_plane, right_far, top_far, v->far_plane);
	vec_plane_create(&v->planes[2], -right_near, -top_near, v->near_plane, -right_near, top_near, v->near_plane, -right_far, -top_far, v->far_plane, -right_far, top_far, v->far_plane);
	vec_plane_create(&v->planes[3], right_near, -top_near, v->near_plane, right_far, -top_far, v->far_plane, right_near, top_near, v->near_plane, right_far, top_far, v->far_plane);
	vec_plane_create(&v->planes[4], -right_near, top_near, v->near_plane, right_near, top_near, v->near_plane, -right_far, top_far, v->far_plane, right_far, top_far, v->far_plane);
	vec_plane_create(&v->planes[5], -right_near, -top_near, v->near_plane, -right_far, -top_far, v->far_plane, right_near, -top_near, v->near_plane, right_far, -top_far, v->far_plane);
}

void camera_set_fov(struct camera *v, float fov)
{
	v->fov = fov;

	v->l = v->w / (2 * math_tan((PI / 360) * v->fov));
	v->y_fov = (2 * math_atan(v->h / (2 * v->l))) * (180 / PI);

	camera_create_frustum(v);
}

void camera_create(struct camera *v, size_t w, size_t h, float near_plane, float far_plane, float fov, unsigned char type)
{
	v->w = w;
	v->h = h;
	v->w_max = w;
	v->h_max = h;

	v->near_plane = near_plane;
	v->far_plane = far_plane;

	v->type = type;

	v->x = 0;
	v->y = 0;
	v->z = 0;

	v->theta_x = 0;
	v->theta_y = 0;
	v->theta_z = 0;

	for (size_t i = 0; i < 16; i++)
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
		v->rotation_inverse[i] = val;
		v->transformation[i] = val;
		v->transformation_inverse[i] = val;
	}

	v->pixels = alloc_calloc(4 * v->w * v->h, (int)sizeof(*v->pixels));
	v->depth_buffer = alloc_calloc(v->w * v->h, sizeof(*v->depth_buffer));

	camera_set_fov(v, fov);
}

void camera_destroy(void *ptr)
{
	struct camera *v = ptr;

	alloc_free(v->pixels);
	alloc_free(v->depth_buffer);
}

void camera_update(struct camera *v)
{
	memset(v->pixels, 100, v->w * v->h * 4 * sizeof(*v->pixels));
	memset(v->depth_buffer, 0, v->w * v->h * sizeof(*v->depth_buffer));

	float rotation[16];
	vec_mult_4x4(v->rotation_x, v->rotation_y, rotation);
	vec_mult_4x4(v->rotation_z, rotation, v->rotation);
	vec_mult_4x4(v->rotation, v->translation, v->transformation);

	v->transformation_inverse[0] = v->rotation[0];
	v->transformation_inverse[1] = v->rotation[4];
	v->transformation_inverse[2] = v->rotation[8];
	v->transformation_inverse[3] = -v->translation[3];
	v->transformation_inverse[4] = v->rotation[1];
	v->transformation_inverse[5] = v->rotation[5];
	v->transformation_inverse[6] = v->rotation[9];
	v->transformation_inverse[7] = -v->translation[7];
	v->transformation_inverse[8] = v->rotation[2];
	v->transformation_inverse[9] = v->rotation[6];
	v->transformation_inverse[10] = v->rotation[10];
	v->transformation_inverse[11] = -v->translation[11];

	memcpy(v->rotation_inverse, v->transformation_inverse, sizeof(v->rotation_inverse));

	v->rotation_inverse[3] = 0;
	v->rotation_inverse[7] = 0;
	v->rotation_inverse[11] = 0;
}

void camera_resize(struct camera *v, size_t w, size_t h)
{
	v->w = w;
	v->h = h;

	if ((v->w * v->h) > (v->w_max * v->h_max))
	{
		v->w_max = v->w;
		v->h_max = v->h;

		v->pixels = alloc_realloc(v->pixels, v->w * v->h * 4 * sizeof(*v->pixels));
		v->depth_buffer = alloc_realloc(v->depth_buffer, v->w * v->h * sizeof(*v->depth_buffer));
	}

	camera_set_fov(v, v->fov);
}

void camera_set_x(struct camera *v, float x)
{
	v->x = x;

	v->translation[3] = -v->x;
}

void camera_set_y(struct camera *v, float y)
{
	v->y = y;

	v->translation[7] = -v->y;
}

void camera_set_z(struct camera *v, float z)
{
	v->z = z;

	v->translation[11] = -v->z;
}

void camera_set_theta_x(struct camera *v, float theta_x)
{
	v->theta_x = theta_x;

	v->rotation_x[5] = math_cos(-v->theta_x);
	v->rotation_x[6] = math_sin(-v->theta_x);
	v->rotation_x[9] = -math_sin(-v->theta_x);
	v->rotation_x[10] = math_cos(-v->theta_x);
}

void camera_set_theta_y(struct camera *v, float theta_y)
{
	v->theta_y = theta_y;

	v->rotation_y[0] = math_cos(-v->theta_y);
	v->rotation_y[2] = -math_sin(-v->theta_y);
	v->rotation_y[8] = math_sin(-v->theta_y);
	v->rotation_y[10] = math_cos(-v->theta_y);
}

void camera_set_theta_z(struct camera *v, float theta_z)
{
	v->theta_z = theta_z;

	v->rotation_z[0] = math_cos(-v->theta_z);
	v->rotation_z[1] = math_sin(-v->theta_z);
	v->rotation_z[4] = -math_sin(-v->theta_z);
	v->rotation_z[5] = math_cos(-v->theta_z);
}

void camera_key_x(struct camera *v, float s)
{
	float x = v->x + (s * math_cos(-v->theta_y));
	float z = v->z - (s * math_sin(-v->theta_y));

	camera_set_x(v, x);
	camera_set_z(v, z);
}

void camera_key_y(struct camera *v, float s)
{
	float y = v->y + s;

	camera_set_y(v, y);
}

void camera_key_z(struct camera *v, float s)
{
	float x = v->x + (s * math_sin(-v->theta_y));
	float z = v->z + (s * math_cos(-v->theta_y));

	camera_set_x(v, x);
	camera_set_z(v, z);
}

void camera_project(struct camera *v, const float *a, float *b)
{
	//Performing perspective divide and converting to screen space
	b[0] = ((v->l * (a[0] / a[2])) + v->w / 2.0f);
	b[1] = ((v->l * (a[1] / a[2])) + v->h / 2.0f);

	//Storing original vertex
	b[2] = a[0];
	b[3] = a[1];
	b[4] = a[2];
}

void camera_deproject(struct camera *camera, const size_t *position_screen, float *position_world)
{
	float z = 1 / camera->depth_buffer[position_screen[0] + camera->w * position_screen[1]];

	position_world[0] = (position_screen[0] - (camera->w / 2.0f)) * z / camera->l;
	position_world[1] = (position_screen[1] - (camera->h / 2.0f)) * z / camera->l;
}

void camera_project_shadow(struct camera *v, const float *a, float *b)
{
	//Orthographic projection
	//b[0] = (a[0] * v->fov + v->w / 2.0f);
	//b[1] = (a[1] * v->fov + v->h / 2.0f);
	b[0] = ((a[0] / v->fov) + 0.5f) * v->w - 1;
	b[1] = ((a[1] / v->fov) + 0.5f) * v->h - 1;

	//Storing original vertex
	b[2] = a[0];
	b[3] = a[1];
	b[4] = a[2];
}

void camera_ray_to_point(struct camera *camera, size_t x, size_t y, float *ray)
{
	ray[0] = (x - (camera->w / 2.0f)) / camera->l;
	ray[1] = (y - (camera->h / 2.0f)) / camera->l;
	ray[2] = 1;

	vec_normalize(ray, ray);

	float ray_inversed[3];
	vec_mult_4x4_to_vec(camera->rotation_inverse, ray, ray_inversed);

	ray[0] = ray_inversed[0];
	ray[1] = ray_inversed[1];
	ray[2] = ray_inversed[2];
}

void camera_test(struct camera *v, float *light_dir, struct camera *cam)
{
	float midpoint = (cam->near_plane + cam->far_plane) / 2.0f;

	float camera_direction[3] = {cam->rotation[8], cam->rotation[9], cam->rotation[10]};
	vec_scale(midpoint, camera_direction, camera_direction);

	float camera_location[3] = {cam->x, cam->y, cam->z};

	float frustum_center[3];
	vec_add(camera_location, camera_direction, frustum_center);

	float dist = midpoint + 30;

	camera_set_x(v, frustum_center[0] - light_dir[0] * dist);
	camera_set_y(v, frustum_center[1] - light_dir[1] * dist);
	camera_set_z(v, frustum_center[2] - light_dir[2] * dist);

	float theta_y = -math_atan2(light_dir[0], light_dir[2]);

	float horiz = math_sqrt(light_dir[0] * light_dir[0] + light_dir[2] * light_dir[2]);
	float theta_x = math_atan2(light_dir[1], horiz);

	camera_set_theta_x(v, theta_x);
	camera_set_theta_y(v, theta_y);

	camera_update(v);
}