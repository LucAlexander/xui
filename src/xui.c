#include "xui.h"
#include "project.h"

#include <string.h>

VECTOR_SOURCE(xui_window_list, xui_window*)

void register_xui_systems(program_state* state, xi_utils* xi){
	system_add(state, system_init(xui_window_update, 2, POSITION_C, XUI_WINDOW_C), XI_STATE_UPDATE);
	system_add(state, system_init(xui_widget_mutate, 1, XUI_WIDGET_C), XI_STATE_UPDATE);
	system_add(state, system_init(xui_button_mutate, 3, XUI_WIDGET_C, XUI_PANEL_C, XUI_BUTTON_C), XI_STATE_UPDATE);
	system_add(state, system_init(xui_slider_mutate, 2, XUI_WIDGET_C, XUI_SLIDER_C), XI_STATE_UPDATE);
	system_add(state, system_init(xui_radio_mutate, 2, XUI_WIDGET_C, XUI_RADIO_C), XI_STATE_UPDATE);
	system_add(state, system_init(xui_textentry_mutate, 2, XUI_WIDGET_C, XUI_TEXTENTRY_C), XI_STATE_UPDATE);
	system_add(state, system_init(xui_shell_mutate, 2, XUI_WIDGET_C, XUI_SHELL_C), XI_STATE_UPDATE);
	system_add(state, system_init(xui_window_draw, 2, POSITION_C, XUI_WINDOW_C), XI_STATE_RENDER);
	system_add(state, system_init(xui_panel_render, 2, XUI_WIDGET_C, XUI_PANEL_C), XI_STATE_RENDER);
	system_add(state, system_init(xui_button_render, 3, XUI_WIDGET_C, XUI_PANEL_C, XUI_BUTTON_C), XI_STATE_RENDER);
	system_add(state, system_init(xui_slider_render, 2, XUI_WIDGET_C, XUI_SLIDER_C), XI_STATE_RENDER);
	system_add(state, system_init(xui_blitable_render, 2, XUI_WIDGET_C, BLITABLE_C), XI_STATE_RENDER);
	system_add(state, system_init(xui_radio_render, 2, XUI_WIDGET_C, XUI_RADIO_C), XI_STATE_RENDER);
	system_add(state, system_init(xui_text_render, 2, XUI_WIDGET_C, XUI_TEXT_C), XI_STATE_RENDER);
	system_add(state, system_init(xui_textentry_render, 2, XUI_WIDGET_C, XUI_TEXTENTRY_C), XI_STATE_RENDER);
	system_add(state, system_init(xui_shell_render, 2, XUI_WIDGET_C, XUI_SHELL_C), XI_STATE_RENDER);
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
	uint32_t panel_w = panel->w;
	uint32_t panel_h = panel->h;
	if (window->w < widget->x+panel->w){
		panel_w = window->w - widget->x;
	}
	if (window->h < widget->y+panel->h){
		panel_h = window->h - widget->y;
	}
	if (window->w < widget->x || window->h < widget->y) return;
	if (xi->project->window_manager.focused == window){
		renderSetColor(xi->graphics, panel->r, panel->g, panel->b, panel->a);
		drawRect(xi->graphics, position->x+widget->x, position->y+widget->y, panel_w, panel_h, FILL);
		renderSetColor(xi->graphics, panel->border_r, panel->border_g, panel->border_b, panel->border_a);
	}
	else{
		renderSetColor(xi->graphics, panel->r/XUI_UNFOCUSED_SCALEFACTOR, panel->g/XUI_UNFOCUSED_SCALEFACTOR, panel->b/XUI_UNFOCUSED_SCALEFACTOR, panel->a);
		drawRect(xi->graphics, position->x+widget->x, position->y+widget->y, panel_w, panel_h, FILL);
		renderSetColor(xi->graphics, panel->border_r/XUI_UNFOCUSED_SCALEFACTOR, panel->border_g/XUI_UNFOCUSED_SCALEFACTOR, panel->border_b/XUI_UNFOCUSED_SCALEFACTOR, panel->border_a/XUI_UNFOCUSED_SCALEFACTOR);
	}
	drawRect(xi->graphics, position->x+widget->x, position->y+widget->y, panel_w, panel_h, OUTLINE);
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
	uint32_t panel_w = panel->w;
	uint32_t panel_h = panel->h;
	if (window->w < widget->x+panel->w){
		panel_w = window->w - widget->x;
	}
	if (window->h < widget->y+panel->h){
		panel_h = window->h - widget->y;
	}
	if (window->w < widget->x || window->h < widget->y) return;
	if (xi->project->window_manager.focused != window) return;
	if (!mousePressed(xi->user_input, 1)) return;
	if (
		mouse.x < widget->x+position->x ||
		mouse.y < widget->y+position->y ||
		mouse.x > widget->x+position->x+panel_w ||
		mouse.y > widget->y+position->y+panel_h
	) return;
	button->f(SYSTEM_ARGS);
}

SYSTEM(xui_button_render){
	ARG(xui_widget* widget, XUI_WIDGET_C);
	ARG(xui_panel* panel, XUI_PANEL_C);
	ARG(xui_button* button, XUI_BUTTON_C);
	v2 mouse = mousePos(xi->user_input);
	v2* position = component_get(xi->ecs, widget->window, POSITION_C);
	xui_window* window = component_get(xi->ecs, widget->window, XUI_WINDOW_C);
	uint32_t panel_w = panel->w;
	uint32_t panel_h = panel->h;
	if (window->w < widget->x+panel->w){
		panel_w = window->w - widget->x;
	}
	if (window->h < widget->y+panel->h){
		panel_h = window->h - widget->y;
	}
	if (window->w < widget->x || window->h < widget->y) return;
	if (
		mouse.x < widget->x+position->x ||
		mouse.y < widget->y+position->y ||
		mouse.x > widget->x+position->x+panel_w ||
		mouse.y > widget->y+position->y+panel_h
	) return;
	renderSetColor(xi->graphics, button->hover_r, button->hover_g, button->hover_b, button->hover_a);
	drawRect(xi->graphics, position->x+widget->x-1, position->y+widget->y+1, panel_w+2, panel_h+2, OUTLINE);
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
	xui_window* window = component_get(xi->ecs, widget->window, XUI_WINDOW_C);
	if (xi->project->window_manager.focused == window){
		renderSetColor(xi->graphics, button->r, button->g, button->b, button->a);
	}
	else{
		renderSetColor(xi->graphics, button->r/XUI_UNFOCUSED_SCALEFACTOR, button->g/XUI_UNFOCUSED_SCALEFACTOR, button->b/XUI_UNFOCUSED_SCALEFACTOR, button->a);
	}
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

uint32_t spawn_xui_slider(xi_utils* xi, uint32_t window, uint32_t x, uint32_t y, uint32_t nob_w, uint32_t nob_h, float min, float max, uint32_t color, XUI_SLIDER_DIR dir){
	uint32_t entity = entity_create(xi->ecs);
	xui_widget widget = {window, x,y, XUI_PANEL_LOCAL_DEPTH};
	xui_color c = xui_color_decode(color);
	xui_slider slide = {min, max, (max-min)/2, nob_w, nob_h, c.r, c.g, c.b, c.a, dir};
	component_add(xi->ecs, entity, XUI_WIDGET_C, &widget);
	component_add(xi->ecs, entity, XUI_SLIDER_C, &slide);
	return entity;
}

SYSTEM(xui_slider_mutate){
	if (!mouseHeld(xi->user_input, 1)) return;
	ARG(xui_widget* widget, XUI_WIDGET_C);
	ARG(xui_slider* slider, XUI_SLIDER_C);
	xui_window* window = component_get(xi->ecs, widget->window, XUI_WINDOW_C);
	if (xi->project->window_manager.focused != window) return;
	v2* position = component_get(xi->ecs, widget->window, POSITION_C);
	v2 mouse = mousePos(xi->user_input);
	float half_len = (slider->max - slider->min) / 2;
	switch(slider->dir){
		case XUI_SLIDER_X:{
			if (
				mouse.x < position->x + widget->x - half_len ||
				mouse.x > position->x + widget->x + half_len ||
				mouse.y < position->y + widget->y - (slider->nob_w/2) || 
				mouse.y > position->y + widget->y + (slider->nob_w/2)
			) return;
			slider->position = mouse.x - (position->x + widget->x - half_len);
		}return;
		case XUI_SLIDER_Y:{
			if (
				mouse.x < position->x + widget->x - (slider->nob_w/2) ||
				mouse.x > position->x + widget->x + (slider->nob_w/2) ||
				mouse.y < position->y + widget->y - half_len || 
				mouse.y > position->y + widget->y + half_len
			) return;
			slider->position = mouse.y - (position->y + widget->y - half_len);
		}return;
	}
}

SYSTEM(xui_slider_render){
	ARG(xui_widget* widget, XUI_WIDGET_C);
	ARG(xui_slider* slider, XUI_SLIDER_C);
	v2* position = component_get(xi->ecs, widget->window, POSITION_C);
	xui_window* window = component_get(xi->ecs, widget->window, XUI_WINDOW_C);
	if (xi->project->window_manager.focused == window){
		renderSetColor(xi->graphics, slider->r, slider->g, slider->b, slider->a);
	}
	else{
		renderSetColor(xi->graphics, slider->r/XUI_UNFOCUSED_SCALEFACTOR, slider->g/XUI_UNFOCUSED_SCALEFACTOR, slider->b/XUI_UNFOCUSED_SCALEFACTOR, slider->a);
	}
	float half_len = (slider->max - slider->min) / 2;
	float start;
	switch(slider->dir){
		case XUI_SLIDER_X:{
			start = position->x + widget->x - half_len;
			drawRect(xi->graphics, start, position->y+widget->y, slider->max-slider->min, 2, FILL);
			drawRect(xi->graphics, (start + slider->position)-(slider->nob_h/2), position->y+widget->y-(slider->nob_w/2), slider->nob_h, slider->nob_w, FILL);
		}break;
		case XUI_SLIDER_Y:{
			start = position->y + widget->y - half_len;
			drawRect(xi->graphics, position->x+widget->x, start, 2, slider->max-slider->min, FILL);
			drawRect(xi->graphics, position->x + widget->x - (slider->nob_w/2), (start+slider->position)-(slider->nob_h/2), slider->nob_w, slider->nob_h, FILL);
		}break;
	}
	renderSetColor(xi->graphics, 0, 0, 0, 0);
}

void strins(char* source, char* str, uint32_t index){
	uint32_t i, n = strlen(source);
	if (n == index){
		strcat(source, str);
		return;
	}
	uint32_t offset = strlen(str);
	for (i = n+offset-1;i>=index+offset;--i){
		source[i] = source[i-offset];
	}
	source[n+offset] = '\0';
	for (i = index;i<index+offset;++i){
		source[i] = str[i-index];
	}
}

void strcut(char* source, int32_t index){
	uint32_t i, n = strlen(source);
	if (n == index){
		source[n-1] = '\0';
		return;
	}
	for (i = index;i<n;++i){
		source[i] = source[i+1];
	}
}

uint32_t spawn_xui_textentry(xi_utils* xi, uint32_t window, uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t text_color){
	uint32_t entity = entity_create(xi->ecs);
	xui_widget widget = {window, x,y, XUI_TEXT_LOCAL_DEPTH};
	xui_color c = xui_color_decode(text_color);
	xui_textentry text = {"", w, h, c.r, c.g, c.b, c.a, 0, 0};
	component_add(xi->ecs, entity, XUI_WIDGET_C, &widget);
	component_add(xi->ecs, entity, XUI_TEXTENTRY_C, &text);
	return entity;
}

SYSTEM(xui_textentry_mutate){
	ARG(xui_widget* widget, XUI_WIDGET_C);
	ARG(xui_textentry* text, XUI_TEXTENTRY_C);
	v2 mouse = mousePos(xi->user_input);
	v2* position = component_get(xi->ecs, widget->window, POSITION_C);
	xui_window* window = component_get(xi->ecs, widget->window, XUI_WINDOW_C);
	if (xi->project->window_manager.focused != window) return;
	if (text->selected){
		char keys[XUI_TEXTENTRY_ALLOWED_KEYS_LEN];
		keystream(xi->user_input, keys, XUI_TEXTENTRY_ALLOWED_KEYS); 
		uint32_t n = strlen(keys);
		if (strcmp(keys, "") && ((strlen(text->text)+n) < XUI_TEXT_MAX)){
			strins(text->text, keys, text->position);
			text->position += n;
		}
		if (keyPressed(xi->user_input, "Return") && (strlen(text->text)+1) < XUI_TEXT_MAX){
			strins(text->text, "\n", text->position);
			text->position++;
		}
		if (keyPressed(xi->user_input, "Tab") && (strlen(text->text)+1) < XUI_TEXT_MAX){
			strins(text->text, "\t", text->position);
			text->position++;
		}
		if (keyPressed(xi->user_input, "Space") && (strlen(text->text)+1) < XUI_TEXT_MAX){
			strins(text->text, " ", text->position);
			text->position++;
		}
		if (keyPressed(xi->user_input, "Backspace") && (strlen(text->text)+1) < XUI_TEXT_MAX){
			if (text->position-1 >= 0){
				strcut(text->text, text->position-1);
				text->position--;
			}
		}
		if (keyPressed(xi->user_input, "Left")){
			if (text->position > 0){
				text->position--;
			}
		}
		if (keyPressed(xi->user_input, "Right")){
			if (text->position < strlen(text->text)){
				text->position ++;
			}
		}
	}
	if (!mousePressed(xi->user_input, 1)) return;
	if (
		mouse.x < widget->x+position->x ||
		mouse.y < widget->y+position->y ||
		mouse.x > widget->x+position->x+text->w ||
		mouse.y > widget->y+position->y+text->h
	){
		text->selected = 0;
		return;
	}
	text->selected = 1;
}

SYSTEM(xui_textentry_render){
	ARG(xui_widget* widget, XUI_WIDGET_C);
	ARG(xui_textentry* text, XUI_TEXTENTRY_C);
	v2* position = component_get(xi->ecs, widget->window, POSITION_C);
	xui_window* window = component_get(xi->ecs, widget->window, XUI_WINDOW_C);
	if (xi->project->window_manager.focused == window){
		renderSetColor(xi->graphics, text->r, text->g, text->b, text->a);
	}
	else{
		renderSetColor(xi->graphics, text->r/XUI_UNFOCUSED_SCALEFACTOR, text->g/XUI_UNFOCUSED_SCALEFACTOR, text->b/XUI_UNFOCUSED_SCALEFACTOR, text->a);
	}
	char copy[XUI_TEXT_MAX];
	strcpy(copy, text->text);
	char* context = NULL;
	char* line = strtok_r(copy, "\n", &context);
	uint32_t start_x = widget->x+position->x;
	uint32_t render_x = start_x;
	uint32_t render_y = widget->y+position->y;
	int32_t cw, ch;
	queryTextSize(xi->graphics, " ", &cw, &ch);
	while (line != NULL){
		char linecopy[XUI_TEXT_MAX];
		strcpy(linecopy, line);
		if (strlen(linecopy) == 0) continue;
		if (linecopy[0] == '\t'){
			render_x += XUI_TAB_W * cw;
		}
		char* segment = strtok(linecopy, "\t");
		while (segment != NULL){
			drawText(xi->graphics, render_x, render_y, segment);
			render_x += getTextWidth(xi->graphics, segment)+(XUI_TAB_W * cw);
			segment = strtok(NULL, "\t");
/*urcute*/	}
		render_y += ch;
		render_x = start_x;
		line = strtok_r(NULL, "\n", &context);
	}
	renderSetColor(xi->graphics, 0, 0, 0, 0);
}

uint32_t spawn_xui_shell(xi_utils* xi, uint32_t window, uint32_t x, uint32_t y, uint32_t text_color, char* (*f)(SYSTEM_ARG_REQUIREMENTS, xui_shell*), void* system){
	uint32_t entity = entity_create(xi->ecs);
	xui_widget widget = {window, x,y, XUI_TEXT_LOCAL_DEPTH};
	xui_color c = xui_color_decode(text_color);
	xui_shell shell;
	for (uint32_t i = 0;i<XUI_SHELL_LINE_COUNT;++i){
		strcpy(shell.text[i], "");
	}
	shell.target = 0;
	shell.position = 0;
	shell.r = c.r;
	shell.g = c.g;
	shell.b = c.b;
	shell.a = c.a;
	shell.scroll = 0;
	shell.f = f;
	shell.system = system;
	component_add(xi->ecs, entity, XUI_WIDGET_C, &widget);
	component_add(xi->ecs, entity, XUI_SHELL_C, &shell);
	return entity;
}

int32_t find_ch_index(char string[], char ch) {
	int32_t i = 0;
	while (string[i] != '\0') if (string[i++] == ch) return i;
	return -1;
}

void xui_shell_new_line(xui_shell* shell){
	if (shell->target < XUI_SHELL_LINE_COUNT-1){
		shell->target ++;
		return;
	}
	for (uint32_t i = 0;i<XUI_SHELL_LINE_COUNT-1;++i){
		strcpy(shell->text[i], shell->text[i+1]);
	}
	strcpy(shell->text[XUI_SHELL_LINE_COUNT-1], "");
}

void xui_shell_buffer_output(xui_shell* shell, char* output){
	char line[XUI_TEXT_MAX];
	int32_t nl = find_ch_index(output, '\n');		
	uint16_t p = 0;
	while (nl != -1){
		strncpy(line, output+p, nl);
		line[nl-1] = '\0';
		output[nl-1] = ' ';
		strcat(shell->text[shell->target], line);
		xui_shell_new_line(shell);
		p += nl;
		nl = find_ch_index(output+p, '\n');
	}
	free(output);
} 

char* xui_shell_default_shell(SYSTEM_ARG_REQUIREMENTS, xui_shell* shell){
	char* output = malloc(16);
	strcpy(output, "hi\n");
	return output;
}

void xui_shell_scroll_to_current(xui_shell* shell, uint32_t visible_h){
	if (shell->target - shell->scroll > visible_h){
		shell->scroll = (shell->target-visible_h)+1;
	}
}

SYSTEM(xui_shell_mutate){
	ARG(xui_widget* widget, XUI_WIDGET_C);
	ARG(xui_shell* shell, XUI_SHELL_C);
	xui_window* window = component_get(xi->ecs, widget->window, XUI_WINDOW_C);
	if (xi->project->window_manager.focused != window) return;
	char keys[XUI_TEXTENTRY_ALLOWED_KEYS_LEN];
	keystream(xi->user_input, keys, XUI_TEXTENTRY_ALLOWED_KEYS); 
	uint32_t n = strlen(keys);
	uint32_t text_h = getTextHeight(xi->graphics, "#");
	uint32_t content_height = window->h - (widget->y*2);
	uint32_t visible_h = (content_height/text_h);
	char* text = shell->text[shell->target];
	if (strcmp(keys, "") && ((strlen(text)+n) < XUI_TEXT_MAX)){
		strins(text, keys, shell->position);
		shell->position += n; 
		xui_shell_scroll_to_current(shell, visible_h);
	}
	if (keyPressed(xi->user_input, "Return")){
		shell->position = 0;
		xui_shell_new_line(shell);
		xui_shell_buffer_output(shell, shell->f(SYSTEM_ARGS, shell));
		xui_shell_scroll_to_current(shell, visible_h);
	}
	if (keyPressed(xi->user_input, "Tab") && (strlen(text)+strlen(XUI_SHELL_TAB)) < XUI_TEXT_MAX){
		strins(text, XUI_SHELL_TAB, shell->position);
		shell->position++;
	}
	if (keyPressed(xi->user_input, "Space") && (strlen(text)+1) < XUI_TEXT_MAX){
		strins(text, " ", shell->position);
		shell->position++;
	}
	if (keyPressed(xi->user_input, "Backspace") && (strlen(text)+1) < XUI_TEXT_MAX){
		if (shell->position-1 >= 0){
			strcut(text, shell->position-1);
			shell->position--;
		}
	}
	if (keyPressed(xi->user_input, "Left")){
		if (shell->position > 0){
			shell->position--;
		}
	}
	if (keyPressed(xi->user_input, "Right")){
		if (shell->position < strlen(text)){
			shell->position ++;
		}
	}
	if (keyPressed(xi->user_input, "Up")){
		if (shell->scroll > 0){
			shell->scroll--;
		}
	}
	if (keyPressed(xi->user_input, "Down")){
		if ((shell->scroll <= shell->target-visible_h) && (shell->scroll < shell->target)){
			shell->scroll ++;
		}
	}
}

SYSTEM(xui_shell_render){
	ARG(xui_widget* widget, XUI_WIDGET_C);
	ARG(xui_shell* shell, XUI_SHELL_C);
	v2* position = component_get(xi->ecs, widget->window, POSITION_C);
	xui_window* window = component_get(xi->ecs, widget->window, XUI_WINDOW_C);
	uint32_t text_h = getTextHeight(xi->graphics, "#");
	uint32_t content_height = window->h - (widget->x*2);
	uint32_t content_width = window->w - (widget->y*2);
	uint32_t visible_h = content_height/text_h;
	uint32_t start_x = position->x + widget->x;
	uint32_t start_y = position->y + widget->y;
	if (xi->project->window_manager.focused == window){
		renderSetColor(xi->graphics, shell->r, shell->g, shell->b, shell->a);
	}
	else{
		renderSetColor(xi->graphics, shell->r/XUI_UNFOCUSED_SCALEFACTOR, shell->g/XUI_UNFOCUSED_SCALEFACTOR, shell->b/XUI_UNFOCUSED_SCALEFACTOR, shell->a);
	}
	for (uint32_t i = shell->scroll;i<shell->scroll+visible_h;++i){
		char* line = shell->text[i];
		uint32_t n = strlen(line);
		if (getTextWidth(xi->graphics, line)>content_width){
			char* segment = malloc(n+1);
			strcpy(segment, line);
			for (int32_t k = n-1;k>=0 && getTextWidth(xi->graphics, segment)>content_width;--k){
				segment[k] = '\0';
			}
			drawText(xi->graphics, start_x, start_y, segment);
			free(segment);
		}
		else{
			drawText(xi->graphics, start_x, start_y, line);
		}
		start_y += text_h;
	}
	renderSetColor(xi->graphics, 0, 0, 0, 0);
}

