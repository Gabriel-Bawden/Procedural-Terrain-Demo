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

	struct gui *label_owner;
	struct gui *label_fps;
	struct gui *textbox_fps;

	struct gui *tab;
	struct gui *window_terrain;
	struct gui *window_simulation;
	struct gui *window_view;

	struct gui *label_seed;
	struct gui *textbox_seed;
	struct gui *button_random_seed;
	struct gui *label_scale;
	struct gui *slider_scale;
	struct gui *button_generate;
	struct gui *button_save;

	struct gui *label_simulation;
	struct gui *checkbox_simulation;
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
	struct gui *label_precipitation;
	struct gui *slider_precipitation;

	struct gui *label_heightmap_color;
	struct gui *label_heightmap_gray;
	struct gui *label_water;
	struct gui *label_velocity;
	struct gui *label_sediment;
	struct gui *label_change_net;
	struct gui *label_watershed;
	struct gui *checkbox_heightmap_color;
	struct gui *checkbox_heightmap_gray;
	struct gui *checkbox_water;
	struct gui *checkbox_velocity;
	struct gui *checkbox_sediment;
	struct gui *checkbox_change_net;
	struct gui *checkbox_watershed;
};

static void menu_clear_checkboxes(struct menu *menu)
{
	gui_set_state(menu->checkbox_heightmap_color, 0);
	gui_set_state(menu->checkbox_heightmap_gray, 0);
	gui_set_state(menu->checkbox_water, 0);
	gui_set_state(menu->checkbox_velocity, 0);
	gui_set_state(menu->checkbox_sediment, 0);
	gui_set_state(menu->checkbox_change_net, 0);
	gui_set_state(menu->checkbox_watershed, 0);
}

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
	os_window_create(&wnd, "Procedural Terrain", cams[0].w, cams[0].h, ico->pixels, ico->w, ico->h);

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

	struct menu menu = {0};

	menu.gui_root = gui_create(&font, NULL, 0, 0, cams[0].w, cams[0].h, 0, GUI_ROOT);

	menu.label_owner = gui_create(&font, menu.gui_root, 10, 305, 155, 15, 25, GUI_LABEL);
	menu.label_fps = gui_create(&font, menu.gui_root, 10, 5, 26, 15, 10, GUI_LABEL);
	menu.textbox_fps = gui_create(&font, menu.gui_root, 36, 5, 40, 15, 10, GUI_LABEL);

	menu.tab = gui_create(&font, menu.gui_root, 10, 285, 200, 15, 0, GUI_TAB);
	menu.window_terrain = gui_add_tab(menu.tab, "  Terrain", 50, 105);
	menu.window_simulation = gui_add_tab(menu.tab, "  Simulation", 60, 145);
	menu.window_view = gui_add_tab(menu.tab, "   View", 40, 145);

	menu.label_seed = gui_create(&font, menu.window_terrain, 5, 85, 40, 15, 10, GUI_LABEL);
	menu.label_scale = gui_create(&font, menu.window_terrain, 5, 45, 40, 15, 10, GUI_LABEL);
	menu.textbox_seed = gui_create(&font, menu.window_terrain, 55, 85, 100, 15, 50, GUI_TEXTBOX);
	menu.button_random_seed = gui_create(&font, menu.window_terrain, 55, 65, 100, 15, 20, GUI_BUTTON);
	menu.button_generate = gui_create(&font, menu.window_terrain, 55, 25, 100, 15, 20, GUI_BUTTON);
	menu.button_save = gui_create(&font, menu.window_terrain, 55, 5, 100, 15, 20, GUI_BUTTON);
	menu.slider_scale = gui_create(&font, menu.window_terrain, 55, 45, 100, 15, 50, GUI_SLIDER);

	menu.label_simulation = gui_create(&font, menu.window_simulation, 5, 125, 65, 15, 15, GUI_LABEL);
	menu.label_flow = gui_create(&font, menu.window_simulation, 5, 105, 65, 15, 15, GUI_LABEL);
	menu.label_capacity = gui_create(&font, menu.window_simulation, 5, 85, 65, 15, 15, GUI_LABEL);
	menu.label_dissolution = gui_create(&font, menu.window_simulation, 5, 65, 65, 15, 15, GUI_LABEL);
	menu.label_deposition = gui_create(&font, menu.window_simulation, 5, 45, 65, 15, 15, GUI_LABEL);
	menu.label_evaporation = gui_create(&font, menu.window_simulation, 5, 25, 65, 15, 15, GUI_LABEL);
	menu.label_precipitation = gui_create(&font, menu.window_simulation, 5, 5, 65, 15, 15, GUI_LABEL);
	menu.checkbox_simulation = gui_create(&font, menu.window_simulation, 80, 125, 15, 15, 0, GUI_CHECKBOX);
	menu.slider_flow = gui_create(&font, menu.window_simulation, 80, 105, 100, 15, 10, GUI_SLIDER);
	menu.slider_capacity = gui_create(&font, menu.window_simulation, 80, 85, 100, 15, 10, GUI_SLIDER);
	menu.slider_dissolution = gui_create(&font, menu.window_simulation, 80, 65, 100, 15, 10, GUI_SLIDER);
	menu.slider_deposition = gui_create(&font, menu.window_simulation, 80, 45, 100, 15, 10, GUI_SLIDER);
	menu.slider_evaporation = gui_create(&font, menu.window_simulation, 80, 25, 100, 15, 10, GUI_SLIDER);
	menu.slider_precipitation = gui_create(&font, menu.window_simulation, 80, 5, 100, 15, 10, GUI_SLIDER);

	menu.label_heightmap_color = gui_create(&font, menu.window_view, 5, 125, 65, 15, 15, GUI_LABEL);
	menu.label_heightmap_gray = gui_create(&font, menu.window_view, 5, 105, 65, 15, 15, GUI_LABEL);
	menu.label_water = gui_create(&font, menu.window_view, 5, 85, 65, 15, 15, GUI_LABEL);
	menu.label_velocity = gui_create(&font, menu.window_view, 5, 65, 65, 15, 15, GUI_LABEL);
	menu.label_sediment = gui_create(&font, menu.window_view, 5, 45, 65, 15, 15, GUI_LABEL);
	menu.label_change_net = gui_create(&font, menu.window_view, 5, 25, 65, 15, 15, GUI_LABEL);
	menu.label_watershed = gui_create(&font, menu.window_view, 5, 5, 65, 15, 15, GUI_LABEL);
	menu.checkbox_heightmap_color = gui_create(&font, menu.window_view, 80, 125, 15, 15, 0, GUI_CHECKBOX);
	menu.checkbox_heightmap_gray = gui_create(&font, menu.window_view, 80, 105, 15, 15, 0, GUI_CHECKBOX);
	menu.checkbox_water = gui_create(&font, menu.window_view, 80, 85, 15, 15, 0, GUI_CHECKBOX);
	menu.checkbox_velocity = gui_create(&font, menu.window_view, 80, 65, 15, 15, 0, GUI_CHECKBOX);
	menu.checkbox_sediment = gui_create(&font, menu.window_view, 80, 45, 15, 15, 0, GUI_CHECKBOX);
	menu.checkbox_change_net = gui_create(&font, menu.window_view, 80, 25, 15, 15, 0, GUI_CHECKBOX);
	menu.checkbox_watershed = gui_create(&font, menu.window_view, 80, 5, 15, 15, 0, GUI_CHECKBOX);

	gui_set_text(menu.label_fps, "FPS:");
	gui_set_text(menu.label_owner, "Gabriel Alexander Bawden");

	gui_set_text(menu.label_seed, "Seed:");
	gui_set_text_int(menu.textbox_seed, terrain.seed);
	gui_set_text(menu.button_random_seed, "     Random Seed");
	gui_set_text(menu.label_scale, "Scale:");
	gui_set_text(menu.button_generate, "         Generate");
	gui_set_text(menu.button_save, "             Save");

	gui_set_text(menu.label_simulation, "Simulation:");
	gui_set_text(menu.label_flow, "Flow:");
	gui_set_text(menu.label_capacity, "Capacity:");
	gui_set_text(menu.label_dissolution, "Dissolution:");
	gui_set_text(menu.label_deposition, "Deposition:");
	gui_set_text(menu.label_evaporation, "Evaporation:");
	gui_set_text(menu.label_precipitation, "Precipitation:");

	gui_set_text(menu.label_heightmap_color, "Color:");
	gui_set_text(menu.label_heightmap_gray, "Grayscale:");
	gui_set_text(menu.label_water, "Water:");
	gui_set_text(menu.label_velocity, "Velocity:");
	gui_set_text(menu.label_sediment, "Sediment:");
	gui_set_text(menu.label_change_net, "Net Change:");
	gui_set_text(menu.label_watershed, "Watershed:");

	gui_set_slider_position(menu.slider_scale, (terrain.scale - 0.001f) / (0.02f - 0.001f));
	gui_set_slider_position(menu.slider_flow, (terrain.k_flow - 0.0f) / (25 - 0.0f));
	gui_set_slider_position(menu.slider_capacity, (terrain.k_capacity - 0.0f) / (15 - 0.0f));
	gui_set_slider_position(menu.slider_dissolution, (terrain.k_dissolution - 0.0f) / (1.0f - 0.0f));
	gui_set_slider_position(menu.slider_deposition, (terrain.k_deposition - 0.0f) / (1.0f - 0.0f));
	gui_set_slider_position(menu.slider_evaporation, (terrain.k_evap - 0.0f) / (0.05f - 0.0f));
	gui_set_slider_position(menu.slider_precipitation, (terrain.rain_drops - 0.0f) / (3000 - 0.0f));

	gui_set_text_float(menu.slider_scale, terrain.scale);
	gui_set_text_float(menu.slider_flow, terrain.k_flow);
	gui_set_text_float(menu.slider_capacity, terrain.k_capacity);
	gui_set_text_float(menu.slider_dissolution, terrain.k_dissolution);
	gui_set_text_float(menu.slider_deposition, terrain.k_deposition);
	gui_set_text_float(menu.slider_evaporation, terrain.k_evap);
	gui_set_text_int(menu.slider_precipitation, terrain.rain_drops);

	gui_set_state(menu.checkbox_heightmap_color, 1);

	gui_hide(menu.label_owner);

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
							//gui_show(&guis, label_owner);
							menu.label_owner->visible = !menu.label_owner->visible;

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
				if (evt.id == menu.button_random_seed)
				{
					terrain.seed = os_time_tick_count();

					gui_set_text_int(menu.textbox_seed, terrain.seed);
				}
				if (evt.id == menu.button_generate)
				{
					terrain.seed = str_str_to_int(gui_get_text(menu.textbox_seed));

					terrain_generate(&terrain);
					terrain_draw(&terrain, 0.05f);
				}
				if (evt.id == menu.button_save)
				{
					//terrain_draw_rivers(&ter, 0.05f);
					terrain_watershed(&terrain.textures[6], terrain.altitude, 0.5f);

					texture_save(&terrain.textures[0]);
					texture_save(&terrain.textures[1]);
					texture_save(&terrain.textures[2]);
					texture_save(&terrain.textures[3]);
					texture_save(&terrain.textures[4]);
					texture_save(&terrain.textures[5]);

					graph_create(&terrain.mesh, terrain.altitude, 0.01f, terrain.w, terrain.h);
					mesh_save(&terrain.mesh);
				}

				break;
			}
			case GUI_EVENT_CHECKBOX_PRESS:
			{
				if (evt.id == menu.checkbox_simulation)
				{
					terrain.erosion_flag = !terrain.erosion_flag;
				}
				if (evt.id == menu.checkbox_heightmap_color)
				{
					menu_clear_checkboxes(&menu);

					gui_set_state(menu.checkbox_heightmap_color, 1);

					terrain_model.base_material.map_kd = &terrain.textures[0];
				}
				if (evt.id == menu.checkbox_heightmap_gray)
				{
					menu_clear_checkboxes(&menu);

					gui_set_state(menu.checkbox_heightmap_gray, 1);

					terrain_model.base_material.map_kd = &terrain.textures[1];
				}
				if (evt.id == menu.checkbox_water)
				{
					menu_clear_checkboxes(&menu);

					gui_set_state(menu.checkbox_water, 1);

					terrain_model.base_material.map_kd = &terrain.textures[2];
				}
				if (evt.id == menu.checkbox_velocity)
				{
					menu_clear_checkboxes(&menu);

					gui_set_state(menu.checkbox_velocity, 1);

					terrain_model.base_material.map_kd = &terrain.textures[3];
				}
				if (evt.id == menu.checkbox_sediment)
				{
					menu_clear_checkboxes(&menu);

					gui_set_state(menu.checkbox_sediment, 1);

					terrain_model.base_material.map_kd = &terrain.textures[4];
				}
				if (evt.id == menu.checkbox_change_net)
				{
					menu_clear_checkboxes(&menu);

					gui_set_state(menu.checkbox_change_net, 1);

					terrain_model.base_material.map_kd = &terrain.textures[5];
				}
				if (evt.id == menu.checkbox_watershed)
				{
					menu_clear_checkboxes(&menu);

					gui_set_state(menu.checkbox_watershed, 1);

					terrain_model.base_material.map_kd = &terrain.textures[6];
				}

				break;
			}
			case GUI_EVENT_SLIDER_MOVE:
			{
				if (evt.id == menu.slider_scale)
				{
					terrain.scale = 0.001f + (0.02f - 0.001f) * evt.data_float;

					gui_set_text_float(menu.slider_scale, terrain.scale);
				}
				if (evt.id == menu.slider_flow)
				{
					terrain.k_flow = 0 + (25 - 0) * evt.data_float;

					gui_set_text_float(menu.slider_flow, terrain.k_flow);
				}
				if (evt.id == menu.slider_capacity)
				{
					terrain.k_capacity = 0 + (15 - 0) * evt.data_float;

					gui_set_text_float(menu.slider_capacity, terrain.k_capacity);
				}
				if (evt.id == menu.slider_dissolution)
				{
					terrain.k_dissolution = 0 + (1.0f - 0.0f) * evt.data_float;

					gui_set_text_float(menu.slider_dissolution, terrain.k_dissolution);
				}
				if (evt.id == menu.slider_deposition)
				{
					terrain.k_deposition = 0 + (1.0f - 0.0f) * evt.data_float;

					gui_set_text_float(menu.slider_deposition, terrain.k_deposition);
				}
				if (evt.id == menu.slider_evaporation)
				{
					terrain.k_evap = 0 + (0.05f - 0) * evt.data_float;

					gui_set_text_float(menu.slider_evaporation, terrain.k_evap);
				}
				if (evt.id == menu.slider_precipitation)
				{
					terrain.rain_drops = (int)(0 + (3000 - 0) * evt.data_float);

					gui_set_text_int(menu.slider_precipitation, terrain.rain_drops);
				}

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

			gui_set_text_int(menu.textbox_fps, FPS);
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

		render_draw(&terrain_model, &cams[0], light_dirs, light_points, 1, 0, 0);

		gui_render_children(menu.gui_root, &cams[0]);

		os_window_draw(&wnd, cams[0].pixels, cams[0].w, cams[0].h);

		frames++;
	}

	return 0;
}