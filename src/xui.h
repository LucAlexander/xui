#ifndef XUI_H
#define XUI_H

#include "xi_engine.h"

#define XUI_WINDOW_BAR_HEIGHT 24
#define XUI_WINDOW_MARGAIN 8
#define XUI_WINDOW_MIN_W 64
#define XUI_WINDOW_MIN_H 32

#define XUI_WINDOW_LAYER_UNFOCUSED 5
#define XUI_WINDOW_LAYER_FOCUSED 10

#define XUI_TEXT_MAX 64
#define XUI_UNFOCUSED_SCALEFACTOR 1.2
#define XUI_TAB_W 4

void register_xui_systems(program_state* state, xi_utils* xi);

typedef struct xui_color{
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;
}xui_color;

xui_color xui_color_decode(uint32_t color);

typedef enum XUI_WINDOW_STATE{
	XUI_WINDOW_IDLE,
	XUI_WINDOW_MOVE,
	XUI_WINDOW_RESIZE
}XUI_WINDOW_STATE;

typedef enum XUI_WINDOW_FLAGS{
	XUI_WINDOW_CAN_RESIZE,
	XUI_WINDOW_CAN_MOVE
}XUI_WINDOW_FLAGS;

typedef enum XUI_WINDOW_RESIZE_DIR{
	XUI_WINDOW_RESIZE_LEFT=1,
	XUI_WINDOW_RESIZE_RIGHT=2
}XUI_WINDOW_RESIZE_DIR;

struct xui_window_manager;

typedef struct xui_window{
	uint32_t w;
	uint32_t h;
	uint32_t start_x;
	uint32_t start_y;
	XUI_WINDOW_FLAGS flags;
	XUI_WINDOW_STATE state;
	struct xui_window_manager* manager;
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;
}xui_window;

uint32_t spawn_xui_window(xi_utils* xi, float x, float y, uint32_t w, uint32_t h, uint8_t resize, uint8_t move, uint32_t color);
SYSTEM(xui_window_update);
SYSTEM(xui_window_draw);

VECTOR(xui_window_list, xui_window*)

typedef struct xui_window_manager{
	xui_window_list windows;
	xui_window* focused;
}xui_window_manager;

void xui_window_manager_init(xui_window_manager* manager);
void xui_window_manager_add_window(xui_window_manager* manager, xui_window* window);

typedef struct xui_widget{
	uint32_t window;
	uint32_t x;
	uint32_t y;
	uint32_t local_depth;
}xui_widget;

SYSTEM(xui_widget_mutate);

#define XUI_PANEL_LOCAL_DEPTH 1

typedef struct xui_panel{
	uint32_t w;
	uint32_t h;
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;
	uint8_t border_r;
	uint8_t border_g;
	uint8_t border_b;
	uint8_t border_a;
}xui_panel;

uint32_t spawn_xui_panel(xi_utils* xi, uint32_t window, uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color, uint32_t border_color);
SYSTEM(xui_panel_render);

typedef struct xui_button{
	void (*f)(SYSTEM_ARG_REQUIREMENTS);
	uint8_t hover_r;
	uint8_t hover_g;
	uint8_t hover_b;
	uint8_t hover_a;
}xui_button;

uint32_t spawn_xui_button(xi_utils* xi, uint32_t window, uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color, uint32_t border_color, uint32_t hover_color, void (*f)(SYSTEM_ARG_REQUIREMENTS));
SYSTEM(xui_button_mutate);
SYSTEM(xui_button_render);

#define XUI_TEXT_LOCAL_DEPTH 3

typedef struct xui_text{
	char text[XUI_TEXT_MAX];
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;
}xui_text;

uint32_t spawn_xui_text(xi_utils* xi, uint32_t window, uint32_t x, uint32_t y, char* text, uint32_t color);
SYSTEM(xui_text_render);

#define XUI_BLITABLE_LOCAL_DEPTH 2

uint32_t spawn_xui_blitable(xi_utils* xi, uint32_t window, uint32_t x, uint32_t y, uint32_t w, uint32_t h, char* src);
SYSTEM(xui_blitable_render);

typedef struct xui_radio{
	uint32_t w;
	uint32_t h;
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;
	uint8_t hover_r;
	uint8_t hover_g;
	uint8_t hover_b;
	uint8_t hover_a;
	uint8_t value;
}xui_radio;

uint32_t spawn_xui_radio(xi_utils* xi, uint32_t window, uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t on_color, uint32_t hover_color);
SYSTEM(xui_radio_mutate);
SYSTEM(xui_radio_render);

typedef enum XUI_SLIDER_DIR{
	XUI_SLIDER_X=0,
	XUI_SLIDER_Y
}XUI_SLIDER_DIR;

typedef struct xui_slider{
	float min;
	float max;
	float position;
	uint32_t nob_w;
	uint32_t nob_h;
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;
	XUI_SLIDER_DIR dir;
}xui_slider;

uint32_t spawn_xui_slider(xi_utils* xi, uint32_t window, uint32_t x, uint32_t y, uint32_t nob_w, uint32_t nob_h, float min, float max, uint32_t color, XUI_SLIDER_DIR dir);
SYSTEM(xui_slider_mutate);
SYSTEM(xui_slider_render);

#define XUI_TEXTENTRY_ALLOWED_KEYS "1234567890-=qazwsxedcrfvtgbyhnujmik,./;'lop[]\\" 
#define XUI_TEXTENTRY_ALLOWED_KEYS_LEN strlen(XUI_TEXTENTRY_ALLOWED_KEYS)

typedef struct xui_textentry{
	char text[XUI_TEXT_MAX];
	uint32_t w;
	uint32_t h;
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;
	uint8_t selected;
	uint8_t position;
}xui_textentry;

void strins(char* source, char* str, uint32_t index);
void strcut(char* source, int32_t index);

uint32_t spawn_xui_textentry(xi_utils* xi, uint32_t window, uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t text_color);
SYSTEM(xui_textentry_mutate);
SYSTEM(xui_textentry_render);

#define XUI_SHELL_LINE_COUNT 32
#define XUI_SHELL_TAB "    "

typedef struct xui_shell{
	char text[XUI_SHELL_LINE_COUNT][XUI_TEXT_MAX];
	uint8_t target;
	uint8_t position;
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;
	uint8_t scroll;
	char* (*f)(SYSTEM_ARG_REQUIREMENTS, struct xui_shell*);
	void* system;
}xui_shell;

int32_t find_ch_index(char string[], char ch);
void xui_shell_new_line(xui_shell* shell);
void xui_shell_buffer_output(xui_shell* shell, char* output);
void xui_shell_scroll_to_current(xui_shell* shell, uint32_t visible_h);
char* xui_shell_default_shell(SYSTEM_ARG_REQUIREMENTS, xui_shell*);
uint32_t spawn_xui_shell(xi_utils* xi, uint32_t window, uint32_t x, uint32_t y, uint32_t text_color, char* (*f)(SYSTEM_ARG_REQUIREMENTS, xui_shell*), void* system);
SYSTEM(xui_shell_mutate);
SYSTEM(xui_shell_render);

#endif
