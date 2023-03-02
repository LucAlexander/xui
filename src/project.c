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

void xistart(xi_utils* xi){
	// setup initial state here
	uint32_t window_color = 0x333333FF;
	uint32_t widget_color = 0x555555FF;

	spawn_xui_window(xi, 32, 32, 100, 100, 1, 1, window_color);
	uint32_t window = spawn_xui_window(xi, 144, 32, 200, 200, 1, 1, window_color);

	spawn_xui_panel(xi, window, 32, 32, 32, 32, widget_color);

	uint32_t button = spawn_xui_button(xi, window, 80, 32, 48, 32, widget_color, button_test);
	xui_text text = {"hello", 255, 255, 255, 255};
	component_add(xi->ecs, button, XUI_TEXT_C, &text);

	spawn_xui_text(xi, window, 64, 64, "world", 0xFF0000FF);
}

