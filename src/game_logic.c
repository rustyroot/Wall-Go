#include "../include/game_logic.h"
#include <SDL3/SDL_stdinc.h>
#include <stdbool.h>

bool is_on_board (void **appstate, float x, float y) {
    appstate_t* as = (appstate_t*)appstate;
    return x >= as->horizontal_margin + as->inboard_margin
        && x <= as->horizontal_margin + as->inboard_margin + as->board_nb_horizontal_cells*as->cells_size 
        && y >= as->vertical_margin + as->inboard_margin
        && y <= as->vertical_margin + as->inboard_margin + as->board_nb_vertical_cells*as->cells_size;
}

bool is_a_surrounder_wall (int cell_i, int cell_j, int wall_i, int wall_j) {
    return (cell_i == wall_i   && cell_j == wall_j  )
        || (cell_i == wall_i+1 && cell_j == wall_j  )
        || (cell_i == wall_i   && cell_j == wall_j+1);
}

bool is_there_a_neighbour_in_this_direction (void **appstate, int i, int j, int DIRECTION) {
    appstate_t* as = (appstate_t*)appstate;
    switch (DIRECTION) {
            //     (Don't go off limit) && (Don't get throught a wall)
        case UP:
            return (j < as->board_nb_vertical_cells-1) && (as->horizontal_walls[i][j] == 0);            
        case RIGHT:
            return (i < as->board_nb_horizontal_cells-1) && (as->vertical_walls[i][j] == 0); 
        case DOWN:
            return (j > 0) && (as->horizontal_walls[i][j-1] == 0); 
        case LEFT:
            return (i > 0) && (as->vertical_walls[i-1][j] == 0); 
        default:
            SDL_Log("Wrong DIRECTION");
            return false;
    }
}

bool is_allowed_mouvement (void **appstate, int i, int j, int DIRECTION) {
    appstate_t* as = (appstate_t*)appstate;

    if (is_there_a_neighbour_in_this_direction(appstate, i, j, DIRECTION)) {
        switch (DIRECTION) {
            //     Don't go to an occupied cell
        case UP:
            return as->cells[i][j+1] == 0;            
        case RIGHT:
            return as->cells[i+1][j] == 0; 
        case DOWN:
            return as->cells[i][j-1] == 0; 
        case LEFT:
            return as->cells[i-1][j] == 0; 
        default:
            SDL_Log("Wrong DIRECTION");
            return false;
        }
    }
    else {
        return false;
    }
}

void deep_limited_search (int limit, void **appstate, int i, int j, int* nb_avaible_moves_ptr) {
    appstate_t* as = (appstate_t*)appstate;
    if (limit > 0 && as->avaible_moves[2 + i - as->selected_piece_i][2 + j - as->selected_piece_j] == false) {
        as->avaible_moves[2 + i - as->selected_piece_i][2 + j - as->selected_piece_j] = true;
        (*nb_avaible_moves_ptr)++;
        if (is_allowed_mouvement(appstate, i, j, UP)) deep_limited_search(limit-1, appstate, i, j+1, nb_avaible_moves_ptr);
        if (is_allowed_mouvement(appstate, i, j, RIGHT)) deep_limited_search(limit-1, appstate, i+1, j, nb_avaible_moves_ptr);
        if (is_allowed_mouvement(appstate, i, j, DOWN)) deep_limited_search(limit-1, appstate, i, j-1, nb_avaible_moves_ptr);
        if (is_allowed_mouvement(appstate, i, j, LEFT)) deep_limited_search(limit-1, appstate, i-1, j, nb_avaible_moves_ptr);
    }
}

int compute_avaible_moves (void **appstate) {
    appstate_t* as = (appstate_t*)appstate;
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            as->avaible_moves[i][j] = false;
        }
    }
    int nb_avaible_moves = 0;
    if (as->is_there_a_selected_piece) {
        deep_limited_search(3, appstate, as->selected_piece_i, as->selected_piece_j, &nb_avaible_moves);
        // aller-retour autorisé mais pas le sur place.
        if (as->avaible_moves[2][2] && !(as->avaible_moves[2][3] ||  as->avaible_moves[2][1] || as->avaible_moves[1][2] || as->avaible_moves[3][2])) {
            as->avaible_moves[2][2] = false;
            nb_avaible_moves--;
        }
    }
    return nb_avaible_moves;
}

int count_total_avaible_moves (void **appstate) {
    appstate_t* as = (appstate_t*)appstate;
    if (as->is_wall_placement_phase) {
        SDL_Log("wall phase moves count isn't implemented");
        return 0; // NEED TO BE IMPLEMENTED
    }
    else {
        int total = 0;
        for (int i = 0; i < as->board_nb_horizontal_cells; i++) {
            for (int j = 0; j < as->board_nb_vertical_cells; j++) {
                if (as->cells[i][j] == as->player_turn) {
                    as->is_there_a_selected_piece = true;
                    as->selected_piece_i = i;
                    as->selected_piece_j = j;
                    total += compute_avaible_moves(appstate);
                    as->is_there_a_selected_piece = false;
                }
            }
        }
        return total;
    }
}

void deepfirstsearch (void **appstate, int** connected_components, int i, int j, int connected_component_index) {
    if (connected_components[i][j] < 0) {
        connected_components[i][j] = connected_component_index;
        if (is_there_a_neighbour_in_this_direction(appstate, i, j, UP)) deepfirstsearch(appstate, connected_components, i, j+1, connected_component_index);
        if (is_there_a_neighbour_in_this_direction(appstate, i, j, RIGHT)) deepfirstsearch(appstate, connected_components, i+1, j, connected_component_index);
        if (is_there_a_neighbour_in_this_direction(appstate, i, j, DOWN)) deepfirstsearch(appstate,connected_components, i, j-1, connected_component_index);
        if (is_there_a_neighbour_in_this_direction(appstate, i, j, LEFT)) deepfirstsearch(appstate, connected_components, i-1, j, connected_component_index);
    }
}

int compute_connected_components (void **appstate, int*** connected_components_ptr, int** connected_component_size_ptr, int*** piece_per_component_per_color_ptr) {
    appstate_t* as = (appstate_t*)appstate;
    *connected_components_ptr = SDL_malloc(sizeof(int*)*as->board_nb_horizontal_cells); 
    for (int i = 0; i < as->board_nb_horizontal_cells; i++) {
        (*connected_components_ptr)[i] = SDL_malloc(sizeof(int)*as->board_nb_vertical_cells);
        for (int j = 0; j < as->board_nb_vertical_cells; j++) {
            (*connected_components_ptr)[i][j] = -1;
        }
    }

    int nb_connected_components = 0;

    for (int i = 0; i < as->board_nb_horizontal_cells; i++) {
        for (int j = 0; j < as->board_nb_vertical_cells; j++) {
            if ((*connected_components_ptr)[i][j] < 0) {
                deepfirstsearch(appstate, *connected_components_ptr, i, j, nb_connected_components);
                nb_connected_components++;
            }
        }
    }

    *connected_component_size_ptr = SDL_malloc(sizeof(int)*nb_connected_components);
    *piece_per_component_per_color_ptr = SDL_malloc(sizeof(int*)*nb_connected_components);
    for (int k = 0; k < nb_connected_components; k++) {
        (*connected_component_size_ptr)[k] = 0;
        (*piece_per_component_per_color_ptr)[k] = SDL_malloc(sizeof(int)*as->nb_players);
        for (int j = 0; j < as->nb_players; j++) {
            (*piece_per_component_per_color_ptr)[k][j] = 0;
        }
    }

    for (int i = 0; i < as->board_nb_horizontal_cells; i++) {
        for (int j = 0; j < as->board_nb_vertical_cells; j++) {
            int connected_component_index = (*connected_components_ptr)[i][j];
            (*connected_component_size_ptr)[connected_component_index]++;
            if (as->cells[i][j] != 0) {
                (*piece_per_component_per_color_ptr)[connected_component_index][as->cells[i][j]-1]++;
            }
        }
    }

    return nb_connected_components;
}

bool is_game_over (void** appstate) {
    // determine if the game is over or not
    // if so, scores array is allocated are fill in a correct way
    // else, scores array remain untouch
    appstate_t* as = (appstate_t*)appstate;
    int** connected_components = NULL;
    int* connected_component_size = NULL;
    int** piece_per_component_per_color = NULL;
    int nb_connected_components = compute_connected_components(appstate, &connected_components, &connected_component_size, &piece_per_component_per_color);
    SDL_Log("There is %d connected components", nb_connected_components);

    int* connected_components_owners = SDL_malloc(sizeof(int) * nb_connected_components);
    for (int k = 0; k < nb_connected_components; k++) connected_components_owners[k] = 0;

    int* piece_per_component = SDL_malloc(sizeof(int)*nb_connected_components);
    int* player_per_component = SDL_malloc(sizeof(int)*nb_connected_components);

    for (int k = 0; k < nb_connected_components; k++) {
        piece_per_component[k] = 0;
        player_per_component[k] = 0;
        for (int j = 0; j < as->nb_players; j++) {
            piece_per_component[k] += piece_per_component_per_color[k][j];
            player_per_component[k] += piece_per_component_per_color[k][j]>0?1:0;
        }
        if (player_per_component[k] > 1 && piece_per_component[k] < connected_component_size[k]) {
            for (int i = 0; i < as->board_nb_horizontal_cells; i++) {
            SDL_free(connected_components[i]);
            }
            SDL_free(connected_components);
            SDL_free(connected_component_size);
            for (int k = 0; k < nb_connected_components; k++) SDL_free(piece_per_component_per_color[k]);
            SDL_free(piece_per_component_per_color);
            SDL_free(piece_per_component);
            SDL_free(player_per_component);
            SDL_free(connected_components_owners);
            return false;
        }
    }

    // The game is over, let count the points
    as->scores = SDL_malloc(sizeof(int)*as->nb_players);
    for (int j = 0; j < as->nb_players; j++) as->scores[j] = 0;
    for (int k = 0; k < nb_connected_components; k++) {
        if (player_per_component[k] > 1) {
            for (int j = 0; j < as->nb_players; j++) {
                as->scores[j] += piece_per_component_per_color[k][j];
            }
        } 
        else if (player_per_component[k] == 1) {
            for (int j = 0; j < as->nb_players; j++) {
                if (piece_per_component_per_color[k][j] >= 1) {
                    as->scores[j] += connected_component_size[k];
                    break;
                }
            }
        }
    }

    for (int i = 0; i < as->board_nb_horizontal_cells; i++) {
        SDL_free(connected_components[i]);
    }
    SDL_free(connected_components);
    SDL_free(connected_component_size);
    for (int k = 0; k < nb_connected_components; k++) SDL_free(piece_per_component_per_color[k]);
    SDL_free(piece_per_component_per_color);
    SDL_free(piece_per_component);
    SDL_free(player_per_component);
    SDL_free(connected_components_owners);

    return true;
}

int increment_loop (int i, int min, int max) {
    return (i == max) ? min : i+1;
}

int decrement_loop (int i, int min, int max) {
    return (i == min) ? max : i-1;
}

int find_next_player_who_is_able_to_play (void **appstate) {
    appstate_t* as = (appstate_t*)appstate;
    int j = increment_loop(as->player_turn, 1, as->nb_players);
    while 

}

void cancel_last_move(void **appstate) {
    appstate_t* as = (appstate_t*)appstate;
    if(as->record == NULL) {
        SDL_Log("No move to cancel");
        return;
    }
    move_t* last_move = as->record->last_move;

    switch (last_move->type) {
        case PIECE_PLACEMENT:
            as->cells[last_move->i][last_move->j] = 0;
            if (!as->is_placement_phase) {
                as->is_placement_phase = true;
                as->is_second_placement_phase = true;
            }
            else if (as->is_second_placement_phase) {
                if (as->player_turn == as->nb_players) {
                    as->is_second_placement_phase = false;
                    as->is_first_placement_phase = true;
                }
                else {
                    increment_turn(appstate);
                }
            }
            else {
                decrement_turn(appstate);
            }
            SDL_Log("Piece placement canceled");
            break;

        case DEPLACEMENT:
            as->cells[last_move->i_destination][last_move->j_destination] = 0;
            as->cells[last_move->i][last_move->j] = as->player_turn;
            as->is_there_a_selected_piece = true;
            as->selected_piece_i = last_move->i;
            as->selected_piece_j = last_move->j;
            as->is_wall_placement_phase = false;
            as->is_there_a_selected_piece = false;
            SDL_Log("Deplacement cancel");
            break;

        case WALL_PLACEMENT:
            if (as->game_over) {
                as->game_over = false;
            }
            else {
                decrement_turn(appstate);
            }
            if (last_move->is_vertical_wall) {
                as->vertical_walls[last_move->i_destination][last_move->j_destination] = 0;
            }
            else {
                as->horizontal_walls[last_move->i_destination][last_move->j_destination] = 0;
            }
            as->is_wall_placement_phase = true;
            as->is_there_a_selected_piece = false;
            as->selected_piece_i = last_move->i;
            as->selected_piece_j = last_move->j;
            
            SDL_Log("Wall placement cancel");
            break;

        case SKIP:
            if (as->player_turn == last_move->i_destination) {
                SDL_Log("Skip cancel");
                as->player_turn = last_move->i;
                as->is_wall_placement_phase = false;
            }
            else {
                SDL_Log("Try to cancel skip %d -> %d, but turn = %d", last_move->i, last_move->i_destination, as->player_turn);
            }
            break;

        default:
            SDL_Log("Unkown move type, failed to cancel this move");
            break;
    }

    removed_last_move(&as->record);
    if (as->record != NULL && as->record->last_move->type == SKIP) {
        cancel_last_move(appstate);
    }
    as->has_to_be_render = true;
}

void place_a_piece (void **appstate, int i, int j) {
    appstate_t* as = (appstate_t*)appstate;
    SDL_Log("Player %d place a piece on %c%d", as->player_turn, 'A'+i, j+1);
    as->cells[i][j] = as->player_turn;
    move_t* piece_placement_move = create_piece_placement_move(i, j);
    record_move(&as->record, piece_placement_move);
}

void select_piece(void **appstate, int i , int j) {
    appstate_t* as = (appstate_t*)appstate;
    SDL_Log("Player %d has selected their piece on %c%d", as->player_turn, 'A'+i, j+1);
    as->selected_piece_i = i;
    as->selected_piece_j = j;
    as->is_there_a_selected_piece = true;
    as->has_to_be_render = true;
    compute_avaible_moves(appstate);
}

void move_selected_piece(void **appstate, int i, int j) {
    appstate_t* as = (appstate_t*)appstate;
    SDL_Log("Player %d moved their piece from %c%d to %c%d", as->player_turn, 'A'+as->selected_piece_i, as->selected_piece_j+1, 'A'+i, j+1);
    as->cells[as->selected_piece_i][as->selected_piece_j] = 0;
    as->cells[i][j] = as->player_turn;
    as->is_there_a_selected_piece = false;
    move_t* deplacement_move = create_deplacement_move(as->selected_piece_i, as->selected_piece_j, i, j);
    record_move(&as->record, deplacement_move);
    as->selected_piece_i = i;
    as->selected_piece_j = j;
    as->is_wall_placement_phase = true;
    as->has_to_be_render = true;
}

void place_a_wall (void **appstate, int** walls_matrix, int i, int j) {
    appstate_t* as = (appstate_t*)appstate;
    SDL_Log("Player %d place a wall", as->player_turn);
    walls_matrix[i][j] = as->player_turn;
    move_t* wall_placement_move = create_wall_placement_move(as->selected_piece_i, as->selected_piece_j, i, j, walls_matrix==as->vertical_walls);
    record_move(&as->record, wall_placement_move);
    as->is_wall_placement_phase = false;
}
