#ifndef INPUT_MANAGING_H
#define INPUT_MANAGING_H INPUT_MANAGING_H

#include "appstate.h"
#include "coordinates.h"
#include "game_logic.h"
#include "moves_recording.h"

void window_update_reaction (void** appstate, char* event);
void switch_fullscreen (void **appstate);
void update_zoom(void **appstate, float amount);
void manage_wall_placement_click(void **appstate, float x_mouse, float y_mouse);
void manage_placement_piece_click (void **appstate, int i, int j);
void manage_on_board_click(void **appstate, float x_mouse, float y_mouse);

#endif