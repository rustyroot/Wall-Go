#ifndef DRAWING_H
#define DRAWING_H DRAWING_H

#include <SDL3/SDL.h>

#include "appstate.h"
#include "coordinates.h"
#include "graphical_logic.h"

int filledCircleRGBA(SDL_Renderer * renderer, int x, int y, int rad, SDL_Color color);
void draw_background(void **appstate);
void draw_board(void **appstate);
void draw_horizontal_wall(void **appstate, int i, int j, SDL_Color color);
void draw_horizontal_walls(void **appstate);
void draw_vertical_wall(void **appstate, int i, int j, SDL_Color color);
void draw_vertical_walls(void **appstate);
void draw_walls(void **appstate);
void draw_pieces (void **appstate);
void draw_selected_piece(void **appstate);
void draw_avaible_moves(void **appstate);
void draw_phantom_walls(void **appstate);

#endif