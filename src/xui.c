#include "xui.h"
#include "project.h"

VECTOR_SOURCE(xui_window_list, xui_window*)

void register_xui_systems(program_state* state, xi_utils* xi){
	system_add(state, system_init(xui_window_update, 2, POSITION_C, XUI_WINDOW_C), XI_STATE_UPDATE);
	system_add(state, system_init(xui_widget_mutate, 1, XUI_WIDGET_C), XI_STATE_UPDATE);
	system_add(state, system_init(xui_button_mutate, 3, XUI_WIDGET_C, XUI_PANEL_C, XUI_BUTTON_C), XI_STATE_UPDATE);
	system_add(state, system_init(xui_radio_mutate, 2, XUI_WIDGET_C, XUI_RADIO_C), XI_STATE_UPDATE);
	system_add(state, system_init(xui_window_draw, 2, POSITION_C, XUI_WINDOW_C), XI_STATE_RENDER);
	system_add(state, system_init(xui_panel_render, 2, XUI_WIDGET_C, XUI_PANEL_C), XI_STATE_RENDER);
	system_add(state, system_init(xui_button_render, 3, XUI_WIDGET_C, XUI_PANEL_C, XUI_BUTTON_C), XI_STATE_RENDER);
	system_add(state, system_init(xui_blitable_render, 2, XUI_WIDGET_C, BLITABLE_C), XI_STATE_RENDER);
	system_add(state, system_init(xui_text_render, 2, XUI_WIDGET_C, XUI_TEXT_C), XI_STATE_RENDER);
	system_add(state, system_init(xui_radio_render, 2, XUI_WIDGET_C, XUI_RADIO_C), XI_STATE_RENDER);
}

xui_color xui_color_decode(uint32_t color){
	xui_color c = {
		(color >> 24) & 0xff,
		(color >> 16) & 0xff,
		(color >> 8) & 0xff,
		(color) & 0xff
	};
	return c;
}

uint32_t spawn_xui_window(xi_utils* xi, float x, float y, uint32_t w, uint32_t h, uint8_t resize, uint8_t move, uint32_t color){
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
	xui_color c = xui_color_decode(color);
	window.r = c.r;
	window.g = c.g;
	window.b = c.b;
	window.a = c.a;
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
	if (xi->project->window_manager.focused != window){
		renderSetColor(xi->graphics, window->r/XUI_UNFOCUSED_SCALEFACTOR, window->g/XUI_UNFOCUSED_SCALEFACTOR, window->b/XUI_UNFOCUSED_SCALEFACTOR, window->a);
	}
	else{
		renderSetColor(xi->graphics, window->r, window->g, window->b, window->a);
	}
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
	manager->windows = xui_window_listInit();
	manager->focused = NULL;
}

void xui_window_manager_add_window(xui_window_manager* manager, xui_window* window){
	xui_window_listPushBack(&manager->windows, window);
	if (manager->focused == NULL) manager->focused = window;
}

uint32_t spawn_xui_panel(xi_utils* xi, uint32_t window, uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color, uint32_t border_color){
	uint32_t entity = entity_create(xi->ecs);
	xui_widget widget = {window, x, y, XUI_PANEL_LOCAL_DEPTH};
	xui_color c = xui_color_decode(color);
	xui_color bc = xui_color_decode(border_color);
	xui_panel panel = {w, h, c.r, c.g, c.b, c.a, bc.r, bc.g, bc.b, bc.a};
	component_add(xi->ecs, entity, XUI_WIDGET_C, &widget);
	component_add(xi->ecs, entity, XUI_PANEL_C, &panel);
	return entity;
}

SYSTEM(xui_widget_mutate){
	ARG(xui_widget* widget, XUI_WIDGET_C);
	if (!entity_active(xi->ecs, widget->window)){
		entity_destroy(xi->ecs, id);
	}
	entity_set_layer(xi->ecs, id, entity_get_layer(xi->ecs, widget->window)+widget->local_depth);
}

SYSTEM(xui_panel_render){
	ARG(xui_widget* widget, XUI_WIDGET_C);
	ARG(xui_panel* panel, XUI_PANEL_C);
	v2* position = component_get(xi->ecs, widget->window, POSITION_C);
	xui_window* window = component_get(xi->ecs, widget->window, XUI_WINDOW_C);
	if (xi->project->window_manager.focused == window){
		renderSetColor(xi->graphics, panel->r, panel->g, panel->b, panel->a);
		drawRect(xi->graphics, position->x+widget->x, position->y+widget->y, panel->w, panel->h, FILL);
		renderSetColor(xi->graphics, panel->border_r, panel->border_g, panel->border_b, panel->border_a);
		drawRect(xi->graphics, position->x+widget->x, position->y+widget->y, panel->w, panel->h, OUTLINE);
	}
	else{
		renderSetColor(xi->graphics, panel->r/XUI_UNFOCUSED_SCALEFACTOR, panel->g/XUI_UNFOCUSED_SCALEFACTOR, panel->b/XUI_UNFOCUSED_SCALEFACTOR, panel->a);
		drawRect(xi->graphics, position->x+widget->x, position->y+widget->y, panel->w, panel->h, FILL);
		renderSetColor(xi->graphics, panel->border_r/XUI_UNFOCUSED_SCALEFACTOR, panel->border_g/XUI_UNFOCUSED_SCALEFACTOR, panel->border_b/XUI_UNFOCUSED_SCALEFACTOR, panel->border_a/XUI_UNFOCUSED_SCALEFACTOR);
		drawRect(xi->graphics, position->x+widget->x, position->y+widget->y, panel->w, panel->h, OUTLINE);
	}
	renderSetColor(xi->graphics, 0, 0, 0, 0);
}

uint32_t spawn_xui_button(xi_utils* xi, uint32_t window, uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color, uint32_t border_color, uint32_t hover_color, void (*f)(SYSTEM_ARG_REQUIREMENTS)){
	uint32_t panel = spawn_xui_panel(xi, window, x, y, w, h, color, border_color);
	xui_button clickable;
	clickable.f = f;
	xui_color c = xui_color_decode(hover_color);
	clickable.hover_r = c.r;
	clickable.hover_g = c.g;
	clickable.hover_b = c.b;
	clickable.hover_a = c.a;
	component_add(xi->ecs, panel, XUI_BUTTON_C, &clickable);
	return panel;
}

SYSTEM(xui_button_mutate){
	ARG(xui_widget* widget, XUI_WIDGET_C);
	ARG(xui_panel* panel, XUI_PANEL_C);
	ARG(xui_button* button, XUI_BUTTON_C);
	v2 mouse = mousePos(xi->user_input);
	v2* position = component_get(xi->ecs, widget->window, POSITION_C);
	xui_window* window = component_get(xi->ecs, widget->window, XUI_WINDOW_C);
	if (xi->project->window_manager.focused != window) return;
	if (!mousePressed(xi->user_input, 1)) return;
	if (
		mouse.x < widget->x+position->x ||
		mouse.y < widget->y+position->y ||
		mouse.x > widget->x+position->x+panel->w ||
		mouse.y > widget->y+position->y+panel->h
	) return;
	button->f(SYSTEM_ARGS);
}

SYSTEM(xui_button_render){
	ARG(xui_widget* widget, XUI_WIDGET_C);
	ARG(xui_panel* panel, XUI_PANEL_C);
	ARG(xui_button* button, XUI_BUTTON_C);
	v2 mouse = mousePos(xi->user_input);
	v2* position = component_get(xi->ecs, widget->window, POSITION_C);
	if (
		mouse.x < widget->x+position->x ||
		mouse.y < widget->y+position->y ||
		mouse.x > widget->x+position->x+panel->w ||
		mouse.y > widget->y+position->y+panel->h
	) return;
	renderSetColor(xi->graphics, button->hover_r, button->hover_g, button->hover_b, button->hover_a);
	drawRect(xi->graphics, position->x+widget->x-1, position->y+widget->y+1, panel->w+2, panel->h+2, OUTLINE);
	renderSetColor(xi->graphics, 0, 0, 0, 0);
}

uint32_t spawn_xui_text(xi_utils* xi, uint32_t window, uint32_t x, uint32_t y, char* text, uint32_t color){
	uint32_t entity = entity_create(xi->ecs);
	xui_widget widget = {window, x, y, XUI_TEXT_LOCAL_DEPTH};
	xui_color c = xui_color_decode(color);
	xui_text message = {"", c.r, c.g, c.b, c.a};
	if (strlen(text) > XUI_TEXT_MAX){
		strcpy(message.text, "");
	}
	else{
		strcpy(message.text, text);
	}
	component_add(xi->ecs, entity, XUI_WIDGET_C, &widget);
	component_add(xi->ecs, entity, XUI_TEXT_C, &message);
	return entity;
}

SYSTEM(xui_text_render){
	ARG(xui_widget* widget, XUI_WIDGET_C);
	ARG(xui_text* text, XUI_TEXT_C);
	v2* position = component_get(xi->ecs, widget->window, POSITION_C);
	xui_window* window = component_get(xi->ecs, widget->window, XUI_WINDOW_C);
	if (xi->project->window_manager.focused == window){
		drawTextC(xi->graphics, widget->x+position->x, widget->y+position->y, text->text, text->r, text->g, text->b, text->a);
		return;
	}
	drawTextC(
		xi->graphics,
		widget->x+position->x,
		widget->y+position->y,
		text->text,
		text->r/XUI_UNFOCUSED_SCALEFACTOR,
		text->g/XUI_UNFOCUSED_SCALEFACTOR,
		text->b/XUI_UNFOCUSED_SCALEFACTOR,
		text->a
	);
}

uint32_t spawn_xui_blitable(xi_utils* xi, uint32_t window, uint32_t x, uint32_t y, uint32_t w, uint32_t h, char* src){
	uint32_t entity = entity_create(xi->ecs);
	xui_widget widget = {window, x, y, XUI_BLITABLE_LOCAL_DEPTH};
	Blitable sprite;
	BlitableInitF_arena(xi->graphics, &sprite, src, w, h);
	sprite.center.x = 0;
	sprite.center.y = 0;
	component_add(xi->ecs, entity, XUI_WIDGET_C, &widget);
	component_add(xi->ecs, entity, BLITABLE_C, &sprite);
	return entity;
}

SYSTEM(xui_blitable_render){
	ARG(xui_widget* widget, XUI_WIDGET_C);
	ARG(Blitable* sprite, BLITABLE_C);
	v2* position = component_get(xi->ecs, widget->window, POSITION_C);
	renderBlitable(xi->graphics, sprite, position->x+widget->x, position->y+widget->y);
}

uint32_t spawn_xui_radio(xi_utils* xi, uint32_t window, uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t on_color, uint32_t hover_color){
	uint32_t entity = entity_create(xi->ecs);
	xui_widget widget = {window, x,y, XUI_PANEL_LOCAL_DEPTH};
	xui_color c = xui_color_decode(on_color);
	xui_color hc = xui_color_decode(hover_color);
	xui_radio radio = {w, h, c.r, c.g, c.b, c.a, hc.r, hc.g, hc.b, hc.a, 0};
	component_add(xi->ecs, entity, XUI_WIDGET_C, &widget);
	component_add(xi->ecs, entity, XUI_RADIO_C, &radio);
	return entity;
}

SYSTEM(xui_radio_mutate){
	ARG(xui_widget* widget, XUI_WIDGET_C);
	ARG(xui_radio* button, XUI_RADIO_C);
	v2 mouse = mousePos(xi->user_input);
	v2* position = component_get(xi->ecs, widget->window, POSITION_C);
	xui_window* window = component_get(xi->ecs, widget->window, XUI_WINDOW_C);
	if (xi->project->window_manager.focused != window) return;
	if (!mousePressed(xi->user_input, 1)) return;
	if (
		mouse.x < widget->x+position->x ||
		mouse.y < widget->y+position->y ||
		mouse.x > widget->x+position->x+button->w ||
		mouse.y > widget->y+position->y+button->h
	) return;
	button->value = !button->value;
}

SYSTEM(xui_radio_render){
	ARG(xui_widget* widget, XUI_WIDGET_C);
	ARG(xui_radio* button, XUI_RADIO_C);
	v2 mouse = mousePos(xi->user_input);
	v2* position = component_get(xi->ecs, widget->window, POSITION_C);
	renderSetColor(xi->graphics, button->r, button->g, button->b, button->a);
	if (button->value){
		drawRect(xi->graphics, position->x+widget->x, position->y+widget->y, button->w, button->h, FILL);
	}
	renderSetColor(xi->graphics, button->hover_r, button->hover_g, button->hover_b, button->hover_a);
	drawRect(xi->graphics, position->x+widget->x, position->y+widget->y, button->w, button->h, OUTLINE);
	drawRect(xi->graphics, position->x+widget->x+1, position->y+widget->y+1, button->w-2, button->h-2, OUTLINE);
	if (
		mouse.x > widget->x+position->x &&
		mouse.y > widget->y+position->y &&
		mouse.x < widget->x+position->x+button->w &&
		mouse.y < widget->y+position->y+button->h
	){
		drawRect(xi->graphics, position->x+widget->x-1, position->y+widget->y-1, button->w+2, button->h+2, OUTLINE);
	}
	renderSetColor(xi->graphics, 0, 0, 0, 0);
}

