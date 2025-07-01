#ifndef GRAPHICAL_LOGIC_H
#define GRAPHICAL_LOGIC_H GRAPHICAL_LOGIC_H

#include <SDL3/SDL.h>

#include "appstate.h"

#define EMPTY_SLOT 0
#define BLUE 1
#define RED 2
#define GREEN 3
#define YELLOW 4

void update_display_parameters (void **appstate);
SDL_Color set_color (void **appstate, int COLOR, bool is_highlighted);

#endif