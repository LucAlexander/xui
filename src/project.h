#ifndef PROJECT_H
#define PROJECT_H

#include "xui.h"

struct xi_utils;

typedef struct project_structs{
	xui_window_manager window_manager;

}project_structs;

void project_structs_init(struct xi_utils*);

#endif
