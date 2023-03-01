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

void xistart(xi_utils* xi){
	// setup initial state here
	spawn_xui_window(xi, 32, 32, 100, 100, 1, 1);
	spawn_xui_window(xi, 144, 32, 100, 100, 1, 1);
}
