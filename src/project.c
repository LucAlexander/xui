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

void xistart(xi_utils* xi){
	// setup initial state here
	uint32_t window_color = 0x333333FF;
	uint32_t widget_color = 0x555555FF;
	uint32_t float_window_color = 0x333333AA;
	uint32_t widget_outline_color = 0xAAAAAAFF;
	uint32_t text_color = 0xEEEEEEFF;
#if 0

	spawn_xui_window(xi, 32, 32, 100, 100, 1, 1, window_color);
	uint32_t window = spawn_xui_window(xi, 144, 32, 200, 200, 1, 1, window_color);

	uint32_t button = spawn_xui_button(xi, window, 80, 32, 48, 32, widget_color, 0, button_test);
	spawn_xui_text(xi, window, 84, 36, "hello", 0xFFFFFFFF);

	spawn_xui_panel(xi, window, 32, 32, 32, 32, widget_color, 0xFFFFFFFF);

	spawn_xui_panel(xi, window, 128, 128, 48, 48, widget_color, 0);
	spawn_xui_blitable(xi, window, 136, 136, 32, 32, XI_SPRITE"catpon.png");
	
	spawn_xui_text(xi, window, 64, 64, "world", 0xFF0000FF);
#else
	uint32_t screen_margain = 4;
	uint32_t window_margain = 8;
	uint32_t lx = screen_margain;
	uint32_t ly = screen_margain;
	uint32_t lw = WINDOW_W/4;
	uint32_t lh = WINDOW_H-(screen_margain*2);
	uint32_t left_window = spawn_xui_window(
		xi,
		lx,ly, 
		lw,lh,
		0, 0,
		window_color
	);
	uint32_t bottom_window = spawn_xui_window(
		xi,
		(screen_margain*2)+(WINDOW_W/4),
		WINDOW_H-(WINDOW_H/3)-screen_margain,
		WINDOW_W-(screen_margain*3)-WINDOW_W/4,
		WINDOW_H/3,
		0, 0,
		window_color
	);
	uint32_t float_window = spawn_xui_window(
		xi,
		(WINDOW_W/4)+screen_margain*2,
		screen_margain,
		(WINDOW_W-(WINDOW_W/4))-(screen_margain*3),
		(WINDOW_H-(WINDOW_H/3))-(screen_margain*3),
		1, 1,
		float_window_color
	);
	uint32_t bw = 80;
	uint32_t bh = 48;
	uint32_t button_margain_w = bh/2;
	uint32_t button_margain_h = (bh/2)-4;
	spawn_xui_button(
		xi, left_window,
		lx + window_margain,
		ly + window_margain,
		bw, bh,
		widget_color,
		widget_outline_color,
		button_quit
	);
	spawn_xui_text(
		xi, left_window,
		lx + window_margain + button_margain_w,
		ly + window_margain + button_margain_h,
		"Quit",
		text_color
	);	
	spawn_xui_button(
		xi, left_window,
		lx + window_margain,
		ly + (window_margain*2) + bh,
		bw, bh,
		widget_color,
		widget_outline_color,
		button_test
	);
#endif
}

