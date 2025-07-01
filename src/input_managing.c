#include "../include/input_managing.h"

void window_update_reaction (void** appstate, char* event) {
    appstate_t* as = (appstate_t*)appstate;
    SDL_GetWindowSize(as->window, &(as->window_width), &(as->window_height));
    SDL_Log("Window event : %s; w = %d; h = %d\n", event, as->window_width, as->window_height);
    as->has_to_be_render = true;
}

void switch_fullscreen (void **appstate) {
    appstate_t* as = (appstate_t*)appstate;
    as->is_fullscreen = !as->is_fullscreen;
    SDL_SetWindowFullscreen(as->window, as->is_fullscreen);
    as->has_to_be_render = true;
}

void update_zoom(void **appstate, float amount) {
    appstate_t* as = (appstate_t*)appstate;
    float old_value = as->zoom;
    as->zoom *= SDL_pow(as->zoom_speed, amount);
    if (as->zoom < as->zoom_min) {
        as->zoom = as->zoom_min;
    }
    else if (as->zoom > as->zoom_max) {
        as->zoom = as->zoom_max;
    }
    SDL_Log("zoom updated : %f to %f\n", old_value, as->zoom);
    as->has_to_be_render = true;
}

void manage_wall_placement_click(void **appstate, float x_mouse, float y_mouse) {
    appstate_t* as = (appstate_t*)appstate;
    int** walls_matrix;
    int i, j;
    window_coords_to_walls_coords(appstate, x_mouse, y_mouse, &walls_matrix, &i, &j);
    if (walls_matrix != NULL) {
        SDL_Log("Wall clicked (%d, %d)", i, j);
        if (is_a_surrounder_wall(as->selected_piece_i, as->selected_piece_j, i, j) && walls_matrix[i][j] == 0) {
            place_a_wall(appstate, walls_matrix, i, j);

            if (is_game_over(appstate)) {
                as->game_over = true;
                for (int k = 0; k < as->nb_players; k++) {
                    SDL_Log("Player %d : %d points", k+1, as->scores[k]);
                }
                SDL_Log("GAME OVER");
            }
            else {
                increment_turn(appstate);
            }

            as->has_to_be_render = true;
        }
    }
}

void manage_placement_piece_click (void **appstate, int i, int j) {
    appstate_t* as = (appstate_t*)appstate;
    if (as->is_first_placement_phase && as->cells[i][j] == 0) {
        place_a_piece(appstate, i, j);
        if (as->player_turn == as->nb_players) {
            SDL_Log("First placement phase ended");
            as->is_first_placement_phase = false;
            as->is_second_placement_phase = true;
        }
        else {
            increment_turn(appstate);
        }
        as->has_to_be_render = true;
    }
    else if (as->is_second_placement_phase && as->cells[i][j] == 0) {
        place_a_piece(appstate, i, j);
        if (as->player_turn == 1) {
            SDL_Log("Second placement phase ended");
            as->is_second_placement_phase = false;
            as->is_placement_phase = false;
            SDL_Log("Player %d has %d moves avaible", as->player_turn, count_total_avaible_moves(appstate));
        }
        else {
            decrement_turn(appstate);
        }
        as->has_to_be_render = true;
    }
    else {
        SDL_Log("%c%d clicked (nothing happened)", 'A'+i, j+1);
    }
}

void manage_on_board_click(void **appstate, float x_mouse, float y_mouse) {
    appstate_t* as = (appstate_t*)appstate;
    if (as->is_wall_placement_phase) {
        manage_wall_placement_click(appstate, x_mouse, y_mouse);
    }
    else {
        int i, j;
        window_coords_to_cells_coords(appstate, x_mouse, y_mouse, &i, &j);
        if (as->is_placement_phase) {
            manage_placement_piece_click(appstate, i, j);
        }
        else if (as->is_there_a_selected_piece 
                && 0 <= 2+i-as->selected_piece_i && 2+j-as->selected_piece_j < 5 
                && 0 <= 2+j-as->selected_piece_j && 2+j-as->selected_piece_j < 5 
                && as->avaible_moves[2 + i - as->selected_piece_i][2 + j - as->selected_piece_j]) {
            move_selected_piece(appstate, i, j);
        }
        else if (as->cells[i][j] == as->player_turn) {
            select_piece(appstate, i , j);
        }
        else {
            SDL_Log("%c%d clicked (nothing happened)", 'A'+i, j+1);
        }
    }    
}