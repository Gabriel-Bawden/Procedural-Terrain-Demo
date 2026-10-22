#ifndef CAMERA_H
#define CAMERA_H

#include <stddef.h>

#include "vec.h"

enum camera_type
{
	CAMERA_PERSPECTIVE,
	CAMERA_ORTHOGRAPHIC
};

struct camera
{
	size_t w;
	size_t h;
	float l;

	size_t w_max;
	size_t h_max;

	float fov;
	float y_fov;

	float near_plane;
	float far_plane;

	unsigned char type;

	unsigned char *pixels;
	float *depth_buffer;

	float translation[16];
	float rotation_x[16];
	float rotation_y[16];
	float rotation_z[16];

	float rotation[16];
	float rotation_inverse[16];
	float transformation[16];
	float transformation_inverse[16];

	float x;
	float y;
	float z;

	float theta_x;
	float theta_y;
	float theta_z;

	struct vec_plane planes[6];
};

void camera_create_frustum(struct camera *v);

void camera_set_fov(struct camera *v, float fov);

void camera_create(struct camera *v, size_t w, size_t h, float near_plane, float far_plane, float fov, unsigned char type);

void camera_destroy(void *ptr);

void camera_update(struct camera *v);

void camera_resize(struct camera *v, size_t w, size_t h);

void camera_set_x(struct camera *v, float x);

void camera_set_y(struct camera *v, float y);

void camera_set_z(struct camera *v, float z);

void camera_set_theta_x(struct camera *v, float theta_x);

void camera_set_theta_y(struct camera *v, float theta_y);

void camera_set_theta_z(struct camera *v, float theta_z);

void camera_key_x(struct camera *v, float s);

void camera_key_y(struct camera *v, float s);

void camera_key_z(struct camera *v, float s);

void camera_project(struct camera *v, const float *a, float *b);

void camera_deproject(struct camera *camera, const size_t *position_projected, float *position_world);

void camera_project_shadow(struct camera *v, const float *a, float *b);

void camera_ray_to_point(struct camera *camera, size_t x, size_t y, float *ray);

void camera_test(struct camera *v, float *light_dir, struct camera *cam);

#endif