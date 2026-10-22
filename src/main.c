#include <stddef.h>

#include "math.h"
#include "vec.h"
#include "str.h"
#include "alloc.h"
#include "os.h"
#include "dynarray.h"
#include "mesh.h"
#include "texture.h"
#include "model.h"
#include "camera.h"
#include "light_dir.h"
#include "light_point.h"
#include "graph.h"
#include "font.h"
#include "render.h"
#include "gui.h"
#include "terrain.h"

struct menu
{
	struct gui *gui_root;

	struct gui *owner_label;
	struct gui *FPS_label;
	struct gui *FPS_value;

	struct gui *tab;
	struct gui *window_terrain;

	struct gui *label_seed;
	struct gui *textbox_seed;
	struct gui *button_random_seed;
	struct gui *label_scale;
	struct gui *slider_scale;
	struct gui *button_generate;
	struct gui *checkbox_erosion;
	struct gui *button_save;
	struct gui *label_flow;
	struct gui *slider_flow;
	struct gui *label_capacity;
	struct gui *slider_capacity;
	struct gui *label_dissolution;
	struct gui *slider_dissolution;
	struct gui *label_deposition;
	struct gui *slider_deposition;
	struct gui *label_evaporation;
	struct gui *slider_evaporation;
	struct gui *label_rain;
	struct gui *slider_rain;
};

int main()
{
	//Load Textures
	char texture_directory[MAXIMUM_PATH] = {0};
	str_cat(texture_directory, os_file_get_directory(), sizeof(texture_directory));
	str_cat(texture_directory, "textures", sizeof(texture_directory));
	struct texture *placeholder_texture = alloc_calloc(1, sizeof(*placeholder_texture));;
	texture_create(placeholder_texture, 15, 15, "placeholder");
	struct dynarray textures;
	dynarray_create(&textures, sizeof(struct texture *), NULL);
	dynarray_push(&textures, &placeholder_texture); //Ensuring that there is always at least one texture
	texture_load_directory(texture_directory, &textures);

	//Create Cameras
	struct camera cams[1] = {0};
	camera_create(&cams[0], 899, 511, 0.01f, 100, 90, CAMERA_PERSPECTIVE);
	
	struct os_window wnd;
	struct texture *ico = texture_find(&textures, "ico");
	os_window_create(&wnd, "Software Renderer", cams[0].w, cams[0].h, ico->pixels, ico->w, ico->h);

	int running = 1;
	int frames = 0;
	int time = 0;

	struct mesh *graph = alloc_calloc(1, sizeof(*graph));
	graph_create_square(graph, -1, 1, -1, 1, 2, 2, 1, "placeholder"); //Square

	//Load Models
	char model_directory[MAXIMUM_PATH] = {0};
	str_cat(model_directory, os_file_get_directory(), sizeof(model_directory));
	str_cat(model_directory, "models", sizeof(model_directory));
	struct dynarray models;
	dynarray_create(&models, sizeof(struct mesh *), NULL);
	dynarray_push(&models, &graph);
	mesh_load_directory(model_directory, &models, &textures);
	//NOTE: cannot add more to models after this point. Consider allocating an array using malloc instead of using dynarray

	//Create Scene
	struct dynarray scene;
	dynarray_create(&scene, sizeof(struct model), NULL);

	struct light_dir light_dirs[1] = {0};
	light_dir_create(&light_dirs[0], 1, -1, 0, 0.3f, 0.3f, 0.3f, 1, 1, 1);

	struct light_point light_points[1] = {0};
	light_point_create(&light_points[0], 0.1f, 0.1f, 0.1f, 1, 1, 1, 0.005f, 0.07f, 1.0f, mesh_find(&models, "sphere_low_poly"));
	light_point_set_x(&light_points[0], -40);
	light_point_set_z(&light_points[0], -5);

	//Load Font
	struct font font;
	font_create(&font, texture_find(&textures, "font"));

	struct terrain terrain;
	terrain_create(&terrain, 1500, 1500, 100);
	terrain_generate(&terrain);
	terrain_draw(&terrain, 0.05f);

	struct model terrain_model;
	model_create(&terrain_model, mesh_find(&models, "placeholder"), &terrain.textures[0]);
	dynarray_push(&scene, &terrain_model);

	struct menu menu = {0};

	menu.gui_root = gui_create(&font, NULL, 0, 0, cams[0].w, cams[0].h, 0, GUI_ROOT);

	menu.owner_label = gui_create(&font, menu.gui_root, 10, 305, 155, 15, 25, GUI_LABEL);
	menu.FPS_label = gui_create(&font, menu.gui_root, 10, 5, 26, 15, 10, GUI_LABEL);
	menu.FPS_value = gui_create(&font, menu.gui_root, 36, 5, 40, 15, 10, GUI_LABEL);

	menu.tab = gui_create(&font, menu.gui_root, 10, 285, 200, 15, 0, GUI_TAB);
	menu.window_terrain = gui_add_tab(menu.tab, " Scene", 40, 260);

	menu.label_seed = gui_create(&font, menu.window_terrain, 5, 205, 40, 15, 10, GUI_LABEL);
	menu.textbox_seed = gui_create(&font, menu.window_terrain, 55, 205, 100, 15, 50, GUI_TEXTBOX);
	menu.button_random_seed = gui_create(&font, menu.window_terrain, 55, 185, 100, 15, 50, GUI_BUTTON);
	menu.label_scale = gui_create(&font, menu.window_terrain, 5, 165, 40, 15, 10, GUI_LABEL);
	menu.slider_scale = gui_create(&font, menu.window_terrain, 55, 165, 100, 15, 50, GUI_SLIDER);
	menu.button_generate = gui_create(&font, menu.window_terrain, 55, 145, 100, 15, 50, GUI_BUTTON);
	menu.checkbox_erosion = gui_create(&font, menu.window_terrain, 165, 145, 25, 15, 50, GUI_CHECKBOX);
	menu.button_save = gui_create(&font, menu.window_terrain, 55, 125, 100, 15, 50, GUI_BUTTON);
	menu.label_flow = gui_create(&font, menu.window_terrain, 5, 105, 40, 15, 10, GUI_LABEL);
	menu.slider_flow = gui_create(&font, menu.window_terrain, 55, 105, 100, 15, 10, GUI_SLIDER);
	menu.label_capacity = gui_create(&font, menu.window_terrain, 5, 85, 40, 15, 10, GUI_LABEL);
	menu.slider_capacity = gui_create(&font, menu.window_terrain, 55, 85, 100, 15, 10, GUI_SLIDER);
	menu.label_dissolution = gui_create(&font, menu.window_terrain, 5, 65, 40, 15, 10, GUI_LABEL);
	menu.slider_dissolution = gui_create(&font, menu.window_terrain, 55, 65, 100, 15, 10, GUI_SLIDER);
	menu.label_deposition = gui_create(&font, menu.window_terrain, 5, 45, 40, 15, 10, GUI_LABEL);
	menu.slider_deposition = gui_create(&font, menu.window_terrain, 55, 45, 100, 15, 10, GUI_SLIDER);
	menu.label_evaporation = gui_create(&font, menu.window_terrain, 5, 25, 40, 15, 10, GUI_LABEL);
	menu.slider_evaporation = gui_create(&font, menu.window_terrain, 55, 25, 100, 15, 10, GUI_SLIDER);
	menu.label_rain = gui_create(&font, menu.window_terrain, 5, 5, 40, 15, 10, GUI_LABEL);
	menu.slider_rain = gui_create(&font, menu.window_terrain, 55, 5, 100, 15, 10, GUI_SLIDER);

	gui_set_text(menu.FPS_label, "FPS:");
	gui_set_text(menu.owner_label, "Gabriel Alexander Bawden");

	gui_set_text(menu.label_seed, "Seed:");
	gui_set_text_int(menu.textbox_seed, terrain.seed);
	gui_set_text(menu.button_random_seed, "     Random Seed");
	gui_set_text(menu.label_scale, "Scale:");
	gui_set_text(menu.button_generate, "         Generate");
	gui_set_text(menu.button_save, "             Save");
	gui_set_text(menu.label_flow, "k_fl:");
	gui_set_text(menu.label_capacity, "k_ca:");
	gui_set_text(menu.label_dissolution, "k_di:");
	gui_set_text(menu.label_deposition, "k_de:");
	gui_set_text(menu.label_evaporation, "k_ev:");
	gui_set_text(menu.label_rain, "rain:");

	gui_hide(menu.owner_label);

	float speed = 0.003f;
	camera_set_z(&cams[0], -2.0f);

	struct dynarray events;
	dynarray_create(&events, sizeof(struct gui_event), NULL);

	while (running)
	{
		for (struct os_event evt = {0}; os_window_get_event(&evt); memset(&evt, 0, sizeof(evt)))
		{
			if (evt.wnd == &wnd)
			{
				switch (evt.event)
				{
				case OS_EVENT_KEY_UP:
				{
					if (!menu.gui_root->active_child)
					{
						switch (evt.id)
						{
						case 'M':
						{
							//gui_show(&guis, owner_label);
							menu.owner_label->visible = !menu.owner_label->visible;

							break;
						}
						case OS_KEY_ESCAPE:
						{
							menu.gui_root->visible = !menu.gui_root->visible;

							break;
						}
						case OS_KEY_ENTER:
						{
							camera_set_x(&cams[0], 0);
							camera_set_y(&cams[0], 0);
							camera_set_z(&cams[0], -2.0f);

							camera_set_theta_x(&cams[0], 0);
							camera_set_theta_y(&cams[0], 0);
							camera_set_theta_z(&cams[0], 0);

							break;
						}
						}
					}

					break;
				}
				case OS_EVENT_LEFT_MOUSE_DOWN:
				{
					if (menu.gui_root->visible)
					{
						menu.gui_root->active_child = 0;

						gui_mouse_down_children(menu.gui_root, evt.id, evt.data, &events, menu.gui_root);
					}

					break;
				}
				case OS_EVENT_LEFT_MOUSE_UP:
				{
					if (menu.gui_root->visible)
					{
						menu.gui_root->active_child = 0;

						gui_mouse_up_children(menu.gui_root, &events, menu.gui_root);
					}

					break;
				}
				case OS_EVENT_RIGHT_MOUSE_UP:
				{
					if (menu.gui_root->visible)
					{
					}

					break;
				}
				case OS_EVENT_MOUSE_MOVE:
				{
					int mouse_x = evt.id;
					int mouse_y = evt.data;

					if (menu.gui_root->visible)
					{
						gui_mouse_move_children(menu.gui_root, mouse_x, mouse_y, &events);
					}

					break;
				}
				case OS_EVENT_TYPE:
				{
					if (menu.gui_root->visible)
					{
						gui_type_children(menu.gui_root, evt.wide);
					}

					break;
				}
				case OS_EVENT_LOSE_FOCUS:
				{
					memset(wnd.keystates, 0, sizeof(wnd.keystates));

					break;
				}
				case OS_EVENT_RESIZE:
				{
					camera_resize(&cams[0], evt.id, evt.data);

					break;
				}
				}
			}

			switch (evt.event)
			{
			case OS_EVENT_END:
			{
				running = 0;

				os_window_destroy(&wnd);
				dynarray_destroy(&textures);
				dynarray_destroy(&models);
				dynarray_destroy(&scene);
				gui_destroy_children(menu.gui_root);
				font_destroy(&font);

				os_process_exit(0);

				break;
			}
			}
		}

		while (dynarray_len(&events))
		{
			struct gui_event evt;
			dynarray_pop(&events, &evt);

			switch (evt.event)
			{
			case GUI_EVENT_BUTTON_PRESS:
			{


				break;
			}
			case GUI_EVENT_CHECKBOX_PRESS:
			{
				terrain.erosion_flag = !terrain.erosion_flag;

				break;
			}
			case GUI_EVENT_SLIDER_MOVE:
			{

				break;
			}
			}
		}

		if (!menu.gui_root->active_child) //Check if any textboxes are currently selected
		{
			if (wnd.keystates['W'])
			{
				if (cams[0].z < -0.03f)
				{
					camera_key_z(&cams[0], speed);
				}
			}
			if (wnd.keystates['S'])
			{
				if (cams[0].z > -2.0f)
				{
					camera_key_z(&cams[0], -speed);
				}
			}
			if (wnd.keystates['D'])
			{
				if (cams[0].x < 1.0f)
				{
					camera_key_x(&cams[0], speed);
				}
			}
			if (wnd.keystates['A'])
			{
				if (cams[0].x > -1.0f)
				{
					camera_key_x(&cams[0], -speed);
				}
			}
			if (wnd.keystates[' '])
			{
				if (cams[0].y < 1.0f)
				{
					camera_key_y(&cams[0], speed);
				}
			}
			if (wnd.keystates[OS_KEY_LSHIFT])
			{
				if (cams[0].y > -1.0f)
				{
					camera_key_y(&cams[0], -speed);
				}
			}
		}

		if (frames % 50 == 0)
		{
			int FPS = (int)(50 / (((os_time_tick_count() - time)) / 1000.0f));

			time = os_time_tick_count();

			gui_set_text_int(menu.FPS_value, FPS);
		}

		camera_update(&cams[0]);

		if (terrain.erosion_flag)
		{
			terrain_water_flow(&terrain);
			//terrain_water_flow_test(&ter);

			if (frames % 10 == 0)
			{
				terrain_draw(&terrain, 0.05f);
			}
		}

		for (size_t i = 0; i < dynarray_len(&scene); i++)
		{
			render_draw(&((struct model *)scene.array)[i], &cams[0], light_dirs, light_points, 1, 0, 0);
		}

		gui_render_children(menu.gui_root, &cams[0]);

		os_window_draw(&wnd, cams[0].pixels, cams[0].w, cams[0].h);

		frames++;
	}

	return 0;
}