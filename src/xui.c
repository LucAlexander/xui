#include "xui.h"
#include "project.h"

VECTOR_SOURCE(xui_window_list, xui_window*)

void register_xui_systems(program_state* state, xi_utils* xi){
	system_add(state, system_init(xui_window_update, 2, POSITION_C, XUI_WINDOW_C), XI_STATE_UPDATE);
	system_add(state, system_init(xui_window_draw, 2, POSITION_C, XUI_WINDOW_C), XI_STATE_RENDER);
}

uint32_t spawn_xui_window(xi_utils* xi, float x, float y, uint32_t w, uint32_t h, uint8_t resize, uint8_t move){
	uint32_t entity = entity_create(xi->ecs);
	v2 position = {x, y};
	xui_window window;
	window.w = w;
	window.h = h;
	window.start_x = 0;
	window.start_y = 0;
	window.flags = (resize*(1<<XUI_WINDOW_CAN_RESIZE)) | (move*(1<<XUI_WINDOW_CAN_MOVE));
	window.state = XUI_WINDOW_IDLE;
	window.manager = &xi->project->window_manager;
	window.r = 64;
	window.g = 255;
	window.b = 64;
	window.a = 128;
	component_add(xi->ecs, entity, POSITION_C, &position);
	component_add(xi->ecs, entity, XUI_WINDOW_C, &window);
	xui_window_manager_add_window(&xi->project->window_manager, component_get(xi->ecs, entity, XUI_WINDOW_C));
	return entity;
}

SYSTEM(xui_window_update){
	ARG(v2* position, POSITION_C);
	ARG(xui_window* window, XUI_WINDOW_C);
	v2 mouse = mousePos(xi->user_input);
	uint8_t leftPressed = mousePressed(xi->user_input, 1);
	uint8_t leftReleased = mouseReleased(xi->user_input, 1);
	uint8_t focused = xi->project->window_manager.focused == window;
	entity_set_layer(xi->ecs, id, XUI_WINDOW_LAYER_UNFOCUSED);
	if (!focused) return;
	entity_set_layer(xi->ecs, id, XUI_WINDOW_LAYER_FOCUSED);
	switch(window->state){
		case XUI_WINDOW_IDLE:{
			if (!leftPressed) return;
			if ( // touching the top bar
				mouse.x > position->x &&
				mouse.y > position->y &&
				mouse.x < position->x+window->w &&
				mouse.y < position->y + XUI_WINDOW_MARGAIN
			){
				window->state = XUI_WINDOW_MOVE;
				window->start_x = mouse.x - position->x;
				window->start_y = mouse.y - position->y;
				return;
			}
			if ( // touching the right or left edges
				mouse.y > position->y && 
				mouse.y < position->y + window->h
			){
				if (mouse.x > position->x && mouse.x < position->x + XUI_WINDOW_MARGAIN){
					window->state = XUI_WINDOW_RESIZE;
					window->start_x = XUI_WINDOW_RESIZE_LEFT;
				}
				if (mouse.x < position->x + window->w && mouse.x > position->x + window->w - XUI_WINDOW_MARGAIN){
					window->state = XUI_WINDOW_RESIZE;
					window->start_x = XUI_WINDOW_RESIZE_RIGHT;
				}
			}
			if ( // touching the bottom
				mouse.x > position->x &&
				mouse.x < position->x + window->w &&
				mouse.y > position->y + window->h - XUI_WINDOW_MARGAIN &&
				mouse.y < position->y + window->h
			){
				window->state = XUI_WINDOW_RESIZE;
				window->start_y = 1;
			}

		}break;
		case XUI_WINDOW_MOVE:{
			if (((window->flags & (1<<XUI_WINDOW_CAN_MOVE)) == 0)) return;
			position->x = mouse.x - window->start_x;
			position->y = mouse.y - window->start_y;
			if (leftReleased){
				window->state = XUI_WINDOW_IDLE;
				window->start_x = 0;
				window->start_y = 0;
			}
		}break;
		case XUI_WINDOW_RESIZE:{
			if (((window->flags & (1<<XUI_WINDOW_CAN_MOVE)) == 0)) return;
			if (leftReleased){
				window->state = XUI_WINDOW_IDLE;
				window->start_x = 0;
				window->start_y = 0;
				return;
			}
			int32_t new_size;
			if (window->start_y){
				new_size = mouse.y - position->y;
				if (new_size <= XUI_WINDOW_MIN_H) return;
				window->h = new_size;
			}
			switch(window->start_x){
				case XUI_WINDOW_RESIZE_LEFT:{
					new_size = (position->x + window->w) - mouse.x;
					if (new_size <= XUI_WINDOW_MIN_W) return;
					window->w = new_size;
					position->x = mouse.x;
				}break;
				case XUI_WINDOW_RESIZE_RIGHT:{
					new_size = mouse.x - position->x;
					if (new_size <= XUI_WINDOW_MIN_W) return;
					window->w = new_size;
				}break;
			}
		}break;
	}
}

SYSTEM(xui_window_draw){
	ARG(v2* position, POSITION_C);
	ARG(xui_window* window, XUI_WINDOW_C);
	uint8_t alpha = window->a;
	if (xi->project->window_manager.focused != window){
		alpha /= 2;
	}
	renderSetColor(xi->graphics, window->r, window->g, window->b, alpha);
	drawRect(xi->graphics, position->x, position->y, window->w, window->h, FILL);
	v2 mouse = mousePos(xi->user_input);
	if (
		mouse.x > position->x &&
		mouse.y > position->y &&
		mouse.x < position->x + window->w &&
		mouse.y < position->y + window->h && 
		mousePressed(xi->user_input, 1)
	){
		xi->project->window_manager.focused = window;
	}
	renderSetColor(xi->graphics, 0, 0, 0, 0);
}

void xui_window_manager_init(xui_window_manager* manager){
	fflush(stdout);
	manager->windows = xui_window_listInit();
	manager->focused = NULL;
}

void xui_window_manager_add_window(xui_window_manager* manager, xui_window* window){
	xui_window_listPushBack(&manager->windows, window);
	if (manager->focused == NULL) manager->focused = window;
}
