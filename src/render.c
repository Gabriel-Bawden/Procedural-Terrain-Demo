#include "render.h"

#include <stddef.h>

#include "math.h"
#include "vec.h"
#include "str.h"
#include "os.h"
#include "dynarray.h"
#include "mesh.h"
#include "texture.h"
#include "model.h"
#include "camera.h"
#include "light_dir.h"
#include "light_point.h"

void render_DrawPixel(unsigned char *pixels, size_t w, size_t x, size_t y, unsigned char r, unsigned char g, unsigned char b)
{
	const size_t offset = 4 * (x + w * y);
	//const size_t offset = 4 * ((x + w / 2) + w * (y + h / 2));

	pixels[offset + 0] = (unsigned char)b;
	pixels[offset + 1] = (unsigned char)g;
	pixels[offset + 2] = (unsigned char)r;
}

void render_draw_pixel(unsigned char *pixels, size_t w, size_t x, size_t y, unsigned char r, unsigned char g, unsigned char b, float a)
{
	const size_t offset = 4 * (x + w * y);

	int b1 = pixels[offset + 0];
	int g1 = pixels[offset + 1];
	int r1 = pixels[offset + 2];

	pixels[offset + 0] = (unsigned char)(b * a + b1 * (1 - a));
	pixels[offset + 1] = (unsigned char)(g * a + g1 * (1 - a));
	pixels[offset + 2] = (unsigned char)(r * a + r1 * (1 - a));
}

static struct mesh_material mat;

void render_fragment_old(struct model *model, struct camera *cam, struct light_dir *light_dirs, struct light_point *light_points, unsigned char lighting, unsigned char shadow, struct triangle2 *t)
{
	unsigned char has_texture = (mat.map_kd && model->mesh->texture_coordinates);

	float v1[5] = {0};
	float v2[5] = {0};
	float v3[5] = {0};

	camera_project(cam, t->vertices[0].v, v1);
	camera_project(cam, t->vertices[1].v, v2);
	camera_project(cam, t->vertices[2].v, v3);

	float z1 = 1 / v1[4];
	float z2 = 1 / v2[4];
	float z3 = 1 / v3[4];

	int xMax = math_min_int((int)cam->w - 1, (int)math_max_float(v1[0], math_max_float(v2[0], v3[0])));
	int xMin = math_max_int(0, (int)math_min_float(v1[0], math_min_float(v2[0], v3[0])));
	int yMax = math_min_int((int)cam->h - 1, (int)math_max_float(v1[1], math_max_float(v2[1], v3[1])));
	int yMin = math_max_int(0, (int)math_min_float(v1[1], math_min_float(v2[1], v3[1])));

	float a0 = (v3[0] - v1[0]) * (v2[1] - v1[1]) + (v1[0] - v2[0]) * (v3[1] - v1[1]);

	float k11 = (v2[0] - v3[0]) / a0;
	float k12 = (v2[1] - v3[1]) / a0;
	float k21 = (v3[0] - v1[0]) / a0;
	float k22 = (v1[1] - v3[1]) / a0;

	v3[0] -= 0.5f; //same effect as: float x = x + 0.5f;
	v3[1] -= 0.5f; //same effect as: float y = y + 0.5f;

	float tbn[9] = {0};

	if (mat.map_bump)
	{
		float edge1[3];
		float edge2[3];

		vec_subtract(t->vertices[1].v, t->vertices[0].v, edge1);
		vec_subtract(t->vertices[2].v, t->vertices[0].v, edge2);

		float uv1[2] = {0};
		float uv2[2] = {0};

		uv1[0] = t->vertices[1].t[0] - t->vertices[0].t[0];
		uv1[1] = t->vertices[1].t[1] - t->vertices[0].t[1];

		uv2[0] = t->vertices[2].t[0] - t->vertices[0].t[0];
		uv2[1] = t->vertices[2].t[1] - t->vertices[0].t[1];

		//

		float f = 1.0f / (uv1[0] * uv2[1] - uv2[0] * uv1[1]);

		float tangent[3] = {0};
		float bitangent[3] = {0};

		tangent[0] = f * (uv2[1] * edge1[0] - uv1[1] * edge2[0]);
		tangent[1] = f * (uv2[1] * edge1[1] - uv1[1] * edge2[1]);
		tangent[2] = f * (uv2[1] * edge1[2] - uv1[1] * edge2[2]);

		//bitangent[0] = f * (-uv2[0] * edge1[0] + uv1[0] * edge2[0]);
		//bitangent[1] = f * (-uv2[0] * edge1[1] + uv1[0] * edge2[1]);
		//bitangent[2] = f * (-uv2[0] * edge1[2] + uv1[0] * edge2[2]);

		//
		float dotTN = vec_dot(tangent, t->nf);

		tangent[0] -= t->nf[0] * dotTN;
		tangent[1] -= t->nf[1] * dotTN;
		tangent[2] -= t->nf[2] * dotTN;

		vec_normalize(tangent, tangent);

		//vec_cross(t->nf, tangent, bitangent);
		vec_cross(tangent, t->nf, bitangent);
		//

		//vec_normalize(tangent, tangent);
		//vec_normalize(bitangent, bitangent);

		//Column 0
		tbn[0] = tangent[0];
		tbn[3] = tangent[1];
		tbn[6] = tangent[2];

		//Column 1
		tbn[1] = bitangent[0];
		tbn[4] = bitangent[1];
		tbn[7] = bitangent[2];

		//Column 2
		tbn[2] = t->nf[0];
		tbn[5] = t->nf[1];
		tbn[8] = t->nf[2];
	}

	for (int x = xMin; x <= xMax; x++)
	{
		float kx = (x - v3[0]);

		for (int y = yMin; y <= yMax; y++)
		{
			float ky = (y - v3[1]);

			const float a1 = ky * k11 - kx * k12;
			const float a2 = ky * k21 + kx * k22;
			const float a3 = 1 - a1 - a2;

			//w/2 and h/2 cancel out here and do not affect final result, only changing x and y pixels values
			//float a0 = (v3[0] - v1[0]) * (v2[1] - v1[1]) + (v1[0] - v2[0]) * (v3[1] - v1[1]);
			//const float a1 = ((v2[0] - v3[0]) * (y + 0.5f - v3[1]) - (x + 0.5f - v3[0]) * (v2[1] - v3[1])) / a01;
			//const float a2 = ((v3[0] - v1[0]) * (y + 0.5f - v3[1]) + (x + 0.5f - v3[0]) * (v1[1] - v3[1])) / a01;
			//const float a3 = 1 - a1 - a2;

			if (a1 >= ZERO && a2 >= ZERO && a3 >= ZERO)
			{
				struct shade s = {x, y, a1 * z1, a2 * z2, a3 * z3, s.d1 + s.d2 + s.d3};

				if (s.depth > cam->depth_buffer[x + cam->w * y])
				{
					float tu = 0;
					float tv = 0;

					if (has_texture || mat.map_bump)
					{
						tu = (s.d1 * t->vertices[0].t[0] + s.d2 * t->vertices[1].t[0] + s.d3 * t->vertices[2].t[0]) / s.depth;
						tv = (s.d1 * t->vertices[0].t[1] + s.d2 * t->vertices[1].t[1] + s.d3 * t->vertices[2].t[1]) / s.depth;
					}

					if (has_texture && !lighting)
					{
						unsigned char *tex = mat.map_kd->pixels;

						size_t usize = mat.map_kd->w;
						size_t vsize = mat.map_kd->h;

						int tx = math_floor(usize * tu);
						int ty = math_floor(vsize * tv);

						//if (tu > usize - 1) tu = tu % usize;
						//if (tv > vsize - 1) tv = tv % vsize;
						tx = math_mod(tx, (int)usize);
						ty = math_mod(ty, (int)vsize);

						size_t tex_ind = 4 * ((size_t)tx + usize * (size_t)ty);

						t->t_ind[0] = tex[tex_ind + 0];
						t->t_ind[1] = tex[tex_ind + 1];
						t->t_ind[2] = tex[tex_ind + 2];
						t->t_ind[3] = tex[tex_ind + 3];
					}
					if (lighting)
					{
						float n[3] = {t->nf[0], t->nf[1], t->nf[2]};

						float depth_real = 1 / s.depth;
						float zk = depth_real / cam->l;
						float pos[3] = {zk * ((x + 0.5f) - (cam->w / 2)), zk * ((y + 0.5f) - (cam->h / 2)), depth_real};

						if (model->mesh->normals)
						{
							n[0] = (s.d1 * t->vertices[0].n[0] + s.d2 * t->vertices[1].n[0] + s.d3 * t->vertices[2].n[0]) / s.depth;
							n[1] = (s.d1 * t->vertices[0].n[1] + s.d2 * t->vertices[1].n[1] + s.d3 * t->vertices[2].n[1]) / s.depth;
							n[2] = (s.d1 * t->vertices[0].n[2] + s.d2 * t->vertices[1].n[2] + s.d3 * t->vertices[2].n[2]) / s.depth;

							vec_normalize(n, n);
						}

						if (mat.map_bump)
						{
							unsigned char *tex = mat.map_bump->pixels;

							size_t usize = mat.map_bump->w;
							size_t vsize = mat.map_bump->h;

							int tx = math_floor(usize * tu);
							int ty = math_floor(vsize * tv);

							//if (tu > usize - 1) tu = tu % usize;
							//if (tv > vsize - 1) tv = tv % vsize;
							tx = math_mod(tx, (int)usize);
							ty = math_mod(ty, (int)vsize);

							size_t tex_ind = 4 * ((size_t)tx + usize * (size_t)ty);

							float n1[3] = {0};
							n1[0] = (tex[tex_ind + 2] - 128) / 128.0f;
							n1[1] = (tex[tex_ind + 1] - 128) / 128.0f;
							n1[2] = (tex[tex_ind + 0] - 128) / 128.0f;

							vec_normalize(n1, n1);

							n[0] = tbn[0] * n1[0] + tbn[1] * n1[1] + tbn[2] * n1[2];
							n[1] = tbn[3] * n1[0] + tbn[4] * n1[1] + tbn[5] * n1[2];
							n[2] = tbn[6] * n1[0] + tbn[7] * n1[1] + tbn[8] * n1[2];

							/*
							n[0] = 0 * n1[0] + -1 * n1[1] + 0 *  n1[2];
							n[1] = 1 * n1[0] + 0 *  n1[1] + 0 *  n1[2];
							n[2] = 0 * n1[0] + 0 *  n1[1] + -1 * n1[2];

							n[0] = 1 * n1[0] + 0 * n1[1] + 0 *  n1[2];
							n[1] = 0 * n1[0] + 1 *  n1[1] + 0 *  n1[2];
							n[2] = 0 * n1[0] + 0 *  n1[1] + -1 * n1[2];

							//Tangent -> [0, 1, 0], [1, 0, 0]
							tbn[0] * n1[0]
							tbn[3] * n1[0]
							tbn[6] * n1[0]

							//Bitangent -> [-1, 0, 0], [0, 1, 0]
							tbn[1] * n1[1]
							tbn[4] * n1[1]
							tbn[7] * n1[1]

							//Normal -> [0, 0, -1], [0, 0, -1]
							tbn[2] * n1[2]
							tbn[5] * n1[2]
							tbn[8] * n1[2]
							*/
						}

						//SHADOW
						
						float shadow_lighting = 1.0f;

						if (shadow)
						{
							float p[3];
							vec_mult_4x4_to_vec(cam->transformation_inverse, pos, p);

							float p2[3];
							vec_mult_4x4_to_vec(light_dirs[0].cam.transformation, p, p2);

							int w_shadow = (int)light_dirs[0].cam.w;
							int h_shadow = (int)light_dirs[0].cam.h;

							float p3[5];
							camera_project_shadow(&light_dirs[0].cam, p2, p3);

							int shadow_x = math_floor(p3[0]);
							int shadow_y = math_floor(p3[1]);
							float shadow_z = p3[4];

							if (shadow_z != 0)
							{
								shadow_z = 1 / shadow_z;
							}
							else
							{
								shadow_z = 100000;
							}

							if ((shadow_x >= 0 && shadow_x < w_shadow) && (shadow_y >= 0 && shadow_y < h_shadow))
							{
								int shadow_index = shadow_x + w_shadow * shadow_y;

								float shadow_depth = light_dirs[0].cam.depth_buffer[shadow_index];

								float bias = 0.001f;
								//float bias = math_max_float(0.003 * (1.0 - vec_dot(n, light_dirs[0].direction_transformed)), 0.0001);

								if (shadow_z < (shadow_depth - bias))
								{
									shadow_lighting = 0.1f;
								}
							}
						}
						
						//

						if (lighting == 2)
						{
							light_point_spec(&light_points[0], &mat, n, pos, t->c);
						}
						else
						{
							light_dir_spec(&light_dirs[0], &mat, n, pos, t->c, shadow_lighting);
						}
					}
					else
					{
						for (size_t i = 0; i < 3; i++)
						{
							t->c[i] = mat.kd[i];
						}
					}

					if (t->t_ind[3] != 0)
					{
						cam->depth_buffer[s.i + cam->w * s.j] = s.depth;
						render_DrawPixel(cam->pixels, cam->w, s.i, s.j, (unsigned char)(t->t_ind[2] * t->c[0]), (unsigned char)(t->t_ind[1] * t->c[1]), (unsigned char)(t->t_ind[0] * t->c[2]));
					
						//const size_t offset = 4 * (s.i + cam->w * s.j);

						//cam->pixels[offset + 0] = (unsigned char)(t->t_ind[0] * t->c[2]);
						//cam->pixels[offset + 1] = (unsigned char)(t->t_ind[1] * t->c[1]);
						//cam->pixels[offset + 2] = (unsigned char)(t->t_ind[2] * t->c[0]);
					}
				}
			}
		}
	}
}

void render_fragment(struct model *model, struct camera *cam, struct light_dir *light_dirs, struct light_point *light_points, unsigned char lighting, unsigned char shadow, struct triangle2 *t)
{
	//Tiling: Can reject if all four corners fail all three checks.
	//		  Can accept whole tile and avoid checks in loop if all four corners pass.

	unsigned char has_texture = (mat.map_kd && model->mesh->texture_coordinates);

	unsigned char c[3] = {0};
	float color_lighting[3] = {0};

	if (lighting)
	{
		light_dir_nospec(&light_dirs[0], &mat, t->nf, color_lighting, 1);

		if (!has_texture)
		{
			for (size_t i = 0; i < 3; i++)
			{
				c[i] = (unsigned char)(255 * color_lighting[i]);
			}
		}
	}
	else
	{
		for (size_t i = 0; i < 3; i++)
		{
			c[i] = (unsigned char)(255 * mat.kd[i]);
		}
	}

	float v1[5] = {0};
	float v2[5] = {0};
	float v3[5] = {0};

	camera_project(cam, t->vertices[0].v, v1);
	camera_project(cam, t->vertices[1].v, v2);
	camera_project(cam, t->vertices[2].v, v3);

	const float z1 = 1 / v1[4];
	const float z2 = 1 / v2[4];
	const float z3 = 1 / v3[4];

	const int xMax = math_min_int((int)cam->w - 1, (int)math_max_float(v1[0], math_max_float(v2[0], v3[0])));
	const int xMin = math_max_int(0, (int)math_min_float(v1[0], math_min_float(v2[0], v3[0])));
	const int yMax = math_min_int((int)cam->h - 1, (int)math_max_float(v1[1], math_max_float(v2[1], v3[1])));
	const int yMin = math_max_int(0, (int)math_min_float(v1[1], math_min_float(v2[1], v3[1])));

	const float a0 = (v3[0] - v1[0]) * (v2[1] - v1[1]) + (v1[0] - v2[0]) * (v3[1] - v1[1]);

	const float a1_dx = (v3[1] - v2[1]) / a0;
	const float a1_dy = (v2[0] - v3[0]) / a0;
	const float a2_dx = (v1[1] - v3[1]) / a0;
	const float a2_dy = (v3[0] - v1[0]) / a0;

	//Adding 0.5f to center pixel
	float a1 = ((yMin + 0.5f) - v3[1]) * a1_dy + ((xMin + 0.5f) - v3[0]) * a1_dx;
	float a2 = ((yMin + 0.5f) - v3[1]) * a2_dy + ((xMin + 0.5f) - v3[0]) * a2_dx;
	float a1_row = a1;
	float a2_row = a2;

	float depth = a1 * (z1 - z3) + a2 * (z2 - z3) + z3;
	const float depth_dx = (z1 - z3) * a1_dx + (z2 - z3) * a2_dx;
	const float depth_dy = (z1 - z3) * a1_dy + (z2 - z3) * a2_dy;
	float depth_row = depth;

	unsigned char *pixels = cam->pixels + 4 * (xMin + cam->w * yMin);
	float *depth_buffer = cam->depth_buffer + (xMin + cam->w * yMin);

	//Unsigned is sometimes slower than signed
	const size_t pixels_row_increment = 4 * (cam->w - (size_t)(xMax - xMin) - 1);
	const size_t depth_buffer_row_increment = cam->w - (size_t)(xMax - xMin) - 1;

	if (!has_texture)
	{
		for (int y = yMin; y <= yMax; y++)
		{
			a1 = a1_row;
			a2 = a2_row;

			depth = depth_row;

			for (int x = xMin; x <= xMax; x++)
			{
				const float a3 = 1 - a1 - a2;

				if (a1 >= ZERO && a2 >= ZERO && a3 >= ZERO)
				{
					if (depth > *depth_buffer)
					{
						pixels[0] = (unsigned char)(c[2]);
						pixels[1] = (unsigned char)(c[1]);
						pixels[2] = (unsigned char)(c[0]);

						*depth_buffer = depth;
					}
				}

				a1 += a1_dx;
				a2 += a2_dx;

				depth += depth_dx;

				pixels += 4;
				depth_buffer++;
			}

			a1_row += a1_dy;
			a2_row += a2_dy;

			depth_row += depth_dy;

			pixels += pixels_row_increment;
			depth_buffer += depth_buffer_row_increment;
		}
	}
	else
	{
		unsigned char *tex = mat.map_kd->pixels;

		size_t usize = mat.map_kd->w;
		size_t vsize = mat.map_kd->h;

		float tex1[2] = {usize * t->vertices[0].t[0] * z1, vsize * t->vertices[0].t[1] * z1};
		float tex2[2] = {usize * t->vertices[1].t[0] * z2, vsize * t->vertices[1].t[1] * z2};
		float tex3[2] = {usize * t->vertices[2].t[0] * z3, vsize * t->vertices[2].t[1] * z3};

		float tu = a1 * (tex1[0] - tex3[0]) + a2 * (tex2[0] - tex3[0]) + tex3[0];
		const float tu_dx = (tex1[0] - tex3[0]) * a1_dx + (tex2[0] - tex3[0]) * a2_dx;
		const float tu_dy = (tex1[0] - tex3[0]) * a1_dy + (tex2[0] - tex3[0]) * a2_dy;
		float tu_row = tu;

		float tv = a1 * (tex1[1] - tex3[1]) + a2 * (tex2[1] - tex3[1]) + tex3[1];
		const float tv_dx = (tex1[1] - tex3[1]) * a1_dx + (tex2[1] - tex3[1]) * a2_dx;
		const float tv_dy = (tex1[1] - tex3[1]) * a1_dy + (tex2[1] - tex3[1]) * a2_dy;
		float tv_row = tv;

		if (!lighting)
		{
			for (int y = yMin; y <= yMax; y++)
			{
				a1 = a1_row;
				a2 = a2_row;

				depth = depth_row;

				tu = tu_row;
				tv = tv_row;

				for (int x = xMin; x <= xMax; x++)
				{
					const float a3 = 1 - a1 - a2;

					if (a1 >= ZERO && a2 >= ZERO && a3 >= ZERO)
					{
						if (depth > *depth_buffer)
						{
							//float depth_inv = 1 / depth;

							//int tx_scaled = math_mod((int)(tu * depth_inv), (int)usize);
							//int ty_scaled = math_mod((int)(tv * depth_inv), (int)vsize);

							int tx_scaled = math_mod((int)(tu / depth), (int)usize);
							int ty_scaled = math_mod((int)(tv / depth), (int)vsize);

							size_t tex_ind = 4 * ((size_t)tx_scaled + usize * (size_t)ty_scaled);

							if (tex[tex_ind + 3] != 0)
							{
								pixels[0] = tex[tex_ind + 0];
								pixels[1] = tex[tex_ind + 1];
								pixels[2] = tex[tex_ind + 2];

								*depth_buffer = depth;
							}
						}
					}

					a1 += a1_dx;
					a2 += a2_dx;

					depth += depth_dx;

					tu += tu_dx;
					tv += tv_dx;

					pixels += 4;
					depth_buffer++;
				}

				a1_row += a1_dy;
				a2_row += a2_dy;

				depth_row += depth_dy;

				tu_row += tu_dy;
				tv_row += tv_dy;

				pixels += pixels_row_increment;
				depth_buffer += depth_buffer_row_increment;
			}
		}
		else
		{
			for (int y = yMin; y <= yMax; y++)
			{
				a1 = a1_row;
				a2 = a2_row;

				depth = depth_row;

				tu = tu_row;
				tv = tv_row;

				for (int x = xMin; x <= xMax; x++)
				{
					const float a3 = 1 - a1 - a2;

					if (a1 >= ZERO && a2 >= ZERO && a3 >= ZERO)
					{
						if (depth > *depth_buffer)
						{
							//float depth_inv = 1 / depth;

							//int tx_scaled = math_mod((int)(tu * depth_inv), (int)usize);
							//int ty_scaled = math_mod((int)(tv * depth_inv), (int)vsize);

							int tx_scaled = math_mod((int)(tu / depth), (int)usize);
							int ty_scaled = math_mod((int)(tv / depth), (int)vsize);

							size_t tex_ind = 4 * ((size_t)tx_scaled + usize * (size_t)ty_scaled);

							if (tex[tex_ind + 3] != 0)
							{
								pixels[0] = (unsigned char)(tex[tex_ind + 0] * color_lighting[0]);
								pixels[1] = (unsigned char)(tex[tex_ind + 1] * color_lighting[1]);
								pixels[2] = (unsigned char)(tex[tex_ind + 2] * color_lighting[2]);

								*depth_buffer = depth;
							}
						}
					}

					a1 += a1_dx;
					a2 += a2_dx;

					depth += depth_dx;

					tu += tu_dx;
					tv += tv_dx;

					pixels += 4;
					depth_buffer++;
				}

				a1_row += a1_dy;
				a2_row += a2_dy;

				depth_row += depth_dy;

				tu_row += tu_dy;
				tv_row += tv_dy;

				pixels += pixels_row_increment;
				depth_buffer += depth_buffer_row_increment;
			}
		}
	}
}

void render_clip(struct model *model, struct camera *cam, struct light_dir *light_dirs, struct light_point *light_points, int cull, unsigned char lighting, unsigned char shadow, struct triangle2 *t, size_t i)
{
	if (i == 6)
	{
		render_fragment(model, cam, light_dirs, light_points, lighting, shadow, t);

		return;
	}

	struct vec_plane *plane = &cam->planes[i];

	size_t p_inside[3] = {0};
	size_t p_outside[3] = {0};

	size_t inside_count = 0;

	for (size_t j = 0; j < 3; j++)
	{
		float len = vec_dot(t->vertices[j].v, plane->n) + plane->d;

		if (len >= 0)
		{
			p_inside[inside_count] = j;

			inside_count++;
		}
		else
		{
			p_outside[j - inside_count] = j;
		}
	}

	unsigned char has_texture = (mat.map_kd && model->mesh->texture_coordinates);

	if (inside_count == 0)
	{
		return;
	}
	else if (inside_count == 1)
	{
		float diff[3];
		vec_subtract(plane->p1, t->vertices[p_inside[0]].v, diff);

		//First
		float r1[3];
		vec_subtract(t->vertices[p_inside[0]].v, t->vertices[p_outside[0]].v, r1);

		float t1 = vec_dot(diff, plane->n) / vec_dot(r1, plane->n);

		vec_scale(t1, r1, r1);

		vec_add(t->vertices[p_inside[0]].v, r1, t->vertices[p_outside[0]].v);

		if (has_texture || mat.map_bump)
		{
			for (size_t j = 0; j < 2; j++)
			{
				t->vertices[p_outside[0]].t[j] = t->vertices[p_inside[0]].t[j] + t1 * (t->vertices[p_inside[0]].t[j] - t->vertices[p_outside[0]].t[j]);
			}
		}

		if (model->mesh->normals)
		{
			for (size_t j = 0; j < 3; j++)
			{
				t->vertices[p_outside[0]].n[j] = t->vertices[p_inside[0]].n[j] + t1 * (t->vertices[p_inside[0]].n[j] - t->vertices[p_outside[0]].n[j]);
			}
		}

		//Second
		float r2[3];
		vec_subtract(t->vertices[p_inside[0]].v, t->vertices[p_outside[1]].v, r2);

		float t2 = vec_dot(diff, plane->n) / vec_dot(r2, plane->n);

		vec_scale(t2, r2, r2);

		vec_add(t->vertices[p_inside[0]].v, r2, t->vertices[p_outside[1]].v);

		if (has_texture || mat.map_bump)
		{
			for (size_t j = 0; j < 2; j++)
			{
				t->vertices[p_outside[1]].t[j] = t->vertices[p_inside[0]].t[j] + t2 * (t->vertices[p_inside[0]].t[j] - t->vertices[p_outside[1]].t[j]);
			}
		}

		if (model->mesh->normals)
		{
			for (size_t j = 0; j < 3; j++)
			{
				t->vertices[p_outside[1]].n[j] = t->vertices[p_inside[0]].n[j] + t2 * (t->vertices[p_inside[0]].n[j] - t->vertices[p_outside[1]].n[j]);
			}
		}

		//Clip new triangle
		render_clip(model, cam, light_dirs, light_points, cull, lighting, shadow, t, i + 1);
	}
	else if (inside_count == 2)
	{
		float diff[3];
		vec_subtract(plane->p1, t->vertices[p_outside[0]].v, diff);

		//First
		float r1[3];
		vec_subtract(t->vertices[p_outside[0]].v, t->vertices[p_inside[0]].v, r1);

		float t1 = vec_dot(diff, plane->n) / vec_dot(r1, plane->n);

		vec_scale(t1, r1, r1);

		float p1[3];
		vec_add(t->vertices[p_outside[0]].v, r1, p1);

		struct triangle2 tri1;
		memcpy(&tri1, t, sizeof(tri1));

		memcpy(&tri1.vertices[p_outside[0]].v, &p1, sizeof(tri1.vertices[p_outside[0]].v));

		if (has_texture || mat.map_bump)
		{
			for (size_t j = 0; j < 2; j++)
			{
				tri1.vertices[p_outside[0]].t[j] = t->vertices[p_outside[0]].t[j] + t1 * (t->vertices[p_outside[0]].t[j] - t->vertices[p_inside[0]].t[j]);
			}
		}

		if (model->mesh->normals)
		{
			for (size_t j = 0; j < 3; j++)
			{
				tri1.vertices[p_outside[0]].n[j] = t->vertices[p_outside[0]].n[j] + t1 * (t->vertices[p_outside[0]].n[j] - t->vertices[p_inside[0]].n[j]);
			}
		}

		render_clip(model, cam, light_dirs, light_points, cull, lighting, shadow, &tri1, i + 1);

		//Second
		float r2[3];
		vec_subtract(t->vertices[p_outside[0]].v, t->vertices[p_inside[1]].v, r2);

		float t2 = vec_dot(diff, plane->n) / vec_dot(r2, plane->n);

		vec_scale(t2, r2, r2);

		float p2[3];
		vec_add(t->vertices[p_outside[0]].v, r2, p2);

		struct triangle2 tri2;
		memcpy(&tri2, t, sizeof(tri2));

		memcpy(&tri2.vertices[p_inside[0]].v, &p1, sizeof(tri2.vertices[p_inside[0]].v));

		memcpy(&tri2.vertices[p_outside[0]].v, &p2, sizeof(tri2.vertices[p_outside[0]].v));

		if (has_texture || mat.map_bump)
		{
			for (size_t j = 0; j < 2; j++)
			{
				tri2.vertices[p_inside[0]].t[j] = t->vertices[p_outside[0]].t[j] + t1 * (t->vertices[p_outside[0]].t[j] - t->vertices[p_inside[0]].t[j]);

				tri2.vertices[p_outside[0]].t[j] = t->vertices[p_outside[0]].t[j] + t2 * (t->vertices[p_outside[0]].t[j] - t->vertices[p_inside[1]].t[j]);
			}
		}

		if (model->mesh->normals)
		{
			for (size_t j = 0; j < 3; j++)
			{
				tri2.vertices[p_inside[0]].n[j] = t->vertices[p_outside[0]].n[j] + t1 * (t->vertices[p_outside[0]].n[j] - t->vertices[p_inside[0]].n[j]);

				tri2.vertices[p_outside[0]].n[j] = t->vertices[p_outside[0]].n[j] + t2 * (t->vertices[p_outside[0]].n[j] - t->vertices[p_inside[1]].n[j]);
			}
		}

		render_clip(model, cam, light_dirs, light_points, cull, lighting, shadow, &tri2, i + 1);
	}
	else if (inside_count == 3)
	{
		render_clip(model, cam, light_dirs, light_points, cull, lighting, shadow, t, i + 1);
	}
}

void render_draw(struct model *model, struct camera *cam, struct light_dir *light_dirs, struct light_point *light_points, int cull, unsigned char lighting, unsigned char shadow)
{
	struct mesh *mesh = model->mesh;

	float rotate[16];
	float trans[16];

	vec_mult_4x4(cam->rotation, model->rotation, rotate);
	vec_mult_4x4(cam->transformation, model->transformation, trans);

	mat = model->base_material;

	unsigned char has_texture = (mat.map_kd && mesh->texture_coordinates);

	unsigned char has_norm_map = (mat.map_bump && mesh->texture_coordinates);

	size_t mat_next = 0;

	for (size_t i = 0; i < mesh->index_count; i++)
	{
		if (mat_next < mesh->material_count && mesh->materials[mat_next].start_index == i)
		{
			mat = mesh->materials[mat_next];

			if (model->base_material.map_kd)
			{
				mat.map_kd = model->base_material.map_kd;
			}

			if (model->base_material.map_bump)
			{
				mat.map_bump = model->base_material.map_bump;
			}

			has_texture = (mat.map_kd && mesh->texture_coordinates);

			has_norm_map = (mat.map_bump && mesh->texture_coordinates);

			mat_next++;
		}

		struct triangle2 t = {0};
		t.c[0] = 1;
		t.c[1] = 1;
		t.c[2] = 1;
		t.t_ind[0] = 255;
		t.t_ind[1] = 255;
		t.t_ind[2] = 255;
		t.t_ind[3] = 255;

		for (size_t j = 0; j < 3; j++)
		{
			float v[3] = {mesh->vertices[3 * mesh->indices[3 * i + j] + 0], mesh->vertices[3 * mesh->indices[3 * i + j] + 1], mesh->vertices[3 * mesh->indices[3 * i + j] + 2]};

			vec_mult_4x4_to_vec(trans, v, t.vertices[j].v);
		}

		vec_normal(t.vertices[0].v, t.vertices[1].v, t.vertices[2].v, t.nf, 1);

		int vis = vec_dot(t.nf, t.vertices[0].v) < 0;

		if (!cull || vis)
		{
			if (!vis)
			{
				vec_scale(-1, t.nf, t.nf); //Corrects normals when backface culling is off
			}

			if (has_texture || has_norm_map)
			{
				for (size_t j = 0; j < 3; j++)
				{
					t.vertices[j].t[0] = mesh->texture_coordinates[2 * mesh->indices[3 * i + j] + 0];
					t.vertices[j].t[1] = mesh->texture_coordinates[2 * mesh->indices[3 * i + j] + 1];
				}
			}

			if (mesh->normals)
			{
				for (size_t j = 0; j < 3; j++)
				{
					float n[3] = {mesh->normals[3 * mesh->indices[3 * i + j] + 0], mesh->normals[3 * mesh->indices[3 * i + j] + 1], mesh->normals[3 * mesh->indices[3 * i + j] + 2]};

					vec_mult_4x4_to_vec(rotate, n, t.vertices[j].n);

					if (!vis)
					{
						vec_scale(-1, t.vertices[j].n, t.vertices[j].n); //Corrects normals when backface culling is off
					}
				}
			}

			render_clip(model, cam, light_dirs, light_points, cull, lighting, shadow, &t, 0);
		}
	}
}

void render_fragment_shadow(struct camera *cam, struct triangle2 *t)
{
	float v1[5] = {0};
	float v2[5] = {0};
	float v3[5] = {0};

	camera_project_shadow(cam, t->vertices[0].v, v1);
	camera_project_shadow(cam, t->vertices[1].v, v2);
	camera_project_shadow(cam, t->vertices[2].v, v3);

	float z1 = 1 / v1[4];
	float z2 = 1 / v2[4];
	float z3 = 1 / v3[4];

	int xMax = math_min_int((int)cam->w - 1, (int)math_max_float(v1[0], math_max_float(v2[0], v3[0])));
	int xMin = math_max_int(0, (int)math_min_float(v1[0], math_min_float(v2[0], v3[0])));
	int yMax = math_min_int((int)cam->h - 1, (int)math_max_float(v1[1], math_max_float(v2[1], v3[1])));
	int yMin = math_max_int(0, (int)math_min_float(v1[1], math_min_float(v2[1], v3[1])));

	float a0 = (v3[0] - v1[0]) * (v2[1] - v1[1]) + (v1[0] - v2[0]) * (v3[1] - v1[1]);

	float k11 = (v2[0] - v3[0]) / a0;
	float k12 = (v2[1] - v3[1]) / a0;
	float k21 = (v3[0] - v1[0]) / a0;
	float k22 = (v1[1] - v3[1]) / a0;

	v3[0] -= 0.5f; //same effect as: float x = x + 0.5f;
	v3[1] -= 0.5f; //same effect as: float y = y + 0.5f;

	for (int x = xMin; x <= xMax; x++)
	{
		float kx = (x - v3[0]);

		for (int y = yMin; y <= yMax; y++)
		{
			float ky = (y - v3[1]);

			const float a1 = ky * k11 - kx * k12;
			const float a2 = ky * k21 + kx * k22;
			const float a3 = 1 - a1 - a2;

			//w/2 and h/2 cancel out here and do not affect final result, only changing x and y pixels values
			//float a0 = (v3[0] - v1[0]) * (v2[1] - v1[1]) + (v1[0] - v2[0]) * (v3[1] - v1[1]);
			//const float a1 = ((v2[0] - v3[0]) * (y + 0.5f - v3[1]) - (x + 0.5f - v3[0]) * (v2[1] - v3[1])) / a01;
			//const float a2 = ((v3[0] - v1[0]) * (y + 0.5f - v3[1]) + (x + 0.5f - v3[0]) * (v1[1] - v3[1])) / a01;
			//const float a3 = 1 - a1 - a2;

			float bias = 0.05f;

			if (a1 >= ZERO - bias && a2 >= ZERO - bias && a3 >= ZERO - bias)
			{
				struct shade s = {x, y, a1 * z1, a2 * z2, a3 * z3, s.d1 + s.d2 + s.d3};

				if (s.depth > cam->depth_buffer[x + cam->w * y])
				{
					cam->depth_buffer[s.i + cam->w * s.j] = s.depth;
				}
			}
		}
	}
}

void render_clip_shadow(struct camera *cam, int cull, struct triangle2 *t, size_t i)
{
	if (i == 6)
	{
		render_fragment_shadow(cam, t);

		return;
	}

	struct vec_plane *plane = &cam->planes[i];

	size_t p_inside[3] = {0};
	size_t p_outside[3] = {0};

	size_t inside_count = 0;

	for (size_t j = 0; j < 3; j++)
	{
		float len = vec_dot(t->vertices[j].v, plane->n) + plane->d;

		if (len >= 0)
		{
			p_inside[inside_count] = j;

			inside_count++;
		}
		else
		{
			p_outside[j - inside_count] = j;
		}
	}

	if (inside_count == 0)
	{
		return;
	}
	else if (inside_count == 1)
	{
		float diff[3];
		vec_subtract(plane->p1, t->vertices[p_inside[0]].v, diff);

		//First
		float r1[3];
		vec_subtract(t->vertices[p_inside[0]].v, t->vertices[p_outside[0]].v, r1);

		float t1 = vec_dot(diff, plane->n) / vec_dot(r1, plane->n);

		vec_scale(t1, r1, r1);

		vec_add(t->vertices[p_inside[0]].v, r1, t->vertices[p_outside[0]].v);

		//Second
		float r2[3];
		vec_subtract(t->vertices[p_inside[0]].v, t->vertices[p_outside[1]].v, r2);

		float t2 = vec_dot(diff, plane->n) / vec_dot(r2, plane->n);

		vec_scale(t2, r2, r2);

		vec_add(t->vertices[p_inside[0]].v, r2, t->vertices[p_outside[1]].v);

		//Clip new triangle
		render_clip_shadow(cam, cull, t, i + 1);
	}
	else if (inside_count == 2)
	{
		float diff[3];
		vec_subtract(plane->p1, t->vertices[p_outside[0]].v, diff);

		//First
		float r1[3];
		vec_subtract(t->vertices[p_outside[0]].v, t->vertices[p_inside[0]].v, r1);

		float t1 = vec_dot(diff, plane->n) / vec_dot(r1, plane->n);

		vec_scale(t1, r1, r1);

		float p1[3];
		vec_add(t->vertices[p_outside[0]].v, r1, p1);

		struct triangle2 tri1;
		memcpy(&tri1, t, sizeof(tri1));

		memcpy(&tri1.vertices[p_outside[0]].v, &p1, sizeof(tri1.vertices[p_outside[0]].v));

		render_clip_shadow(cam, cull, &tri1, i + 1);

		//Second
		float r2[3];
		vec_subtract(t->vertices[p_outside[0]].v, t->vertices[p_inside[1]].v, r2);

		float t2 = vec_dot(diff, plane->n) / vec_dot(r2, plane->n);

		vec_scale(t2, r2, r2);

		float p2[3];
		vec_add(t->vertices[p_outside[0]].v, r2, p2);

		struct triangle2 tri2;
		memcpy(&tri2, t, sizeof(tri2));

		memcpy(&tri2.vertices[p_inside[0]].v, &p1, sizeof(tri2.vertices[p_inside[0]].v));

		memcpy(&tri2.vertices[p_outside[0]].v, &p2, sizeof(tri2.vertices[p_outside[0]].v));

		render_clip_shadow(cam, cull, &tri2, i + 1);
	}
	else if (inside_count == 3)
	{
		render_clip_shadow(cam, cull, t, i + 1);
	}
}

void render_draw_shadow(struct model *model, struct camera *cam, int cull)
{
	struct mesh *mesh = model->mesh;

	float trans[16];

	vec_mult_4x4(cam->transformation, model->transformation, trans);

	for (size_t i = 0; i < mesh->index_count; i++)
	{
		struct triangle2 t = {0};
		t.c[0] = 1;
		t.c[1] = 1;
		t.c[2] = 1;
		t.t_ind[0] = 255;
		t.t_ind[1] = 255;
		t.t_ind[2] = 255;
		t.t_ind[3] = 255;

		for (size_t j = 0; j < 3; j++)
		{
			float v[3] = {mesh->vertices[3 * mesh->indices[3 * i + j] + 0], mesh->vertices[3 * mesh->indices[3 * i + j] + 1], mesh->vertices[3 * mesh->indices[3 * i + j] + 2]};

			vec_mult_4x4_to_vec(trans, v, t.vertices[j].v);
		}

		vec_normal(t.vertices[0].v, t.vertices[1].v, t.vertices[2].v, t.nf, 1);

		int vis = vec_dot(t.nf, t.vertices[0].v) < 0;

		if (!cull || !vis) //Frontface culling
		{
			render_clip_shadow(cam, cull, &t, 0);
		}
	}
}

void render_draw_text(struct mesh *mesh, struct camera *cam, struct texture *tex, int highlight, int x_off, int y_off, char *c)
{
	unsigned char *pixels = cam->pixels;

	const float *texture_coordinates = mesh->texture_coordinates;

	const unsigned int *indices = mesh->indices;

	size_t index_count = mesh->index_count;

	size_t w = cam->w;
	size_t h = cam->h;

	const unsigned char *t = tex->pixels;;

	size_t usize = tex->w;
	size_t vsize = tex->h;

	if (highlight)
	{
		c[0] /= 3;
		c[1] /= 3;
		c[2] /= 3;
	}

	//float zero = -0.0000001f; //can cause texture issues if too far from 0, can cause holes to appear if too close to 0
	float zero = -0.00001f;

	for (size_t i = 0; i < index_count; i++)
	{
		struct triangle t1 =
		{
			{0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {1,1,1}, {0,0,0}, {0,0,0}, {0,0,0}, {255,255,255,255},
			{mesh->vertices[3 * indices[3 * i + 0] + 0], mesh->vertices[3 * indices[3 * i + 0] + 1], mesh->vertices[3 * indices[3 * i + 0] + 2]},
			{mesh->vertices[3 * indices[3 * i + 1] + 0], mesh->vertices[3 * indices[3 * i + 1] + 1], mesh->vertices[3 * indices[3 * i + 1] + 2]},
			{mesh->vertices[3 * indices[3 * i + 2] + 0], mesh->vertices[3 * indices[3 * i + 2] + 1], mesh->vertices[3 * indices[3 * i + 2] + 2]}
		};

		t1.tx[0] = usize * texture_coordinates[2 * indices[3 * i + 0] + 0];
		t1.ty[0] = vsize * texture_coordinates[2 * indices[3 * i + 0] + 1];

		t1.tx[1] = usize * texture_coordinates[2 * indices[3 * i + 1] + 0];
		t1.ty[1] = vsize * texture_coordinates[2 * indices[3 * i + 1] + 1];

		t1.tx[2] = usize * texture_coordinates[2 * indices[3 * i + 2] + 0];
		t1.ty[2] = vsize * texture_coordinates[2 * indices[3 * i + 2] + 1];

		float v1[5] = {t1.p1[0] + x_off, t1.p1[1] + y_off, t1.p1[0], t1.p1[1], t1.p1[2]};
		float v2[5] = {t1.p2[0] + x_off, t1.p2[1] + y_off, t1.p2[0], t1.p2[1], t1.p2[2]};
		float v3[5] = {t1.p3[0] + x_off, t1.p3[1] + y_off, t1.p3[0], t1.p3[1], t1.p3[2]};

		int xMax = math_min_int((int)w - 1, (int)math_max_float(v1[0], math_max_float(v2[0], v3[0])));
		int xMin = math_max_int(0, (int)math_min_float(v1[0], math_min_float(v2[0], v3[0])));
		int yMax = math_min_int((int)h - 1, (int)math_max_float(v1[1], math_max_float(v2[1], v3[1])));
		int yMin = math_max_int(0, (int)math_min_float(v1[1], math_min_float(v2[1], v3[1])));

		float a0 = (v3[0] - v1[0]) * (v2[1] - v1[1]) + (v1[0] - v2[0]) * (v3[1] - v1[1]);

		float k11 = (v2[0] - v3[0]) / a0;
		float k12 = (v2[1] - v3[1]) / a0;
		float k21 = (v3[0] - v1[0]) / a0;
		float k22 = (v1[1] - v3[1]) / a0;

		v3[0] -= 0.5f; //same effect as: float x = i + 0.5f;
		v3[1] -= 0.5f; //same effect as: float y = j + 0.5f;

		for (int x = xMin; x <= xMax; x++)
		{
			float kx = (x - v3[0]);

			for (int y = yMin; y <= yMax; y++)
			{
				float ky = (y - v3[1]);

				const float a1 = ky * k11 - kx * k12;
				const float a2 = ky * k21 + kx * k22;
				const float a3 = 1 - a1 - a2;

				if (a1 >= zero && a2 >= zero && a3 >= zero)
				{
					struct shade s = {x, y, a1, a2, a3, s.d1 + s.d2 + s.d3};

					if (t1.tx[0] >= 0)
					{
						int tu = (int)(a1 * t1.tx[0] + a2 * t1.tx[1] + a3 * t1.tx[2]);
						int tv = (int)(a1 * t1.ty[0] + a2 * t1.ty[1] + a3 * t1.ty[2]);

						if (tu > (int)usize - 1) tu = tu % usize;
						if (tv > (int)vsize - 1) tv = tv % vsize;

						size_t tex_ind = 4 * ((size_t)tu + usize * (size_t)tv);

						t1.t_ind[0] = t[tex_ind + 0];
						t1.t_ind[1] = t[tex_ind + 1];
						t1.t_ind[2] = t[tex_ind + 2];
						t1.t_ind[3] = t[tex_ind + 3];

						if (t1.t_ind[3] != 0)
						{
							//render_DrawPixel(pixels, w, s.i, s.j, 255, 255, 0);
							render_DrawPixel(pixels, w, s.i, s.j, 255, 255, 255);
						}
					}
					else
					{
						//render_DrawPixel(pixels, w, s.i, s.j, r, g, b);
						render_draw_pixel(pixels, w, s.i, s.j, (unsigned char)c[0], (unsigned char)c[1], (unsigned char)c[2], 1.0f); //0.7f
					}
				}
			}
		}
	}
}