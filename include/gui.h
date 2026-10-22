#ifndef GUI_H
#define GUI_H

#include <stddef.h>

#include "dynarray.h"
#include "mesh.h"
#include "camera.h"
#include "font.h"

struct gui
{
	struct font *font;

	int x;
	int y;
	size_t w;
	size_t h;

	char *text;
	size_t char_limit;

	struct mesh model;

	unsigned char type;

	//Flags
	unsigned char mouse_over;
	unsigned char mouse_down;
	unsigned char active;
	unsigned char visible;
	unsigned char active_child;

	//Mouse position on element when clicked
	int mouse_x;
	int mouse_y;

	struct gui *parent;
	struct dynarray children;
};

struct gui_event
{
	int event;
	struct gui *id;
	int data;
	float data_float;
};

enum gui_events
{
	GUI_EVENT_BUTTON_PRESS,
	GUI_EVENT_CHECKBOX_PRESS,
	GUI_EVENT_SLIDER_MOVE
};

enum gui_types
{
	GUI_ROOT,
	GUI_LABEL,
	GUI_TEXTBOX,
	GUI_BUTTON,
	GUI_CHECKBOX,
	GUI_WINDOW,
	GUI_SLIDER,
	GUI_SLIDER_CHILD,
	GUI_TAB,
	GUI_TAB_CHILD
};

void gui_get_color(struct gui *v, char *c);

void gui_destroy_children(struct gui *v);

void gui_delete_character(unsigned char *str, const char *str_base);

size_t gui_byte_length(const unsigned char byte);

unsigned long gui_code_point(const unsigned char *str, size_t byte_length);

void gui_update_text(struct gui *v);

void gui_set_text(struct gui *v, const char *str);

void gui_set_text_int(struct gui *v, int value);

void gui_set_text_float(struct gui *v, float value);

char *gui_get_text(struct gui *v);

void gui_add_child(struct gui *parent, struct gui *child);

struct gui *gui_create(struct font *font, struct gui *parent, int x, int y, size_t w, size_t h, size_t limit, unsigned char type);

void gui_show(struct gui *v);

void gui_hide(struct gui *v);

void gui_set_state(struct gui *gui, unsigned char state);

void gui_set_slider_position(struct gui *v, float p);

struct gui *gui_add_tab(struct gui *v, char *name, size_t w_tab, size_t h_page);

void gui_render_children(struct gui *v, struct camera *cam);

void gui_mouse_move_children(struct gui *v, int m_x, int m_y, struct dynarray *events);

void gui_mouse_down_children(struct gui *v, int m_x, int m_y, struct dynarray *events, struct gui *original);

void gui_mouse_up_children(struct gui *v, struct dynarray *events, struct gui *original);

void gui_type_children(struct gui *v, const unsigned char *c);

#endif