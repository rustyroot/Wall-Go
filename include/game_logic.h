#ifndef GAME_LOGIC_H
#define GAME_LOGIC_H GAME_LOGIC_H

#include <SDL3/SDL.h>

#include "appstate.h"
#include "moves_recording.h"

#define UP 0
#define RIGHT 1
#define DOWN 2
#define LEFT 3

bool is_on_board (void **appstate, float x, float y);
bool is_a_surrounder_wall (int cell_i, int cell_j, int wall_i, int wall_j);
bool is_there_a_neighbour_in_this_direction (void **appstate, int i, int j, int DIRECTION);
bool is_allowed_mouvement (void **appstate, int i, int j, int DIRECTION);
void deep_limited_search (int limit, void **appstate, int i, int j, int* nb_avaible_moves_ptr);
int compute_avaible_moves (void **appstate);
int count_total_avaible_moves (void **appstate);
void deepfirstsearch (void **appstate, int** connected_components, int i, int j, int connected_component_index);
int compute_connected_components (void **appstate, int*** connected_components, int** connected_component_size, int*** piece_per_component_per_color);
bool is_game_over (void** appstate);
void increment_turn (void **appstate);
void decrement_turn (void **appstate);
void cancel_last_move(void **appstate);
void place_a_piece (void **appstate, int i, int j);
void select_piece(void **appstate, int i , int j);
void move_selected_piece(void **appstate, int i, int j);
void place_a_wall (void **appstate, int** walls_matrix, int i, int j);

#endif