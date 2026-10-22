#include "os.h"

#include <stddef.h>
#include <windows.h>

#include "str.h"
#include "alloc.h"

//Entry Point
int main(void);

__declspec(noreturn) void os_start()
{
	int exit_code = main();

	os_process_exit(exit_code);
}

//Error
static void os_error(int line)
{
	AttachConsole(ATTACH_PARENT_PROCESS);

	wchar_t buffer[256];
	wsprintfW(buffer, L"Error at line %d\n", line);

	HANDLE hconsole = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hconsole && (hconsole != INVALID_HANDLE_VALUE))
	{
		DWORD written;
		WriteConsoleW(hconsole, buffer, (DWORD)lstrlenW(buffer), &written, NULL);
	}

	ExitProcess(1);
}

//Memory
void *os_alloc_request_memory(size_t size)
{
	//This function must return properly aligned memory
	void *ptr = VirtualAlloc(NULL, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

	if (!ptr)
	{
		os_error(__LINE__);
	}

	return ptr;
}

static unsigned char os_utf8_to_utf16(wchar_t *utf16, const char *utf8, size_t utf16_len)
{
	if (!utf8 || !utf16 || utf16_len == 0)
	{
		return 0;
	}

	int len = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8, -1, NULL, 0);
	
	if (len <= 0 || (size_t)len > utf16_len)
	{
		return 0;
	}

	int result = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8, -1, utf16, (int)utf16_len);
	
	return result > 0;
}

static unsigned char os_utf16_to_utf8(char *utf8, const wchar_t *utf16, size_t utf8_len)
{
	if (!utf16 || !utf8 || utf8_len == 0)
	{
		return 0;
	}

	int len = WideCharToMultiByte(CP_UTF8, 0, utf16, -1, NULL, 0, NULL, NULL);
	
	if (len <= 0 || (size_t)len > utf8_len)
	{
		return 0;
	}

	int result = WideCharToMultiByte(CP_UTF8, 0, utf16, -1, utf8, (int)utf8_len, NULL, NULL);
	
	return result > 0;
}

//File
static char working_directory[MAXIMUM_PATH] = {0};

char *os_file_get_directory()
{
	if (str_len(working_directory) == 0)
	{
		wchar_t working_directory_utf16[MAXIMUM_PATH] = {0};

		if (!GetModuleFileNameW(NULL, working_directory_utf16, MAXIMUM_PATH))
		{
			os_error(__LINE__);
		}

		os_utf16_to_utf8(working_directory, working_directory_utf16, sizeof(working_directory));

		size_t count = 0;
		size_t len = str_len(working_directory);

		for (size_t i = len - 1; i < len; i--)
		{
			if (working_directory[i] == '\\')
			{
				count++;

				if (count == 2)
				{
					working_directory[i + 1] = '\0';

					break;
				}
			}
		}
	}

	return working_directory;
}

struct os_file_private
{
	HANDLE file;
};

unsigned char os_file_create(struct os_file *v, const char *file_name, char mode)
{
	wchar_t file_name_utf16[MAXIMUM_PATH] = {0};
	os_utf8_to_utf16(file_name_utf16, file_name, sizeof(file_name_utf16));

	HANDLE file = NULL;

	if (mode == 'r')
	{
		file = CreateFileW(file_name_utf16, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	}
	else if (mode == 'w')
	{
		file = CreateFileW(file_name_utf16, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	}

	if (file != INVALID_HANDLE_VALUE)
	{
		v->private_data = alloc_malloc(sizeof(struct os_file_private));

		struct os_file_private *priv = v->private_data;
		priv->file = file;

		return 1;
	}
	else
	{
		return 0;
	}
}

void os_file_destroy(struct os_file *v)
{
	struct os_file_private *priv = v->private_data;

	if (!CloseHandle(priv->file))
	{
		os_error(__LINE__);
	}

	alloc_free(v->private_data);
}

size_t os_file_size(struct os_file *v)
{
	struct os_file_private *priv = v->private_data;

	LARGE_INTEGER size;

	if (!GetFileSizeEx(priv->file, &size))
	{
		os_error(__LINE__);
	}

	return (size_t)size.LowPart;
}

void os_file_read(struct os_file *v, void *buffer, size_t size)
{
	struct os_file_private *priv = v->private_data;

	unsigned long size_read;

	if (!ReadFile(priv->file, buffer, (DWORD)size, &size_read, NULL))
	{
		os_error(__LINE__);
	}
}

void os_file_write(struct os_file *v, void *buffer, size_t size)
{
	struct os_file_private *priv = v->private_data;

	unsigned long size_read;

	if (!WriteFile(priv->file, buffer, (DWORD)size, &size_read, NULL))
	{
		os_error(__LINE__);
	}
}

void os_file_clear(const char *file_name)
{
	wchar_t file_name_utf16[MAXIMUM_PATH] = {0};
	os_utf8_to_utf16(file_name_utf16, file_name, sizeof(file_name_utf16));

	HANDLE file = CreateFileW(file_name_utf16, FILE_APPEND_DATA, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (file != INVALID_HANDLE_VALUE)
	{
		if (!CloseHandle(file))
		{
			os_error(__LINE__);
		}
	}
}

//File_directory
struct os_file_directory_private
{
	WIN32_FIND_DATAW win32_find_dataw;
	HANDLE hfind;
};

unsigned char os_file_directory_create(struct os_file_directory *v, const char *directory_name)
{
	v->type = -1;
	memset(&v->file_name, 0, sizeof(v->file_name));

	char directory[MAXIMUM_PATH] = {0};
	str_cat(directory, directory_name, sizeof(directory));
	str_cat(directory, "\\*", sizeof(directory));

	wchar_t directory_utf16[MAXIMUM_PATH] = {0};
	os_utf8_to_utf16(directory_utf16, directory, sizeof(directory_utf16));

	WIN32_FIND_DATAW win32_find_dataw = {0};

	HANDLE hfind = FindFirstFileW(directory_utf16, &win32_find_dataw);

	if (hfind != INVALID_HANDLE_VALUE)
	{
		if (FindNextFileW(hfind, &win32_find_dataw))
		{
			v->private_data = alloc_malloc(sizeof(struct os_file_directory_private));

			struct os_file_directory_private *priv = v->private_data;

			memcpy(&priv->win32_find_dataw, &win32_find_dataw, sizeof(win32_find_dataw));
			priv->hfind = hfind;

			return 1;
		}
		else if (GetLastError() == ERROR_NO_MORE_FILES)
		{
			return 0;
		}
		else
		{
			os_error(__LINE__);

			return 0;
		}
	}
	else
	{
		return 0;
	}
}

unsigned char os_file_directory_next(struct os_file_directory *v)
{
	struct os_file_directory_private *priv = v->private_data;

	if (FindNextFileW(priv->hfind, &priv->win32_find_dataw))
	{
		if (priv->win32_find_dataw.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY)
		{
			v->type = 1;
		}
		else
		{
			v->type = 0;
		}

		os_utf16_to_utf8(v->file_name, priv->win32_find_dataw.cFileName, sizeof(working_directory));

		return 1;
	}
	else if (GetLastError() == ERROR_NO_MORE_FILES)
	{
		return 0;
	}
	else
	{
		os_error(__LINE__);

		return 0;
	}
}

void os_file_directory_destroy(struct os_file_directory *v)
{
	struct os_file_directory_private *priv = v->private_data;

	if (!FindClose(priv->hfind))
	{
		os_error(__LINE__);
	}

	alloc_free(v->private_data);
}

//Time
int os_time_tick_count()
{
	return (int)GetTickCount();
}

//Window
struct os_window_private
{
	wchar_t high_surrogate;
	HINSTANCE hinstance;
	HWND hwnd;
	HICON hicon;
	wchar_t name[MAXIMUM_PATH]; //Not ideal for this to be MAXIMUM_PATH, though unlikely to exceed
};

static struct os_event events[20];
static unsigned char events_length = 0;

static void os_event_add(struct os_event *evt)
{
	if (events_length < sizeof(events) / sizeof(struct os_event))
	{
		memmove(&events[1], &events[0], events_length * sizeof(*events));
		events[0] = *evt;
		events_length++;
	}
}

static int os_key_convert(UINT msg, WPARAM wParam)
{
	int key = -1;

	if (wParam >= ' ' && wParam <= '~')
	{
		key = (int)wParam;
	}

	switch (wParam)
	{
	case VK_LEFT:
	{
		key = OS_KEY_LEFT;

		break;
	}
	case VK_RIGHT:
	{
		key = OS_KEY_RIGHT;

		break;
	}
	case VK_UP:
	{
		key = OS_KEY_UP;

		break;
	}
	case VK_DOWN:
	{
		key = OS_KEY_DOWN;

		break;
	}
	case VK_ESCAPE:
	{
		key = OS_KEY_ESCAPE;

		break;
	}
	case VK_RETURN:
	{
		key = OS_KEY_ENTER;

		break;
	}
	case VK_SHIFT:
	{
		if (((GetAsyncKeyState(VK_LSHIFT) & 0x8000) && (msg == WM_KEYDOWN)) || (!(GetAsyncKeyState(VK_LSHIFT) & 0x8000) && (msg == WM_KEYUP)))
		{
			key = OS_KEY_LSHIFT;
		}

		break;
	}
	}

	return key;
}

static LRESULT CALLBACK os_window_wnd_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	SetLastError(0);

	struct os_window *v = (struct os_window *)GetWindowLongPtrW(hwnd, GWLP_USERDATA);

	if (!v && GetLastError())
	{
		os_error(__LINE__);
	}

	struct os_window_private *priv = NULL;

	if (v)
	{
		priv = v->private_data;
	}

	struct os_event evt = {0};
	evt.wnd = v;

	switch (msg)
	{
	case WM_NCCREATE:
	{
		CREATESTRUCT *createstruct = (CREATESTRUCT *)lParam;
		v = (struct os_window *)createstruct->lpCreateParams;

		SetLastError(0);

		if (!SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR)v) && GetLastError())
		{
			os_error(__LINE__);
		}

		break;
	}
	case WM_CLOSE:
	{
		PostQuitMessage(0);

		break;
	}
	case WM_KILLFOCUS:
	{
		evt.event = OS_EVENT_LOSE_FOCUS;
		evt.id = (int)wParam;
		evt.data = (int)lParam;

		os_event_add(&evt);

		break;
	}
	case WM_ENTERSIZEMOVE:
	{
		v->resizing_flag = 1;

		break;
	}
	case WM_EXITSIZEMOVE:
	{
		v->resizing_flag = 0;

		RECT rect;
		if (!GetClientRect(hwnd, &rect))
		{
			os_error(__LINE__);
		}

		size_t w_new = (size_t)rect.right;
		size_t h_new = (size_t)rect.bottom;

		if (v->w != w_new || v->h != h_new)
		{
			evt.event = OS_EVENT_RESIZE;
			evt.id = (int)w_new;
			evt.data = (int)h_new;

			os_event_add(&evt);

			v->w = w_new;
			v->h = h_new;
		}

		break;
	}
	case WM_SIZE:
	{
		switch (wParam)
		{
		case SIZE_MAXIMIZED:
		case SIZE_RESTORED:
		{
			if (!v->resizing_flag)
			{
				RECT rect;
				if (!GetClientRect(hwnd, &rect))
				{
					os_error(__LINE__);
				}

				size_t w_new = (size_t)rect.right;
				size_t h_new = (size_t)rect.bottom;

				if (v->w != w_new || v->h != h_new)
				{
					evt.event = OS_EVENT_RESIZE;
					evt.id = (int)w_new;
					evt.data = (int)h_new;

					os_event_add(&evt);

					v->w = w_new;
					v->h = h_new;
				}
			}

			break;
		}
		}

		break;
	}
	case WM_CHAR:
	{
		//NOTE: STILL HAS ISSUES WITH EXTRA MESSAGES FOR COLOR, SKIN TONE, ETC. SHOULD DEAL WITH IN GUI

		wchar_t w_param[3] = {0};

		if (IS_HIGH_SURROGATE(wParam))
		{
			if (priv->high_surrogate)
			{
				priv->high_surrogate = 0;

				break; //High surrogate but high surrogate already stored
			}
			else
			{
				priv->high_surrogate = (wchar_t)wParam;

				break;
			}
		}
		else if (IS_LOW_SURROGATE(wParam))
		{
			if (priv->high_surrogate)
			{
				w_param[0] = priv->high_surrogate;
				w_param[1] = (wchar_t)wParam;
				w_param[2] = L'\0';

				priv->high_surrogate = 0;
			}
			else
			{
				break; //Low surrogate but no stored high surrogate
			}
		}
		else
		{
			w_param[0] = (wchar_t)wParam;
			w_param[1] = L'\0';
		}

		evt.event = OS_EVENT_TYPE;

		char utf8[5] = {0};

		os_utf16_to_utf8(utf8, w_param, sizeof(utf8));

		memcpy(evt.wide, utf8, sizeof(evt.wide));

		os_event_add(&evt);

		break;
	}
	case WM_LBUTTONDOWN:
	{
		evt.event = OS_EVENT_LEFT_MOUSE_DOWN;
		evt.id = ((int)(short)LOWORD(lParam));
		evt.data = ((int)v->h - 1) - ((int)(short)HIWORD(lParam));

		SetCapture(hwnd);

		os_event_add(&evt);

		break;
	}
	case WM_LBUTTONUP:
	{
		evt.event = OS_EVENT_LEFT_MOUSE_UP;

		if (!ReleaseCapture())
		{
			os_error(__LINE__);
		}

		os_event_add(&evt);

		break;
	}
	case WM_RBUTTONDOWN:
	{
		evt.event = OS_EVENT_RIGHT_MOUSE_DOWN;
		evt.id = ((int)(short)LOWORD(lParam));
		evt.data = ((int)v->h - 1) - ((int)(short)HIWORD(lParam));

		SetCapture(hwnd);

		os_event_add(&evt);

		break;
	}
	case WM_RBUTTONUP:
	{
		evt.event = OS_EVENT_RIGHT_MOUSE_UP;

		if (!ReleaseCapture())
		{
			os_error(__LINE__);
		}

		os_event_add(&evt);

		break;
	}
	case WM_MOUSEMOVE:
	{
		evt.event = OS_EVENT_MOUSE_MOVE;
		evt.id = ((int)(short)LOWORD(lParam));
		evt.data = ((int)v->h - 1) - ((int)(short)HIWORD(lParam));

		os_event_add(&evt);

		break;
	}
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN: //Must check SYSKEY for alt and f10 inputs
	{
		int key = os_key_convert(msg, wParam);

		if (key != -1)
		{
			evt.event = OS_EVENT_KEY_DOWN;
			evt.id = key;

			if (!(lParam & 0x40000000) || v->autorepeat) //Check for autorepeat
			{
				evt.data = 1;
			}

			if (evt.data)
			{
				v->keystates[key] = 1;
			}

			os_event_add(&evt);
		}

		break;
	}
	case WM_KEYUP:
	case WM_SYSKEYUP:
	{
		int key = os_key_convert(msg, wParam);

		if (key != -1)
		{
			evt.event = OS_EVENT_KEY_UP;
			evt.id = key;

			v->keystates[key] = 0;

			os_event_add(&evt);
		}

		break;
	}
	case WM_NCLBUTTONDOWN:
	{
		evt.event = OS_EVENT_LOSE_FOCUS;

		os_event_add(&evt);

		break;
	}
	}

	return DefWindowProcW(hwnd, msg, wParam, lParam);
}

void os_window_create(struct os_window *v, const char *name, size_t w, size_t h, unsigned char *texture, size_t texture_w, size_t texture_h)
{
	if (name)
	{
		v->private_data = alloc_malloc(sizeof(struct os_window_private));

		struct os_window_private* priv = v->private_data;

		priv->high_surrogate = 0;

		v->w = w;
		v->h = h;

		//Initialize key states to zero
		memset(v->keystates, 0, 256 * sizeof(*v->keystates));
		v->autorepeat = 0;

		v->resizing_flag = 0;

		//Setting name and hInstance
		os_utf8_to_utf16(priv->name, name, sizeof(priv->name));

		priv->hinstance = GetModuleHandleW(NULL);

		if (!priv->hinstance)
		{
			os_error(__LINE__);
		}

		//Icon Setup
		if (texture)
		{
			HBITMAP img = CreateBitmap((int)texture_w, (int)texture_h, 1, 32, texture);

			if (!img)
			{
				os_error(__LINE__);
			}

			ICONINFO iconinfo = {0};
			iconinfo.hbmMask = img;
			iconinfo.hbmColor = img;

			priv->hicon = CreateIconIndirect(&iconinfo);

			if (!priv->hicon)
			{
				os_error(__LINE__);
			}

			if (!DeleteObject(img))
			{
				os_error(__LINE__);
			}
		}
		else
		{
			priv->hicon = NULL;
		}

		//Register Class
		WNDCLASSEXW wc = {0};
		wc.cbSize = sizeof(wc);
		wc.lpfnWndProc = os_window_wnd_proc;
		wc.hInstance = priv->hinstance;
		wc.hIcon = priv->hicon;
		wc.lpszClassName = priv->name;
		wc.hIconSm = priv->hicon;
		wc.cbWndExtra = sizeof(v);
		wc.hCursor = LoadCursorW(NULL, (LPCWSTR)IDC_ARROW);

		if (!wc.hCursor)
		{
			os_error(__LINE__);
		}

		if (!RegisterClassExW(&wc))
		{
			os_error(__LINE__);
		}

		RECT rect = {0};
		rect.left = 0;
		rect.top = 0;
		rect.right = (LONG)w;
		rect.bottom = (LONG)h;

		//DWORD dwStyle = WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_VISIBLE | WS_THICKFRAME;
		DWORD dwStyle = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
		DWORD dwExStyle = WS_EX_LEFT;

		if (!AdjustWindowRectEx(&rect, dwStyle, 0, dwExStyle))
		{
			os_error(__LINE__);
		}

		priv->hwnd = CreateWindowExW(dwExStyle, priv->name, priv->name, dwStyle, 100, 100, rect.right - rect.left, rect.bottom - rect.top, NULL, NULL, priv->hinstance, v);

		if (!priv->hwnd)
		{
			os_error(__LINE__);
		}
	}
}

void os_window_destroy(void *ptr)
{
	struct os_window *v = ptr;

	struct os_window_private *priv = v->private_data;

	if (!UnregisterClassW(priv->name, priv->hinstance))
	{
		os_error(__LINE__);
	}

	if (priv->hicon && !DestroyIcon(priv->hicon)) //Must check priv->hicon is not NULL
	{
		os_error(__LINE__);
	}

	alloc_free(v->private_data);
}

unsigned char os_window_get_event(struct os_event *evt)
{
	MSG msg;

	while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE))
	{
		if (msg.message == WM_QUIT)
		{
			evt->event = OS_EVENT_END;

			os_event_add(evt);

			break;
		}

		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}

	if (events_length)
	{
		events_length--;

		memcpy(evt, &events[events_length], sizeof(*evt));

		return 1;
	}
	else
	{
		return 0;
	}
}

void os_window_draw(struct os_window *v, unsigned char *pixels, size_t w, size_t h)
{
	struct os_window_private *priv = v->private_data;

	BITMAPINFO bitmapinfo = {0};
	bitmapinfo.bmiHeader.biSize = sizeof(bitmapinfo.bmiHeader);
	bitmapinfo.bmiHeader.biWidth = (LONG)w;
	bitmapinfo.bmiHeader.biHeight = (LONG)h;
	bitmapinfo.bmiHeader.biPlanes = 1;
	bitmapinfo.bmiHeader.biBitCount = 32;
	bitmapinfo.bmiHeader.biCompression = BI_RGB;

	HDC hdc = GetDC(priv->hwnd);

	if (!hdc)
	{
		os_error(__LINE__);
	}

	if (!StretchDIBits(hdc, 0, 0, bitmapinfo.bmiHeader.biWidth, bitmapinfo.bmiHeader.biHeight, 0, 0, bitmapinfo.bmiHeader.biWidth, bitmapinfo.bmiHeader.biHeight, pixels, &bitmapinfo, DIB_RGB_COLORS, SRCCOPY))
	{
		os_error(__LINE__);
	}

	if (!ReleaseDC(priv->hwnd, hdc))
	{
		os_error(__LINE__);
	}
}

//Mouse

void os_mouse_show()
{
	while (ShowCursor(TRUE) < 0);
}

void os_mouse_hide()
{
	while (ShowCursor(FALSE) >= 0);
}

void os_mouse_set_position(struct os_window *window, int x, int y)
{
	POINT point = {0};
	point.x = x;
	point.y = ((int)window->h - 1) - y;

	struct os_window_private *priv = window->private_data;

	if (!ClientToScreen(priv->hwnd, &point))
	{
		os_error(__LINE__);
	}

	if (!SetCursorPos(point.x, point.y))
	{
		os_error(__LINE__);
	}
}

//Process
void os_process_exit(int exit_code)
{
	ExitProcess(exit_code);
}