#ifndef OS_H
#define OS_H

#include <stddef.h>

int _fltused; //Needed when default libraries are being ignored

//Mem
void *memset(void *dst, int value, size_t size);

void *memcpy(void *dst, const void* src, size_t size);

void *memmove(void *dst, const void* src, size_t size);

//Memory

//Returns an aligned pointer
void *os_alloc_request_memory(size_t size);

//File
#define MAXIMUM_PATH 260

char *os_file_get_directory();

struct os_file
{
	void *private_data;
};

unsigned char os_file_create(struct os_file *v, const char *file_name, char mode);

void os_file_destroy(struct os_file *v);

size_t os_file_size(struct os_file *v);

void os_file_read(struct os_file *v, void *buffer, size_t size);

void os_file_write(struct os_file *v, void *buffer, size_t size);

void os_file_clear(const char *file_name);

//File_directory
struct os_file_directory
{
	int type;
	char file_name[MAXIMUM_PATH];

	void *private_data;
};

unsigned char os_file_directory_create(struct os_file_directory *v, const char *directory_name);

unsigned char os_file_directory_next(struct os_file_directory *v);

void os_file_directory_destroy(struct os_file_directory *v);

//Time
int os_time_tick_count();

//Window
struct os_window
{
	size_t w;
	size_t h;

	unsigned char keystates[256];
	int autorepeat;

	unsigned char resizing_flag;

	void *private_data;
};

//Events
enum os_events
{
	OS_EVENT_OTHER, //OTHER must be the first
	OS_EVENT_KEY_UP,
	OS_EVENT_LEFT_MOUSE_DOWN,
	OS_EVENT_LEFT_MOUSE_UP,
	OS_EVENT_RIGHT_MOUSE_DOWN,
	OS_EVENT_RIGHT_MOUSE_UP,
	OS_EVENT_MOUSE_MOVE,
	OS_EVENT_TYPE,
	OS_EVENT_END,
	OS_EVENT_LOSE_FOCUS,
	OS_EVENT_KEY_DOWN,
	OS_EVENT_RESIZE
};

struct os_event
{
	int event;
	int id;
	int data;
	unsigned char wide[4];
	struct os_window *wnd;
};

enum os_key
{
	OS_KEY_LEFT,
	OS_KEY_RIGHT,
	OS_KEY_UP,
	OS_KEY_DOWN,
	OS_KEY_ESCAPE,
	OS_KEY_ENTER,
	OS_KEY_LSHIFT
};

void os_window_create(struct os_window *v, const char *name, size_t w, size_t h, unsigned char *texture, size_t texture_w, size_t texture_h);

void os_window_destroy(void *ptr);

unsigned char os_window_get_event(struct os_event *evt);

void os_window_draw(struct os_window *v, unsigned char *pixels, size_t w, size_t h);

//Mouse

void os_mouse_show();

void os_mouse_hide();

void os_mouse_set_position(struct os_window *window, int x, int y);

//Process
void os_process_exit(int exit_code);

#endif