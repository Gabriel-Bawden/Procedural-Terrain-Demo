#ifndef RENDER_H
#define RENDER_H

#include <stddef.h>

#include "dynarray.h"
#include "mesh.h"
#include "texture.h"
#include "model.h"
#include "camera.h"
#include "light_dir.h"
#include "light_point.h"

//Can cause texture issues if too far from 0, can cause holes to appear if too close to 0
#define ZERO -0.00001f

void render_DrawPixel(unsigned char *pixels, size_t w, size_t x, size_t y, unsigned char r, unsigned char g, unsigned char b);

void render_draw_pixel(unsigned char *pixels, size_t w, size_t x, size_t y, unsigned char r, unsigned char g, unsigned char b, float a);

struct triangle
{
	//Texture coordinates
	float tx[3];
	float ty[3];

	//Normals
	float nf[3];
	float n1[3];
	float n2[3];
	float n3[3];

	//Color Buffers
	float c[3];
	float color1[3];
	float color2[3];
	float color3[3];
	int t_ind[4];

	//Coordinates
	float p1[3];
	float p2[3];
	float p3[3];
};

struct vertex
{
	float v[3];

	float n[3];
	float c[3];
	float t[2];
};

struct triangle2
{
	//Normals
	float nf[3];

	//Color Buffers
	float c[3];
	unsigned char t_ind[4];

	struct vertex vertices[3];
};

struct shade
{
	int i;
	int j;
	float d1;
	float d2;
	float d3;
	float depth; //NOTE: Changing these values to double fixes the texture issues on placeholder with the terrain texture
};

void render_fragment_old(struct model *model, struct camera *cam, struct light_dir *light_dirs, struct light_point *light_points, unsigned char lighting, unsigned char shadow, struct triangle2 *t);

void render_fragment(struct model *model, struct camera *cam, struct light_dir *light_dirs, struct light_point *light_points, unsigned char lighting, unsigned char shadow, struct triangle2 *t);

void render_clip(struct model *model, struct camera *cam, struct light_dir *light_dirs, struct light_point *light_points, int cull, unsigned char lighting, unsigned char shadow, struct triangle2 *t, size_t i);

void render_draw(struct model *model, struct camera *cam, struct light_dir *light_dirs, struct light_point *light_points, int cull, unsigned char lighting, unsigned char shadow);

void render_fragment_shadow(struct camera *cam, struct triangle2 *t);

void render_clip_shadow(struct camera *cam, int cull, struct triangle2 *t, size_t i);

void render_draw_shadow(struct model *model, struct camera *cam, int cull);

void render_draw_text(struct mesh *mesh, struct camera *cam, struct texture *tex, int highlight, int x_off, int y_off, char *c);

#endif