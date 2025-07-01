#ifndef COORDINATES_H
#define COORDINATES_H COORDINATES_H

#include "appstate.h"
#include "game_logic.h"

void cells_coords_to_window_coords(void **appstate, int i, int j, float* x, float* y);
void window_coords_to_cells_coords(void **appstate, float x, float y, int* i, int* j);
void horizontal_walls_coords_to_window_coords(void **appstate, int i, int j, float* x, float* y);
void vertical_walls_coords_to_window_coords(void **appstate, int i, int j, float* x, float* y);
void window_coords_to_walls_coords (void **appstate, float x, float y, int*** walls_matrix_ptr, int* i, int* j);

#endif