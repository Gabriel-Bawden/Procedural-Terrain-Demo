#ifndef GRAPH_H
#define GRAPH_H

#include <stddef.h>

#include "mesh.h"

float graph_x_t(float t, float u);

float graph_y_t(float t, float u);

float graph_z_t(float t, float u);

float graph_z(float x, float y);

void graph_merge(struct mesh *v);

void graph_remove(struct mesh *v);

void graph_generate_normals(struct mesh *v);

void graph_test(struct mesh *v);

void graph_wave(struct mesh *v, int frames, float speed, float scale, unsigned char animation);

void graph_create(struct mesh *v, const float *alt, float sea, size_t w, size_t h);

void graph_create_2(struct mesh *v, float x1, float x2, float y1, float y2, float x_del, float y_del, int dir, int cull, char *mod, unsigned char normals);

void graph_create_square(struct mesh *v, float x1, float x2, float y1, float y2, float x_del, float y_del, int dir, char *mod);

#endif