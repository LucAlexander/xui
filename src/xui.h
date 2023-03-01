#ifndef XUI_H
#define XUI_H

#include "xi_engine.h"

#define XUI_WINDOW_BAR_HEIGHT 16
#define XUI_WINDOW_MARGAIN 8
#define XUI_WINDOW_MIN_W 64
#define XUI_WINDOW_MIN_H 32

#define XUI_WINDOW_LAYER_UNFOCUSED 5
#define XUI_WINDOW_LAYER_FOCUSED 10

void register_xui_systems(program_state* state, xi_utils* xi);

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

uint32_t spawn_xui_window(xi_utils* xi, float x, float y, uint32_t w, uint32_t h, uint8_t resize, uint8_t move);
SYSTEM(xui_window_update);
SYSTEM(xui_window_draw);

VECTOR(xui_window_list, xui_window*)

typedef struct xui_window_manager{
	xui_window_list windows;
	xui_window* focused;
}xui_window_manager;

void xui_window_manager_init(xui_window_manager* manager);
void xui_window_manager_add_window(xui_window_manager* manager, xui_window* window);

#endif
