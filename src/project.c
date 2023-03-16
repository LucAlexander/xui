#include <stdio.h>

#include "project.h"
#include "xi_engine.h"

#include "xui.h"

void project_structs_init(xi_utils* xi){	
	project_structs* project = malloc(sizeof(project_structs));
	xi->project = project;
	xui_window_manager_init(&xi->project->window_manager);
}

void xisetup(program_state* state, xi_utils* xi){
	project_structs_init(xi);
	register_xui_systems(state, xi);
}

void button_test(SYSTEM_ARG_REQUIREMENTS){
	printf("wow\n");
}

void button_quit(SYSTEM_ARG_REQUIREMENTS){
	xi_quit(xi);
}

void button_close_window(SYSTEM_ARG_REQUIREMENTS){
	ARG(xui_widget* widget, XUI_WIDGET_C);
	entity_destroy(xi->ecs, widget->window);
}

void xistart(xi_utils* xi){
	// setup initial state here
	uint32_t window_color = 0x333333FF;
	uint32_t widget_color = 0x555555FF;
	uint32_t float_window_color = 0x333333AA;
	uint32_t widget_outline_color = 0xAAAAAAFF;
	uint32_t text_color = 0xEEEEEEFF;
	uint32_t screen_margain = 4;
	uint32_t window_margain = 8;
	uint32_t lx = screen_margain;
	uint32_t ly = screen_margain;
	uint32_t lw = WINDOW_W/4;
	uint32_t lh = WINDOW_H-(screen_margain*2);
	uint32_t left_window = spawn_xui_window(xi, lx,ly, lw,lh, 0, 0, window_color);
	uint32_t bottom_window = spawn_xui_window(xi, (screen_margain*2)+(WINDOW_W/4), WINDOW_H-(WINDOW_H/3)-screen_margain, WINDOW_W-(screen_margain*3)-WINDOW_W/4, WINDOW_H/3, 0, 0, window_color);
	uint32_t float_window = spawn_xui_window(xi, (WINDOW_W/4)+screen_margain*2, screen_margain, (WINDOW_W-(WINDOW_W/4))-(screen_margain*3), (WINDOW_H-(WINDOW_H/3))-(screen_margain*3), 1, 1, float_window_color);
	uint32_t bw = 80;
	uint32_t bh = 48;
	uint32_t button_margain_w = bh/2;
	uint32_t button_margain_h = (bh/2)-4;
	spawn_xui_button(xi, left_window,lx + window_margain,ly + window_margain,bw, bh,widget_color,widget_outline_color,widget_outline_color,button_quit);
	spawn_xui_text(xi, left_window,lx + window_margain + button_margain_w,ly + window_margain + button_margain_h,"Quit",text_color);
	spawn_xui_button(xi, left_window, lx + window_margain, ly + (window_margain*2) + bh, bw, bh, widget_color, widget_outline_color, widget_outline_color, button_test);
	spawn_xui_button(xi, bottom_window, lx + window_margain, ly + window_margain, bw, bh, widget_color, widget_outline_color, widget_outline_color, button_close_window);
	spawn_xui_text(xi, bottom_window, lx + window_margain + button_margain_w, ly + window_margain + button_margain_h, "Close", text_color);
	spawn_xui_blitable(xi, bottom_window, window_margain, window_margain, 32, 32, XI_SPRITE"catpon.png");
	spawn_xui_radio(xi, float_window, window_margain, window_margain, 32, 32, 0xFF0000FF, widget_outline_color);
	spawn_xui_slider(xi, float_window, (window_margain*2)+128, window_margain, 16, 8, 0, 100, widget_color, XUI_SLIDER_X);
	spawn_xui_slider(xi, float_window, window_margain, (window_margain*2)+128, 16, 8, 50, 200, widget_color, XUI_SLIDER_Y);
	spawn_xui_textentry(xi, float_window, (window_margain*2)+128, (window_margain*2)+128, 128, 128, text_color);
	spawn_xui_panel(xi, float_window, 256, 256, 64, 64, widget_color, widget_outline_color);
	spawn_xui_button(xi, float_window, 256, 128, 64, 64, widget_color, widget_outline_color, widget_outline_color, button_close_window);
}

