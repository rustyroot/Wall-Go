#include "../include/coordinates.h"

void cells_coords_to_window_coords(void **appstate, int i, int j, float* x, float* y) {
    // return (x, y) is the center of the cell
    appstate_t* as = (appstate_t*)appstate;
    *x = as->horizontal_margin + as->inboard_margin + (i + 0.5)*as->cells_size;
    *y = as->vertical_margin + as->inboard_margin + (as->board_nb_vertical_cells-0.5-j) * as->cells_size;
}

void window_coords_to_cells_coords(void **appstate, float x, float y, int* i, int* j) {
    // (x, y) is one of the points of the cell
    appstate_t* as = (appstate_t*)appstate;
    *i = (x - (as->horizontal_margin + as->inboard_margin))/as->cells_size;
    *j = as->board_nb_vertical_cells - (y - (as->vertical_margin + as->inboard_margin))/as->cells_size;
}

void horizontal_walls_coords_to_window_coords(void **appstate, int i, int j, float* x, float* y) {
    // return (x, y) is the up-left corner of the wall
    appstate_t* as = (appstate_t*)appstate;
    *x = as->horizontal_margin + as->inboard_margin + 0.5 * (as->cells_size - as->wall_length) + i * as->cells_size; 
    *y = as->vertical_margin + as->inboard_margin + (as->board_nb_vertical_cells-j-1)*as->cells_size - 0.5 * as->wall_width;
}

void vertical_walls_coords_to_window_coords(void **appstate, int i, int j, float* x, float* y) {
    // return (x, y) is the up-left corner of the wall
    appstate_t* as = (appstate_t*)appstate;
    *x = as->horizontal_margin + as->inboard_margin - 0.5 * as->wall_width + (i+1) * as->cells_size; 
    *y = as->vertical_margin + as->inboard_margin + (as->board_nb_vertical_cells-j-1)*as->cells_size + 0.5 * (as->cells_size - as->wall_length);
}

void window_coords_to_walls_coords (void **appstate, float x, float y, int*** walls_matrix_ptr, int* i, int* j) {
    appstate_t* as = (appstate_t*)appstate;
    // if (x, y) is a point of a wall, this function set wall_matrix to as->horizontal_walls or as->vertical_walls
    // and (i, j) to the according wall index
    // if (x, y) is not a point of a wall, walls_matrix is set to NULL and (i,j) is not touch.

    if (is_on_board(appstate, x, y)) {
        int cell_i, cell_j;
        window_coords_to_cells_coords(appstate, x, y, &cell_i, &cell_j);
        float top_left_corner_x, top_left_corner_y;
        cells_coords_to_window_coords(appstate, cell_i, cell_j, &top_left_corner_x, &top_left_corner_y);
        top_left_corner_x -= 0.5 * as->cells_size;
        top_left_corner_y -= 0.5 * as->cells_size;
        x -= top_left_corner_x;
        y -= top_left_corner_y;

        // top
        if (y <= 0.5 * as->wall_width && x >= 0.5*(as->cells_size-as->wall_length) && x <= 0.5*(as->cells_size+as->wall_length)) {
            if (cell_j >= as->board_nb_vertical_cells-1) {
                *walls_matrix_ptr = NULL;
                return;
            }
            *walls_matrix_ptr = as->horizontal_walls;
            *i = cell_i;
            *j = cell_j;
        }
        // right
        else if (x >= as->cells_size-0.5*as->wall_width && y >= 0.5*(as->cells_size-as->wall_length) && y <= 0.5*(as->cells_size+as->wall_length)) {
            if (cell_i >= as->board_nb_horizontal_cells-1) {
                *walls_matrix_ptr = NULL;
                return;
            }
            *walls_matrix_ptr = as->vertical_walls;
            *i = cell_i;
            *j = cell_j;
        }
        // down
        else if (y >= as->cells_size-0.5*as->wall_width && x >= 0.5*(as->cells_size-as->wall_length) && x <= 0.5*(as->cells_size+as->wall_length)) {
            if (cell_j == 0) {
                *walls_matrix_ptr = NULL;
                return;
            }
            *walls_matrix_ptr = as->horizontal_walls;
            *i = cell_i;
            *j = cell_j-1;
        }
        // left 
        else if (x <= 0.5 * as->wall_width && y >= 0.5*(as->cells_size-as->wall_length) && y <= 0.5*(as->cells_size+as->wall_length)) {
            if (cell_i == 0) {
                *walls_matrix_ptr = NULL;
                return;
            }
            *walls_matrix_ptr = as->vertical_walls;
            *i = cell_i-1;
            *j = cell_j;
        }
        // None of the above
        else {
            *walls_matrix_ptr = NULL;
        }
    }
    else {
        *walls_matrix_ptr = NULL;
    }
}