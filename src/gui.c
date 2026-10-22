#include "gui.h"

#include <stddef.h>

#include "math.h"
#include "str.h"
#include "alloc.h"
#include "os.h"
#include "dynarray.h"
#include "mesh.h"
#include "camera.h"
#include "font.h"
#include "render.h"

#define NUM_BOXES 1

#define BORDER_HORIZONTAL 4
#define BORDER_VERTICAL 2

void gui_get_color(struct gui *v, char *c)
{
	switch (v->type)
	{
	case GUI_LABEL:
	{
		//c[0] = 50;
		//c[1] = 50;
		//c[2] = 50;

		c[0] = 30;
		c[1] = 30;
		c[2] = 30;

		break;
	}
	case GUI_TEXTBOX:
	{
		//c[0] = 50;
		//c[1] = 0;
		//c[2] = 150;

		//c[0] = 50;
		//c[1] = 50;
		//c[2] = 50;

		c[0] = 30;
		c[1] = 30;
		c[2] = 30;

		break;
	}
	case GUI_BUTTON:
	{
		c[0] = 0;
		c[1] = 0;
		c[2] = 150;

		break;
	}
	case GUI_CHECKBOX:
	{
		c[0] = 0;
		c[1] = 0;
		c[2] = 150;

		break;
	}
	case GUI_WINDOW:
	{
		//c[0] = 30;
		//c[1] = 30;
		//c[2] = 30;

		c[0] = 50;
		c[1] = 50;
		c[2] = 50;

		break;
	}
	case GUI_SLIDER:
	{
		c[0] = 60;
		c[1] = 90;
		c[2] = 120;

		break;
	}
	case GUI_SLIDER_CHILD:
	{
		c[0] = 0;
		c[1] = 0;
		c[2] = 150;

		break;
	}
	case GUI_TAB:
	{
		//c[0] = 30;
		//c[1] = 30;
		//c[2] = 30;

		c[0] = 50;
		c[1] = 50;
		c[2] = 50;

		break;
	}
	case GUI_TAB_CHILD:
	{
		c[0] = 0;
		c[1] = 0;
		c[2] = 150;

		break;
	}
	}
}

void gui_destroy_children(struct gui *v)
{
	for (unsigned i = 0; i < dynarray_len(&v->children); i++)
	{
		gui_destroy_children(*(struct gui **)dynarray_get(&v->children, i));
	}

	alloc_free(v->text);

	mesh_destroy(&v->model);

	dynarray_destroy(&v->children);
}

void gui_delete_character(unsigned char *str, const char *str_base)
{
	//Loop to delete the full multibyte character
	for (size_t i = 0; i < 3; i++)
	{
		if((*str & 0xC0) == 0x80)
		{
			if ((str - 1) >= (unsigned char *)str_base) //Ensuring str does not go out of bounds
			{
				str--;
			}
		}
		else
		{
			break;
		}
	}

	*str = '\0';
}

size_t gui_byte_length(const unsigned char byte)
{
	if ((byte & 0x80) == 0x00)
	{
		return 1;
	}
	else if ((byte & 0xE0) == 0xC0)
	{
		return 2;
	}
	else if ((byte & 0xF0) == 0xE0)
	{
		return 3;
	}
	else if ((byte & 0xF8) == 0xF0)
	{
		return 4;
	}
	else
	{
		return 0;
	}
}

unsigned long gui_code_point(const unsigned char *str, size_t byte_length)
{
	switch (byte_length)
	{
	case 1:
	{
		return

			str[0];

		break;
	}
	case 2:
	{
		return
		
			((unsigned long)(str[0] & 0x1F) << 6) |
			(str[1] & 0x3F);

		break;
	}
	case 3:
	{
		return

			((unsigned long)(str[0] & 0x0F) << 12) |
			((unsigned long)(str[1] & 0x3F) << 6) |
			(str[2] & 0x3F);

		break;
	}
	case 4:
	{
		return

			((unsigned long)(str[0] & 0x07) << 18) |
			((unsigned long)(str[1] & 0x3F) << 12) |
			((unsigned long)(str[2] & 0x3F) << 6) |
			(str[3] & 0x3F);

		break;
	}
	default:
	{
		return 0;
	}
	}
}

void gui_update_text(struct gui *v)
{
	//Create border so the text is not right against the edges of the box
	int box_x = 2;
	int box_y = 0;
	size_t box_width = v->w - BORDER_HORIZONTAL;
	size_t box_height = v->h - BORDER_VERTICAL;

	//Current x and y positions of cursor
	int xc = box_x;
	int yc = box_y;

	//Getting width and height of the texture
	size_t width = v->font->width;
	size_t height = v->font->height;

	//Set to num_boxes to skip the vertices that are reserved for the boxes
	size_t model_index = NUM_BOXES;

	char *str = v->text;

	for (size_t i = 0; i < str_len(str); i++)
	{
		if (str[i] == '\n' || str[i] == '\r')
		{
			xc = box_x;
			yc += (int)v->font->line_height;

			//Stop text from going out of the box
			if (yc + v->font->line_height > (box_y + box_height))
			{
				str[i] = '\0';

				break;
			}
		}
		else
		{
			size_t index = 0;

			size_t byte_length = gui_byte_length(str[i]);

			if (!byte_length)
			{
				break;
			}

			unsigned long code_point = gui_code_point((unsigned char*)&str[i], byte_length);

			i += byte_length - 1;

			for (size_t j = 0; j < v->font->mapping_count; j++)
			{
				size_t lower_bound = v->font->character_mappings[3 * j + 0];
				size_t upper_bound = v->font->character_mappings[3 * j + 1];

				if (code_point >= lower_bound && code_point <= upper_bound)
				{
					size_t offset = v->font->character_mappings[3 * j + 2];

					index = code_point - offset;

					break;
				}
			}

			int x = v->font->characters[7 * index + 0];
			int y = ((int)height - v->font->characters[7 * index + 1]);
			int w = v->font->characters[7 * index + 2];
			int h = v->font->characters[7 * index + 3];
			int x_off = v->font->characters[7 * index + 4];
			int y_off = v->font->characters[7 * index + 5];
			int x_adv = v->font->characters[7 * index + 6];

			//Advance to the next line and reset xc back to the left when the text reaches the end of the line
			if ((xc + x_off + w) > box_x + (int)box_width)
			{
				xc = box_x;
				yc += (int)v->font->line_height;

				//Text Wrap Test
				//if (str[i] != ' ')
				//{
				//	for (int k = i; k >= 0; k--)
				//	{
				//		if (str[k] == ' ' && !(yc + v->f->line_height > (box_y + box_height)))
				//		{
				//			str[k] = '\n';
				//			gui_update_text(v);
				//			return;
				//		}
				//	}
				//}
			}

			//Stop text from going out of the box
			if (yc + v->font->line_height > (box_y + box_height))
			{
				gui_delete_character((unsigned char *)&str[i], str);

				break;
			}

			//Texture Coordinates
			v->model.texture_coordinates[2 * (4 * model_index + 2) + 0] = (float)(x) / width;
			v->model.texture_coordinates[2 * (4 * model_index + 2) + 1] = (float)(y) / height;

			v->model.texture_coordinates[2 * (4 * model_index + 3) + 0] = (float)(x + w) / width;
			v->model.texture_coordinates[2 * (4 * model_index + 3) + 1] = (float)(y) / height;

			v->model.texture_coordinates[2 * (4 * model_index + 0) + 0] = (float)(x) / width;
			v->model.texture_coordinates[2 * (4 * model_index + 0) + 1] = (float)(y - h) / height;

			v->model.texture_coordinates[2 * (4 * model_index + 1) + 0] = (float)(x + w) / width;
			v->model.texture_coordinates[2 * (4 * model_index + 1) + 1] = (float)(y - h) / height;

			//Vertices
			v->model.vertices[3 * (4 * model_index + 2) + 0] = (float)(xc + x_off);
			v->model.vertices[3 * (4 * model_index + 2) + 1] = (float)v->h - (yc + y_off);

			v->model.vertices[3 * (4 * model_index + 3) + 0] = (float)(xc + x_off + w);
			v->model.vertices[3 * (4 * model_index + 3) + 1] = (float)v->h - (yc + y_off);

			v->model.vertices[3 * (4 * model_index + 0) + 0] = (float)(xc + x_off);
			v->model.vertices[3 * (4 * model_index + 0) + 1] = (float)v->h - (yc + y_off + h);

			v->model.vertices[3 * (4 * model_index + 1) + 0] = (float)(xc + x_off + w);
			v->model.vertices[3 * (4 * model_index + 1) + 1] = (float)v->h - (yc + y_off + h);

			xc += x_adv;

			model_index++;
		}
	}

	//Each character uses four vertices. j indicates how many vertices have been used so far. The remaining vertices are set to zero.
	for (size_t i = 4 * model_index; i < v->model.vertex_count; i++)
	{
		v->model.vertices[3 * i + 0] = 0;
		v->model.vertices[3 * i + 1] = 0;
		v->model.vertices[3 * i + 2] = 0; //Not required since mesh_create_2 uses calloc

		v->model.texture_coordinates[2 * i + 0] = 0;
		v->model.texture_coordinates[2 * i + 1] = 0;
	}
}

void gui_set_text(struct gui *v, const char *str)
{
	size_t len = str_len(str);

	if (len > v->char_limit)
	{
		len = v->char_limit;
	}

	memcpy(v->text, str, len * sizeof(*v->text));
	v->text[len] = '\0';

	gui_update_text(v);
}

void gui_set_text_int(struct gui *v, int value)
{
	char buffer[12];

	if (str_int_to_str(buffer, sizeof(buffer), value))
	{
		gui_set_text(v, buffer);
	}
}

void gui_set_text_float(struct gui *v, float value)
{
	char buffer[35];
	str_float_to_str(buffer, sizeof(buffer), value);

	gui_set_text(v, buffer);
}

char *gui_get_text(struct gui *v)
{
	return v->text;
}

void gui_add_child(struct gui *parent, struct gui *child)
{
	child->x += parent->x;
	child->y += parent->y;

	dynarray_push(&parent->children, &child);
}

struct gui *gui_create(struct font *font, struct gui *parent, int x, int y, size_t w, size_t h, size_t limit, unsigned char type)
{
	if (w <= BORDER_HORIZONTAL || h <= BORDER_VERTICAL)
	{
		return NULL;
	}

	struct gui *v = alloc_malloc(sizeof(*v));

	v->font = font;

	v->parent = parent;

	v->x = x;
	v->y = y;
	v->w = w;
	v->h = h;

	v->char_limit = limit;

	v->type = type;
	v->mouse_x = 0;
	v->mouse_y = 0;
	v->active = 0;
	v->visible = 1;
	v->active_child = 0;

	v->mouse_over = 0;
	v->mouse_down = 0;

	if (v->parent)
	{
		gui_add_child(v->parent, v);
	}

	//The additional 1 is for the null terminator
	v->text = alloc_calloc(v->char_limit + 1, sizeof(*v->text));

	mesh_create(&v->model, 2 * (NUM_BOXES + v->char_limit), 4 * (NUM_BOXES + v->char_limit), 1, 0, NULL);

	//Creating the boxes
	for (size_t i = 0; i < NUM_BOXES; i++)
	{
		//Texture Coordinates
		v->model.texture_coordinates[8 * i + 4] = -1;
		v->model.texture_coordinates[8 * i + 5] = -1;

		v->model.texture_coordinates[8 * i + 6] = -1;
		v->model.texture_coordinates[8 * i + 7] = -1;

		v->model.texture_coordinates[8 * i + 0] = -1;
		v->model.texture_coordinates[8 * i + 1] = -1;

		v->model.texture_coordinates[8 * i + 2] = -1;
		v->model.texture_coordinates[8 * i + 3] = -1;

		//Vertices
		v->model.vertices[12 * i + 6] = 0;
		v->model.vertices[12 * i + 7] = 0;
		v->model.vertices[12 * i + 8] = 0; //Not required since mesh_create_2 uses calloc

		v->model.vertices[12 * i + 9] = (float)(v->w);
		v->model.vertices[12 * i + 10] = 0;
		v->model.vertices[12 * i + 11] = 0; //Not required since mesh_create_2 uses calloc

		v->model.vertices[12 * i + 0] = 0;
		v->model.vertices[12 * i + 1] = (float)(v->h);
		v->model.vertices[12 * i + 2] = 0; //Not required since mesh_create_2 uses calloc

		v->model.vertices[12 * i + 3] = (float)(v->w);
		v->model.vertices[12 * i + 4] = (float)(v->h);
		v->model.vertices[12 * i + 5] = 0; //Not required since mesh_create_2 uses calloc
	}

	for (size_t i = 0; i < NUM_BOXES + v->char_limit; i++)
	{
		//Creating triangles in pairs to form rectangles
		v->model.indices[3 * (2 * i) + 0] = (unsigned int)(4 * i + 2);
		v->model.indices[3 * (2 * i) + 1] = (unsigned int)(4 * i + 0);
		v->model.indices[3 * (2 * i) + 2] = (unsigned int)(4 * i + 1);

		v->model.indices[3 * (2 * i) + 3] = (unsigned int)(4 * i + 1);
		v->model.indices[3 * (2 * i) + 4] = (unsigned int)(4 * i + 3);
		v->model.indices[3 * (2 * i) + 5] = (unsigned int)(4 * i + 2);
	}

	dynarray_create(&v->children, sizeof(struct gui *), NULL);

	if (type == GUI_SLIDER)
	{
		gui_create(v->font, v, 1, 1, 10, h - 2, 0, GUI_SLIDER_CHILD);
	}

	return v;
}

void gui_show(struct gui *v)
{
	v->visible = 1;
}

void gui_hide(struct gui *v)
{
	v->visible = 0;
}

void gui_set_state(struct gui *gui, unsigned char state)
{
	gui->active = state;
}

void gui_set_slider_position(struct gui *v, float p)
{
	if (v->type == GUI_SLIDER && dynarray_len(&v->children))
	{
		struct gui *child = *(struct gui **)dynarray_get(&v->children, 0);

		int min_x = v->x + 1;
		int max_x = v->x + (int)(v->w - child->w) - 1;

		int x = (int)(min_x + (max_x - min_x) * p);

		child->x = math_min_int(math_max_int(x, min_x), max_x);
	}
}

struct gui *gui_add_tab(struct gui *v, char *name, size_t w_tab, size_t h_page)
{
	if (v->type == GUI_TAB)
	{
		size_t total_width = 0;

		for (size_t i = 0; i < dynarray_len(&v->children); i++)
		{
			struct gui* child = *(struct gui **)dynarray_get(&v->children, i);

			total_width += child->w;
		}

		struct gui* tab = gui_create(v->font, v, (int)total_width, 0, w_tab, v->h, str_len(name), GUI_TAB_CHILD);
		gui_set_text(tab, name);

		struct gui* window = gui_create(v->font, tab, v->x - tab->x, -(int)h_page, v->w, h_page, 0, GUI_WINDOW);

		if (dynarray_len(&v->children) > 1)
		{
			gui_hide(window);
		}
		else
		{
			tab->active = 1;
		}

		return window;
	}
	else
	{
		return NULL;
	}
}

void gui_render_children(struct gui *v, struct camera *cam)
{
	if (v->visible)
	{
		if (v->type != GUI_ROOT)
		{
			char c[3] = {0};
			gui_get_color(v, c);

			render_draw_text(&v->model, cam, v->font->texture, v->mouse_over || v->active, v->x, v->y, c);
		}

		for (unsigned i = 0; i < dynarray_len(&v->children); i++)
		{
			gui_render_children(*(struct gui **)dynarray_get(&v->children, i), cam);
		}
	}
}

void gui_mouse_move_children(struct gui *v, int m_x, int m_y, struct dynarray *events)
{
	if (v->visible)
	{
		int left = v->x;
		int right = v->x + (int)v->w;
		int bottom = v->y;
		int top = v->y + (int)v->h;

		if (m_x >= left && m_x < right && m_y >= bottom && m_y < top && v->type != GUI_ROOT && v->type != GUI_LABEL && v->type != GUI_WINDOW && v->type != GUI_SLIDER && v->type != GUI_TAB)
		{
			v->mouse_over = 1;
		}
		else
		{
			v->mouse_over = 0;

			if (v->mouse_down)
			{
				v->mouse_down = 0;
			}
		}

		if (v->type == GUI_SLIDER_CHILD)
		{
			if (v->active)
			{
				int min_x = v->parent->x + 1;
				int max_x = v->parent->x + (int)(v->parent->w - v->w) - 1;

				v->x = math_min_int(math_max_int(m_x - v->mouse_x, min_x), max_x);

				float value = (float)(v->x - min_x) / (max_x - min_x);

				struct gui_event event_temp = {GUI_EVENT_SLIDER_MOVE, v->parent, 0, value};
				dynarray_insert(events, 0, &event_temp);
			}
		}

		for (unsigned i = 0; i < dynarray_len(&v->children); i++)
		{
			gui_mouse_move_children(*(struct gui **)dynarray_get(&v->children, i), m_x, m_y, events);
		}
	}
}

void gui_mouse_down_children(struct gui *v, int m_x, int m_y, struct dynarray *events, struct gui *original)
{
	if (v->visible)
	{
		if (v->mouse_over)
		{
			v->mouse_down = 1;

			v->mouse_x = m_x - v->x;
			v->mouse_y = m_y - v->y;
		}

		if (v->type == GUI_SLIDER_CHILD)
		{
			if (v->mouse_over)
			{
				original->active_child = 1;

				v->active = 1;
			}
		}

		for (unsigned i = 0; i < dynarray_len(&v->children); i++)
		{
			gui_mouse_down_children(*(struct gui **)dynarray_get(&v->children, i), m_x, m_y, events, original);
		}
	}
}

void gui_mouse_up_children(struct gui *v, struct dynarray *events, struct gui *original)
{
	if (v->visible)
	{
		if (v->type == GUI_TEXTBOX)
		{
			if (v->mouse_down)
			{
				original->active_child = 1;

				v->active = 1;

				v->mouse_down = 0;
			}
			else
			{
				v->active = 0;
			}
		}
		else if (v->type == GUI_BUTTON)
		{
			if (v->mouse_down)
			{
				v->mouse_down = 0;

				struct gui_event event_temp = {GUI_EVENT_BUTTON_PRESS, v, 0, 0};
				dynarray_insert(events, 0, &event_temp);
			}
		}
		else if (v->type == GUI_CHECKBOX)
		{
			if (v->mouse_down)
			{
				v->active = !v->active;

				v->mouse_down = 0;

				struct gui_event event_temp = {GUI_EVENT_CHECKBOX_PRESS, v, v->active, 0};
				dynarray_insert(events, 0, &event_temp);
			}
		}
		else if (v->type == GUI_SLIDER_CHILD)
		{
			if (v->mouse_down)
			{
				v->mouse_down = 0;
			}

			if (v->active)
			{
				v->active = !v->active;
			}
		}
		else if (v->type == GUI_TAB_CHILD)
		{
			if (v->mouse_down)
			{
				v->mouse_down = 0;

				struct gui *parent = v->parent;

				for (size_t i = 0; i < dynarray_len(&parent->children); i++)
				{
					struct gui *child = *(struct gui **)dynarray_get(&parent->children, i);

					child->active = 0;

					for (size_t j = 0; j < dynarray_len(&child->children); j++)
					{
						gui_hide(*(struct gui **)dynarray_get(&child->children, j));
					}
				}

				v->active = 1;

				for (size_t i = 0; i < dynarray_len(&v->children); i++)
				{
					gui_show(*(struct gui **)dynarray_get(&v->children, i));
				}
			}
		}

		for (unsigned i = 0; i < dynarray_len(&v->children); i++)
		{
			gui_mouse_up_children(*(struct gui **)dynarray_get(&v->children, i), events, original);
		}
	}
}

void gui_type_children(struct gui *v, const unsigned char *c)
{
	if (v->visible)
	{
		if (v->active && v->type == GUI_TEXTBOX)
		{
			size_t len = str_len(v->text);

			if (c[0] == '\b')
			{
				if (len)
				{
					gui_delete_character((unsigned char*)&v->text[len - 1], v->text);

					gui_update_text(v);
				}
			}
			else
			{
				size_t byte_length = gui_byte_length(c[0]);

				if (byte_length)
				{
					if (((len - 1) + byte_length) < v->char_limit)
					{
						memcpy(&v->text[len], c, byte_length * sizeof(*c));
						v->text[len + byte_length] = '\0';

						gui_update_text(v);
					}
				}
			}
		}

		for (unsigned i = 0; i < dynarray_len(&v->children); i++)
		{
			gui_type_children(*(struct gui **)dynarray_get(&v->children, i), c);
		}
	}
}